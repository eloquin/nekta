#include <string>
#include <vector>

#include "core/RtCheck.h"
#include "params/Mapping.h"
#include "params/ParamRegistry.h"
#include "test/TestFramework.h"

using namespace nekta;

namespace {

ParamRegistry makeRegistry() {
  ParamRegistry r(64);
  r.add("fx.1.cutoff", {20.0f, 20000.0f, 1000.0f, 15.0f});
  r.add("fx.1.res", {0.0f, 1.0f, 0.2f, 15.0f});
  r.add("send.delay", {0.0f, 1.0f, 0.0f, 20.0f});
  r.add("roll.rate", {1.0f, 64.0f, 16.0f, 0.0f});  // stepped, no smoothing
  r.prepare(48000.0, 128);
  return r;
}

}  // namespace

NEKTA_TEST(pathAddressingRoundTrip) {
  ParamRegistry r = makeRegistry();
  CHECK_EQ_INT(r.size(), 4);
  const ParamId cutoff = r.find("fx.1.cutoff");
  const ParamId res = r.find("fx.1.res");
  CHECK(cutoff != kInvalidParam);
  CHECK(res != kInvalidParam);
  CHECK(cutoff != res);
  CHECK(r.find("nope.not.here") == kInvalidParam);
  CHECK(std::string(r.path(cutoff)) == "fx.1.cutoff");
}

NEKTA_TEST(duplicatePathIsIdempotent) {
  ParamRegistry r = makeRegistry();
  const ParamId first = r.find("send.delay");
  const ParamId again = r.add("send.delay", {0.0f, 1.0f, 0.5f, 5.0f});
  CHECK_EQ_INT(first, again);
  CHECK_EQ_INT(r.size(), 4);
}

NEKTA_TEST(capacityIsBoundedAndReported) {
  ParamRegistry r(2);
  CHECK(r.add("a", {}) != kInvalidParam);
  CHECK(r.add("b", {}) != kInvalidParam);
  CHECK(r.add("c", {}) == kInvalidParam);  // full: no growth, no allocation later
}

NEKTA_TEST(defaultsApplyAndClamp) {
  ParamRegistry r = makeRegistry();
  const ParamId cutoff = r.find("fx.1.cutoff");
  CHECK_NEAR(r.value(cutoff), 1000.0f, 1e-6);
  r.setTarget(cutoff, 1.0e9f);
  CHECK_NEAR(r.target(cutoff), 20000.0f, 1e-6);
  r.setTarget(cutoff, -50.0f);
  CHECK_NEAR(r.target(cutoff), 20.0f, 1e-6);
}

NEKTA_TEST(onePoleReachesTimeConstant) {
  ParamRegistry r = makeRegistry();
  const ParamId delay = r.find("send.delay");  // 20 ms
  r.setTarget(delay, 1.0f);
  // One time constant: 63.2% of the step.
  r.advanceBlock(static_cast<FrameCount>(0.020 * 48000.0));
  CHECK_NEAR(r.value(delay), 0.6321f, 0.002);
  // Five time constants total: within 1%.
  r.advanceBlock(static_cast<FrameCount>(0.080 * 48000.0));
  CHECK(r.value(delay) > 0.99f);
  CHECK(r.value(delay) < 1.0f);
}

NEKTA_TEST(blockAdvanceMatchesPerSampleAdvance) {
  ParamRegistry a = makeRegistry();
  ParamRegistry b = makeRegistry();
  const ParamId id = a.find("fx.1.cutoff");
  a.setTarget(id, 8000.0f);
  b.setTarget(id, 8000.0f);
  a.advanceBlock(128);
  for (int i = 0; i < 128; ++i) b.advanceBlock(1);
  CHECK_NEAR(a.value(id), b.value(id), 1e-2);
}

NEKTA_TEST(blockSizeChangeStaysAccurate) {
  ParamRegistry a = makeRegistry();
  ParamRegistry b = makeRegistry();
  const ParamId id = a.find("fx.1.res");
  a.setTarget(id, 0.9f);
  b.setTarget(id, 0.9f);
  a.advanceBlock(128);
  a.advanceBlock(37);
  a.advanceBlock(128);
  for (int i = 0; i < 128 + 37 + 128; ++i) b.advanceBlock(1);
  CHECK_NEAR(a.value(id), b.value(id), 1e-5);
}

NEKTA_TEST(zeroSmoothingIsImmediate) {
  ParamRegistry r = makeRegistry();
  const ParamId rate = r.find("roll.rate");
  r.setTarget(rate, 32.0f);
  r.advanceBlock(1);
  CHECK_NEAR(r.value(rate), 32.0f, 1e-6);
}

NEKTA_TEST(snapAppliesImmediatelyForSampleAccurateEvents) {
  ParamRegistry r = makeRegistry();
  const ParamId cutoff = r.find("fx.1.cutoff");
  r.setTarget(cutoff, 5000.0f);
  r.snap(cutoff);
  CHECK_NEAR(r.value(cutoff), 5000.0f, 1e-6);
}

NEKTA_TEST(normalisedRoundTrip) {
  ParamRegistry r = makeRegistry();
  const ParamId cutoff = r.find("fx.1.cutoff");
  r.setNormalised(cutoff, 0.25f);
  r.snap(cutoff);
  CHECK_NEAR(r.value(cutoff), 20.0f + 0.25f * (20000.0f - 20.0f), 1e-3);
  CHECK_NEAR(r.normalised(cutoff), 0.25f, 1e-6);
}

NEKTA_TEST(macroCurvesMapAsSpecified) {
  // SPEC 2.6: XY destinations map through lin / exp / step curves.
  CHECK_NEAR(mapCurve(MapCurve::Linear, 0.5f, 0.0f, 100.0f), 50.0f, 1e-4);
  CHECK_NEAR(mapCurve(MapCurve::Exponential, 0.0f, 200.0f, 12000.0f), 200.0f, 1e-2);
  CHECK_NEAR(mapCurve(MapCurve::Exponential, 1.0f, 200.0f, 12000.0f), 12000.0f, 1e-2);
  // Exponential is geometric: halfway is the geometric mean, not the average.
  CHECK_NEAR(mapCurve(MapCurve::Exponential, 0.5f, 200.0f, 12000.0f),
             std::sqrt(200.0f * 12000.0f), 1.0);
  // Step quantises to integers in the target range.
  CHECK_NEAR(mapCurve(MapCurve::Step, 0.4f, 8.0f, 32.0f), 18.0f, 1e-4);
  CHECK_NEAR(mapCurve(MapCurve::Step, 1.0f, 8.0f, 32.0f), 32.0f, 1e-4);
  // Out-of-range input is clamped, not extrapolated.
  CHECK_NEAR(mapCurve(MapCurve::Linear, 2.0f, 0.0f, 10.0f), 10.0f, 1e-6);
  CHECK_NEAR(mapCurve(MapCurve::Linear, -1.0f, 0.0f, 10.0f), 0.0f, 1e-6);
}

NEKTA_TEST(audioThreadPathAllocatesNothing) {
  ParamRegistry r = makeRegistry();
  const ParamId cutoff = r.find("fx.1.cutoff");
  const ParamId res = r.find("fx.1.res");
  rt::resetViolations();
  {
    rt::ScopedAudioThread audio;
    for (int block = 0; block < 64; ++block) {
      r.setTarget(cutoff, 400.0f + static_cast<float>(block));
      r.setTarget(res, 0.5f);
      r.advanceBlock(128);
      volatile float sink = r.value(cutoff) + r.value(res);
      (void)sink;
    }
  }
  CHECK(rt::checksEnabled());
  CHECK_EQ_INT(rt::allocationViolations(), 0);
}

NEKTA_TEST(allocationGuardActuallyFires) {
  // Guards the guard: if operator new were not hooked, every "no allocation"
  // assertion in this suite would pass vacuously.
  REQUIRE(rt::checksEnabled());
  rt::resetViolations();
  volatile int wanted = 32;
  {
    rt::ScopedAudioThread audio;
    std::vector<float> offending;
    offending.resize(static_cast<std::size_t>(wanted));
    offending[0] = 1.0f;
    CHECK(offending.size() == 32u);
  }
  CHECK(rt::allocationViolations() >= 1);
  rt::resetViolations();
}

NEKTA_TEST_MAIN("params")
