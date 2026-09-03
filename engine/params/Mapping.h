#pragma once

#include <cmath>

namespace nekta {

// Curve used when a macro (XY axis, envelope, automation) drives a parameter.
// SPEC §2.6: { target, min, max, curve }.
enum class MapCurve { Linear, Exponential, Step };

inline float clamp01(float t) noexcept { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }

// Maps a normalised 0..1 macro position onto [minValue, maxValue].
// Exponential is geometric (musical for frequencies and times) and falls back to
// linear if either endpoint is non-positive. Step quantises to integer values,
// which is what roll rates and bit depths want.
inline float mapCurve(MapCurve curve, float t, float minValue, float maxValue) noexcept {
  t = clamp01(t);
  switch (curve) {
    case MapCurve::Exponential:
      if (minValue > 0.0f && maxValue > 0.0f) {
        return minValue * std::pow(maxValue / minValue, t);
      }
      break;
    case MapCurve::Step: {
      const float raw = minValue + t * (maxValue - minValue);
      return std::round(raw);
    }
    case MapCurve::Linear:
      break;
  }
  return minValue + t * (maxValue - minValue);
}

}  // namespace nekta
