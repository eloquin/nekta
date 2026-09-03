# Nekta Sampler — Build Framework & Spec

Working name: **Nekta Sampler** (in-app: "Capture")
Version: v0.1 spec.

---

## 0. One-liner

Tap the capture button, record any sound in the wild, and instantly perform it —
tempo-synced jungle rolls, XY-macro FX, live glitch — over a reactive visualiser,
then export a shareable clip. Sample packs, FX/XY presets, roll banks and
visualiser skins are marketplace items.

Why it fits Nekta: it turns passive event attendance into UGC, it gives artist
cards a real payload (a dub becomes a playable pack), and every export is a
watermarked growth loop. Marketplace items ride the existing 3% take.

---

## 1. Locked architecture decisions

Do not let the implementation drift from these.

### 1.1 One DSP core, three hosts

Write the engine **once in C++17** (no JUCE dependency in the core — plain
headers + a thin platform layer, so it compiles to WASM cleanly).

| Host | Audio I/O | Bindings |
|---|---|---|
| iOS | AVAudioEngine -> AVAudioSourceNode / AURemoteIO, 48 kHz, 128–256 frame buffer | Obj-C++ -> JSI (React Native) |
| Android | Oboe / AAudio | JNI -> JSI |
| Web | AudioWorklet + WASM (emscripten, `-sSINGLE_FILE`) | direct JS |

**Do not build the performance engine in a WebView.** WKWebView input latency
and audio-session handling make live rolls unusable. Web target is for
playback/preview and the desktop remix view, not for the primary capture flow.

### 1.2 Real-time rules (enforce in review)

- No allocation, no locks, no logging, no `std::string`, no exceptions on the audio thread.
- Audio <-> UI communication is lock-free SPSC ring buffers only.
- Static DSP graph. Adding an FX does not rebuild the graph — slots are pre-allocated and bypassed.
- All parameters are smoothed (one-pole, 10–20 ms) or applied at sample-accurate event boundaries.
- Target: **< 12 ms round trip on iOS**. Measure it in CI with a loopback test, fail the build if it regresses.

### 1.3 UI layer

React Native app. Audio feature data crosses to JS via **JSI + a shared
`Float32Array` view**, never the async bridge. Visualiser reads that buffer
directly on the render thread.

### 1.4 Visualiser

Skins are **GLSL fragment shaders against a fixed uniform contract** (§6.2).
That contract is the product decision that makes skins sellable — freeze it
early and version it. Launch skins reuse the existing Chladni / cymatics /
fractal work.

---

## 2. Feature spec

### 2.1 Capture

- Big single button. Hold to record, or tap to arm a fixed 4-bar capture.
- **Pre-roll buffer**: a rolling 2 s input ring buffer is always running once the
  screen is open. Tapping capture includes the 2 s *before* the tap. You never
  miss the sound.
- Input chain: HPF @ 40 Hz -> DC removal -> optional AGC preview (not destructive)
  -> write to disk as 48 kHz 32-bit float WAV.
- Post-capture auto-treatment (all reversible, stored as non-destructive params):
  silence trim to first/last transient, peak normalise to -1 dBFS, zero-crossing
  snap on both edges.
- Metadata: timestamp, geo (opt-in), linked Nekta event ID if the user is at a
  venue, device, input gain. Event-linked captures unlock the "recorded at" badge.
- Permissions: mic + (optional) location. Handle the iOS audio session category
  switch (`.playAndRecord`, `.measurement` mode, Bluetooth allowed) without
  killing other audio ungracefully.

### 2.2 Slicing

- Onset detection: spectral flux with adaptive median threshold; fall back to
  HFC for percussive material. Tunable sensitivity slider.
- Modes: `auto-transient`, `divide into N` (4/8/16/32), `manual drag`.
- Slice edges snap to nearest zero crossing within +/-128 samples.
- Assign slices -> pads left-to-right, or scatter randomly.
- Break-detect: if the capture is 1/2/4 bars of a loop, estimate BPM from the
  onset grid and set it as the kit tempo.

### 2.3 Pad engine

- 16 pads, 16-voice polyphony, per-pad:
  - trigger mode: one-shot / gate / loop / toggle
  - choke group (0–8)
  - start/end/loop points, reverse flag
  - pitch (semitones + cents) and varispeed toggle (repitch vs. time-stretch off
    by default — varispeed is the correct jungle behaviour)
  - AHDSR amp envelope, velocity -> amp/filter depth
  - per-pad SVF filter (cutoff, res, mode) and send levels to delay/reverb
- Playback read: 4-point Hermite interpolation. Anti-click 2 ms fade on start/stop/choke.

### 2.4 Roll / retrigger engine — the headline feature

Two distinct mechanisms; build both.

**A. Pad roll (retrigger)** — hold a pad + roll control:
- Rate: 1/4 … 1/64, with triplet and dotted variants. Snapped to transport.
- **Rate ramp**: rate slews across the hold (e.g. 1/8 -> 1/32 over 1 bar) with
  linear/exp curve. This is the accelerating jungle fill — a first-class
  parameter, not a hidden mod.
- Per-repeat modifiers: amp decay curve, pitch delta (semitones per repeat,
  +/- with wrap), reverse alternation, sample start offset walk, pan alternation.
- Gate/duty control per repeat (stab vs. smear).

**B. Buffer glitch (beat repeat)** — operates on the *master bus*, not a pad:
- Continuously captures the last 2 bars of output.
- Freeze/repeat the last 1/2, 1/4, 1/8, 1/16, 1/32 with hold, reverse, pitch,
  and a "scatter" probability that randomly picks slice/direction/rate per repeat.
- Tape-stop and vinyl-brake as ramp-out gestures.

**C. Chop patterns** — pattern banks applied to a sliced break:
- A pattern is a 16/32-step list of `{sliceIndex, reverse, pitch, gate, rollRate}`.
- Ship classic jungle edit templates (straight, ghost-note shuffle, last-16th
  roll, reverse-hat, 2-step displacement). Users can record their own from a live
  performance and save it as a pattern.
- Pattern banks are a marketplace item.

### 2.5 FX chain

Fixed graph — 4 insert slots on the master + 2 sends. Preset-portable, no
runtime graph rebuilds.

| Module | Notes |
|---|---|
| SVF filter | LP/BP/HP/notch morph, resonance, drive; Zavalishin TPT topology, stable at audio-rate mod |
| Bitcrush / downsample | bit depth 1–16, SR reduce with optional smoothing |
| Waveshaper / tape sat | tanh + asymmetry + tone tilt, oversampled 4x |
| Comb / formant | tuned comb for metallic pitch, formant filter bank for vowel sweeps |
| Granular cloud | grain 5–200 ms, density, spray, pitch +/- 24, freeze |
| Stutter / gate | trance gate with pattern, sync'd |
| Delay (send) | ping-pong, dub mode (filter + saturation in feedback loop), tape mode with pitch on time change, sync'd divisions |
| Reverb (send) | FDN (8x8 Hadamard) with modulated delays, size/decay/damp/diffusion, plus a **convolution mode for venue IRs** |
| Sidechain duck | triggered off pad 1 or transport, for pumping |
| Limiter | master, look-ahead 1.5 ms, always last |

### 2.6 XY pad + macros

The XY pad drives a **macro map**, not a hardwired filter+reverb:

```
XYMap {
  x: [ { target: "fx.1.cutoff", min: 200, max: 12000, curve: "exp" },
       { target: "send.delay",  min: 0,   max: 0.6,   curve: "lin" } ],
  y: [ { target: "fx.3.grain",  min: 0.02, max: 0.2,  curve: "exp" },
       { target: "roll.rate",   min: 8,    max: 32,   curve: "step" } ]
}
```

- Up to 6 destinations per axis, any engine parameter addressable by string path.
- Latch mode, spring-back mode, and a gesture recorder (records XY path as
  automation, loops it at bar length).
- Snap-to-grid option so XY moves quantise to the beat.
- **This is the core marketplace asset.** "BOU Roll Kit" = a sample pack + XY map
  + roll settings + skin. Artists can author one in ~10 minutes.

### 2.7 Transport & clock

- Master clock, sample-accurate, BPM 60–200, tap tempo + detected tempo.
- Everything sync'd: rolls, delay, gate, glitch, pattern playback, visualiser phase.
- Quantise-launch option for pads (off / 1/16 / 1/8 / 1/4 / 1 bar).
- Metronome (monitor only, excluded from render).

### 2.8 Performance recording & export

Record **two** things simultaneously:
1. Master bus audio (48 kHz float -> AAC/WAV on export).
2. A **session event list** — every pad hit, XY point, param change, with sample
   timestamps.

The session file is what makes remixing social: another user can load your
session, swap the sample pack, and re-render. Remix chains become a Nekta
content graph (`sessionId -> parentSessionId`). Prioritise this — it is the
defensible bit vs. a generic sampler app.

Export targets:
- Audio only (WAV/AAC)
- **Video**: offline-render the visualiser at 60 fps + mux audio
  (AVAssetWriter on iOS; ffmpeg.wasm on web). 9:16 for stories, 1:1 for feed.
  Nekta watermark + "recorded at [venue]" tag.

---

## 3. Marketplace items

| Item | Contents | Notes |
|---|---|---|
| Sample pack | N samples + kit mapping | Artist cards can drop these as hidden rewards |
| Kit preset | FX chain + XY map + pad params | Ships with a demo session |
| Roll / chop bank | Pattern arrays | Cheap to author, high volume |
| Visualiser skin | Shader + palette + params | Reuse existing Chladni/cymatics work |
| Venue IR pack | Convolution impulses captured in real rooms | Capture these at partner events |

Preset format: JSON, `schemaVersion` + `minEngineVersion`, samples referenced by
content hash on CDN, signed manifest. Free and paid tiers, existing 3% take,
artist revenue share on the rest.

**Rights guardrail (do not skip):** users will record other people's music at
events. Run audio fingerprinting on anything submitted to the marketplace and
block matched commercial recordings. Personal captures stay private/local.
Terms need to cover venue recording consent. Flag this to legal before launch.

---

## 4. Data model

```
Sample     { id, ownerId, uri, sha256, duration, sampleRate, peak,
             capturedAt, eventId?, geo?, transientMap[], bpm? }
Pad        { index, sampleId, start, end, loop, reverse, pitch, env,
             filter, sends, chokeGroup, triggerMode }
Kit        { id, name, pads[16], bpm, masterChain, xyMap, rollDefaults }
Pattern    { id, length, steps[{slice, reverse, pitch, gate, rollRate}] }
Skin       { id, shaderSrc, palette, uniformDefaults, thumbnail }
Session    { id, kitId, skinId, bpm, events[], duration,
             parentSessionId?, renderUri? }
Listing    { id, type, assetIds[], price, creatorId, revShare, status }
```

Local-first: everything works offline, syncs on connect. Samples are large —
upload only on share/publish.

---

## 5. Repo layout

```
/engine                 # C++ core, zero platform deps
  /dsp                  # filters, reverb, delay, granular, shaper
  /voice                # sampler voice, envelopes, interpolation
  /roll                 # retrigger, rate ramps, pattern playback
  /glitch               # master buffer, beat repeat, tape stop
  /clock                # transport, scheduler, quantiser
  /params               # param registry, smoothing, string-path addressing
  /session              # event recorder/player, preset (de)serialisation
  /analysis             # onset detection, FFT, feature extraction
  /test                 # offline render tests, null tests, fuzz
/platform
  /ios                  # AVAudioEngine host, JSI bindings, AVAssetWriter
  /android              # Oboe host, JNI
  /web                  # AudioWorklet + emscripten glue
/app                    # React Native
  /features/sampler     # capture, pads, XY, transport UI
  /visualiser           # shader host, uniform bridge, skin loader
  /marketplace
/skins                  # .frag + manifest per skin
```

Deviations in this repo (documented, deliberate):
- `/app` is already the existing Next.js marketing site. The React Native app
  will land under `/mobile` rather than colliding with it.
- `/engine/core` (shared RT primitives: SPSC ring buffer, alloc guard, denormal
  guard) and `/engine/io` (WAV, offline only) are additions to the layout above.
- `/engine/harness` holds the offline render CLI (P0 deliverable §10.6).

---

## 6. Visualiser contract

### 6.1 Feature extraction (audio thread -> ring buffer, 60 Hz)

`rms`, `peak`, `bands[64]` (log-spaced FFT magnitudes, 2048 window, Hann,
smoothed with asymmetric attack/release), `centroid`, `flux`, `onset` (bool),
`transportPhase` (0–1 within bar), `bpm`.

### 6.2 Shader uniform contract — freeze this

```glsl
uniform float uTime;        // seconds
uniform vec2  uResolution;
uniform float uRMS;         // 0..1 smoothed
uniform float uPeak;        // 0..1 fast
uniform float uLow;         // band group sums
uniform float uMid;
uniform float uHigh;
uniform float uCentroid;    // 0..1 normalised
uniform float uOnset;       // 1.0 on hit, decays over ~120ms
uniform vec2  uXY;          // live XY pad position 0..1
uniform float uPhase;       // bar phase 0..1
uniform float uBPM;
uniform vec3  uPalette[5];
uniform sampler2D uSpectrum; // 64x1 texture, full band detail
```

Any skin that compiles against this contract is sellable and forward-compatible.
Version the contract (`uniformVersion` in the skin manifest) and never remove a
uniform — only add.

### 6.3 Rendering

- iOS/Android: `react-native-skia` runtime shaders (SkSL) or a native Metal view.
  Write skins in GLSL and transpile — keep to a shader subset that maps cleanly.
- Web: Three.js / raw WebGL2, same fragment source.
- Offline video render must be deterministic: drive `uTime` from frame index,
  not wall clock, so exports match what was seen.

---

## 7. Build phases

**P0 — Spike (1–2 weeks).** Prove the hard part before building UI.
iOS only. Record 2 s -> one pad -> tempo-synced roll with rate ramp -> SVF filter ->
output. Measure round-trip latency and log it. Ship nothing else.
*Exit criteria: < 12 ms RTL, rolls are tight at 1/32, no glitches under load.*

**P1 — MVP.** Capture with pre-roll, auto-trim, 8 pads, slice-to-pads, roll
engine (A + B), 4 FX inserts + 2 sends, XY with 6 factory macro maps, transport,
1 visualiser skin, audio + video export, local save.

**P2 — Marketplace.** Preset/skin/pattern format, publishing flow, purchase,
artist packs wired to collectible cards, fingerprint check.

**P3 — Social.** Session replay, remix chains, venue IR packs, collab sessions,
leaderboard/feed integration.

---

## 8. Testing requirements

- **Offline render tests**: engine renders to a buffer deterministically; golden
  file comparison for each DSP module.
- **Null tests**: bypassed chain must be bit-identical to input.
- **Allocation guard**: hook `operator new` on the audio thread in debug, assert.
- **Timing test**: assert roll onsets land within +/-1 sample of the expected grid
  across all rates and BPMs, including ramps.
- **Latency test in CI**: loopback measurement on a device runner, fail on regression.
- **Preset round-trip**: serialise -> deserialise -> render must match.
- **Fuzz**: random param automation at audio rate, assert no NaN/inf/denormal
  blowup in filters and reverb feedback paths.

---

## 9. Explicit non-goals for v1

No time-stretch (varispeed only — it's the correct sound anyway). No MIDI. No
multitrack arrangement. No cloud DSP. No AI generation. No Android at P0/P1.
No plugin/AUv3 export (revisit at P3).

---

## 10. P0 task list (engine only)

Build `/engine` as a standalone C++17 library with no platform dependencies and
an offline render harness, so the whole engine can be tested from the command
line before any iOS code exists. Order:

1. `params` — registry with string-path addressing and one-pole smoothing.
2. `clock` — sample-accurate transport and quantiser, with tests asserting grid
   accuracy across 60–200 BPM.
3. `voice` — sampler voice with Hermite interpolation, AHDSR, anti-click fades.
4. `roll` — retrigger with rate ramps and per-repeat modifiers (§2.4A).
5. `dsp/svf` — TPT state variable filter, stable under audio-rate modulation.
6. An offline harness: load a WAV, trigger a scripted performance, render to WAV.

Tests first per module. No React Native app, visualiser or marketplace at P0.
No third-party dependencies without asking first.

---

## 11. Open decisions

1. **RN vs. native SwiftUI for the sampler screen.** RN keeps one codebase but
   gesture latency on the XY pad matters — prototype the XY pad in both at P0 and
   measure touch-to-sound.
2. **Skia SkSL vs. native Metal** for the visualiser — depends on how much of the
   existing GLSL survives transpilation. Test with the Chladni skin first.
3. **Freemium line** — how many pads/FX/skins are free, and whether artist packs
   are always paid or can be card-unlock rewards.
4. **Storage/egress cost** of user captures at scale — set a retention policy for
   unpublished captures before launch.
