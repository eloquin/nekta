#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "core/RtCheck.h"
#include "dsp/Svf.h"
#include "test/TestFramework.h"

using namespace nekta;

namespace {

constexpr double kSr = 48000.0;
constexpr double kPi = 3.14159265358979323846;

// Steady-state magnitude response at one frequency, measured by rendering a
// sine through the filter and comparing RMS in to RMS out.
double magnitudeAt(Svf& filter, double freq) {
  filter.reset();
  const int settle = static_cast<int>(kSr * 0.25);
  const int measure = static_cast<int>(kSr * 0.25);
  double sumIn = 0.0;
  double sumOut = 0.0;
  for (int i = 0; i < settle + measure; ++i) {
    float l = static_cast<float>(std::sin(2.0 * kPi * freq * i / kSr));
    float r = l;
    const float in = l;
    filter.processBlock(&l, &r, 1);
    if (i >= settle) {
      sumIn += static_cast<double>(in) * in;
      sumOut += static_cast<double>(l) * l;
    }
  }
  if (sumIn <= 0.0) return 0.0;
  return std::sqrt(sumOut / sumIn);
}

double toDb(double mag) { return 20.0 * std::log10(mag < 1e-12 ? 1e-12 : mag); }

Svf makeFilter(Svf::Mode mode, float cutoff, float q) {
  Svf f;
  f.prepare(kSr);
  f.setMode(mode);
  f.setCutoffImmediate(cutoff);
  f.setQ(q);
  return f;
}

// xorshift so the fuzz run is reproducible across machines and runs.
struct Rng {
  std::uint32_t s = 0x9e3779b9u;
  float next01() {
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return static_cast<float>(s & 0xffffffu) / static_cast<float>(0xffffff);
  }
  float bipolar() { return next01() * 2.0f - 1.0f; }
};

}  // namespace

NEKTA_TEST(bypassIsBitIdentical) {
  // SPEC §8 null test.
  Svf f = makeFilter(Svf::Mode::Lowpass, 800.0f, 4.0f);
  f.setBypass(true);
  Rng rng;
  for (int block = 0; block < 100; ++block) {
    float l[64], r[64], origL[64], origR[64];
    for (int i = 0; i < 64; ++i) {
      l[i] = origL[i] = rng.bipolar();
      r[i] = origR[i] = rng.bipolar();
    }
    f.processBlock(l, r, 64);
    for (int i = 0; i < 64; ++i) {
      CHECK(l[i] == origL[i]);
      CHECK(r[i] == origR[i]);
    }
  }
}

NEKTA_TEST(lowpassPassesDcAndCutsHighs) {
  Svf f = makeFilter(Svf::Mode::Lowpass, 1000.0f, 0.70710678f);
  CHECK_NEAR(toDb(magnitudeAt(f, 50.0)), 0.0, 0.5);
  CHECK(toDb(magnitudeAt(f, 12000.0)) < -35.0);
}

NEKTA_TEST(highpassBlocksDcAndPassesHighs) {
  Svf f = makeFilter(Svf::Mode::Highpass, 1000.0f, 0.70710678f);
  CHECK(toDb(magnitudeAt(f, 50.0)) < -40.0);
  CHECK_NEAR(toDb(magnitudeAt(f, 15000.0)), 0.0, 0.5);
}

NEKTA_TEST(minusThreeDbAtCutoffForButterworthQ) {
  Svf f = makeFilter(Svf::Mode::Lowpass, 1000.0f, 0.70710678f);
  CHECK_NEAR(toDb(magnitudeAt(f, 1000.0)), -3.0, 0.5);
}

NEKTA_TEST(twelveDbPerOctaveSlope) {
  Svf f = makeFilter(Svf::Mode::Lowpass, 1000.0f, 0.70710678f);
  const double a = toDb(magnitudeAt(f, 4000.0));
  const double b = toDb(magnitudeAt(f, 8000.0));
  CHECK_NEAR(a - b, 12.0, 1.5);
}

NEKTA_TEST(bandpassPeaksAtCutoff) {
  Svf f = makeFilter(Svf::Mode::Bandpass, 1000.0f, 4.0f);
  const double at = toDb(magnitudeAt(f, 1000.0));
  CHECK(at > toDb(magnitudeAt(f, 100.0)) + 20.0);
  CHECK(at > toDb(magnitudeAt(f, 10000.0)) + 20.0);
}

NEKTA_TEST(notchRejectsAtCutoff) {
  Svf f = makeFilter(Svf::Mode::Notch, 1000.0f, 2.0f);
  CHECK(toDb(magnitudeAt(f, 1000.0)) < -25.0);
  CHECK_NEAR(toDb(magnitudeAt(f, 50.0)), 0.0, 1.0);
  CHECK_NEAR(toDb(magnitudeAt(f, 15000.0)), 0.0, 1.0);
}

NEKTA_TEST(resonanceLiftsTheCutoffPeak) {
  Svf low = makeFilter(Svf::Mode::Lowpass, 1000.0f, 0.0f);
  low.setResonance(0.0f);
  Svf high = makeFilter(Svf::Mode::Lowpass, 1000.0f, 0.0f);
  high.setResonance(0.9f);
  CHECK(toDb(magnitudeAt(high, 1000.0)) > toDb(magnitudeAt(low, 1000.0)) + 10.0);
}

NEKTA_TEST(morphMovesContinuouslyBetweenModes) {
  Svf f;
  f.prepare(kSr);
  f.setCutoffImmediate(1000.0f);
  f.setQ(0.70710678f);
  f.setMorph(0.0f);
  const double lpLow = toDb(magnitudeAt(f, 50.0));
  f.setMorph(2.0f);
  const double hpLow = toDb(magnitudeAt(f, 50.0));
  f.setMorph(1.0f);
  const double bpLow = toDb(magnitudeAt(f, 50.0));
  CHECK(lpLow > bpLow);
  CHECK(bpLow > hpLow);
  // Morph 0/1/2/3 must equal the discrete modes.
  Svf discrete = makeFilter(Svf::Mode::Highpass, 1000.0f, 0.70710678f);
  f.setMorph(2.0f);
  CHECK_NEAR(magnitudeAt(f, 5000.0), magnitudeAt(discrete, 5000.0), 1e-4);
}

// SPEC §2.5: stable at audio-rate modulation.
NEKTA_TEST(stableUnderAudioRateCutoffModulation) {
  Svf f;
  f.prepare(kSr);
  f.setMode(Svf::Mode::Lowpass);
  f.setQ(15.0f);
  float peak = 0.0f;
  for (int i = 0; i < static_cast<int>(kSr * 2); ++i) {
    // 300 Hz sweep of the cutoff over its full range, every single sample.
    const double lfo = 0.5 + 0.5 * std::sin(2.0 * kPi * 300.0 * i / kSr);
    f.setCutoff(static_cast<float>(20.0 + lfo * 15000.0));
    float l = static_cast<float>(std::sin(2.0 * kPi * 220.0 * i / kSr));
    float r = l;
    f.processBlock(&l, &r, 1);
    CHECK(std::isfinite(l) && std::isfinite(r));
    peak = std::max(peak, std::fabs(l));
  }
  CHECK(peak < 40.0f);  // resonant but bounded
}

NEKTA_TEST(fuzzRandomAutomationStaysFinite) {
  // SPEC §8 fuzz: random param automation at audio rate, no NaN/inf/denormal.
  Svf f;
  f.prepare(kSr);
  Rng rng;
  for (int i = 0; i < 400000; ++i) {
    f.setCutoff(20.0f + rng.next01() * 20000.0f);
    f.setResonance(rng.next01());
    f.setDrive(1.0f + rng.next01() * 10.0f);
    f.setMorph(rng.next01() * 3.0f);
    float l = rng.bipolar();
    float r = rng.bipolar();
    f.processBlock(&l, &r, 1);
    REQUIRE(std::isfinite(l));
    REQUIRE(std::isfinite(r));
    REQUIRE(std::fabs(l) < 100.0f);
  }
}

NEKTA_TEST(stateFlushesToExactZeroAfterSilence) {
  // Denormals in the state would otherwise idle the CPU at full tilt.
  Svf f = makeFilter(Svf::Mode::Lowpass, 200.0f, 8.0f);
  float l = 1.0f, r = 1.0f;
  f.processBlock(&l, &r, 1);
  for (int i = 0; i < 200000; ++i) {
    l = 0.0f;
    r = 0.0f;
    f.processBlock(&l, &r, 1);
  }
  CHECK(l == 0.0f);
  CHECK(r == 0.0f);
}

NEKTA_TEST(driveSaturatesWithoutBlowingUp) {
  Svf f = makeFilter(Svf::Mode::Lowpass, 8000.0f, 0.70710678f);
  f.setDrive(8.0f);
  float peak = 0.0f;
  for (int i = 0; i < 4800; ++i) {
    float l = 4.0f * static_cast<float>(std::sin(2.0 * kPi * 100.0 * i / kSr));
    float r = l;
    f.processBlock(&l, &r, 1);
    peak = std::max(peak, std::fabs(l));
  }
  CHECK(peak <= 1.5f);
}

NEKTA_TEST(cutoffJumpIsRampedNotStepped) {
  // A big cutoff jump is ramped across the block, so the block boundary carries
  // no discontinuity. Contrasted against setCutoffImmediate, which by design
  // applies the jump at once - if the ramp were not working the two would match.
  auto boundaryStep = [](bool immediate) {
    Svf f = makeFilter(Svf::Mode::Lowpass, 200.0f, 0.70710678f);
    std::vector<float> l(512), r(512);
    for (int i = 0; i < 512; ++i) {
      l[static_cast<std::size_t>(i)] = r[static_cast<std::size_t>(i)] =
          static_cast<float>(std::sin(2.0 * kPi * 100.0 * i / kSr));
    }
    f.processBlock(l.data(), r.data(), 256);
    if (immediate) {
      f.setCutoffImmediate(12000.0f);
    } else {
      f.setCutoff(12000.0f);
    }
    f.processBlock(l.data() + 256, r.data() + 256, 256);
    const float atBoundary = std::fabs(l[256] - l[255]);
    const float before = std::fabs(l[255] - l[254]);
    return std::make_pair(atBoundary, before);
  };

  const std::pair<float, float> ramped = boundaryStep(false);
  const std::pair<float, float> stepped = boundaryStep(true);
  // The ramped boundary step is in line with the sample before it: no click.
  CHECK(ramped.first < ramped.second * 2.0f);
  // And it is an order of magnitude smaller than applying the jump at once.
  CHECK(ramped.first < stepped.first * 0.2f);
}

NEKTA_TEST(svfAudioPathAllocatesNothing) {
  Svf f = makeFilter(Svf::Mode::Lowpass, 1000.0f, 2.0f);
  float l[128] = {0}, r[128] = {0};
  rt::resetViolations();
  {
    rt::ScopedAudioThread audio;
    for (int block = 0; block < 500; ++block) {
      f.setCutoff(200.0f + static_cast<float>(block));
      f.processBlock(l, r, 128);
    }
  }
  CHECK_EQ_INT(rt::allocationViolations(), 0);
}

NEKTA_TEST_MAIN("svf")
