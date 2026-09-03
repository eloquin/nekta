#include <cmath>
#include <vector>

#include "clock/Rate.h"
#include "clock/Transport.h"
#include "core/RtCheck.h"
#include "test/TestFramework.h"

using namespace nekta;

namespace {

const double kBpms[] = {60.0, 72.0, 86.0, 100.0, 120.0, 137.5, 140.0, 160.0, 174.0, 200.0};

const Rate kRates[] = {
    {4, RateMod::Straight},  {8, RateMod::Straight},  {16, RateMod::Straight},
    {32, RateMod::Straight}, {64, RateMod::Straight}, {8, RateMod::Triplet},
    {16, RateMod::Triplet},  {32, RateMod::Triplet},  {8, RateMod::Dotted},
    {16, RateMod::Dotted},
};

// Independent oracle, deliberately computed a different way from the engine:
// long double, from BPM rather than from a cached samples-per-beat.
long double idealGridSample(double bpm, double sr, const Rate& r, long long k) {
  const long double beatsPerRepeat = static_cast<long double>(r.beats());
  const long double secondsPerBeat = 60.0L / static_cast<long double>(bpm);
  return static_cast<long double>(k) * beatsPerRepeat * secondsPerBeat * static_cast<long double>(sr);
}

}  // namespace

NEKTA_TEST(rateDurations) {
  CHECK_NEAR(Rate(4, RateMod::Straight).beats(), 1.0, 1e-12);
  CHECK_NEAR(Rate(8, RateMod::Straight).beats(), 0.5, 1e-12);
  CHECK_NEAR(Rate(16, RateMod::Straight).beats(), 0.25, 1e-12);
  CHECK_NEAR(Rate(64, RateMod::Straight).beats(), 0.0625, 1e-12);
  // A triplet fits three in the space of two.
  CHECK_NEAR(Rate(8, RateMod::Triplet).beats(), 0.5 * 2.0 / 3.0, 1e-12);
  // A dotted note is one and a half times as long.
  CHECK_NEAR(Rate(8, RateMod::Dotted).beats(), 0.75, 1e-12);
  // Repeats-per-beat is the roll engine's domain.
  CHECK_NEAR(Rate(16, RateMod::Straight).repeatsPerBeat(), 4.0, 1e-12);
  CHECK_NEAR(Rate(32, RateMod::Straight).repeatsPerBeat(), 8.0, 1e-12);
}

NEKTA_TEST(samplesPerBeatMatchesBpm) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(120.0);
  CHECK_NEAR(t.samplesPerBeat(), 24000.0, 1e-9);
  t.setBpm(174.0);
  CHECK_NEAR(t.samplesPerBeat(), 48000.0 * 60.0 / 174.0, 1e-9);
}

NEKTA_TEST(ppqAndSampleConversionsRoundTrip) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(174.0);
  for (double ppq = 0.0; ppq < 256.0; ppq += 0.3125) {
    const double s = t.sampleAtPpq(ppq);
    CHECK_NEAR(t.ppqAtSample(static_cast<SampleIndex>(std::llround(s))), ppq, 1e-4);
  }
}

// SPEC §8: roll/grid onsets must land within +/-1 sample of the expected grid
// across all rates and BPMs.
NEKTA_TEST(gridOnsetsWithinOneSampleAcrossBpmsAndRates) {
  Transport t;
  t.prepare(48000.0);
  long long worst = 0;
  for (double bpm : kBpms) {
    for (const Rate& r : kRates) {
      t.reset();
      t.setBpm(bpm);
      for (long long k = 0; k < 128; ++k) {
        const double gridPpq = static_cast<double>(k) * r.beats();
        const SampleIndex got = t.sampleAtPpqRounded(gridPpq);
        const long double want = idealGridSample(bpm, 48000.0, r, k);
        const long long err = std::llabs(got - static_cast<long long>(std::llround(want)));
        if (err > worst) worst = err;
        CHECK(err <= 1);
      }
    }
  }
  CHECK_EQ_INT(worst, 0);
}

NEKTA_TEST(nextGridIsStrictlyAheadAndSpacedCorrectly) {
  Transport t;
  t.prepare(48000.0);
  for (double bpm : kBpms) {
    for (const Rate& r : kRates) {
      t.reset();
      t.setBpm(bpm);
      const double spacing = r.beats() * t.samplesPerBeat();
      SampleIndex pos = 0;
      SampleIndex prev = -1;
      for (int k = 0; k < 64; ++k) {
        const SampleIndex g = t.nextGridSample(pos, r);
        CHECK(g >= pos);
        if (prev >= 0) CHECK_NEAR(static_cast<double>(g - prev), spacing, 1.0);
        prev = g;
        pos = g + 1;
      }
    }
  }
}

NEKTA_TEST(gridIteratorCoversEveryOnsetExactlyOnce) {
  const FrameCount blockSizes[] = {128, 256, 100, 64};
  for (FrameCount block : blockSizes) {
    Transport t;
    t.prepare(48000.0);
    t.setBpm(174.0);
    t.setPlaying(true);
    const Rate rate(16, RateMod::Straight);

    std::vector<SampleIndex> hits;
    const int blocks = 400;
    for (int b = 0; b < blocks; ++b) {
      GridIterator it(t, rate, block);
      FrameCount offset = 0;
      std::int64_t index = 0;
      while (it.next(offset, index)) {
        CHECK(offset >= 0 && offset < block);
        hits.push_back(t.samplePosition() + offset);
      }
      t.advance(block);
    }
    const double spacing = rate.beats() * t.samplesPerBeat();
    const SampleIndex end = static_cast<SampleIndex>(blocks) * block;
    const std::size_t expectedCount =
        static_cast<std::size_t>(std::ceil(static_cast<double>(end) / spacing));
    CHECK_EQ_INT(hits.size(), expectedCount);
    for (std::size_t i = 0; i < hits.size(); ++i) {
      const double want = static_cast<double>(i) * spacing;
      CHECK_NEAR(static_cast<double>(hits[i]), want, 1.0);
      if (i > 0) CHECK(hits[i] > hits[i - 1]);
    }
  }
}

NEKTA_TEST(positionDoesNotDriftOverTenMinutes) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(174.0);
  t.setPlaying(true);
  const SampleIndex total = 48000LL * 600;
  for (SampleIndex s = 0; s < total; s += 128) t.advance(128);
  CHECK_EQ_INT(t.samplePosition(), total);
  const double expectedPpq = static_cast<double>(total) / t.samplesPerBeat();
  CHECK_NEAR(t.ppqPosition(), expectedPpq, 1e-9);
}

NEKTA_TEST(tempoChangeReAnchorsWithoutJumping) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(120.0);
  t.setPlaying(true);
  t.advance(48000);  // two beats at 120
  CHECK_NEAR(t.ppqPosition(), 2.0, 1e-9);
  t.setBpm(174.0);
  CHECK_NEAR(t.ppqPosition(), 2.0, 1e-9);  // musical position preserved
  t.advance(static_cast<FrameCount>(std::llround(48000.0 * 60.0 / 174.0)));
  CHECK_NEAR(t.ppqPosition(), 3.0, 1e-3);
}

NEKTA_TEST(quantiseLaunchSnapsToGrid) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(120.0);
  const Rate bar(1, RateMod::Straight);  // 1/1 = 4 beats
  CHECK_NEAR(bar.beats(), 4.0, 1e-12);
  const SampleIndex barSamples = static_cast<SampleIndex>(4.0 * 24000.0);
  CHECK_EQ_INT(t.nextGridSample(1, bar), barSamples);
  CHECK_EQ_INT(t.nextGridSample(0, bar), 0);
  CHECK_EQ_INT(t.nearestGridSample(barSamples - 10, bar), barSamples);
  CHECK_EQ_INT(t.nearestGridSample(10, bar), 0);
}

NEKTA_TEST(barPhaseTracksPosition) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(120.0);
  t.setPlaying(true);
  t.setBeatsPerBar(4);
  CHECK_NEAR(t.barPhase(), 0.0, 1e-9);
  t.advance(24000);  // one beat
  CHECK_NEAR(t.barPhase(), 0.25, 1e-9);
  t.advance(24000 * 3);
  CHECK_NEAR(t.barPhase(), 0.0, 1e-9);
}

NEKTA_TEST(pausedTransportHolds) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(120.0);
  t.setPlaying(false);
  t.advance(4800);
  CHECK_EQ_INT(t.samplePosition(), 0);
  t.setPlaying(true);
  t.advance(4800);
  CHECK_EQ_INT(t.samplePosition(), 4800);
}

NEKTA_TEST(bpmIsClampedToSpecRange) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(5.0);
  CHECK_NEAR(t.bpm(), kMinBpm, 1e-9);
  t.setBpm(10000.0);
  CHECK_NEAR(t.bpm(), kMaxBpm, 1e-9);
}

NEKTA_TEST(transportAudioPathAllocatesNothing) {
  Transport t;
  t.prepare(48000.0);
  t.setBpm(174.0);
  t.setPlaying(true);
  rt::resetViolations();
  {
    rt::ScopedAudioThread audio;
    for (int b = 0; b < 500; ++b) {
      GridIterator it(t, Rate(32, RateMod::Triplet), 128);
      FrameCount offset = 0;
      std::int64_t index = 0;
      while (it.next(offset, index)) {
        volatile FrameCount sink = offset;
        (void)sink;
      }
      t.advance(128);
    }
  }
  CHECK_EQ_INT(rt::allocationViolations(), 0);
}

NEKTA_TEST_MAIN("clock")
