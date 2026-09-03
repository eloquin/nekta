#pragma once

namespace nekta {

enum class RateMod { Straight, Triplet, Dotted };

// A musical division: 4 = quarter note, 16 = sixteenth, 1 = whole note (one bar
// in 4/4). SPEC §2.4A: 1/4 ... 1/64 with triplet and dotted variants.
struct Rate {
  int division = 16;
  RateMod mod = RateMod::Straight;

  constexpr Rate() = default;
  constexpr Rate(int div, RateMod m = RateMod::Straight) : division(div), mod(m) {}

  // Length of one repeat in beats (quarter notes).
  double beats() const noexcept {
    const int d = division < 1 ? 1 : division;
    double b = 4.0 / static_cast<double>(d);
    if (mod == RateMod::Triplet) b *= 2.0 / 3.0;
    if (mod == RateMod::Dotted) b *= 1.5;
    return b;
  }

  // Repeats per beat - the domain the roll engine ramps in.
  double repeatsPerBeat() const noexcept { return 1.0 / beats(); }

  bool operator==(const Rate& o) const noexcept {
    return division == o.division && mod == o.mod;
  }
  bool operator!=(const Rate& o) const noexcept { return !(*this == o); }
};

}  // namespace nekta
