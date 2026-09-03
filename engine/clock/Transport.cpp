#include "clock/Transport.h"

#include <cmath>

namespace nekta {
namespace {
// Guards against a grid position landing a hair before its ideal sample due to
// double rounding, which would emit the same onset twice.
constexpr double kPpqEpsilon = 1.0e-9;
}  // namespace

void Transport::prepare(double sampleRate) noexcept {
  sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
  samplesPerBeat_ = sampleRate_ * 60.0 / bpm_;
  reset(0.0);
}

void Transport::reset(double ppq) noexcept {
  samplePos_ = 0;
  anchorSample_ = 0;
  anchorPpq_ = ppq;
}

void Transport::setBpm(double bpm) noexcept {
  if (bpm < kMinBpm) bpm = kMinBpm;
  if (bpm > kMaxBpm) bpm = kMaxBpm;
  // Re-anchor at the current position first, so the musical position is
  // continuous across the tempo change.
  anchorPpq_ = ppqAtSample(samplePos_);
  anchorSample_ = samplePos_;
  bpm_ = bpm;
  samplesPerBeat_ = sampleRate_ * 60.0 / bpm_;
}

double Transport::barPhase() const noexcept {
  const double beats = ppqPosition() / static_cast<double>(beatsPerBar_);
  double phase = beats - std::floor(beats);
  if (phase < 0.0) phase += 1.0;
  return phase;
}

SampleIndex Transport::sampleAtPpqRounded(double ppq) const noexcept {
  return static_cast<SampleIndex>(std::llround(sampleAtPpq(ppq)));
}

double Transport::nextGridPpq(double ppq, const Rate& rate) const noexcept {
  const double grid = rate.beats();
  const double k = std::ceil(ppq / grid - kPpqEpsilon);
  return k * grid;
}

SampleIndex Transport::nextGridSample(SampleIndex from, const Rate& rate) const noexcept {
  const double grid = rate.beats();
  const double fromPpq = ppqAtSample(from);
  double k = std::ceil(fromPpq / grid - kPpqEpsilon);
  SampleIndex s = sampleAtPpqRounded(k * grid);
  // Rounding can put the boundary one sample behind `from`; step forward.
  while (s < from) s = sampleAtPpqRounded((++k) * grid);
  return s;
}

SampleIndex Transport::nearestGridSample(SampleIndex near, const Rate& rate) const noexcept {
  const double grid = rate.beats();
  const double k = std::round(ppqAtSample(near) / grid);
  return sampleAtPpqRounded(k * grid);
}

GridIterator::GridIterator(const Transport& transport, const Rate& rate,
                           FrameCount frames) noexcept
    : transport_(transport),
      gridBeats_(rate.beats()),
      frames_(frames),
      blockStart_(transport.samplePosition()) {
  const double startPpq = transport.ppqAtSample(blockStart_);
  index_ = static_cast<std::int64_t>(std::ceil(startPpq / gridBeats_ - 1.0e-9));
}

bool GridIterator::next(FrameCount& offsetOut, std::int64_t& gridIndexOut) noexcept {
  while (true) {
    const SampleIndex gridSample =
        transport_.sampleAtPpqRounded(static_cast<double>(index_) * gridBeats_);
    if (gridSample >= blockStart_ + frames_) return false;
    const std::int64_t offset = gridSample - blockStart_;
    const std::int64_t index = index_++;
    if (offset < 0) continue;  // rounded just behind the block start
    offsetOut = static_cast<FrameCount>(offset);
    gridIndexOut = index;
    return true;
  }
}

}  // namespace nekta
