#include <cmath>
#include <vector>

#include "clock/Transport.h"
#include "core/RtCheck.h"
#include "roll/RollEngine.h"
#include "test/TestFramework.h"

using namespace nekta;

namespace {

constexpr double kSr = 48000.0;

Transport makeTransport(double bpm) {
  Transport t;
  t.prepare(kSr);
  t.setBpm(bpm);
  t.setPlaying(true);
  return t;
}

// Independent oracle for the rate ramp: numerically integrates the rate curve
// (repeats per beat) and records where the accumulated repeat count crosses each
// integer. Deliberately shares no code with RollEngine's closed-form inversion.
std::vector<double> integrateOnsets(const RollConfig& cfg, int count) {
  const double r0 = cfg.rate.repeatsPerBeat();
  const double r1 = cfg.ramp.endRate.repeatsPerBeat();
  const double len = cfg.ramp.lengthBeats;
  auto rateAt = [&](double b) {
    if (!cfg.ramp.enabled || len <= 0.0) return r0;
    const double t = b >= len ? 1.0 : b / len;
    if (cfg.ramp.curve == RampCurve::Exponential) return r0 * std::pow(r1 / r0, t);
    return r0 + (r1 - r0) * t;
  };

  std::vector<double> onsets;
  onsets.push_back(0.0);
  const double h = 1.0e-6;
  double phase = 0.0;
  double b = 0.0;
  int next = 1;
  while (next < count && b < 1000.0) {
    const double before = phase;
    phase += 0.5 * (rateAt(b) + rateAt(b + h)) * h;
    b += h;
    if (before < next && phase >= next) {
      // Linear interpolation between the two integration steps.
      const double frac = (next - before) / (phase - before);
      onsets.push_back(b - h + frac * h);
      ++next;
    }
  }
  return onsets;
}

RollConfig straightConfig(const Rate& rate) {
  RollConfig cfg;
  cfg.rate = rate;
  cfg.ramp.enabled = false;
  return cfg;
}

// Collects every event the engine emits over `seconds` of rendering.
std::vector<RollEvent> collect(RollEngine& roll, Transport& t, FrameCount block, double seconds) {
  std::vector<RollEvent> all;
  RollEvent buf[64];
  const int blocks = static_cast<int>(std::ceil(seconds * kSr / static_cast<double>(block)));
  for (int i = 0; i < blocks; ++i) {
    const int n = roll.process(t, block, buf, 64);
    for (int e = 0; e < n; ++e) all.push_back(buf[e]);
    t.advance(block);
  }
  return all;
}

}  // namespace

NEKTA_TEST(rateCurveEndpoints) {
  RollEngine roll;
  roll.prepare(kSr);
  RollConfig cfg;
  cfg.rate = Rate(8);
  cfg.ramp.enabled = true;
  cfg.ramp.endRate = Rate(32);
  cfg.ramp.lengthBeats = 4.0;
  cfg.ramp.curve = RampCurve::Linear;
  roll.setConfig(cfg);
  CHECK_NEAR(roll.rateAtBeats(0.0), 2.0, 1e-9);   // 1/8 = 2 per beat
  CHECK_NEAR(roll.rateAtBeats(4.0), 8.0, 1e-9);   // 1/32 = 8 per beat
  CHECK_NEAR(roll.rateAtBeats(2.0), 5.0, 1e-9);   // linear midpoint
  CHECK_NEAR(roll.rateAtBeats(99.0), 8.0, 1e-9);  // holds after the ramp

  cfg.ramp.curve = RampCurve::Exponential;
  roll.setConfig(cfg);
  CHECK_NEAR(roll.rateAtBeats(2.0), std::sqrt(2.0 * 8.0), 1e-9);  // geometric midpoint
}

// SPEC §8: roll onsets land within +/-1 sample of the expected grid across all
// rates and BPMs.
NEKTA_TEST(constantRateOnsetsSitOnTheTransportGrid) {
  const double bpms[] = {60.0, 87.0, 120.0, 140.0, 174.0, 200.0};
  const Rate rates[] = {Rate(4),   Rate(8),  Rate(16), Rate(32),
                        Rate(64),  Rate(16, RateMod::Triplet), Rate(8, RateMod::Dotted)};
  long long worst = 0;
  for (double bpm : bpms) {
    for (const Rate& r : rates) {
      Transport t = makeTransport(bpm);
      RollEngine roll;
      roll.prepare(kSr);
      roll.setConfig(straightConfig(r));
      roll.start(t, 0);
      const std::vector<RollEvent> events = collect(roll, t, 128, 20.0);
      REQUIRE(events.size() > 8);
      const double spacing = r.beats() * t.samplesPerBeat();
      for (std::size_t i = 0; i < events.size(); ++i) {
        const double want = static_cast<double>(i) * spacing;
        const long long err =
            std::llabs(events[i].sampleTime - static_cast<SampleIndex>(std::llround(want)));
        if (err > worst) worst = err;
        CHECK(err <= 1);
        CHECK_EQ_INT(events[i].repeatIndex, static_cast<long long>(i));
      }
    }
  }
  CHECK(worst <= 1);
}

NEKTA_TEST(rampedOnsetsMatchNumericalIntegration) {
  struct Case {
    Rate from;
    Rate to;
    double lengthBeats;
    RampCurve curve;
  };
  const Case cases[] = {
      {Rate(8), Rate(32), 4.0, RampCurve::Linear},
      {Rate(8), Rate(32), 4.0, RampCurve::Exponential},
      {Rate(16), Rate(64), 2.0, RampCurve::Exponential},
      {Rate(32), Rate(8), 4.0, RampCurve::Linear},        // decelerating
      {Rate(32), Rate(8), 4.0, RampCurve::Exponential},   // decelerating
      {Rate(16), Rate(16), 4.0, RampCurve::Linear},       // degenerate: no change
  };
  for (const Case& c : cases) {
    RollConfig cfg;
    cfg.rate = c.from;
    cfg.ramp.enabled = true;
    cfg.ramp.endRate = c.to;
    cfg.ramp.lengthBeats = c.lengthBeats;
    cfg.ramp.curve = c.curve;

    RollEngine roll;
    roll.prepare(kSr);
    roll.setConfig(cfg);

    const int count = 40;
    const std::vector<double> oracle = integrateOnsets(cfg, count);
    REQUIRE(oracle.size() >= static_cast<std::size_t>(count));

    Transport t = makeTransport(174.0);
    const double spb = t.samplesPerBeat();
    for (int n = 0; n < count; ++n) {
      const double got = roll.beatsForRepeat(n);
      const double errSamples = std::fabs(got - oracle[static_cast<std::size_t>(n)]) * spb;
      CHECK(errSamples <= 1.0);
    }
  }
}

NEKTA_TEST(acceleratingRampTightensAndThenHolds) {
  RollConfig cfg;
  cfg.rate = Rate(8);
  cfg.ramp.enabled = true;
  cfg.ramp.endRate = Rate(32);
  cfg.ramp.lengthBeats = 4.0;
  cfg.ramp.curve = RampCurve::Linear;

  RollEngine roll;
  roll.prepare(kSr);
  roll.setConfig(cfg);

  double prevInterval = 1e9;
  for (int n = 1; n < 60; ++n) {
    const double interval = roll.beatsForRepeat(n) - roll.beatsForRepeat(n - 1);
    CHECK(interval <= prevInterval + 1e-9);
    prevInterval = interval;
  }
  // Past the ramp the interval is exactly the end rate.
  const double late = roll.beatsForRepeat(80) - roll.beatsForRepeat(79);
  CHECK_NEAR(late, Rate(32).beats(), 1e-9);
}

NEKTA_TEST(everyRepeatIsEmittedOnceAcrossBlockSizes) {
  const FrameCount blocks[] = {64, 128, 100, 256};
  for (FrameCount block : blocks) {
    Transport t = makeTransport(174.0);
    RollEngine roll;
    roll.prepare(kSr);
    RollConfig cfg;
    cfg.rate = Rate(8);
    cfg.ramp.enabled = true;
    cfg.ramp.endRate = Rate(64);
    cfg.ramp.lengthBeats = 4.0;
    cfg.ramp.curve = RampCurve::Exponential;
    roll.setConfig(cfg);
    roll.start(t, 0);

    const std::vector<RollEvent> events = collect(roll, t, block, 10.0);
    REQUIRE(events.size() > 20);
    for (std::size_t i = 0; i < events.size(); ++i) {
      CHECK_EQ_INT(events[i].repeatIndex, static_cast<long long>(i));
      CHECK(events[i].offsetInBlock >= 0 && events[i].offsetInBlock < block);
      if (i > 0) CHECK(events[i].sampleTime > events[i - 1].sampleTime);
      const SampleIndex want = static_cast<SampleIndex>(
          std::llround(roll.beatsForRepeat(static_cast<long long>(i)) * t.samplesPerBeat()));
      CHECK(std::llabs(events[i].sampleTime - want) <= 1);
    }
  }
}

NEKTA_TEST(perRepeatModifiers) {
  Transport t = makeTransport(174.0);
  RollEngine roll;
  roll.prepare(kSr);
  RollConfig cfg;
  cfg.rate = Rate(16);
  cfg.mods.ampDecay = 0.8f;
  cfg.mods.pitchDeltaSemis = 2.0;
  cfg.mods.pitchWrapRepeats = 4;
  cfg.mods.reverseAlternate = true;
  cfg.mods.panAlternate = true;
  cfg.mods.panAmount = 0.7f;
  cfg.mods.startOffsetWalkMs = 5.0;
  cfg.mods.gate = 0.5f;
  roll.setConfig(cfg);
  roll.start(t, 0);

  const std::vector<RollEvent> events = collect(roll, t, 128, 4.0);
  REQUIRE(events.size() >= 8);
  const double interval = Rate(16).beats() * 60.0 / 174.0;  // seconds
  for (std::size_t i = 0; i < 8; ++i) {
    const RollEvent& e = events[i];
    CHECK_NEAR(e.gain, std::pow(0.8f, static_cast<float>(i)), 1e-4);
    CHECK_NEAR(e.pitchSemitones, 2.0 * static_cast<double>(i % 4), 1e-9);
    CHECK(e.reverse == (i % 2 == 1));
    CHECK_NEAR(e.pan, (i % 2 == 1) ? 0.7f : -0.7f, 1e-6);
    CHECK_NEAR(e.startOffsetSeconds, 0.005 * static_cast<double>(i), 1e-9);
    CHECK_NEAR(e.gateSeconds, interval * 0.5, 1e-4);
  }
}

NEKTA_TEST(fullGateMeansNoGate) {
  Transport t = makeTransport(120.0);
  RollEngine roll;
  roll.prepare(kSr);
  RollConfig cfg = straightConfig(Rate(16));
  cfg.mods.gate = 1.0f;
  roll.setConfig(cfg);
  roll.start(t, 0);
  const std::vector<RollEvent> events = collect(roll, t, 128, 2.0);
  REQUIRE(!events.empty());
  CHECK(events[0].gateSeconds < 0.0);
}

NEKTA_TEST(stopHaltsEmission) {
  Transport t = makeTransport(120.0);
  RollEngine roll;
  roll.prepare(kSr);
  roll.setConfig(straightConfig(Rate(16)));
  roll.start(t, 0);
  const std::vector<RollEvent> first = collect(roll, t, 128, 2.0);
  CHECK(!first.empty());
  roll.stop();
  CHECK(!roll.running());
  const std::vector<RollEvent> after = collect(roll, t, 128, 2.0);
  CHECK(after.empty());
}

NEKTA_TEST(startInTheFutureWaits) {
  Transport t = makeTransport(120.0);
  RollEngine roll;
  roll.prepare(kSr);
  roll.setConfig(straightConfig(Rate(16)));
  const SampleIndex startAt = 24000;  // one beat in
  roll.start(t, startAt);
  RollEvent buf[16];
  // Blocks before the start emit nothing.
  for (int i = 0; i < 24000 / 128; ++i) {
    CHECK_EQ_INT(roll.process(t, 128, buf, 16), 0);
    t.advance(128);
  }
  const int n = roll.process(t, 128, buf, 16);
  CHECK_EQ_INT(n, 1);
  CHECK_EQ_INT(buf[0].sampleTime, startAt);
}

NEKTA_TEST(eventBufferCapacityIsRespected) {
  Transport t = makeTransport(200.0);
  RollEngine roll;
  roll.prepare(kSr);
  roll.setConfig(straightConfig(Rate(64)));
  roll.start(t, 0);
  RollEvent buf[2];
  // A big block at 1/64 would emit more than two events; the engine must not
  // write past the caller's array and must not lose the rest.
  const int n = roll.process(t, 2048, buf, 2);
  CHECK(n <= 2);
  t.advance(2048);
  const int m = roll.process(t, 2048, buf, 2);
  CHECK(m > 0);
}

NEKTA_TEST(rollAudioPathAllocatesNothing) {
  Transport t = makeTransport(174.0);
  RollEngine roll;
  roll.prepare(kSr);
  RollConfig cfg;
  cfg.rate = Rate(8);
  cfg.ramp.enabled = true;
  cfg.ramp.endRate = Rate(64);
  cfg.ramp.lengthBeats = 4.0;
  cfg.ramp.curve = RampCurve::Exponential;
  roll.setConfig(cfg);
  roll.start(t, 0);
  RollEvent buf[32];
  rt::resetViolations();
  {
    rt::ScopedAudioThread audio;
    for (int i = 0; i < 2000; ++i) {
      const int n = roll.process(t, 128, buf, 32);
      for (int e = 0; e < n; ++e) {
        CHECK(std::isfinite(buf[e].gain));
        CHECK(std::isfinite(buf[e].pitchSemitones));
      }
      t.advance(128);
    }
  }
  CHECK_EQ_INT(rt::allocationViolations(), 0);
}

NEKTA_TEST_MAIN("roll")
