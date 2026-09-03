#pragma once

#include <cstdint>

#include "core/Types.h"
#include "voice/Envelope.h"
#include "voice/SampleBuffer.h"

namespace nekta {

// SPEC §2.3
enum class TriggerMode { OneShot, Gate, Loop, Toggle };

// Anti-click fade applied on start, stop, choke and end-of-sample.
inline constexpr float kAntiClickMs = 2.0f;

struct VoiceSpec {
  const SampleBuffer* sample = nullptr;
  double startFrame = 0.0;
  double endFrame = -1.0;  // < 0 means "to the end of the sample"
  bool loop = false;
  bool reverse = false;
  double pitchSemitones = 0.0;  // varispeed: repitch, never time-stretch
  float gain = 1.0f;
  float pan = 0.0f;  // -1 left .. +1 right, equal power
  AhdsrParams env{};
  TriggerMode mode = TriggerMode::OneShot;
  int chokeGroup = 0;  // 0 = none
  int padIndex = 0;
  // > 0 auto-releases after this long (roll gate/duty, SPEC §2.4A).
  double gateSeconds = -1.0;
};

// One sampler voice. process() adds into the output; it never allocates, locks
// or throws.
class SamplerVoice {
 public:
  void prepare(double sampleRate) noexcept;
  void reset() noexcept;

  // delayFrames places the start inside the block for sample-accurate triggering.
  void start(const VoiceSpec& spec, FrameCount delayFrames = 0) noexcept;
  void release() noexcept;  // enter the envelope release stage
  void choke() noexcept;    // anti-click fade to silence, then idle

  void process(float* left, float* right, FrameCount frames) noexcept;

  bool active() const noexcept { return active_; }
  bool choking() const noexcept { return active_ && fadeStep_ < 0.0f; }
  int padIndex() const noexcept { return padIndex_; }
  int chokeGroup() const noexcept { return chokeGroup_; }
  std::uint64_t startOrder() const noexcept { return startOrder_; }
  void setStartOrder(std::uint64_t order) noexcept { startOrder_ = order; }

 private:
  void beginFadeOut() noexcept;
  void finish() noexcept;

  double sampleRate_ = 48000.0;
  int fadeFrames_ = 96;   // nominal, from kAntiClickMs
  int voiceFade_ = 96;    // this voice's fade, capped at half its playable length

  const SampleBuffer* sample_ = nullptr;
  double position_ = 0.0;
  double startFrame_ = 0.0;
  double endFrame_ = 0.0;
  double increment_ = 1.0;
  bool reverse_ = false;
  bool loop_ = false;
  bool active_ = false;

  float gain_ = 1.0f;
  float gainLeft_ = 0.70710678f;
  float gainRight_ = 0.70710678f;

  float fadeGain_ = 0.0f;
  float fadeStep_ = 0.0f;

  AhdsrEnvelope env_;
  FrameCount delayFrames_ = 0;
  long long gateFrames_ = -1;
  int chokeGroup_ = 0;
  int padIndex_ = 0;
  std::uint64_t startOrder_ = 0;
};

}  // namespace nekta
