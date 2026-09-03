#pragma once

#include <cmath>

namespace nekta {

// Denormals in feedback paths cost hundreds of cycles per sample on some ARM
// cores. Flush anything below this to zero (SPEC §8: no denormal blowup).
inline constexpr float kDenormalFloor = 1.0e-20f;

inline float flushDenormal(float x) noexcept {
  return (std::fabs(x) < kDenormalFloor) ? 0.0f : x;
}

// Safety net for feedback state: replaces NaN/inf with zero. Costs one compare;
// worth it in reverb/delay/filter state where a single inf poisons the buffer.
inline float sanitise(float x) noexcept {
  if (!std::isfinite(x)) return 0.0f;
  return flushDenormal(x);
}

}  // namespace nekta
