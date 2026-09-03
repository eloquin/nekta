#include "params/ParamRegistry.h"

#include <cmath>

namespace nekta {
namespace {

float clampTo(const ParamDesc& d, float v) noexcept {
  if (v < d.minValue) return d.minValue;
  if (v > d.maxValue) return d.maxValue;
  return v;
}

// One-pole coefficient such that a step reaches 63.2% after `ms` milliseconds.
float coeffFor(float ms, double sampleRate) noexcept {
  if (ms <= 0.0f) return 1.0f;
  const double tau = static_cast<double>(ms) * 0.001 * sampleRate;
  return static_cast<float>(1.0 - std::exp(-1.0 / tau));
}

}  // namespace

ParamRegistry::ParamRegistry(int capacity)
    : capacity_(capacity < 1 ? 1 : capacity),
      targets_(new std::atomic<float>[static_cast<std::size_t>(capacity_)]),
      values_(new float[static_cast<std::size_t>(capacity_)]),
      perSampleCoeff_(new float[static_cast<std::size_t>(capacity_)]),
      blockCoeff_(new float[static_cast<std::size_t>(capacity_)]) {
  paths_.reserve(static_cast<std::size_t>(capacity_));
  descs_.reserve(static_cast<std::size_t>(capacity_));
  for (int i = 0; i < capacity_; ++i) {
    targets_[static_cast<std::size_t>(i)].store(0.0f, std::memory_order_relaxed);
    values_[static_cast<std::size_t>(i)] = 0.0f;
    perSampleCoeff_[static_cast<std::size_t>(i)] = 1.0f;
    blockCoeff_[static_cast<std::size_t>(i)] = 1.0f;
  }
}

ParamId ParamRegistry::add(const char* path, const ParamDesc& desc) {
  const ParamId existing = find(path);
  if (existing != kInvalidParam) return existing;
  if (count_ >= capacity_) return kInvalidParam;
  const ParamId id = count_++;
  paths_.emplace_back(path);
  descs_.push_back(desc);
  const std::size_t i = static_cast<std::size_t>(id);
  const float def = clampTo(desc, desc.defaultValue);
  targets_[i].store(def, std::memory_order_relaxed);
  values_[i] = def;
  perSampleCoeff_[i] = coeffFor(desc.smoothingMs, sampleRate_);
  blockCoeff_[i] = perSampleCoeff_[i];
  return id;
}

ParamId ParamRegistry::find(const char* path) const {
  for (int i = 0; i < count_; ++i) {
    if (paths_[static_cast<std::size_t>(i)] == path) return i;
  }
  return kInvalidParam;
}

const char* ParamRegistry::path(ParamId id) const {
  return valid(id) ? paths_[static_cast<std::size_t>(id)].c_str() : "";
}

const ParamDesc& ParamRegistry::desc(ParamId id) const {
  static const ParamDesc kEmpty{};
  return valid(id) ? descs_[static_cast<std::size_t>(id)] : kEmpty;
}

void ParamRegistry::prepare(double sampleRate, FrameCount nominalBlock) {
  sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
  for (int i = 0; i < count_; ++i) {
    const std::size_t u = static_cast<std::size_t>(i);
    perSampleCoeff_[u] = coeffFor(descs_[u].smoothingMs, sampleRate_);
    const float def = clampTo(descs_[u], descs_[u].defaultValue);
    targets_[u].store(def, std::memory_order_relaxed);
    values_[u] = def;
  }
  cachedBlock_ = 0;
  recomputeBlockCoeffs(nominalBlock > 0 ? nominalBlock : 1);
}

void ParamRegistry::recomputeBlockCoeffs(FrameCount n) noexcept {
  // (1 - c)^n in closed form. Called once per block-size change, never per
  // block in steady state, and allocates nothing.
  for (int i = 0; i < count_; ++i) {
    const std::size_t u = static_cast<std::size_t>(i);
    const float c = perSampleCoeff_[u];
    blockCoeff_[u] = (c >= 1.0f) ? 1.0f : 1.0f - std::pow(1.0f - c, static_cast<float>(n));
  }
  cachedBlock_ = n;
}

void ParamRegistry::snapAll() noexcept {
  for (int i = 0; i < count_; ++i) {
    const std::size_t u = static_cast<std::size_t>(i);
    values_[u] = targets_[u].load(std::memory_order_relaxed);
  }
}

void ParamRegistry::setTarget(ParamId id, float v) noexcept {
  if (!valid(id)) return;
  const std::size_t u = static_cast<std::size_t>(id);
  targets_[u].store(clampTo(descs_[u], v), std::memory_order_relaxed);
}

void ParamRegistry::setNormalised(ParamId id, float t01) noexcept {
  if (!valid(id)) return;
  const ParamDesc& d = descs_[static_cast<std::size_t>(id)];
  const float t = t01 < 0.0f ? 0.0f : (t01 > 1.0f ? 1.0f : t01);
  setTarget(id, d.minValue + t * (d.maxValue - d.minValue));
}

float ParamRegistry::target(ParamId id) const noexcept {
  return valid(id) ? targets_[static_cast<std::size_t>(id)].load(std::memory_order_relaxed) : 0.0f;
}

float ParamRegistry::normalised(ParamId id) const noexcept {
  if (!valid(id)) return 0.0f;
  const ParamDesc& d = descs_[static_cast<std::size_t>(id)];
  const float span = d.maxValue - d.minValue;
  if (span <= 0.0f) return 0.0f;
  return (values_[static_cast<std::size_t>(id)] - d.minValue) / span;
}

void ParamRegistry::snap(ParamId id) noexcept {
  if (!valid(id)) return;
  const std::size_t u = static_cast<std::size_t>(id);
  values_[u] = targets_[u].load(std::memory_order_relaxed);
}

void ParamRegistry::advanceBlock(FrameCount n) noexcept {
  if (n <= 0) return;
  if (n != cachedBlock_) recomputeBlockCoeffs(n);
  for (int i = 0; i < count_; ++i) {
    const std::size_t u = static_cast<std::size_t>(i);
    const float t = targets_[u].load(std::memory_order_relaxed);
    values_[u] += (t - values_[u]) * blockCoeff_[u];
  }
}

}  // namespace nekta
