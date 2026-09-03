#pragma once

#include "core/Types.h"
#include "voice/SamplerVoice.h"

namespace nekta {

// Fixed pool of voices (SPEC §2.3: 16-voice polyphony). No allocation: voices
// are constructed once and reused; when the pool is exhausted the oldest voice
// is stolen rather than dropping the hit, which is what a roll needs.
class VoicePool {
 public:
  void prepare(double sampleRate) noexcept;
  void reset() noexcept;

  // Chokes the spec's choke group, then starts a voice. Never returns nullptr
  // unless the spec has no sample.
  SamplerVoice* trigger(const VoiceSpec& spec, FrameCount delayFrames) noexcept;

  void releasePad(int padIndex) noexcept;
  void chokeGroup(int group) noexcept;
  void chokeAll() noexcept;

  void process(float* left, float* right, FrameCount frames) noexcept;

  int activeVoices() const noexcept;
  SamplerVoice& voice(int index) noexcept { return voices_[index]; }

 private:
  SamplerVoice voices_[kNumVoices];
  std::uint64_t order_ = 0;
};

}  // namespace nekta
