#include "voice/Envelope.h"

#include <cmath>

namespace nekta {

void AhdsrEnvelope::prepare(double sampleRate) noexcept {
  sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
  reset();
}

int AhdsrEnvelope::framesFor(float ms) const noexcept {
  if (ms <= 0.0f) return 0;
  const int n = static_cast<int>(std::lround(static_cast<double>(ms) * 0.001 * sampleRate_));
  return n < 1 ? 1 : n;
}

void AhdsrEnvelope::reset() noexcept {
  stage_ = Stage::Idle;
  value_ = 0.0f;
  increment_ = 0.0f;
  remaining_ = 0;
}

void AhdsrEnvelope::trigger(const AhdsrParams& params) noexcept {
  params_ = params;
  value_ = 0.0f;
  const int attack = framesFor(params_.attackMs);
  if (attack > 0) {
    stage_ = Stage::Attack;
    remaining_ = attack;
    increment_ = 1.0f / static_cast<float>(attack);
  } else {
    value_ = 1.0f;
    stage_ = Stage::Hold;
    remaining_ = framesFor(params_.holdMs);
    increment_ = 0.0f;
  }
}

void AhdsrEnvelope::release() noexcept {
  if (stage_ == Stage::Idle || stage_ == Stage::Release) return;
  const int frames = framesFor(params_.releaseMs);
  stage_ = Stage::Release;
  remaining_ = frames;
  increment_ = frames > 0 ? -value_ / static_cast<float>(frames) : -value_;
  if (frames == 0) {
    value_ = 0.0f;
    stage_ = Stage::Idle;
  }
}

float AhdsrEnvelope::nextSample() noexcept {
  switch (stage_) {
    case Stage::Idle:
      return 0.0f;

    case Stage::Attack:
      value_ += increment_;
      if (--remaining_ <= 0) {
        value_ = 1.0f;
        stage_ = Stage::Hold;
        remaining_ = framesFor(params_.holdMs);
        increment_ = 0.0f;
      }
      break;

    case Stage::Hold:
      if (--remaining_ <= 0) {
        const int decay = framesFor(params_.decayMs);
        stage_ = Stage::Decay;
        remaining_ = decay;
        increment_ = decay > 0 ? (params_.sustain - value_) / static_cast<float>(decay) : 0.0f;
        if (decay == 0) {
          value_ = params_.sustain;
          stage_ = params_.sustain > 0.0f ? Stage::Sustain : Stage::Idle;
        }
      }
      break;

    case Stage::Decay:
      value_ += increment_;
      if (--remaining_ <= 0) {
        value_ = params_.sustain;
        stage_ = params_.sustain > 0.0f ? Stage::Sustain : Stage::Idle;
        increment_ = 0.0f;
      }
      break;

    case Stage::Sustain:
      break;

    case Stage::Release:
      value_ += increment_;
      if (--remaining_ <= 0 || value_ <= 0.0f) {
        value_ = 0.0f;
        stage_ = Stage::Idle;
        increment_ = 0.0f;
      }
      break;
  }
  if (value_ < 0.0f) value_ = 0.0f;
  if (value_ > 1.0f) value_ = 1.0f;
  return value_;
}

}  // namespace nekta
