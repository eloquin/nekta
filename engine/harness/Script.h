#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Engine.h"

namespace nekta {
namespace harness {

// One timed command from a performance script.
struct ScriptCommand {
  double beats = 0.0;
  std::string kind;  // trigger | release | rollstart | rollstop | param
  int index = 0;
  float value = 1.0f;
  std::string path;  // parameter path for "param"
};

struct Script {
  double sampleRate = 48000.0;
  double bpm = 174.0;
  int beatsPerBar = 4;
  double lengthBeats = 8.0;
  std::string output;
  std::vector<std::string> samples;  // slot -> WAV path
  std::vector<std::pair<int, PadConfig>> pads;
  std::vector<std::pair<std::string, float>> initialParams;
  RollConfig roll;
  bool quantiseLaunch = false;
  Rate launchGrid{16};
  std::vector<ScriptCommand> commands;
};

// "1/16", "1/16t" (triplet), "1/8d" (dotted), or a bare division like "16".
bool parseRate(const std::string& token, Rate& out);

bool parseScript(const std::string& path, Script& out, std::string& error);

}  // namespace harness
}  // namespace nekta
