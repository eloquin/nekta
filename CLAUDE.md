@AGENTS.md

# Nekta Sampler engine (`/engine`)

Full spec: `SPEC.md`. §1 (locked architecture) and §8 (testing) are
non-negotiable. The engine is plain C++17 with **zero third-party
dependencies** — ask before adding one.

## Real-time rules (SPEC §1.2) — enforced in review

On the audio thread (anything reachable from `Engine::process`):

- **No allocation.** No `new`/`delete`, no `std::vector` growth, no
  `std::string`, no `std::function`, no `shared_ptr` copies.
- **No locks, no logging, no exceptions, no file or system I/O.**
- Audio <-> UI communication is **lock-free SPSC ring buffers only**
  (`core/RingBuffer.h`) or per-parameter atomics (`params/ParamRegistry.h`).
- **Static DSP graph.** Slots are pre-allocated and bypassed; adding an FX never
  rebuilds the graph.
- Every parameter is either **one-pole smoothed (10–20 ms)** or applied at a
  **sample-accurate event boundary** (the block is split at event times).
- Denormals are flushed in every feedback path (`core/Denormal.h`).

Wrap any audio-thread entry point in `nekta::rt::ScopedAudioThread`. Test builds
define `NEKTA_RT_CHECKS`, which hooks `operator new` and fails the test on any
allocation inside that scope.

## Conventions

- C++17, `namespace nekta`, 2-space indent, `.h`/`.cpp`, headers self-contained.
- Types `PascalCase`, functions/vars `camelCase`, members `trailingUnderscore_`,
  constants `kPascalCase`, macros `NEKTA_UPPER`.
- `prepare(sampleRate, ...)` allocates and precomputes; `reset()` clears state;
  `process*()` is real-time and allocation-free. Never allocate outside
  `prepare`/constructors.
- Processing methods **add** into their output buffers unless named `replace*`.
- Audio is stereo planar `float*`; time is `SampleIndex` (int64 absolute samples)
  or `double` beats — never wall clock.
- Tests first, one suite per module, deterministic (seeded RNG, no wall clock).

## Build

```
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build engine/build -j
ctest --test-dir engine/build --output-on-failure
```

The Next.js site at the repo root is unrelated to the engine; keep them separate.
