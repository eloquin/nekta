#pragma once

#include "clock/Transport.h"
#include "core/RingBuffer.h"
#include "core/Types.h"
#include "dsp/Svf.h"
#include "params/ParamRegistry.h"
#include "roll/RollEngine.h"
#include "voice/SamplerVoice.h"
#include "voice/VoicePool.h"

namespace nekta {

inline constexpr int kMaxSamples = 16;
inline constexpr int kMaxEventsPerBlock = 128;

// Per-pad playback settings (SPEC §2.3). Setup-thread data: written before
// rendering starts, read by the audio thread.
struct PadConfig {
  int sampleSlot = -1;
  double startFrame = 0.0;
  double endFrame = -1.0;
  bool loop = false;
  bool reverse = false;
  double pitchSemitones = 0.0;
  float gain = 1.0f;
  float pan = 0.0f;
  AhdsrParams env{};
  TriggerMode mode = TriggerMode::OneShot;
  int chokeGroup = 0;
};

enum class EventType { PadTrigger, PadRelease, RollStart, RollStop, ParamSet };

// Crosses the UI -> audio boundary through a lock-free SPSC queue (SPEC §1.2).
// POD by construction: no strings, no pointers, no destructor.
struct EngineEvent {
  EventType type = EventType::PadTrigger;
  SampleIndex time = 0;  // absolute sample time; earlier than "now" fires at once
  int index = 0;         // pad index, or ParamId for ParamSet
  float value = 1.0f;    // velocity, or the parameter's target value
};

// The P0 signal path (SPEC §7): pads -> voices -> SVF insert -> master gain.
// The graph is static; nothing here is built or destroyed while rendering.
class Engine {
 public:
  Engine();

  void prepare(double sampleRate, FrameCount maxBlock);
  void reset() noexcept;

  // --- setup thread -------------------------------------------------------
  bool setSample(int slot, SampleBuffer&& buffer);
  const SampleBuffer* sample(int slot) const;
  void setPadConfig(int pad, const PadConfig& config);
  const PadConfig& padConfig(int pad) const;
  void setRollConfig(const RollConfig& config) noexcept { roll_.setConfig(config); }
  const RollConfig& rollConfig() const noexcept { return roll_.config(); }

  ParamRegistry& params() noexcept { return params_; }
  const ParamRegistry& params() const noexcept { return params_; }
  Transport& transport() noexcept { return transport_; }
  const Transport& transport() const noexcept { return transport_; }

  // Quantise-launch grid for rolls and pads (SPEC §2.7). Division 0 = off.
  void setLaunchQuantise(const Rate& rate, bool enabled) noexcept {
    launchGrid_ = rate;
    launchQuantise_ = enabled;
  }

  // --- UI thread (lock-free) ----------------------------------------------
  bool pushEvent(const EngineEvent& event) noexcept { return queue_.push(event); }

  // --- audio thread -------------------------------------------------------
  // Replaces the contents of the output buffers.
  void process(float* left, float* right, FrameCount frames) noexcept;

  int activeVoices() const noexcept { return voices_.activeVoices(); }
  bool rollRunning() const noexcept { return roll_.running(); }
  // Number of events dropped because the queue was full - a host-side bug
  // indicator, not something the audio thread should ever cause.
  std::uint64_t droppedEvents() const noexcept { return dropped_; }

  ParamId paramCutoff() const noexcept { return pCutoff_; }
  ParamId paramResonance() const noexcept { return pRes_; }
  ParamId paramDrive() const noexcept { return pDrive_; }
  ParamId paramMorph() const noexcept { return pMorph_; }
  ParamId paramMasterGain() const noexcept { return pMasterGain_; }
  ParamId paramFilterBypass() const noexcept { return pBypass_; }

 private:
  struct PendingEvent {
    FrameCount offset = 0;
    EventType type = EventType::PadTrigger;
    int index = 0;
    float value = 1.0f;
    float pan = 0.0f;
    double pitchOffset = 0.0;
    bool reverse = false;
    double startOffsetSeconds = 0.0;
    double gateSeconds = -1.0;
    bool fromRoll = false;
  };

  void registerParams();
  int gatherEvents(FrameCount frames) noexcept;
  void applyEvent(const PendingEvent& event) noexcept;
  void triggerPad(const PendingEvent& event) noexcept;
  void renderSegment(float* left, float* right, FrameCount offset, FrameCount frames) noexcept;

  double sampleRate_ = 48000.0;
  FrameCount maxBlock_ = 512;

  ParamRegistry params_{128};
  Transport transport_;
  VoicePool voices_;
  RollEngine roll_;
  Svf filter_;

  SampleBuffer samples_[kMaxSamples];
  PadConfig pads_[kNumPads];

  SpscRingBuffer<EngineEvent, 1024> queue_;
  PendingEvent pending_[kMaxEventsPerBlock];
  RollEvent rollEvents_[kMaxEventsPerBlock / 2];
  std::uint64_t dropped_ = 0;

  Rate launchGrid_{16};
  bool launchQuantise_ = false;
  float masterGain_ = 1.0f;

  ParamId pCutoff_ = kInvalidParam;
  ParamId pRes_ = kInvalidParam;
  ParamId pDrive_ = kInvalidParam;
  ParamId pMorph_ = kInvalidParam;
  ParamId pMasterGain_ = kInvalidParam;
  ParamId pBypass_ = kInvalidParam;
};

}  // namespace nekta
