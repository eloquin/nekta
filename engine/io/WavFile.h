#pragma once

#include <string>

#include "voice/SampleBuffer.h"

namespace nekta {
namespace wav {

// Offline only - never called from the audio thread.
// Reads 16/24/32-bit PCM and 32/64-bit IEEE float WAV, including
// WAVE_FORMAT_EXTENSIBLE. Channels are kept planar in the SampleBuffer.
bool read(const char* path, SampleBuffer& out, std::string& error);

// Writes 32-bit float WAV, the capture format from SPEC §2.1.
bool writeFloat32(const char* path, const float* left, const float* right, int frames,
                  double sampleRate, std::string& error);

}  // namespace wav
}  // namespace nekta
