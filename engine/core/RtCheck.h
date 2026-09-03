#pragma once

#include <cstdint>

namespace nekta {
namespace rt {

// Marks the calling thread as "inside the audio callback". While the flag is
// set and the library is built with NEKTA_RT_CHECKS, every heap allocation is
// counted as a violation (SPEC §8: allocation guard).
void enterAudioThread() noexcept;
void exitAudioThread() noexcept;
bool inAudioThread() noexcept;

// Violation counters are process-wide (atomic) so a test can assert on work
// done by a render thread it did not create.
std::uint64_t allocationViolations() noexcept;
void resetViolations() noexcept;

// True when the build actually hooks operator new.
bool checksEnabled() noexcept;

struct ScopedAudioThread {
  ScopedAudioThread() noexcept { enterAudioThread(); }
  ~ScopedAudioThread() noexcept { exitAudioThread(); }
  ScopedAudioThread(const ScopedAudioThread&) = delete;
  ScopedAudioThread& operator=(const ScopedAudioThread&) = delete;
};

// Temporarily lifts the guard - only for code that is provably not on the audio
// thread but lexically inside a scope that is (e.g. offline harness bookkeeping).
struct ScopedAllocPermit {
  ScopedAllocPermit() noexcept;
  ~ScopedAllocPermit() noexcept;
  ScopedAllocPermit(const ScopedAllocPermit&) = delete;
  ScopedAllocPermit& operator=(const ScopedAllocPermit&) = delete;

 private:
  bool wasInAudioThread_ = false;
};

}  // namespace rt
}  // namespace nekta
