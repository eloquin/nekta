#!/usr/bin/env python3
"""Synthesises a demo break for the offline harness.

Keeps the repo free of committed binaries: run this once to produce
engine/examples/break.wav, which examples/jungle_roll.nks loads.
Standard library only.
"""

import math
import os
import struct
import wave

SAMPLE_RATE = 48000
BPM = 174.0
BARS = 1
BEATS_PER_BAR = 4


def envelope(index, length, decay):
    return math.exp(-decay * index / length)


def render():
    total = int(SAMPLE_RATE * 60.0 / BPM * BEATS_PER_BAR * BARS)
    out = [0.0] * total
    step = total / 16.0  # sixteenth notes

    # A crude two-step pattern: kick, snare, hats. Enough to slice and roll.
    kicks = [0, 6, 10]
    snares = [4, 12]
    hats = [2, 3, 7, 9, 11, 14, 15]

    def add(start, length, generator):
        for i in range(length):
            pos = start + i
            if 0 <= pos < total:
                out[pos] += generator(i, length)

    seed = 12345

    def noise():
        nonlocal seed
        seed = (1103515245 * seed + 12345) & 0x7FFFFFFF
        return (seed / 0x3FFFFFFF) - 1.0

    for s in kicks:
        start = int(s * step)
        add(start, 4000, lambda i, n: 0.9 * envelope(i, n, 6.0) *
            math.sin(2 * math.pi * (110.0 * math.exp(-4.0 * i / n)) * i / SAMPLE_RATE))
    for s in snares:
        start = int(s * step)
        add(start, 6000, lambda i, n: 0.5 * envelope(i, n, 5.0) * noise() +
            0.3 * envelope(i, n, 8.0) * math.sin(2 * math.pi * 190.0 * i / SAMPLE_RATE))
    for s in hats:
        start = int(s * step)
        add(start, 1200, lambda i, n: 0.25 * envelope(i, n, 12.0) * noise())

    peak = max(abs(v) for v in out) or 1.0
    return [v / peak * 0.89 for v in out]


def main():
    samples = render()
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "break.wav")
    with wave.open(path, "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(SAMPLE_RATE)
        f.writeframes(b"".join(
            struct.pack("<h", max(-32768, min(32767, int(v * 32767)))) for v in samples))
    print("wrote {} ({} frames, {:.2f} s at {} BPM)".format(
        path, len(samples), len(samples) / SAMPLE_RATE, BPM))


if __name__ == "__main__":
    main()
