// SPEC §8 fuzz: random parameter automation at audio rate through the whole
// engine, asserting no NaN, no inf, no runaway and no audio-thread allocation.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "Engine.h"
#include "core/RtCheck.h"
#include "test/TestFramework.h"

using namespace nekta;

namespace {

constexpr double kSr = 48000.0;

struct Rng {
  std::uint32_t s;
  explicit Rng(std::uint32_t seed) : s(seed ? seed : 1u) {}
  std::uint32_t next() {
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return s;
  }
  float unit() { return static_cast<float>(next() & 0xffffffu) / static_cast<float>(0xffffff); }
  int range(int lo, int hi) { return lo + static_cast<int>(next() % static_cast<std::uint32_t>(hi - lo + 1)); }
  bool chance(float p) { return unit() < p; }
};

SampleBuffer makeNoise(int frames, Rng& rng) {
  SampleBuffer b;
  b.setSize(1, frames);
  b.setSampleRate(kSr);
  for (int i = 0; i < frames; ++i) b.writePtr(0)[i] = rng.unit() * 2.0f - 1.0f;
  return b;
}

}  // namespace

NEKTA_TEST(engineSurvivesRandomAutomation) {
  for (std::uint32_t seed = 1; seed <= 8; ++seed) {
    Rng rng(seed * 2654435761u);
    Engine engine;
    engine.prepare(kSr, 128);
    engine.transport().setBpm(60.0 + rng.unit() * 140.0);
    engine.transport().setPlaying(true);
    engine.setSample(0, makeNoise(20000, rng));
    engine.setSample(1, makeNoise(3000, rng));

    for (int pad = 0; pad < 4; ++pad) {
      PadConfig cfg;
      cfg.sampleSlot = rng.range(0, 1);
      cfg.pitchSemitones = rng.unit() * 48.0 - 24.0;
      cfg.gain = rng.unit();
      cfg.pan = rng.unit() * 2.0f - 1.0f;
      cfg.reverse = rng.chance(0.3f);
      cfg.loop = rng.chance(0.2f);
      cfg.chokeGroup = rng.range(0, 3);
      cfg.env.attackMs = rng.unit() * 20.0f;
      cfg.env.decayMs = rng.unit() * 200.0f;
      cfg.env.sustain = rng.unit();
      cfg.env.releaseMs = rng.unit() * 100.0f;
      engine.setPadConfig(pad, cfg);
    }

    RollConfig roll;
    roll.rate = Rate(1 << rng.range(2, 6));
    roll.ramp.enabled = true;
    roll.ramp.endRate = Rate(1 << rng.range(2, 6));
    roll.ramp.lengthBeats = 0.25 + rng.unit() * 8.0;
    roll.ramp.curve = rng.chance(0.5f) ? RampCurve::Exponential : RampCurve::Linear;
    roll.mods.ampDecay = 0.5f + rng.unit();
    roll.mods.ampCurve = 0.5f + rng.unit();
    roll.mods.pitchDeltaSemis = rng.unit() * 6.0 - 3.0;
    roll.mods.pitchWrapRepeats = rng.range(0, 8);
    roll.mods.reverseAlternate = rng.chance(0.5f);
    roll.mods.panAlternate = rng.chance(0.5f);
    roll.mods.panAmount = rng.unit();
    roll.mods.gate = rng.unit();
    roll.mods.startOffsetWalkMs = rng.unit() * 10.0;
    engine.setRollConfig(roll);

    const ParamId params[] = {engine.paramCutoff(), engine.paramResonance(),
                              engine.paramDrive(), engine.paramMorph(),
                              engine.paramMasterGain()};

    std::vector<float> left(128, 0.0f), right(128, 0.0f);
    rt::resetViolations();
    double peak = 0.0;
    for (int block = 0; block < 3000; ++block) {
      // Host side: hammer the queue with events at random sample offsets.
      const SampleIndex blockStart = engine.transport().samplePosition();
      for (int e = 0; e < 4; ++e) {
        EngineEvent ev;
        ev.time = blockStart + rng.range(0, 127);
        const int which = rng.range(0, 5);
        if (which == 0) {
          ev.type = EventType::PadTrigger;
          ev.index = rng.range(0, 3);
          ev.value = rng.unit();
        } else if (which == 1) {
          ev.type = EventType::PadRelease;
          ev.index = rng.range(0, 3);
        } else if (which == 2) {
          ev.type = EventType::RollStart;
        } else if (which == 3) {
          ev.type = EventType::RollStop;
        } else {
          ev.type = EventType::ParamSet;
          const ParamId id = params[rng.range(0, 4)];
          ev.index = id;
          const ParamDesc& desc = engine.params().desc(id);
          ev.value = desc.minValue + rng.unit() * (desc.maxValue - desc.minValue);
        }
        engine.pushEvent(ev);
      }
      {
        rt::ScopedAudioThread audio;
        engine.process(left.data(), right.data(), 128);
      }
      for (int i = 0; i < 128; ++i) {
        REQUIRE(std::isfinite(left[static_cast<std::size_t>(i)]));
        REQUIRE(std::isfinite(right[static_cast<std::size_t>(i)]));
        peak = std::max(peak, static_cast<double>(std::fabs(left[static_cast<std::size_t>(i)])));
      }
    }
    CHECK_EQ_INT(rt::allocationViolations(), 0);
    CHECK(peak < 32.0);  // resonant filter + drive can exceed 1, but not run away
  }
}

NEKTA_TEST(outOfRangeInputIsRejectedNotCrashed) {
  Engine engine;
  engine.prepare(kSr, 128);
  engine.transport().setPlaying(true);
  Rng rng(12345);
  engine.setSample(0, makeNoise(1000, rng));
  PadConfig cfg;
  cfg.sampleSlot = 0;
  engine.setPadConfig(0, cfg);

  // Pad indices, param ids and slots outside every valid range.
  const int bad[] = {-1, -1000, kNumPads, kNumPads + 50, 1 << 20};
  for (int index : bad) {
    EngineEvent ev;
    ev.type = EventType::PadTrigger;
    ev.index = index;
    engine.pushEvent(ev);
    ev.type = EventType::PadRelease;
    engine.pushEvent(ev);
    ev.type = EventType::ParamSet;
    ev.value = 1.0f;
    engine.pushEvent(ev);
  }
  CHECK(engine.sample(-1) == nullptr);
  CHECK(engine.sample(kMaxSamples) == nullptr);
  CHECK(!engine.setSample(-1, SampleBuffer{}));

  std::vector<float> left(128, 0.0f), right(128, 0.0f);
  rt::resetViolations();
  {
    rt::ScopedAudioThread audio;
    engine.process(left.data(), right.data(), 128);
  }
  CHECK_EQ_INT(rt::allocationViolations(), 0);
  for (int i = 0; i < 128; ++i) {
    CHECK(std::isfinite(left[static_cast<std::size_t>(i)]));
  }
}

NEKTA_TEST(fullQueueDropsEventsWithoutCorruption) {
  Engine engine;
  engine.prepare(kSr, 128);
  engine.transport().setPlaying(true);
  Rng rng(99);
  engine.setSample(0, makeNoise(1000, rng));
  PadConfig cfg;
  cfg.sampleSlot = 0;
  engine.setPadConfig(0, cfg);

  int accepted = 0;
  for (int i = 0; i < 5000; ++i) {
    EngineEvent ev;
    ev.type = EventType::PadTrigger;
    ev.time = i;
    if (engine.pushEvent(ev)) ++accepted;
  }
  CHECK(accepted > 0);
  CHECK(accepted < 5000);  // bounded queue, and push() reports the refusal

  std::vector<float> left(128, 0.0f), right(128, 0.0f);
  {
    rt::ScopedAudioThread audio;
    engine.process(left.data(), right.data(), 128);
  }
  for (int i = 0; i < 128; ++i) CHECK(std::isfinite(left[static_cast<std::size_t>(i)]));
}

NEKTA_TEST_MAIN("fuzz")
