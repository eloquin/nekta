#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "Engine.h"
#include "core/RtCheck.h"
#include "harness/Script.h"
#include "io/WavFile.h"
#include "test/TestFramework.h"

using namespace nekta;

namespace {

constexpr double kSr = 48000.0;
constexpr double kBpm = 174.0;

// A short burst: long enough to detect, short enough that repeats at 1/32 do
// not overlap.
SampleBuffer makeBurst(int frames = 100) {
  SampleBuffer b;
  b.setSize(1, frames);
  b.setSampleRate(kSr);
  for (int i = 0; i < frames; ++i) b.writePtr(0)[i] = 1.0f;
  return b;
}

PadConfig burstPad() {
  PadConfig pad;
  pad.sampleSlot = 0;
  pad.pan = -1.0f;  // hard left: the right channel stays clean for comparisons
  pad.env.attackMs = 0.0f;
  pad.env.decayMs = 0.0f;
  pad.env.sustain = 1.0f;
  pad.env.releaseMs = 0.5f;
  pad.chokeGroup = 1;
  return pad;
}

void configureBypassed(Engine& engine, int burstFrames = 100) {
  engine.prepare(kSr, 128);
  engine.transport().setBpm(kBpm);
  engine.transport().setPlaying(true);
  engine.setSample(0, makeBurst(burstFrames));
  engine.setPadConfig(0, burstPad());
  engine.params().setTarget(engine.paramFilterBypass(), 1.0f);
  engine.params().setTarget(engine.paramMasterGain(), 1.0f);
  engine.params().snapAll();
}

struct Rendered {
  std::vector<float> left, right;
};

Rendered render(Engine& engine, const std::vector<EngineEvent>& events, int totalFrames,
                FrameCount block) {
  Rendered out;
  out.left.assign(static_cast<std::size_t>(totalFrames), 0.0f);
  out.right.assign(static_cast<std::size_t>(totalFrames), 0.0f);
  std::size_t next = 0;
  for (int position = 0; position < totalFrames; position += block) {
    const FrameCount frames =
        static_cast<FrameCount>(std::min<int>(block, totalFrames - position));
    while (next < events.size() && events[next].time < position + frames) {
      if (!engine.pushEvent(events[next])) break;
      ++next;
    }
    rt::ScopedAudioThread audio;
    engine.process(out.left.data() + position, out.right.data() + position, frames);
  }
  return out;
}

// Rising edges: where the signal crosses up through `on` having been below `off`.
std::vector<int> findOnsets(const std::vector<float>& x, float on = 0.005f,
                            float off = 0.001f) {
  std::vector<int> onsets;
  bool armed = true;
  for (std::size_t i = 0; i < x.size(); ++i) {
    const float a = std::fabs(x[i]);
    if (armed && a > on) {
      onsets.push_back(static_cast<int>(i));
      armed = false;
    } else if (!armed && a < off) {
      armed = true;
    }
  }
  return onsets;
}

EngineEvent makeEvent(EventType type, SampleIndex time, int index = 0, float value = 1.0f) {
  EngineEvent e;
  e.type = type;
  e.time = time;
  e.index = index;
  e.value = value;
  return e;
}

std::string scratchPath(const char* name) {
  return std::string("nekta_test_") + name;
}

}  // namespace

NEKTA_TEST(wavRoundTrip) {
  const int frames = 5000;
  std::vector<float> l(static_cast<std::size_t>(frames)), r(static_cast<std::size_t>(frames));
  for (int i = 0; i < frames; ++i) {
    l[static_cast<std::size_t>(i)] = std::sin(static_cast<float>(i) * 0.01f);
    r[static_cast<std::size_t>(i)] = -std::sin(static_cast<float>(i) * 0.02f);
  }
  const std::string path = scratchPath("roundtrip.wav");
  std::string error;
  REQUIRE(wav::writeFloat32(path.c_str(), l.data(), r.data(), frames, kSr, error));

  SampleBuffer read;
  REQUIRE(wav::read(path.c_str(), read, error));
  CHECK_EQ_INT(read.channels(), 2);
  CHECK_EQ_INT(read.frames(), frames);
  CHECK_NEAR(read.sampleRate(), kSr, 1e-9);
  for (int i = 0; i < frames; ++i) {
    CHECK(read.data(0)[i] == l[static_cast<std::size_t>(i)]);
    CHECK(read.data(1)[i] == r[static_cast<std::size_t>(i)]);
  }
  std::remove(path.c_str());
}

NEKTA_TEST(silentWithNoEvents) {
  Engine engine;
  configureBypassed(engine);
  const Rendered out = render(engine, {}, 24000, 128);
  for (std::size_t i = 0; i < out.left.size(); ++i) {
    CHECK(out.left[i] == 0.0f);
    CHECK(out.right[i] == 0.0f);
  }
}

// SPEC §8 null test, at engine level: with the insert bypassed and unity gain,
// the rendered pad is the sample itself (times the anti-click fade).
NEKTA_TEST(bypassedChainReproducesTheSample) {
  Engine engine;
  configureBypassed(engine, 4000);
  const std::vector<EngineEvent> events{makeEvent(EventType::PadTrigger, 0)};
  const Rendered out = render(engine, events, 4800, 128);
  const int fade = static_cast<int>(kAntiClickMs * 0.001f * kSr);
  const SampleBuffer* sample = engine.sample(0);
  REQUIRE(sample != nullptr);
  // Between the fades, output == input exactly. The end fade begins `fade`
  // frames before the last sample index, hence the extra frame of margin.
  for (int i = fade; i < sample->frames() - fade - 2; ++i) {
    CHECK(out.left[static_cast<std::size_t>(i)] == sample->data(0)[i]);
  }
  // Hard left: nothing leaks to the right channel.
  for (std::size_t i = 0; i < out.right.size(); ++i) CHECK(out.right[i] == 0.0f);
}

NEKTA_TEST(rendersAreDeterministic) {
  std::vector<EngineEvent> events;
  events.push_back(makeEvent(EventType::PadTrigger, 0));
  events.push_back(makeEvent(EventType::RollStart, 4800));
  events.push_back(makeEvent(EventType::ParamSet, 12000, 1, 800.0f));
  events.push_back(makeEvent(EventType::RollStop, 48000));

  Rendered first, second;
  for (int pass = 0; pass < 2; ++pass) {
    Engine engine;
    engine.prepare(kSr, 128);
    engine.transport().setBpm(kBpm);
    engine.transport().setPlaying(true);
    engine.setSample(0, makeBurst());
    engine.setPadConfig(0, burstPad());
    RollConfig roll;
    roll.rate = Rate(16);
    roll.ramp.enabled = true;
    roll.ramp.endRate = Rate(32);
    roll.ramp.lengthBeats = 4.0;
    roll.ramp.curve = RampCurve::Exponential;
    roll.mods.ampDecay = 0.95f;
    engine.setRollConfig(roll);
    std::vector<EngineEvent> local = events;
    local[2].index = engine.paramCutoff();
    Rendered& out = (pass == 0) ? first : second;
    out = render(engine, local, 96000, 128);
  }
  for (std::size_t i = 0; i < first.left.size(); ++i) {
    CHECK(first.left[i] == second.left[i]);
    CHECK(first.right[i] == second.right[i]);
  }
}

// Sample-accurate event boundaries (SPEC §1.2): a trigger at an arbitrary
// sample offset sounds at exactly that sample, whatever the block size.
NEKTA_TEST(eventsLandOnTheExactSampleRegardlessOfBlockSize) {
  const SampleIndex triggerAt = 12345;
  const FrameCount blocks[] = {64, 128, 100, 256};
  for (FrameCount block : blocks) {
    Engine engine;
    configureBypassed(engine);
    const std::vector<EngineEvent> events{makeEvent(EventType::PadTrigger, triggerAt)};
    const Rendered out = render(engine, events, 24000, block);
    int firstNonZero = -1;
    for (std::size_t i = 0; i < out.left.size(); ++i) {
      if (out.left[i] != 0.0f) {
        firstNonZero = static_cast<int>(i);
        break;
      }
    }
    CHECK_EQ_INT(firstNonZero, triggerAt);
  }
}

NEKTA_TEST(renderIsIdenticalAcrossBlockSizes) {
  // With the insert bypassed and no parameter moves, block size must not change
  // a single sample - proof that scheduling, not buffering, decides timing.
  std::vector<EngineEvent> events;
  events.push_back(makeEvent(EventType::PadTrigger, 1000));
  events.push_back(makeEvent(EventType::RollStart, 5000));
  events.push_back(makeEvent(EventType::RollStop, 60000));

  Rendered reference;
  const FrameCount blocks[] = {128, 64, 100, 256, 32};
  for (std::size_t b = 0; b < sizeof(blocks) / sizeof(blocks[0]); ++b) {
    Engine engine;
    configureBypassed(engine);
    RollConfig roll;
    roll.rate = Rate(32);
    engine.setRollConfig(roll);
    const Rendered out = render(engine, events, 96000, blocks[b]);
    if (b == 0) {
      reference = out;
      continue;
    }
    int mismatches = 0;
    for (std::size_t i = 0; i < reference.left.size(); ++i) {
      if (reference.left[i] != out.left[i]) ++mismatches;
    }
    CHECK_EQ_INT(mismatches, 0);
  }
}

// The P0 exit criterion: rolls are tight at 1/32 (SPEC §7).
NEKTA_TEST(rollOnsetsInTheRenderSitOnTheGrid) {
  const Rate rates[] = {Rate(8), Rate(16), Rate(32), Rate(16, RateMod::Triplet)};
  for (const Rate& rate : rates) {
    Engine engine;
    configureBypassed(engine);
    RollConfig roll;
    roll.rate = rate;
    engine.setRollConfig(roll);
    const std::vector<EngineEvent> events{makeEvent(EventType::RollStart, 0)};
    const Rendered out = render(engine, events, 96000, 128);

    const std::vector<int> onsets = findOnsets(out.left);
    REQUIRE(onsets.size() > 4);
    const double spacing = rate.beats() * engine.transport().samplesPerBeat();
    const int firstOffset = onsets[0];
    for (std::size_t i = 0; i < onsets.size(); ++i) {
      const double expected = static_cast<double>(i) * spacing + firstOffset;
      CHECK_NEAR(static_cast<double>(onsets[i]), expected, 1.0);
    }
  }
}

NEKTA_TEST(rampedRollAcceleratesInTheRender) {
  Engine engine;
  configureBypassed(engine);
  RollConfig roll;
  roll.rate = Rate(8);
  roll.ramp.enabled = true;
  roll.ramp.endRate = Rate(32);
  roll.ramp.lengthBeats = 4.0;
  roll.ramp.curve = RampCurve::Exponential;
  engine.setRollConfig(roll);
  const std::vector<EngineEvent> events{makeEvent(EventType::RollStart, 0)};
  const Rendered out = render(engine, events, static_cast<int>(kSr * 3), 128);

  const std::vector<int> onsets = findOnsets(out.left);
  REQUIRE(onsets.size() > 10);
  // Intervals shrink monotonically, then settle at the end rate.
  int previous = onsets[1] - onsets[0];
  for (std::size_t i = 2; i < onsets.size(); ++i) {
    const int interval = onsets[i] - onsets[i - 1];
    CHECK(interval <= previous + 1);
    previous = interval;
  }
  const double endSpacing = Rate(32).beats() * engine.transport().samplesPerBeat();
  const int last = onsets.back() - onsets[onsets.size() - 2];
  CHECK_NEAR(static_cast<double>(last), endSpacing, 2.0);
}

NEKTA_TEST(fullRenderAllocatesNothingAndDropsNothing) {
  Engine engine;
  engine.prepare(kSr, 128);
  engine.transport().setBpm(kBpm);
  engine.transport().setPlaying(true);
  engine.setSample(0, makeBurst());
  engine.setPadConfig(0, burstPad());
  RollConfig roll;
  roll.rate = Rate(16);
  roll.ramp.enabled = true;
  roll.ramp.endRate = Rate(64);
  roll.ramp.lengthBeats = 2.0;
  roll.ramp.curve = RampCurve::Exponential;
  roll.mods.ampDecay = 0.9f;
  roll.mods.pitchDeltaSemis = 1.0;
  roll.mods.pitchWrapRepeats = 8;
  roll.mods.reverseAlternate = true;
  roll.mods.panAlternate = true;
  roll.mods.panAmount = 0.5f;
  roll.mods.gate = 0.6f;
  engine.setRollConfig(roll);

  std::vector<EngineEvent> events;
  events.push_back(makeEvent(EventType::RollStart, 0));
  for (int i = 1; i < 40; ++i) {
    events.push_back(makeEvent(EventType::ParamSet, i * 2000, engine.paramCutoff(),
                               300.0f + static_cast<float>(i) * 300.0f));
  }
  events.push_back(makeEvent(EventType::RollStop, 200000));

  rt::resetViolations();
  const Rendered out = render(engine, events, 240000, 128);
  CHECK_EQ_INT(rt::allocationViolations(), 0);
  CHECK_EQ_INT(engine.droppedEvents(), 0);
  double peak = 0.0;
  for (float v : out.left) {
    CHECK(std::isfinite(v));
    peak = std::max(peak, static_cast<double>(std::fabs(v)));
  }
  CHECK(peak > 0.1);
  CHECK(peak < 4.0);
}

NEKTA_TEST(scriptParsesRates) {
  Rate r;
  CHECK(harness::parseRate("1/16", r));
  CHECK(r == Rate(16, RateMod::Straight));
  CHECK(harness::parseRate("1/32t", r));
  CHECK(r == Rate(32, RateMod::Triplet));
  CHECK(harness::parseRate("1/8d", r));
  CHECK(r == Rate(8, RateMod::Dotted));
  CHECK(harness::parseRate("64", r));
  CHECK(r == Rate(64, RateMod::Straight));
  CHECK(!harness::parseRate("1/0", r));
  CHECK(!harness::parseRate("", r));
}

NEKTA_TEST(scriptRoundTripDrivesTheEngine) {
  // Write a sample and a script, parse it, and render it the way the CLI does.
  const std::string wavPath = scratchPath("break.wav");
  const std::string scriptPath = scratchPath("perf.nks");
  {
    std::vector<float> l(2400, 0.0f), r(2400, 0.0f);
    for (int i = 0; i < 400; ++i) {
      l[static_cast<std::size_t>(i)] = r[static_cast<std::size_t>(i)] = 0.8f;
    }
    std::string error;
    REQUIRE(wav::writeFloat32(wavPath.c_str(), l.data(), r.data(), 2400, kSr, error));
  }
  {
    std::ofstream out(scriptPath);
    REQUIRE(out.good());
    out << "# generated by test\n"
        << "samplerate 48000\n"
        << "bpm 174\n"
        << "length 8\n"
        << "sample 0 " << wavPath << "\n"
        << "pad 0 sample 0 choke 1 release 2 gain 0.9\n"
        << "roll pad 0 rate 1/16 to 1/32 over 4 curve exp gate 0.7 decay 0.9\n"
        << "param fx.1.cutoff 6000\n"
        << "@0 trigger 0 1.0\n"
        << "@1 rollstart\n"
        << "@1.5 param fx.1.cutoff 1200\n"
        << "@5 rollstop\n";
  }

  harness::Script script;
  std::string error;
  REQUIRE(harness::parseScript(scriptPath, script, error));
  CHECK_NEAR(script.bpm, 174.0, 1e-9);
  CHECK_NEAR(script.lengthBeats, 8.0, 1e-9);
  CHECK_EQ_INT(script.samples.size(), 1);
  CHECK_EQ_INT(script.pads.size(), 1);
  CHECK_EQ_INT(script.commands.size(), 4);
  CHECK(script.roll.ramp.enabled);
  CHECK(script.roll.ramp.curve == RampCurve::Exponential);

  Engine engine;
  engine.prepare(script.sampleRate, 128);
  engine.transport().setBpm(script.bpm);
  engine.transport().setPlaying(true);
  engine.setRollConfig(script.roll);
  SampleBuffer buffer;
  REQUIRE(wav::read(script.samples[0].c_str(), buffer, error));
  engine.setSample(0, std::move(buffer));
  for (const std::pair<int, PadConfig>& pad : script.pads) {
    engine.setPadConfig(pad.first, pad.second);
  }
  for (const std::pair<std::string, float>& p : script.initialParams) {
    const ParamId id = engine.params().find(p.first.c_str());
    CHECK(id != kInvalidParam);
    engine.params().setTarget(id, p.second);
  }
  engine.params().snapAll();

  const double spb = engine.transport().samplesPerBeat();
  std::vector<EngineEvent> events;
  for (const harness::ScriptCommand& cmd : script.commands) {
    EngineEvent e;
    e.time = static_cast<SampleIndex>(std::llround(cmd.beats * spb));
    e.index = cmd.index;
    e.value = cmd.value;
    if (cmd.kind == "trigger") {
      e.type = EventType::PadTrigger;
    } else if (cmd.kind == "rollstart") {
      e.type = EventType::RollStart;
    } else if (cmd.kind == "rollstop") {
      e.type = EventType::RollStop;
    } else if (cmd.kind == "param") {
      e.type = EventType::ParamSet;
      e.index = engine.params().find(cmd.path.c_str());
      CHECK(e.index != kInvalidParam);
    }
    events.push_back(e);
  }

  const int totalFrames = static_cast<int>(std::llround(script.lengthBeats * spb));
  rt::resetViolations();
  const Rendered out = render(engine, events, totalFrames, 128);
  CHECK_EQ_INT(rt::allocationViolations(), 0);
  double peak = 0.0;
  for (float v : out.left) peak = std::max(peak, static_cast<double>(std::fabs(v)));
  CHECK(peak > 0.05);

  std::remove(wavPath.c_str());
  std::remove(scriptPath.c_str());
}

NEKTA_TEST(unknownDirectivesAreReportedWithLineNumbers) {
  const std::string scriptPath = scratchPath("bad.nks");
  {
    std::ofstream out(scriptPath);
    out << "bpm 174\n"
        << "wobble 3\n";
  }
  harness::Script script;
  std::string error;
  CHECK(!harness::parseScript(scriptPath, script, error));
  CHECK(error.find(":2:") != std::string::npos);
  std::remove(scriptPath.c_str());
}

NEKTA_TEST_MAIN("render")
