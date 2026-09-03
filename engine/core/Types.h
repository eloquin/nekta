#pragma once

#include <cstdint>

namespace nekta {

// Absolute position on the transport timeline, in samples. Never wraps in any
// realistic session length; always signed so deltas are well behaved.
using SampleIndex = std::int64_t;

// Frame counts inside one process() call.
using FrameCount = std::int32_t;

inline constexpr FrameCount kMaxBlockFrames = 2048;
inline constexpr int kNumPads = 16;
inline constexpr int kNumVoices = 16;
inline constexpr int kMaxChokeGroups = 9;  // 0 = none, 1..8

// Stereo planar block. Non-owning: the host owns the memory.
struct StereoBlock {
  float* left = nullptr;
  float* right = nullptr;
  FrameCount frames = 0;
};

}  // namespace nekta
