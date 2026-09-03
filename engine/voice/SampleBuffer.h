#pragma once

#include <vector>

#include "core/Types.h"
#include "voice/Interpolation.h"

namespace nekta {

// Owns decoded audio. Planar so a voice can read one channel without striding.
// All mutators are setup-thread only; readInterpolated() is real-time safe.
class SampleBuffer {
 public:
  void setSize(int channels, int frames);
  void setSampleRate(double sr) noexcept { sampleRate_ = sr; }

  int channels() const noexcept { return channels_; }
  int frames() const noexcept { return frames_; }
  double sampleRate() const noexcept { return sampleRate_; }
  bool empty() const noexcept { return frames_ == 0 || channels_ == 0; }

  float* writePtr(int channel) noexcept { return data_.data() + channelOffset(channel); }
  const float* data(int channel) const noexcept { return data_.data() + channelOffset(channel); }

  // Hermite read at a fractional frame position. Positions outside the buffer
  // clamp to the edge sample - never reads out of bounds.
  float readInterpolated(int channel, double position) const noexcept;

  float peak() const noexcept;
  void normalise(float targetPeak) noexcept;

 private:
  std::size_t channelOffset(int channel) const noexcept {
    if (channel >= channels_) channel = channels_ > 0 ? channels_ - 1 : 0;
    if (channel < 0) channel = 0;
    return static_cast<std::size_t>(channel) * static_cast<std::size_t>(frames_);
  }
  float at(const float* d, long long index) const noexcept {
    if (index < 0) index = 0;
    if (index >= frames_) index = frames_ - 1;
    return d[index];
  }

  std::vector<float> data_;
  int channels_ = 0;
  int frames_ = 0;
  double sampleRate_ = 48000.0;
};

}  // namespace nekta
