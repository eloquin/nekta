#pragma once

#include "clock/Rate.h"
#include "clock/Transport.h"
#include "core/Types.h"

namespace nekta {

enum class RampCurve { Linear, Exponential };

// SPEC §2.4A: the rate slews across the hold (e.g. 1/8 -> 1/32 over one bar).
// This is the accelerating jungle fill, and it is a first-class parameter.
struct RollRamp {
  bool enabled = false;
  Rate endRate{32};
  double lengthBeats = 4.0;
  RampCurve curve = RampCurve::Linear;
};

// Per-repeat modifiers (SPEC §2.4A).
struct RollModifiers {
  float ampDecay = 1.0f;         // gain multiplier per repeat (1 = flat)
  float ampCurve = 1.0f;         // shapes the decay: gain = ampDecay^(n^ampCurve)
  double pitchDeltaSemis = 0.0;  // semitones added per repeat
  int pitchWrapRepeats = 0;      // wrap the pitch walk every N repeats (0 = never)
  bool reverseAlternate = false;
  double startOffsetWalkMs = 0.0;  // sample start offset added per repeat
  bool panAlternate = false;
  float panAmount = 0.0f;
  float gate = 1.0f;  // duty cycle of the interval; 1 = no gate (smear)
};

struct RollConfig {
  Rate rate{16};
  RollRamp ramp;
  RollModifiers mods;
  int padIndex = 0;
  float baseGain = 1.0f;
};

struct RollEvent {
  long long repeatIndex = 0;
  FrameCount offsetInBlock = 0;
  SampleIndex sampleTime = 0;
  float gain = 1.0f;
  float pan = 0.0f;
  double pitchSemitones = 0.0;
  bool reverse = false;
  double startOffsetSeconds = 0.0;
  double gateSeconds = -1.0;  // < 0 = no gate
  int padIndex = 0;
};

// Pad retrigger (SPEC §2.4A).
//
// Onset times come from a closed-form inversion of the phase integral
// phi(b) = integral of rate(b) db, so repeat n lands at an exact musical
// position instead of an accumulated one - no drift, and no per-sample work.
class RollEngine {
 public:
  void prepare(double sampleRate) noexcept;
  void reset() noexcept;

  void setConfig(const RollConfig& config) noexcept { config_ = config; }
  const RollConfig& config() const noexcept { return config_; }

  // startSample must already be quantised by the caller (SPEC §2.7).
  void start(const Transport& transport, SampleIndex startSample) noexcept;
  void stop() noexcept { running_ = false; }
  bool running() const noexcept { return running_; }

  // Emits every repeat falling inside [blockStart, blockStart + frames).
  // Offsets are relative to blockStart. Returns the number written; if the
  // caller's array fills up the remaining repeats are emitted at the start of
  // the next call rather than being dropped.
  int process(const Transport& transport, SampleIndex blockStart, FrameCount frames,
              RollEvent* out, int maxEvents) noexcept;

  // Convenience overload for hosts that render whole blocks.
  int process(const Transport& transport, FrameCount frames, RollEvent* out,
              int maxEvents) noexcept {
    return process(transport, transport.samplePosition(), frames, out, maxEvents);
  }

  // Rate in repeats per beat at `beats` after the roll started.
  double rateAtBeats(double beats) const noexcept;
  // Musical position of repeat n, in beats after the roll started.
  double beatsForRepeat(long long n) const noexcept;

 private:
  RollEvent makeEvent(long long n, FrameCount offset, SampleIndex when,
                      const Transport& transport) const noexcept;

  double sampleRate_ = 48000.0;
  RollConfig config_{};
  bool running_ = false;
  double startPpq_ = 0.0;
  SampleIndex startSample_ = 0;
  long long nextRepeat_ = 0;
};

}  // namespace nekta
