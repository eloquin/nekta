#pragma once

#include "clock/Rate.h"
#include "core/Types.h"

namespace nekta {

inline constexpr double kMinBpm = 60.0;   // SPEC §2.7
inline constexpr double kMaxBpm = 200.0;

// Master clock. Musical position is derived from the sample position through a
// (sample, ppq) anchor rather than accumulated per block, so it cannot drift and
// a tempo change never makes the musical position jump.
//
// All query methods are const and allocation-free; advance() is the only
// mutator called from the audio thread.
class Transport {
 public:
  void prepare(double sampleRate) noexcept;
  void reset(double ppq = 0.0) noexcept;

  void setBpm(double bpm) noexcept;
  void setPlaying(bool playing) noexcept { playing_ = playing; }
  void setBeatsPerBar(int beats) noexcept { beatsPerBar_ = beats < 1 ? 1 : beats; }

  // Audio thread: call once per block, after processing it.
  void advance(FrameCount frames) noexcept {
    if (playing_) samplePos_ += frames;
  }

  double sampleRate() const noexcept { return sampleRate_; }
  double bpm() const noexcept { return bpm_; }
  bool playing() const noexcept { return playing_; }
  int beatsPerBar() const noexcept { return beatsPerBar_; }
  double samplesPerBeat() const noexcept { return samplesPerBeat_; }
  SampleIndex samplePosition() const noexcept { return samplePos_; }
  double ppqPosition() const noexcept { return ppqAtSample(samplePos_); }
  double barPhase() const noexcept;

  double ppqAtSample(SampleIndex sample) const noexcept {
    return anchorPpq_ + static_cast<double>(sample - anchorSample_) / samplesPerBeat_;
  }
  double sampleAtPpq(double ppq) const noexcept {
    return static_cast<double>(anchorSample_) + (ppq - anchorPpq_) * samplesPerBeat_;
  }
  SampleIndex sampleAtPpqRounded(double ppq) const noexcept;

  // Quantisation (SPEC §2.7: quantise-launch off / 1/16 / 1/8 / 1/4 / 1 bar).
  // Returns the first grid position at or after `ppq` / `from`.
  double nextGridPpq(double ppq, const Rate& rate) const noexcept;
  SampleIndex nextGridSample(SampleIndex from, const Rate& rate) const noexcept;
  SampleIndex nearestGridSample(SampleIndex near, const Rate& rate) const noexcept;

 private:
  double sampleRate_ = 48000.0;
  double bpm_ = 120.0;
  double samplesPerBeat_ = 24000.0;
  bool playing_ = false;
  int beatsPerBar_ = 4;
  SampleIndex samplePos_ = 0;
  SampleIndex anchorSample_ = 0;
  double anchorPpq_ = 0.0;
};

// Walks every grid crossing inside one block. Trivially constructed on the
// stack in the audio callback: no allocation, no virtual calls.
class GridIterator {
 public:
  GridIterator(const Transport& transport, const Rate& rate, FrameCount frames) noexcept;

  // Yields the offset of the next crossing within the block and its absolute
  // grid index. Returns false when the block is exhausted.
  bool next(FrameCount& offsetOut, std::int64_t& gridIndexOut) noexcept;

 private:
  const Transport& transport_;
  double gridBeats_ = 0.25;
  FrameCount frames_ = 0;
  SampleIndex blockStart_ = 0;
  std::int64_t index_ = 0;
};

}  // namespace nekta
