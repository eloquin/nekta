#include "voice/SamplerVoice.h"

#include <cmath>

namespace nekta {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMaxPitchRatio = 32.0;
}  // namespace

void SamplerVoice::prepare(double sampleRate) noexcept {
  sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
  fadeFrames_ = static_cast<int>(std::lround(kAntiClickMs * 0.001f * sampleRate_));
  if (fadeFrames_ < 1) fadeFrames_ = 1;
  env_.prepare(sampleRate_);
  reset();
}

void SamplerVoice::reset() noexcept {
  active_ = false;
  sample_ = nullptr;
  position_ = 0.0;
  fadeGain_ = 0.0f;
  fadeStep_ = 0.0f;
  delayFrames_ = 0;
  gateFrames_ = -1;
  env_.reset();
}

void SamplerVoice::start(const VoiceSpec& spec, FrameCount delayFrames) noexcept {
  sample_ = spec.sample;
  if (sample_ == nullptr || sample_->empty()) {
    active_ = false;
    return;
  }

  const double lastFrame = static_cast<double>(sample_->frames() - 1);
  startFrame_ = spec.startFrame < 0.0 ? 0.0 : spec.startFrame;
  if (startFrame_ > lastFrame) startFrame_ = lastFrame;
  endFrame_ = (spec.endFrame < 0.0 || spec.endFrame > lastFrame) ? lastFrame : spec.endFrame;
  if (endFrame_ < startFrame_) endFrame_ = startFrame_;

  reverse_ = spec.reverse;
  loop_ = spec.loop || spec.mode == TriggerMode::Loop;
  position_ = reverse_ ? endFrame_ : startFrame_;

  double ratio = std::pow(2.0, spec.pitchSemitones / 12.0);
  ratio *= sample_->sampleRate() / sampleRate_;
  if (!(ratio > 0.0) || !std::isfinite(ratio)) ratio = 1.0;
  if (ratio > kMaxPitchRatio) ratio = kMaxPitchRatio;
  increment_ = ratio;

  gain_ = spec.gain;
  const double pan = spec.pan < -1.0f ? -1.0 : (spec.pan > 1.0f ? 1.0 : spec.pan);
  const double angle = (pan + 1.0) * kPi * 0.25;  // equal power
  gainLeft_ = static_cast<float>(std::cos(angle));
  gainRight_ = static_cast<float>(std::sin(angle));

  // A slice shorter than two fades would be swallowed by them, so scale the
  // fade to the voice: never more than half its playable length.
  const double playable = (endFrame_ - startFrame_) / increment_;
  const int halfPlayable = static_cast<int>(playable * 0.5);
  voiceFade_ = fadeFrames_;
  if (halfPlayable < voiceFade_) voiceFade_ = halfPlayable;
  if (voiceFade_ < 1) voiceFade_ = 1;

  env_.trigger(spec.env);
  fadeGain_ = 0.0f;
  fadeStep_ = 1.0f / static_cast<float>(voiceFade_);
  delayFrames_ = delayFrames > 0 ? delayFrames : 0;
  gateFrames_ = spec.gateSeconds > 0.0
                    ? static_cast<long long>(std::llround(spec.gateSeconds * sampleRate_))
                    : -1;
  chokeGroup_ = spec.chokeGroup;
  padIndex_ = spec.padIndex;
  active_ = true;
}

void SamplerVoice::release() noexcept {
  if (active_) env_.release();
}

void SamplerVoice::beginFadeOut() noexcept {
  if (fadeStep_ < 0.0f) return;  // already fading out
  fadeStep_ = -fadeGain_ / static_cast<float>(voiceFade_);
  if (fadeStep_ >= 0.0f) fadeStep_ = -1.0f / static_cast<float>(voiceFade_);
}

void SamplerVoice::choke() noexcept {
  if (!active_) return;
  beginFadeOut();
}

void SamplerVoice::finish() noexcept {
  active_ = false;
  sample_ = nullptr;
  fadeGain_ = 0.0f;
  fadeStep_ = 0.0f;
  env_.reset();
}

void SamplerVoice::process(float* left, float* right, FrameCount frames) noexcept {
  if (!active_ || sample_ == nullptr) return;

  const double span = endFrame_ - startFrame_;
  const double fadeOutDistance = static_cast<double>(voiceFade_) * increment_;

  for (FrameCount i = 0; i < frames; ++i) {
    if (delayFrames_ > 0) {
      --delayFrames_;
      continue;
    }

    // Start the end-of-sample fade early so the voice stops exactly at its end
    // point instead of running past it.
    if (!loop_ && fadeStep_ >= 0.0f) {
      const double remaining = reverse_ ? (position_ - startFrame_) : (endFrame_ - position_);
      if (remaining <= fadeOutDistance) beginFadeOut();
    }

    const float s = sample_->readInterpolated(0, position_);
    const float e = env_.nextSample();

    fadeGain_ += fadeStep_;
    if (fadeGain_ > 1.0f) {
      fadeGain_ = 1.0f;
      fadeStep_ = 0.0f;
    } else if (fadeGain_ < 0.0f) {
      fadeGain_ = 0.0f;
    }

    const float out = s * e * fadeGain_ * gain_;
    left[i] += out * gainLeft_;
    right[i] += out * gainRight_;

    if (fadeStep_ < 0.0f && fadeGain_ <= 0.0f) {
      finish();
      return;
    }

    position_ += reverse_ ? -increment_ : increment_;

    if (loop_ && span > 0.0) {
      if (!reverse_ && position_ > endFrame_) position_ -= span;
      if (reverse_ && position_ < startFrame_) position_ += span;
    } else if ((!reverse_ && position_ > endFrame_) || (reverse_ && position_ < startFrame_)) {
      finish();
      return;
    }

    if (gateFrames_ > 0 && --gateFrames_ == 0) {
      gateFrames_ = -1;
      env_.release();
    }

    if (!env_.active()) {
      finish();
      return;
    }
  }
}

}  // namespace nekta
