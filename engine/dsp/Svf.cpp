#include "dsp/Svf.h"

#include <cmath>

#include "core/Denormal.h"

namespace nekta {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr float kMinCutoff = 10.0f;
constexpr float kMinQ = 0.3f;
constexpr float kMaxQ = 40.0f;
constexpr float kMaxDrive = 24.0f;

float clampf(float v, float lo, float hi) noexcept {
  return v < lo ? lo : (v > hi ? hi : v);
}
}  // namespace

void Svf::prepare(double sampleRate) noexcept {
  sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
  maxCutoff_ = static_cast<float>(sampleRate_ * 0.45);
  reset();
}

void Svf::reset() noexcept {
  left_ = State{};
  right_ = State{};
  currentCutoff_ = targetCutoff_;
  currentQ_ = targetQ_;
  currentMorph_ = targetMorph_;
  currentDrive_ = targetDrive_;
}

void Svf::setCutoff(float hz) noexcept {
  if (!std::isfinite(hz)) return;
  targetCutoff_ = clampf(hz, kMinCutoff, maxCutoff_);
}

void Svf::setCutoffImmediate(float hz) noexcept {
  setCutoff(hz);
  currentCutoff_ = targetCutoff_;
}

void Svf::setQ(float q) noexcept {
  if (!std::isfinite(q)) return;
  targetQ_ = clampf(q, kMinQ, kMaxQ);
}

void Svf::setResonance(float r01) noexcept {
  const float t = clampf(r01, 0.0f, 1.0f);
  setQ(0.5f * std::pow(40.0f, t));
}

void Svf::setDrive(float drive) noexcept {
  if (!std::isfinite(drive)) return;
  targetDrive_ = clampf(drive, 1.0f, kMaxDrive);
}

void Svf::setMorph(float morph) noexcept {
  if (!std::isfinite(morph)) return;
  targetMorph_ = clampf(morph, 0.0f, 3.0f);
}

float Svf::gFor(float cutoffHz) const noexcept {
  const double wd = kPi * static_cast<double>(cutoffHz) / sampleRate_;
  return static_cast<float>(std::tan(wd));
}

// One TPT step. Returns the morphed output; morph blends LP -> BP -> HP -> Notch.
float Svf::tick(State& s, float x, float g, float k, float a1, float a2, float a3,
                float morph) const noexcept {
  const float v3 = x - s.ic2eq;
  const float v1 = a1 * s.ic1eq + a2 * v3;
  const float v2 = s.ic2eq + a2 * s.ic1eq + a3 * v3;
  s.ic1eq = sanitise(2.0f * v1 - s.ic1eq);
  s.ic2eq = sanitise(2.0f * v2 - s.ic2eq);
  (void)g;

  const float lp = v2;
  const float bp = v1;
  const float hp = x - k * v1 - v2;
  const float notch = hp + lp;

  const float outs[4] = {lp, bp, hp, notch};
  int idx = static_cast<int>(morph);
  if (idx > 2) idx = 2;
  const float frac = morph - static_cast<float>(idx);
  return outs[idx] + (outs[idx + 1] - outs[idx]) * frac;
}

void Svf::processBlock(float* left, float* right, FrameCount frames) noexcept {
  if (frames <= 0) return;
  if (bypass_) {
    // Null path must be bit-identical (SPEC §8), so touch nothing.
    currentCutoff_ = targetCutoff_;
    currentQ_ = targetQ_;
    currentMorph_ = targetMorph_;
    currentDrive_ = targetDrive_;
    return;
  }

  const float gStart = gFor(currentCutoff_);
  const float gEnd = gFor(targetCutoff_);
  const float kStart = 1.0f / currentQ_;
  const float kEnd = 1.0f / targetQ_;
  const float mStart = currentMorph_;
  const float mEnd = targetMorph_;
  const float dStart = currentDrive_;
  const float dEnd = targetDrive_;
  const float inc = 1.0f / static_cast<float>(frames);

  for (FrameCount i = 0; i < frames; ++i) {
    const float t = static_cast<float>(i + 1) * inc;
    const float g = gStart + (gEnd - gStart) * t;
    const float k = kStart + (kEnd - kStart) * t;
    const float morph = mStart + (mEnd - mStart) * t;
    const float drive = dStart + (dEnd - dStart) * t;

    const float a1 = 1.0f / (1.0f + g * (g + k));
    const float a2 = g * a1;
    const float a3 = g * a2;

    float xl = left[i];
    float xr = right[i];
    if (drive > 1.0001f) {
      const float norm = 1.0f / std::tanh(drive);
      xl = std::tanh(xl * drive) * norm;
      xr = std::tanh(xr * drive) * norm;
    }

    left[i] = tick(left_, xl, g, k, a1, a2, a3, morph);
    right[i] = tick(right_, xr, g, k, a1, a2, a3, morph);
  }

  currentCutoff_ = targetCutoff_;
  currentQ_ = targetQ_;
  currentMorph_ = targetMorph_;
  currentDrive_ = targetDrive_;
  driveNorm_ = 1.0f / std::tanh(currentDrive_);
}

}  // namespace nekta
