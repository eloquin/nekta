// Offline render harness (SPEC §10.6): load a WAV, run a scripted performance,
// render to WAV. Everything the audio thread will do at runtime happens here
// under the same real-time guard, so the whole engine is testable from the
// command line before any iOS code exists.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Engine.h"
#include "core/RtCheck.h"
#include "harness/Script.h"
#include "io/WavFile.h"

namespace {

constexpr nekta::FrameCount kBlockFrames = 128;

void usage() {
  std::printf(
      "usage: nekta_render <script.nks> [-o out.wav] [--block N]\n"
      "\n"
      "  Renders a scripted performance offline and writes a 48 kHz stereo\n"
      "  32-bit float WAV. See engine/README.md for the script format.\n");
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    usage();
    return 2;
  }

  std::string scriptPath;
  std::string outputPath;
  nekta::FrameCount block = kBlockFrames;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      usage();
      return 0;
    }
    if (arg == "-o" && i + 1 < argc) {
      outputPath = argv[++i];
    } else if (arg == "--block" && i + 1 < argc) {
      block = static_cast<nekta::FrameCount>(std::atoi(argv[++i]));
    } else if (scriptPath.empty()) {
      scriptPath = arg;
    } else {
      std::fprintf(stderr, "unexpected argument: %s\n", arg.c_str());
      return 2;
    }
  }
  if (scriptPath.empty() || block <= 0 || block > nekta::kMaxBlockFrames) {
    usage();
    return 2;
  }

  nekta::harness::Script script;
  std::string error;
  if (!nekta::harness::parseScript(scriptPath, script, error)) {
    std::fprintf(stderr, "%s\n", error.c_str());
    return 1;
  }
  if (outputPath.empty()) outputPath = script.output.empty() ? "out.wav" : script.output;

  nekta::Engine engine;
  engine.prepare(script.sampleRate, block);
  engine.transport().setBpm(script.bpm);
  engine.transport().setBeatsPerBar(script.beatsPerBar);
  engine.transport().setPlaying(true);
  engine.setLaunchQuantise(script.launchGrid, script.quantiseLaunch);
  engine.setRollConfig(script.roll);

  for (std::size_t slot = 0; slot < script.samples.size(); ++slot) {
    if (script.samples[slot].empty()) continue;
    nekta::SampleBuffer buffer;
    if (!nekta::wav::read(script.samples[slot].c_str(), buffer, error)) {
      std::fprintf(stderr, "%s\n", error.c_str());
      return 1;
    }
    std::printf("sample %zu: %s (%d ch, %d frames, %.0f Hz, peak %.3f)\n", slot,
                script.samples[slot].c_str(), buffer.channels(), buffer.frames(),
                buffer.sampleRate(), static_cast<double>(buffer.peak()));
    engine.setSample(static_cast<int>(slot), std::move(buffer));
  }

  for (const std::pair<int, nekta::PadConfig>& pad : script.pads) {
    engine.setPadConfig(pad.first, pad.second);
  }
  for (const std::pair<std::string, float>& p : script.initialParams) {
    const nekta::ParamId id = engine.params().find(p.first.c_str());
    if (id == nekta::kInvalidParam) {
      std::fprintf(stderr, "unknown parameter: %s\n", p.first.c_str());
      return 1;
    }
    engine.params().setTarget(id, p.second);
  }
  engine.params().snapAll();

  // Resolve every scripted command to a POD engine event with a sample time.
  const double samplesPerBeat = engine.transport().samplesPerBeat();
  std::vector<nekta::EngineEvent> events;
  events.reserve(script.commands.size());
  for (const nekta::harness::ScriptCommand& cmd : script.commands) {
    nekta::EngineEvent e;
    e.time = static_cast<nekta::SampleIndex>(std::llround(cmd.beats * samplesPerBeat));
    e.index = cmd.index;
    e.value = cmd.value;
    if (cmd.kind == "trigger") {
      e.type = nekta::EventType::PadTrigger;
    } else if (cmd.kind == "release") {
      e.type = nekta::EventType::PadRelease;
    } else if (cmd.kind == "rollstart") {
      e.type = nekta::EventType::RollStart;
    } else if (cmd.kind == "rollstop") {
      e.type = nekta::EventType::RollStop;
    } else if (cmd.kind == "param") {
      e.type = nekta::EventType::ParamSet;
      e.index = engine.params().find(cmd.path.c_str());
      if (e.index == nekta::kInvalidParam) {
        std::fprintf(stderr, "unknown parameter: %s\n", cmd.path.c_str());
        return 1;
      }
    }
    events.push_back(e);
  }

  const int totalFrames =
      static_cast<int>(std::llround(script.lengthBeats * samplesPerBeat));
  std::vector<float> left(static_cast<std::size_t>(totalFrames), 0.0f);
  std::vector<float> right(static_cast<std::size_t>(totalFrames), 0.0f);

  nekta::rt::resetViolations();
  std::size_t nextEvent = 0;
  const auto started = std::chrono::steady_clock::now();
  for (int position = 0; position < totalFrames; position += block) {
    const nekta::FrameCount frames =
        static_cast<nekta::FrameCount>(std::min<int>(block, totalFrames - position));
    // Host side: feed the lock-free queue just ahead of the audio thread.
    while (nextEvent < events.size() && events[nextEvent].time < position + frames) {
      if (!engine.pushEvent(events[nextEvent])) break;
      ++nextEvent;
    }
    {
      nekta::rt::ScopedAudioThread audio;
      engine.process(left.data() + position, right.data() + position, frames);
    }
  }
  const auto finished = std::chrono::steady_clock::now();

  double peak = 0.0;
  double sumSquares = 0.0;
  for (int i = 0; i < totalFrames; ++i) {
    peak = std::max(peak, static_cast<double>(std::fabs(left[static_cast<std::size_t>(i)])));
    peak = std::max(peak, static_cast<double>(std::fabs(right[static_cast<std::size_t>(i)])));
    sumSquares += static_cast<double>(left[static_cast<std::size_t>(i)]) *
                  left[static_cast<std::size_t>(i)];
  }
  const double rms = totalFrames > 0 ? std::sqrt(sumSquares / totalFrames) : 0.0;

  if (!nekta::wav::writeFloat32(outputPath.c_str(), left.data(), right.data(), totalFrames,
                                script.sampleRate, error)) {
    std::fprintf(stderr, "%s\n", error.c_str());
    return 1;
  }

  const double renderSeconds =
      std::chrono::duration<double>(finished - started).count();
  const double audioSeconds = static_cast<double>(totalFrames) / script.sampleRate;
  std::printf(
      "rendered %s\n"
      "  %.2f s of audio (%d frames) at %.0f Hz, %.1f BPM, block %d\n"
      "  peak %.3f (%.1f dBFS), rms %.4f\n"
      "  %zu events, %llu dropped\n"
      "  render took %.3f s (%.0fx realtime)\n"
      "  audio-thread allocations: %llu%s\n",
      outputPath.c_str(), audioSeconds, totalFrames, script.sampleRate, script.bpm, block,
      peak, 20.0 * std::log10(peak > 0.0 ? peak : 1e-9), rms, events.size(),
      static_cast<unsigned long long>(engine.droppedEvents()), renderSeconds,
      renderSeconds > 0.0 ? audioSeconds / renderSeconds : 0.0,
      static_cast<unsigned long long>(nekta::rt::allocationViolations()),
      nekta::rt::checksEnabled() ? "" : " (guard not compiled in)");

  return nekta::rt::allocationViolations() == 0 ? 0 : 1;
}
