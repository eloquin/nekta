#include "voice/SampleBuffer.h"

#include <cmath>

namespace nekta {

void SampleBuffer::setSize(int channels, int frames) {
  channels_ = channels < 1 ? 1 : channels;
  frames_ = frames < 0 ? 0 : frames;
  data_.assign(static_cast<std::size_t>(channels_) * static_cast<std::size_t>(frames_), 0.0f);
}

float SampleBuffer::readInterpolated(int channel, double position) const noexcept {
  if (frames_ == 0) return 0.0f;
  const float* d = data(channel);
  const double floored = std::floor(position);
  const long long i = static_cast<long long>(floored);
  const float t = static_cast<float>(position - floored);
  return hermite4(at(d, i - 1), at(d, i), at(d, i + 1), at(d, i + 2), t);
}

float SampleBuffer::peak() const noexcept {
  float p = 0.0f;
  for (float v : data_) {
    const float a = std::fabs(v);
    if (a > p) p = a;
  }
  return p;
}

void SampleBuffer::normalise(float targetPeak) noexcept {
  const float p = peak();
  if (p <= 0.0f) return;
  const float g = targetPeak / p;
  for (float& v : data_) v *= g;
}

}  // namespace nekta
