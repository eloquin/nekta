#include "voice/VoicePool.h"

namespace nekta {

void VoicePool::prepare(double sampleRate) noexcept {
  for (SamplerVoice& v : voices_) v.prepare(sampleRate);
  order_ = 0;
}

void VoicePool::reset() noexcept {
  for (SamplerVoice& v : voices_) v.reset();
  order_ = 0;
}

SamplerVoice* VoicePool::trigger(const VoiceSpec& spec, FrameCount delayFrames) noexcept {
  if (spec.sample == nullptr || spec.sample->empty()) return nullptr;

  if (spec.chokeGroup > 0) {
    for (SamplerVoice& v : voices_) {
      if (v.active() && v.chokeGroup() == spec.chokeGroup) v.choke();
    }
  }

  SamplerVoice* chosen = nullptr;
  for (SamplerVoice& v : voices_) {
    if (!v.active()) {
      chosen = &v;
      break;
    }
  }
  if (chosen == nullptr) {
    // Steal the oldest. Its replacement fades in over 2 ms, which keeps the
    // steal from clicking.
    chosen = &voices_[0];
    for (SamplerVoice& v : voices_) {
      if (v.startOrder() < chosen->startOrder()) chosen = &v;
    }
  }

  chosen->start(spec, delayFrames);
  chosen->setStartOrder(++order_);
  return chosen;
}

void VoicePool::releasePad(int padIndex) noexcept {
  for (SamplerVoice& v : voices_) {
    if (v.active() && v.padIndex() == padIndex) v.release();
  }
}

void VoicePool::chokeGroup(int group) noexcept {
  for (SamplerVoice& v : voices_) {
    if (v.active() && v.chokeGroup() == group) v.choke();
  }
}

void VoicePool::chokeAll() noexcept {
  for (SamplerVoice& v : voices_) v.choke();
}

void VoicePool::process(float* left, float* right, FrameCount frames) noexcept {
  for (SamplerVoice& v : voices_) {
    if (v.active()) v.process(left, right, frames);
  }
}

int VoicePool::activeVoices() const noexcept {
  int n = 0;
  for (const SamplerVoice& v : voices_) {
    if (v.active()) ++n;
  }
  return n;
}

}  // namespace nekta
