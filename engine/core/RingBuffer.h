#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>

namespace nekta {

// Single-producer / single-consumer lock-free ring buffer (SPEC §1.2).
// Capacity must be a power of two. T must be trivially copyable POD - no
// destructors run on the audio thread.
template <typename T, std::size_t Capacity>
class SpscRingBuffer {
 public:
  static_assert(Capacity >= 2, "capacity must be >= 2");
  static_assert((Capacity & (Capacity - 1)) == 0, "capacity must be a power of two");
  static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

  // Producer thread. Returns false if full (never blocks, never allocates).
  bool push(const T& item) noexcept {
    const std::size_t w = write_.load(std::memory_order_relaxed);
    const std::size_t next = (w + 1) & kMask;
    if (next == read_.load(std::memory_order_acquire)) return false;  // full
    storage_[w] = item;
    write_.store(next, std::memory_order_release);
    return true;
  }

  // Consumer thread (audio). Returns false if empty.
  bool pop(T& out) noexcept {
    const std::size_t r = read_.load(std::memory_order_relaxed);
    if (r == write_.load(std::memory_order_acquire)) return false;  // empty
    out = storage_[r];
    read_.store((r + 1) & kMask, std::memory_order_release);
    return true;
  }

  // Consumer thread. Peeks without consuming.
  bool peek(T& out) const noexcept {
    const std::size_t r = read_.load(std::memory_order_relaxed);
    if (r == write_.load(std::memory_order_acquire)) return false;
    out = storage_[r];
    return true;
  }

  void discardFront() noexcept {
    const std::size_t r = read_.load(std::memory_order_relaxed);
    if (r == write_.load(std::memory_order_acquire)) return;
    read_.store((r + 1) & kMask, std::memory_order_release);
  }

  bool empty() const noexcept {
    return read_.load(std::memory_order_acquire) == write_.load(std::memory_order_acquire);
  }

  std::size_t size() const noexcept {
    const std::size_t w = write_.load(std::memory_order_acquire);
    const std::size_t r = read_.load(std::memory_order_acquire);
    return (w - r) & kMask;
  }

  static constexpr std::size_t capacity() noexcept { return Capacity - 1; }

  void clear() noexcept {
    read_.store(write_.load(std::memory_order_acquire), std::memory_order_release);
  }

 private:
  static constexpr std::size_t kMask = Capacity - 1;
  T storage_[Capacity] = {};
  std::atomic<std::size_t> write_{0};
  std::atomic<std::size_t> read_{0};
};

}  // namespace nekta
