#include "Engine.h"

#include <cmath>
#include <cstdio>

namespace nekta {
namespace {
const PadConfig kEmptyPad{};
}  // namespace

Engine::Engine() { registerParams(); }

void Engine::registerParams() {
  // String-path addressing is what XY macro maps and presets bind to (SPEC §2.6).
  pMasterGain_ = params_.add("master.gain", {0.0f, 2.0f, 1.0f, 15.0f});
  pCutoff_ = params_.add("fx.1.cutoff", {20.0f, 20000.0f, 20000.0f, 15.0f});
  pRes_ = params_.add("fx.1.res", {0.0f, 1.0f, 0.0f, 15.0f});
  pDrive_ = params_.add("fx.1.drive", {1.0f, 24.0f, 1.0f, 15.0f});
  pMorph_ = params_.add("fx.1.morph", {0.0f, 3.0f, 0.0f, 15.0f});
  pBypass_ = params_.add("fx.1.bypass", {0.0f, 1.0f, 0.0f, 0.0f});
  for (int i = 0; i < kNumPads; ++i) {
    char path[32];
    std::snprintf(path, sizeof(path), "pad.%d.gain", i);
    params_.add(path, {0.0f, 2.0f, 1.0f, 15.0f});
    std::snprintf(path, sizeof(path), "pad.%d.pitch", i);
    params_.add(path, {-24.0f, 24.0f, 0.0f, 15.0f});
  }
}

void Engine::prepare(double sampleRate, FrameCount maxBlock) {
  sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
  maxBlock_ = maxBlock > 0 ? maxBlock : 512;
  params_.prepare(sampleRate_, maxBlock_);
  transport_.prepare(sampleRate_);
  voices_.prepare(sampleRate_);
  roll_.prepare(sampleRate_);
  filter_.prepare(sampleRate_);
  masterGain_ = params_.value(pMasterGain_);
  queue_.clear();
  dropped_ = 0;
}

void Engine::reset() noexcept {
  voices_.reset();
  roll_.reset();
  filter_.reset();
  params_.snapAll();
  queue_.clear();
  masterGain_ = params_.value(pMasterGain_);
}

bool Engine::setSample(int slot, SampleBuffer&& buffer) {
  if (slot < 0 || slot >= kMaxSamples) return false;
  samples_[slot] = std::move(buffer);
  return true;
}

const SampleBuffer* Engine::sample(int slot) const {
  if (slot < 0 || slot >= kMaxSamples) return nullptr;
  return &samples_[slot];
}

void Engine::setPadConfig(int pad, const PadConfig& config) {
  if (pad < 0 || pad >= kNumPads) return;
  pads_[pad] = config;
}

const PadConfig& Engine::padConfig(int pad) const {
  if (pad < 0 || pad >= kNumPads) return kEmptyPad;
  return pads_[pad];
}

// Collects the queued host events that fall inside this block, ordered by
// offset. Roll repeats are gathered per segment instead, so a roll started
// mid-block still fires its first repeat in that same block. Fixed arrays,
// insertion sort: no allocation.
int Engine::gatherEvents(FrameCount frames) noexcept {
  const SampleIndex blockStart = transport_.samplePosition();
  const SampleIndex blockEnd = blockStart + frames;
  int count = 0;

  EngineEvent queued;
  while (count < kMaxEventsPerBlock && queue_.peek(queued)) {
    if (queued.time >= blockEnd) break;  // not yet: leave it queued
    queue_.discardFront();
    PendingEvent p;
    p.offset = queued.time > blockStart ? static_cast<FrameCount>(queued.time - blockStart) : 0;
    p.type = queued.type;
    p.index = queued.index;
    p.value = queued.value;
    pending_[count++] = p;
  }

  for (int i = 1; i < count; ++i) {
    PendingEvent key = pending_[i];
    int j = i - 1;
    while (j >= 0 && pending_[j].offset > key.offset) {
      pending_[j + 1] = pending_[j];
      --j;
    }
    pending_[j + 1] = key;
  }
  return count;
}

void Engine::triggerPad(const PendingEvent& event) noexcept {
  if (event.index < 0 || event.index >= kNumPads) return;
  const PadConfig& pad = pads_[event.index];
  if (pad.sampleSlot < 0 || pad.sampleSlot >= kMaxSamples) return;
  const SampleBuffer& buffer = samples_[pad.sampleSlot];
  if (buffer.empty()) return;

  VoiceSpec spec;
  spec.sample = &buffer;
  spec.startFrame = pad.startFrame + event.startOffsetSeconds * buffer.sampleRate();
  spec.endFrame = pad.endFrame;
  spec.loop = pad.loop;
  spec.reverse = pad.reverse != event.reverse;  // roll can alternate direction
  spec.pitchSemitones = pad.pitchSemitones + event.pitchOffset;
  spec.gain = pad.gain * event.value;
  spec.pan = pad.pan + event.pan;
  if (spec.pan < -1.0f) spec.pan = -1.0f;
  if (spec.pan > 1.0f) spec.pan = 1.0f;
  spec.env = pad.env;
  spec.mode = pad.mode;
  spec.chokeGroup = pad.chokeGroup;
  spec.padIndex = event.index;
  spec.gateSeconds = event.gateSeconds;
  voices_.trigger(spec, 0);
}

void Engine::applyEvent(const PendingEvent& event) noexcept {
  switch (event.type) {
    case EventType::PadTrigger:
      triggerPad(event);
      break;
    case EventType::PadRelease:
      voices_.releasePad(event.index);
      break;
    case EventType::RollStart: {
      SampleIndex when = transport_.samplePosition() + event.offset;
      if (launchQuantise_) when = transport_.nextGridSample(when, launchGrid_);
      roll_.start(transport_, when);
      break;
    }
    case EventType::RollStop:
      roll_.stop();
      break;
    case EventType::ParamSet:
      params_.setTarget(event.index, event.value);
      break;
  }
}

void Engine::renderSegment(float* left, float* right, FrameCount offset,
                           FrameCount frames) noexcept {
  if (frames <= 0) return;
  float* l = left + offset;
  float* r = right + offset;

  voices_.process(l, r, frames);

  filter_.setBypass(params_.value(pBypass_) >= 0.5f);
  filter_.setCutoff(params_.value(pCutoff_));
  filter_.setResonance(params_.value(pRes_));
  filter_.setDrive(params_.value(pDrive_));
  filter_.setMorph(params_.value(pMorph_));
  filter_.processBlock(l, r, frames);

  // Master gain gets its own per-sample ramp so a jump between segments cannot
  // click even though the registry smooths at block rate.
  const float gainEnd = params_.value(pMasterGain_);
  const float step = (gainEnd - masterGain_) / static_cast<float>(frames);
  float g = masterGain_;
  for (FrameCount i = 0; i < frames; ++i) {
    g += step;
    l[i] *= g;
    r[i] *= g;
  }
  masterGain_ = gainEnd;

  params_.advanceBlock(frames);
}

void Engine::process(float* left, float* right, FrameCount frames) noexcept {
  if (frames <= 0) return;
  for (FrameCount i = 0; i < frames; ++i) {
    left[i] = 0.0f;
    right[i] = 0.0f;
  }

  const int count = gatherEvents(frames);
  const SampleIndex blockStart = transport_.samplePosition();
  const int maxRoll = static_cast<int>(sizeof(rollEvents_) / sizeof(rollEvents_[0]));

  FrameCount cursor = 0;
  int index = 0;
  while (cursor < frames) {
    while (index < count && pending_[index].offset <= cursor) {
      applyEvent(pending_[index]);
      ++index;
    }
    FrameCount next = (index < count) ? pending_[index].offset : frames;
    if (next > frames) next = frames;

    // Split the segment again on any roll repeats inside it.
    if (roll_.running()) {
      const int rolls =
          roll_.process(transport_, blockStart + cursor, next - cursor, rollEvents_, maxRoll);
      for (int i = 0; i < rolls; ++i) {
        const FrameCount at = cursor + rollEvents_[i].offsetInBlock;
        renderSegment(left, right, cursor, at - cursor);
        cursor = at;
        PendingEvent p;
        p.offset = at;
        p.type = EventType::PadTrigger;
        p.index = rollEvents_[i].padIndex;
        p.value = rollEvents_[i].gain;
        p.pan = rollEvents_[i].pan;
        p.pitchOffset = rollEvents_[i].pitchSemitones;
        p.reverse = rollEvents_[i].reverse;
        p.startOffsetSeconds = rollEvents_[i].startOffsetSeconds;
        p.gateSeconds = rollEvents_[i].gateSeconds;
        p.fromRoll = true;
        applyEvent(p);
      }
    }

    renderSegment(left, right, cursor, next - cursor);
    cursor = next;
  }

  transport_.advance(frames);
}

}  // namespace nekta
