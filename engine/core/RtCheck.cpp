#include "core/RtCheck.h"

#include <atomic>

#if NEKTA_RT_CHECKS
#include <cstdlib>
#include <new>
#endif

namespace nekta {
namespace rt {
namespace {
thread_local bool tAudioThread = false;
std::atomic<std::uint64_t> gViolations{0};
}  // namespace

void enterAudioThread() noexcept { tAudioThread = true; }
void exitAudioThread() noexcept { tAudioThread = false; }
bool inAudioThread() noexcept { return tAudioThread; }

std::uint64_t allocationViolations() noexcept {
  return gViolations.load(std::memory_order_relaxed);
}

void resetViolations() noexcept { gViolations.store(0, std::memory_order_relaxed); }

bool checksEnabled() noexcept {
#if NEKTA_RT_CHECKS
  return true;
#else
  return false;
#endif
}

ScopedAllocPermit::ScopedAllocPermit() noexcept : wasInAudioThread_(tAudioThread) {
  tAudioThread = false;
}
ScopedAllocPermit::~ScopedAllocPermit() noexcept { tAudioThread = wasInAudioThread_; }

#if NEKTA_RT_CHECKS
void noteAllocation() noexcept {
  if (tAudioThread) gViolations.fetch_add(1, std::memory_order_relaxed);
}
#endif

}  // namespace rt
}  // namespace nekta

#if NEKTA_RT_CHECKS
// Global operator new/delete replacements. They must not allocate themselves,
// so they go straight to malloc/free and only bump an atomic counter.
namespace nekta {
namespace rt {
void noteAllocation() noexcept;
}
}  // namespace nekta

namespace {
void* nektaAlloc(std::size_t size) {
  nekta::rt::noteAllocation();
  if (size == 0) size = 1;
  void* p = std::malloc(size);
  return p;
}
}  // namespace

void* operator new(std::size_t size) {
  void* p = nektaAlloc(size);
  if (p == nullptr) throw std::bad_alloc();
  return p;
}
void* operator new[](std::size_t size) {
  void* p = nektaAlloc(size);
  if (p == nullptr) throw std::bad_alloc();
  return p;
}
void* operator new(std::size_t size, const std::nothrow_t&) noexcept { return nektaAlloc(size); }
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept { return nektaAlloc(size); }

void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
void operator delete(void* p, const std::nothrow_t&) noexcept { std::free(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { std::free(p); }
#endif  // NEKTA_RT_CHECKS
