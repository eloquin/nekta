# Nekta Sampler engine

Standalone C++17 audio engine for Nekta Sampler. No platform dependencies, no
third-party libraries — it builds and runs from the command line, so the whole
engine is testable before any iOS code exists (SPEC §10).

Read `../SPEC.md` first. §1.2 (real-time rules) and §8 (testing) are
non-negotiable; `../CLAUDE.md` restates them alongside the coding conventions.

## Build and test

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build engine/build -j
ctest --test-dir engine/build --output-on-failure
```

`NEKTA_RT_CHECKS` (on by default) replaces `operator new` and counts any
allocation made while a thread is inside `rt::ScopedAudioThread`. Every test and
the render harness run under that guard, and a violation fails the run.

## Offline render harness

```bash
python3 engine/examples/make_demo_wav.py          # generates examples/break.wav
engine/build/nekta_render engine/examples/jungle_roll.nks -o roll.wav
```

It prints peak/RMS, event and drop counts, the realtime factor, and the
audio-thread allocation count, and exits non-zero if anything allocated.

### Script format

Blank lines and everything after `#` are ignored. Setup directives come first;
timed commands start with `@<beats>` (decimal beats from the start).

| Directive | Meaning |
|---|---|
| `samplerate <hz>` | render rate (default 48000) |
| `bpm <n>` / `beatsperbar <n>` | transport, 60–200 BPM |
| `length <beats>` | render length |
| `output <file>` | default output path (`-o` overrides) |
| `sample <slot> <path.wav>` | loads a WAV into a sample slot |
| `pad <index> <key> <value> …` | `sample start end pitch gain pan choke reverse loop mode attack hold decay sustain release` |
| `roll <key> <value> …` | `pad rate to over curve gate decay ampcurve pitch wrap reverse walk pan gain` |
| `param <path> <value>` | initial value for any registry path |
| `quantise <rate\|off>` | launch quantise grid |

Timed commands: `trigger <pad> [vel]`, `release <pad>`, `rollstart`,
`rollstop`, `param <path> <value>`.

Rates are written `1/16`, `1/16t` (triplet), `1/8d` (dotted), or a bare `32`.

## Layout

| Directory | Contents |
|---|---|
| `core/` | RT primitives: SPSC ring buffer, allocation guard, denormal guard |
| `params/` | string-path parameter registry, one-pole smoothing, macro curves |
| `clock/` | sample-accurate transport, rates, quantiser, grid iterator |
| `voice/` | sample buffer, Hermite interpolation, AHDSR, sampler voice, voice pool |
| `roll/` | retrigger with rate ramps and per-repeat modifiers |
| `dsp/` | TPT state variable filter |
| `io/` | WAV read/write (offline only) |
| `harness/` | performance script parser and the `nekta_render` CLI |
| `test/` | one suite per module plus render and fuzz suites |

`Engine.{h,cpp}` assembles the P0 signal path: pads → 16 voices → SVF insert →
master gain, with events applied at sample-accurate block boundaries.

## What the tests cover (SPEC §8)

| Requirement | Where |
|---|---|
| Offline render determinism | `test_render.cpp` — two identical renders are bit-identical, and block size never changes a sample |
| Null test | `test_svf.cpp` (bypassed filter is bit-identical) and `test_render.cpp` (bypassed chain reproduces the sample) |
| Allocation guard | every suite; `test_params.cpp` also asserts the guard itself fires |
| Roll timing within ±1 sample | `test_clock.cpp` (grid, 60–200 BPM × 10 rates), `test_roll.cpp` (including ramps, against numerical integration), `test_render.cpp` (onsets in the rendered audio) |
| Fuzz: no NaN/inf/denormal | `test_svf.cpp`, `test_fuzz.cpp` |

## Not done yet

- **Round-trip latency measurement (SPEC §1.2, §7).** Needs a device runner with
  loopback hardware; nothing in this repo can measure it. The harness reports an
  offline realtime factor instead, which bounds CPU cost but is not RTL.
- **iOS host, JSI bindings, RN app, visualiser, marketplace.** Out of P0 scope by
  §10; the engine is deliberately host-free.
- **Preset round-trip test (SPEC §8).** Waits on `session/` serialisation, which
  is a P1 module.
- **Buffer glitch (§2.4B), chop patterns (§2.4C), the remaining FX modules,
  onset detection and slicing.** P1.
