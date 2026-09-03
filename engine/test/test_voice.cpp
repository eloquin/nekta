#include <algorithm>
#include <cmath>
#include <vector>

#include "core/RtCheck.h"
#include "test/TestFramework.h"
#include "voice/Envelope.h"
#include "voice/Interpolation.h"
#include "voice/SampleBuffer.h"
#include "voice/SamplerVoice.h"
#include "voice/VoicePool.h"

using namespace nekta;

namespace {

constexpr double kSr = 48000.0;
constexpr double kPi = 3.14159265358979323846;

SampleBuffer makeDc(int frames, float level = 1.0f) {
  SampleBuffer b;
  b.setSize(1, frames);
  b.setSampleRate(kSr);
  for (int i = 0; i < frames; ++i) b.writePtr(0)[i] = level;
  return b;
}

SampleBuffer makeRamp(int frames) {
  SampleBuffer b;
  b.setSize(1, frames);
  b.setSampleRate(kSr);
  for (int i = 0; i < frames; ++i) {
    b.writePtr(0)[i] = static_cast<float>(i) / static_cast<float>(frames - 1);
  }
  return b;
}

struct Render {
  std::vector<float> l, r;
  Render(int n) : l(static_cast<std::size_t>(n), 0.0f), r(static_cast<std::size_t>(n), 0.0f) {}
};

VoiceSpec basicSpec(const SampleBuffer& s) {
  VoiceSpec v;
  v.sample = &s;
  v.env.attackMs = 0.0f;
  v.env.holdMs = 0.0f;
  v.env.decayMs = 0.0f;
  v.env.sustain = 1.0f;
  v.env.releaseMs = 1.0f;
  return v;
}

}  // namespace

NEKTA_TEST(hermitePassesThroughKnots) {
  // 4-point Hermite must interpolate (not approximate) the two centre points.
  CHECK_NEAR(hermite4(0.0f, 1.0f, 2.0f, 3.0f, 0.0f), 1.0f, 1e-6);
  CHECK_NEAR(hermite4(0.0f, 1.0f, 2.0f, 3.0f, 1.0f), 2.0f, 1e-6);
  // A straight line is reproduced exactly.
  for (double t = 0.0; t <= 1.0; t += 0.05) {
    CHECK_NEAR(hermite4(0.0f, 1.0f, 2.0f, 3.0f, static_cast<float>(t)), 1.0 + t, 1e-5);
  }
}

NEKTA_TEST(hermiteBeatsLinearOnASine) {
  const double freq = 1000.0;
  double hermiteErr = 0.0;
  double linearErr = 0.0;
  auto sine = [&](double n) { return std::sin(2.0 * kPi * freq * n / kSr); };
  for (double pos = 100.0; pos < 200.0; pos += 0.1) {
    const int i = static_cast<int>(pos);
    const float t = static_cast<float>(pos - i);
    const float h = hermite4(static_cast<float>(sine(i - 1)), static_cast<float>(sine(i)),
                             static_cast<float>(sine(i + 1)), static_cast<float>(sine(i + 2)), t);
    const float lin = static_cast<float>(sine(i) + t * (sine(i + 1) - sine(i)));
    hermiteErr += std::fabs(h - sine(pos));
    linearErr += std::fabs(lin - sine(pos));
  }
  CHECK(hermiteErr < linearErr * 0.25);
}

NEKTA_TEST(sampleBufferReadsExactAtIntegerPositions) {
  SampleBuffer b = makeRamp(64);
  for (int i = 0; i < 64; ++i) {
    CHECK_NEAR(b.readInterpolated(0, static_cast<double>(i)), b.data(0)[i], 1e-6);
  }
  // Out of range reads are clamped, never out of bounds.
  CHECK_NEAR(b.readInterpolated(0, -5.0), b.data(0)[0], 1e-6);
  CHECK_NEAR(b.readInterpolated(0, 1000.0), b.data(0)[63], 1e-6);
}

NEKTA_TEST(envelopeSegmentsHaveTheRightShape) {
  AhdsrEnvelope env;
  env.prepare(kSr);
  AhdsrParams p;
  p.attackMs = 10.0f;
  p.holdMs = 5.0f;
  p.decayMs = 20.0f;
  p.sustain = 0.5f;
  p.releaseMs = 10.0f;
  env.trigger(p);
  CHECK(env.active());
  float v = 0.0f;
  for (int i = 0; i < static_cast<int>(0.010 * kSr); ++i) v = env.nextSample();
  CHECK_NEAR(v, 1.0f, 1e-3);  // attack complete
  for (int i = 0; i < static_cast<int>(0.005 * kSr); ++i) v = env.nextSample();
  CHECK_NEAR(v, 1.0f, 1e-3);  // hold
  for (int i = 0; i < static_cast<int>(0.020 * kSr); ++i) v = env.nextSample();
  CHECK_NEAR(v, 0.5f, 1e-3);  // decayed to sustain
  for (int i = 0; i < 1000; ++i) v = env.nextSample();
  CHECK_NEAR(v, 0.5f, 1e-3);  // sustains
  env.release();
  for (int i = 0; i < static_cast<int>(0.010 * kSr) + 2; ++i) v = env.nextSample();
  CHECK_NEAR(v, 0.0f, 1e-3);
  CHECK(!env.active());
}

NEKTA_TEST(envelopeWithZeroSustainEndsAfterDecay) {
  AhdsrEnvelope env;
  env.prepare(kSr);
  AhdsrParams p;
  p.attackMs = 1.0f;
  p.decayMs = 10.0f;
  p.sustain = 0.0f;
  env.trigger(p);
  for (int i = 0; i < static_cast<int>(0.012 * kSr); ++i) env.nextSample();
  CHECK(!env.active());
}

NEKTA_TEST(voicePlaysSampleAtUnityPitch) {
  SampleBuffer s = makeDc(2000);
  SamplerVoice v;
  v.prepare(kSr);
  VoiceSpec spec = basicSpec(s);
  spec.pan = -1.0f;  // hard left keeps the maths simple
  v.start(spec);
  Render out(2000);
  v.process(out.l.data(), out.r.data(), 2000);
  // After the 2 ms anti-click fade the level is unity.
  const int settled = static_cast<int>(0.003 * kSr);
  CHECK_NEAR(out.l[static_cast<std::size_t>(settled)], 1.0f, 1e-4);
  CHECK_NEAR(out.r[static_cast<std::size_t>(settled)], 0.0f, 1e-4);
}

NEKTA_TEST(antiClickFadeOnStartAndChoke) {
  SampleBuffer s = makeDc(48000);
  SamplerVoice v;
  v.prepare(kSr);
  v.start(basicSpec(s));
  Render out(4800);
  v.process(out.l.data(), out.r.data(), 4800);
  // No full-scale step on the first sample.
  CHECK(std::fabs(out.l[0]) < 0.05f);
  // The fade is monotonic over its 2 ms.
  const int fade = static_cast<int>(0.002 * kSr);
  for (int i = 1; i < fade; ++i) {
    CHECK(out.l[static_cast<std::size_t>(i)] >= out.l[static_cast<std::size_t>(i - 1)] - 1e-6f);
  }
  // Choke fades out rather than cutting.
  v.choke();
  Render tail(4800);
  v.process(tail.l.data(), tail.r.data(), 4800);
  float maxStep = 0.0f;
  for (std::size_t i = 1; i < tail.l.size(); ++i) {
    maxStep = std::max(maxStep, std::fabs(tail.l[i] - tail.l[i - 1]));
  }
  CHECK(maxStep < 0.01f);
  CHECK_NEAR(tail.l.back(), 0.0f, 1e-6);
  CHECK(!v.active());
}

NEKTA_TEST(varispeedRepitchesAndShortens) {
  SampleBuffer s = makeDc(4800);  // 100 ms
  SamplerVoice v;
  v.prepare(kSr);
  VoiceSpec spec = basicSpec(s);
  spec.pitchSemitones = 12.0;  // one octave up = half the duration
  v.start(spec);
  Render out(9600);
  int played = 0;
  for (int i = 0; i < 9600; ++i) {
    v.process(out.l.data() + i, out.r.data() + i, 1);
    if (v.active()) ++played;
  }
  CHECK_NEAR(played, 2400, 60);
}

NEKTA_TEST(reversePlaysBackwards) {
  SampleBuffer s = makeRamp(1000);
  SamplerVoice v;
  v.prepare(kSr);
  VoiceSpec spec = basicSpec(s);
  spec.reverse = true;
  spec.pan = -1.0f;
  v.start(spec);
  Render out(1000);
  v.process(out.l.data(), out.r.data(), 1000);
  const int settled = static_cast<int>(0.003 * kSr);
  // Reversed ramp: descending after the fade-in region.
  CHECK(out.l[static_cast<std::size_t>(settled)] > out.l[static_cast<std::size_t>(settled + 200)]);
  CHECK_NEAR(out.l[static_cast<std::size_t>(settled)],
             s.data(0)[1000 - 1 - settled], 2e-3);
}

NEKTA_TEST(loopModeKeepsPlaying) {
  SampleBuffer s = makeDc(480);
  SamplerVoice v;
  v.prepare(kSr);
  VoiceSpec spec = basicSpec(s);
  spec.loop = true;
  spec.mode = TriggerMode::Loop;
  v.start(spec);
  Render out(48000);
  v.process(out.l.data(), out.r.data(), 48000);
  CHECK(v.active());
  CHECK(std::fabs(out.l[47000]) > 0.1f);
}

NEKTA_TEST(startDelayIsSampleAccurate) {
  SampleBuffer s = makeDc(1000);
  SamplerVoice v;
  v.prepare(kSr);
  VoiceSpec spec = basicSpec(s);
  v.start(spec, 64);
  Render out(256);
  v.process(out.l.data(), out.r.data(), 256);
  for (int i = 0; i < 64; ++i) CHECK_NEAR(out.l[static_cast<std::size_t>(i)], 0.0f, 1e-9);
  CHECK(std::fabs(out.l[64]) > 0.0f || std::fabs(out.l[65]) > 0.0f);
}

NEKTA_TEST(gateSecondsReleasesTheVoice) {
  SampleBuffer s = makeDc(48000);
  SamplerVoice v;
  v.prepare(kSr);
  VoiceSpec spec = basicSpec(s);
  spec.gateSeconds = 0.010;
  spec.env.releaseMs = 2.0f;
  v.start(spec);
  Render out(48000);
  v.process(out.l.data(), out.r.data(), 48000);
  CHECK(!v.active());
  // Silent well before the sample runs out.
  CHECK_NEAR(out.l[static_cast<std::size_t>(0.030 * kSr)], 0.0f, 1e-6);
}

NEKTA_TEST(equalPowerPan) {
  SampleBuffer s = makeDc(2000);
  SamplerVoice v;
  v.prepare(kSr);
  VoiceSpec spec = basicSpec(s);
  spec.pan = 0.0f;
  v.start(spec);
  Render out(2000);
  v.process(out.l.data(), out.r.data(), 2000);
  const std::size_t i = static_cast<std::size_t>(0.003 * kSr);
  CHECK_NEAR(out.l[i], 0.70710678f, 1e-4);
  CHECK_NEAR(out.r[i], 0.70710678f, 1e-4);
}

NEKTA_TEST(chokeGroupsCutTheirOwnGroupOnly) {
  SampleBuffer s = makeDc(48000);
  VoicePool pool;
  pool.prepare(kSr);
  VoiceSpec a = basicSpec(s);
  a.chokeGroup = 1;
  a.padIndex = 0;
  VoiceSpec b = basicSpec(s);
  b.chokeGroup = 2;
  b.padIndex = 1;
  SamplerVoice* first = pool.trigger(a, 0);
  SamplerVoice* other = pool.trigger(b, 0);
  REQUIRE(first != nullptr);
  REQUIRE(other != nullptr);
  VoiceSpec a2 = a;
  SamplerVoice* second = pool.trigger(a2, 0);
  REQUIRE(second != nullptr);
  CHECK(second != first);
  CHECK(first->choking());   // same group: faded out
  CHECK(other->active());    // different group: untouched
  CHECK(!other->choking());
}

NEKTA_TEST(voicePoolStealsInsteadOfFailing) {
  SampleBuffer s = makeDc(48000);
  VoicePool pool;
  pool.prepare(kSr);
  for (int i = 0; i < kNumVoices * 3; ++i) {
    VoiceSpec spec = basicSpec(s);
    spec.padIndex = i % kNumPads;
    CHECK(pool.trigger(spec, 0) != nullptr);
  }
  CHECK(pool.activeVoices() <= kNumVoices);
}

NEKTA_TEST(renderIsDeterministic) {
  SampleBuffer s = makeRamp(4000);
  Render a(8000), b(8000);
  for (int pass = 0; pass < 2; ++pass) {
    SamplerVoice v;
    v.prepare(kSr);
    VoiceSpec spec = basicSpec(s);
    spec.pitchSemitones = 3.5;
    v.start(spec);
    Render& out = (pass == 0) ? a : b;
    v.process(out.l.data(), out.r.data(), 8000);
  }
  for (std::size_t i = 0; i < a.l.size(); ++i) {
    CHECK(a.l[i] == b.l[i]);
    CHECK(a.r[i] == b.r[i]);
  }
}

NEKTA_TEST(voicePathAllocatesNothingAndStaysFinite) {
  SampleBuffer s = makeRamp(48000);
  VoicePool pool;
  pool.prepare(kSr);
  Render out(128);
  rt::resetViolations();
  {
    rt::ScopedAudioThread audio;
    for (int block = 0; block < 200; ++block) {
      if (block % 3 == 0) {
        VoiceSpec spec = basicSpec(s);
        spec.padIndex = block % kNumPads;
        spec.chokeGroup = block % 4;
        spec.pitchSemitones = static_cast<double>(block % 24) - 12.0;
        pool.trigger(spec, static_cast<FrameCount>(block % 128));
      }
      std::fill(out.l.begin(), out.l.end(), 0.0f);
      std::fill(out.r.begin(), out.r.end(), 0.0f);
      pool.process(out.l.data(), out.r.data(), 128);
      for (std::size_t i = 0; i < out.l.size(); ++i) {
        CHECK(std::isfinite(out.l[i]) && std::isfinite(out.r[i]));
      }
    }
  }
  CHECK_EQ_INT(rt::allocationViolations(), 0);
}

NEKTA_TEST_MAIN("voice")
