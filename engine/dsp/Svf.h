#pragma once

#include "core/Types.h"

namespace nekta {

// Zavalishin TPT (topology-preserving transform) state variable filter.
// Chosen because it stays stable when the cutoff is modulated at audio rate,
// which the XY pad and the roll engine both do (SPEC §2.5).
//
// Cutoff, Q, morph and drive are ramped linearly across each block, so a big
// parameter jump never clicks (SPEC §1.2).
class Svf {
 public:
  enum class Mode { Lowpass = 0, Bandpass = 1, Highpass = 2, Notch = 3 };

  void prepare(double sampleRate) noexcept;
  void reset() noexcept;

  void setCutoff(float hz) noexcept;           // ramped over the next block
  void setCutoffImmediate(float hz) noexcept;  // snaps (preset load)
  void setQ(float q) noexcept;
  void setResonance(float r01) noexcept;  // 0..1 -> Q 0.5 .. 20
  void setDrive(float drive) noexcept;    // 1 = clean
  void setMode(Mode mode) noexcept { setMorph(static_cast<float>(mode)); }
  void setMorph(float morph) noexcept;  // 0=LP 1=BP 2=HP 3=Notch, continuous
  void setBypass(bool bypass) noexcept { bypass_ = bypass; }
  bool bypassed() const noexcept { return bypass_; }

  // In-place stereo processing.
  void processBlock(float* left, float* right, FrameCount frames) noexcept;

  float cutoff() const noexcept { return targetCutoff_; }
  float q() const noexcept { return targetQ_; }

 private:
  struct State {
    float ic1eq = 0.0f;
    float ic2eq = 0.0f;
  };

  float gFor(float cutoffHz) const noexcept;
  float tick(State& s, float x, float g, float k, float a1, float a2, float a3,
             float morph) const noexcept;

  double sampleRate_ = 48000.0;
  float maxCutoff_ = 21600.0f;

  float targetCutoff_ = 1000.0f;
  float currentCutoff_ = 1000.0f;
  float targetQ_ = 0.70710678f;
  float currentQ_ = 0.70710678f;
  float targetMorph_ = 0.0f;
  float currentMorph_ = 0.0f;
  float targetDrive_ = 1.0f;
  float currentDrive_ = 1.0f;
  float driveNorm_ = 1.0f;
  bool bypass_ = false;

  State left_;
  State right_;
};

}  // namespace nekta
