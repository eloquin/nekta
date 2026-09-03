#include "roll/RollEngine.h"

#include <cmath>

namespace nekta {
namespace {
constexpr double kRateEpsilon = 1.0e-9;
}  // namespace

void RollEngine::prepare(double sampleRate) noexcept {
  sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
  reset();
}

void RollEngine::reset() noexcept {
  running_ = false;
  startPpq_ = 0.0;
  startSample_ = 0;
  nextRepeat_ = 0;
}

void RollEngine::start(const Transport& transport, SampleIndex startSample) noexcept {
  startSample_ = startSample;
  startPpq_ = transport.ppqAtSample(startSample);
  nextRepeat_ = 0;
  running_ = true;
}

double RollEngine::rateAtBeats(double beats) const noexcept {
  const double r0 = config_.rate.repeatsPerBeat();
  if (!config_.ramp.enabled || config_.ramp.lengthBeats <= 0.0) return r0;
  const double r1 = config_.ramp.endRate.repeatsPerBeat();
  const double len = config_.ramp.lengthBeats;
  const double t = beats >= len ? 1.0 : (beats <= 0.0 ? 0.0 : beats / len);
  if (config_.ramp.curve == RampCurve::Exponential && r0 > 0.0 && r1 > 0.0) {
    return r0 * std::pow(r1 / r0, t);
  }
  return r0 + (r1 - r0) * t;
}

// Inverts phi(b) = n. phi is the integral of rateAtBeats, i.e. how many repeats
// have elapsed after b beats.
double RollEngine::beatsForRepeat(long long n) const noexcept {
  const double count = static_cast<double>(n);
  const double r0 = config_.rate.repeatsPerBeat();
  if (r0 <= 0.0) return 0.0;

  const double r1 = config_.ramp.endRate.repeatsPerBeat();
  const double len = config_.ramp.lengthBeats;
  const bool ramped = config_.ramp.enabled && len > 0.0 && r1 > 0.0 &&
                      std::fabs(r1 - r0) > kRateEpsilon;
  if (!ramped) return count / r0;

  if (config_.ramp.curve == RampCurve::Exponential) {
    const double k = r1 / r0;
    const double lnk = std::log(k);
    const double c = r0 * len / lnk;          // phi(b) = c * (k^(b/len) - 1)
    const double phiEnd = c * (k - 1.0);      // repeats elapsed at the end of the ramp
    if (count <= phiEnd) return len * std::log1p(count / c) / lnk;
    return len + (count - phiEnd) / r1;
  }

  // Linear: phi(b) = r0*b + a*b^2, a = (r1 - r0) / (2*len).
  const double a = (r1 - r0) / (2.0 * len);
  const double phiEnd = (r0 + r1) * len * 0.5;
  if (count <= phiEnd) {
    const double disc = r0 * r0 + 4.0 * a * count;
    const double root = std::sqrt(disc > 0.0 ? disc : 0.0);
    // Numerically stable form of (-r0 + sqrt(disc)) / (2a); also correct as a->0.
    return 2.0 * count / (r0 + root);
  }
  return len + (count - phiEnd) / r1;
}

RollEvent RollEngine::makeEvent(long long n, FrameCount offset, SampleIndex when,
                                const Transport& transport) const noexcept {
  const RollModifiers& m = config_.mods;
  RollEvent e;
  e.repeatIndex = n;
  e.offsetInBlock = offset;
  e.sampleTime = when;
  e.padIndex = config_.padIndex;

  float gain = config_.baseGain;
  if (m.ampDecay != 1.0f && n > 0) {
    const float shaped = (m.ampCurve == 1.0f)
                             ? static_cast<float>(n)
                             : std::pow(static_cast<float>(n), m.ampCurve);
    gain *= std::pow(m.ampDecay, shaped);
  }
  if (!std::isfinite(gain) || gain < 0.0f) gain = 0.0f;
  if (gain > 4.0f) gain = 4.0f;
  e.gain = gain;

  const long long walk = (m.pitchWrapRepeats > 0) ? (n % m.pitchWrapRepeats) : n;
  e.pitchSemitones = m.pitchDeltaSemis * static_cast<double>(walk);

  e.reverse = m.reverseAlternate && (n % 2 == 1);
  e.pan = m.panAlternate ? ((n % 2 == 1) ? m.panAmount : -m.panAmount) : 0.0f;
  e.startOffsetSeconds = m.startOffsetWalkMs * 0.001 * static_cast<double>(n);

  if (m.gate < 1.0f && m.gate > 0.0f) {
    const double intervalBeats = beatsForRepeat(n + 1) - beatsForRepeat(n);
    const double secondsPerBeat = 60.0 / transport.bpm();
    e.gateSeconds = intervalBeats * secondsPerBeat * static_cast<double>(m.gate);
  } else {
    e.gateSeconds = -1.0;
  }
  return e;
}

int RollEngine::process(const Transport& transport, SampleIndex blockStart, FrameCount frames,
                        RollEvent* out, int maxEvents) noexcept {
  if (!running_ || out == nullptr || maxEvents <= 0 || frames <= 0) return 0;

  const SampleIndex blockEnd = blockStart + frames;
  int emitted = 0;

  while (emitted < maxEvents) {
    const double beats = beatsForRepeat(nextRepeat_);
    const SampleIndex when = transport.sampleAtPpqRounded(startPpq_ + beats);
    if (when >= blockEnd) break;
    // A repeat that should already have sounded (roll started in the past, or
    // the caller's event array filled up last block) fires at the block start.
    const SampleIndex clamped = when > blockStart ? when : blockStart;
    const FrameCount offset = static_cast<FrameCount>(clamped - blockStart);
    out[emitted] = makeEvent(nextRepeat_, offset, when, transport);
    ++emitted;
    ++nextRepeat_;
  }
  return emitted;
}

}  // namespace nekta
