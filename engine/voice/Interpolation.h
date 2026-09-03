#pragma once

namespace nekta {

// 4-point, 3rd-order Hermite interpolation (SPEC §2.3). Interpolates x0..x1 for
// t in [0,1) using the two neighbouring points for slope estimation. Cheap
// enough for 16 voices of varispeed on a phone and clean enough for breaks.
inline float hermite4(float xm1, float x0, float x1, float x2, float t) noexcept {
  const float c = (x1 - xm1) * 0.5f;
  const float v = x0 - x1;
  const float w = c + v;
  const float a = w + v + (x2 - x0) * 0.5f;
  const float b = w + a;
  return ((a * t - b) * t + c) * t + x0;
}

}  // namespace nekta
