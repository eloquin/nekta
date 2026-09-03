#pragma once

namespace nekta {

// SPEC §2.3: AHDSR amp envelope. Linear segments - predictable for the short
// percussive shapes a break sampler lives on, and cheap per sample.
struct AhdsrParams {
  float attackMs = 1.0f;
  float holdMs = 0.0f;
  float decayMs = 50.0f;
  float sustain = 1.0f;  // 0..1
  float releaseMs = 20.0f;
};

class AhdsrEnvelope {
 public:
  enum class Stage { Idle, Attack, Hold, Decay, Sustain, Release };

  void prepare(double sampleRate) noexcept;
  void trigger(const AhdsrParams& params) noexcept;
  void release() noexcept;
  void reset() noexcept;

  float nextSample() noexcept;
  bool active() const noexcept { return stage_ != Stage::Idle; }
  Stage stage() const noexcept { return stage_; }
  float value() const noexcept { return value_; }

 private:
  int framesFor(float ms) const noexcept;

  double sampleRate_ = 48000.0;
  Stage stage_ = Stage::Idle;
  AhdsrParams params_{};
  float value_ = 0.0f;
  float increment_ = 0.0f;
  int remaining_ = 0;
};

}  // namespace nekta
