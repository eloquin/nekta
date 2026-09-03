#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "core/Types.h"

namespace nekta {

using ParamId = std::int32_t;
inline constexpr ParamId kInvalidParam = -1;

struct ParamDesc {
  float minValue = 0.0f;
  float maxValue = 1.0f;
  float defaultValue = 0.0f;
  // One-pole time constant in milliseconds (SPEC §1.2: 10-20 ms). 0 = stepped,
  // for parameters that must jump (roll rate, trigger mode, filter type).
  float smoothingMs = 15.0f;
};

// String-path addressable parameter store with per-parameter one-pole smoothing.
//
// Threading: add()/find()/prepare() are setup-thread only and are the only
// places that allocate. setTarget()/value()/advanceBlock() are lock-free and
// allocation-free, so both the UI and the audio thread can call them.
class ParamRegistry {
 public:
  explicit ParamRegistry(int capacity = 512);

  ParamRegistry(ParamRegistry&&) noexcept = default;
  ParamRegistry& operator=(ParamRegistry&&) noexcept = default;

  // --- setup thread -------------------------------------------------------
  // Registers a parameter. Returns the existing id if the path is already
  // registered, kInvalidParam if the registry is full.
  ParamId add(const char* path, const ParamDesc& desc);
  ParamId find(const char* path) const;
  const char* path(ParamId id) const;
  const ParamDesc& desc(ParamId id) const;
  int size() const noexcept { return count_; }
  int capacity() const noexcept { return capacity_; }

  // Computes smoothing coefficients and snaps every value to its default.
  void prepare(double sampleRate, FrameCount nominalBlock);
  // Snaps every current value to its target (use after loading a preset).
  void snapAll() noexcept;

  // --- any thread (lock-free) ---------------------------------------------
  void setTarget(ParamId id, float value) noexcept;
  void setNormalised(ParamId id, float t01) noexcept;
  float target(ParamId id) const noexcept;

  // --- audio thread -------------------------------------------------------
  // Advances every smoother by n frames. Exact: equivalent to n single-sample
  // steps, using a cached per-block coefficient.
  void advanceBlock(FrameCount n) noexcept;
  float value(ParamId id) const noexcept { return values_[static_cast<std::size_t>(id)]; }
  float normalised(ParamId id) const noexcept;
  // Applies the target immediately - for sample-accurate event boundaries.
  void snap(ParamId id) noexcept;
  bool valid(ParamId id) const noexcept { return id >= 0 && id < count_; }

 private:
  void recomputeBlockCoeffs(FrameCount n) noexcept;

  int capacity_ = 0;
  int count_ = 0;
  double sampleRate_ = 48000.0;
  FrameCount cachedBlock_ = 0;

  std::vector<std::string> paths_;   // setup thread only
  std::vector<ParamDesc> descs_;     // setup thread only after prepare()
  std::unique_ptr<std::atomic<float>[]> targets_;
  std::unique_ptr<float[]> values_;
  std::unique_ptr<float[]> perSampleCoeff_;
  std::unique_ptr<float[]> blockCoeff_;
};

}  // namespace nekta
