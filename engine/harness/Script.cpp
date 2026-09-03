#include "harness/Script.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace nekta {
namespace harness {
namespace {

double toDouble(const std::string& s) { return std::strtod(s.c_str(), nullptr); }
int toInt(const std::string& s) { return static_cast<int>(std::strtol(s.c_str(), nullptr, 10)); }
float toFloat(const std::string& s) { return static_cast<float>(toDouble(s)); }
bool toBool(const std::string& s) { return s == "1" || s == "true" || s == "yes" || s == "on"; }

std::vector<std::string> tokenise(const std::string& line) {
  std::vector<std::string> tokens;
  const std::size_t hash = line.find('#');
  std::istringstream stream(hash == std::string::npos ? line : line.substr(0, hash));
  std::string token;
  while (stream >> token) tokens.push_back(token);
  return tokens;
}

bool parsePad(const std::vector<std::string>& t, std::size_t from, PadConfig& pad,
              std::string& error) {
  for (std::size_t i = from; i + 1 < t.size(); i += 2) {
    const std::string& key = t[i];
    const std::string& value = t[i + 1];
    if (key == "sample") {
      pad.sampleSlot = toInt(value);
    } else if (key == "start") {
      pad.startFrame = toDouble(value);
    } else if (key == "end") {
      pad.endFrame = toDouble(value);
    } else if (key == "pitch") {
      pad.pitchSemitones = toDouble(value);
    } else if (key == "gain") {
      pad.gain = toFloat(value);
    } else if (key == "pan") {
      pad.pan = toFloat(value);
    } else if (key == "choke") {
      pad.chokeGroup = toInt(value);
    } else if (key == "reverse") {
      pad.reverse = toBool(value);
    } else if (key == "loop") {
      pad.loop = toBool(value);
    } else if (key == "attack") {
      pad.env.attackMs = toFloat(value);
    } else if (key == "hold") {
      pad.env.holdMs = toFloat(value);
    } else if (key == "decay") {
      pad.env.decayMs = toFloat(value);
    } else if (key == "sustain") {
      pad.env.sustain = toFloat(value);
    } else if (key == "release") {
      pad.env.releaseMs = toFloat(value);
    } else if (key == "mode") {
      if (value == "oneshot") {
        pad.mode = TriggerMode::OneShot;
      } else if (value == "gate") {
        pad.mode = TriggerMode::Gate;
      } else if (value == "loop") {
        pad.mode = TriggerMode::Loop;
      } else if (value == "toggle") {
        pad.mode = TriggerMode::Toggle;
      } else {
        error = "unknown pad mode: " + value;
        return false;
      }
    } else {
      error = "unknown pad key: " + key;
      return false;
    }
  }
  return true;
}

bool parseRoll(const std::vector<std::string>& t, std::size_t from, RollConfig& roll,
               std::string& error) {
  for (std::size_t i = from; i + 1 < t.size(); i += 2) {
    const std::string& key = t[i];
    const std::string& value = t[i + 1];
    if (key == "pad") {
      roll.padIndex = toInt(value);
    } else if (key == "rate") {
      if (!parseRate(value, roll.rate)) {
        error = "bad rate: " + value;
        return false;
      }
    } else if (key == "to") {
      if (!parseRate(value, roll.ramp.endRate)) {
        error = "bad rate: " + value;
        return false;
      }
      roll.ramp.enabled = true;
    } else if (key == "over") {
      roll.ramp.lengthBeats = toDouble(value);
      roll.ramp.enabled = true;
    } else if (key == "curve") {
      roll.ramp.curve = (value == "exp") ? RampCurve::Exponential : RampCurve::Linear;
    } else if (key == "gate") {
      roll.mods.gate = toFloat(value);
    } else if (key == "decay") {
      roll.mods.ampDecay = toFloat(value);
    } else if (key == "ampcurve") {
      roll.mods.ampCurve = toFloat(value);
    } else if (key == "pitch") {
      roll.mods.pitchDeltaSemis = toDouble(value);
    } else if (key == "wrap") {
      roll.mods.pitchWrapRepeats = toInt(value);
    } else if (key == "reverse") {
      roll.mods.reverseAlternate = toBool(value);
    } else if (key == "walk") {
      roll.mods.startOffsetWalkMs = toDouble(value);
    } else if (key == "pan") {
      roll.mods.panAlternate = true;
      roll.mods.panAmount = toFloat(value);
    } else if (key == "gain") {
      roll.baseGain = toFloat(value);
    } else {
      error = "unknown roll key: " + key;
      return false;
    }
  }
  return true;
}

}  // namespace

bool parseRate(const std::string& token, Rate& out) {
  std::string body = token;
  RateMod mod = RateMod::Straight;
  if (!body.empty() && (body.back() == 't' || body.back() == 'T')) {
    mod = RateMod::Triplet;
    body.pop_back();
  } else if (!body.empty() && (body.back() == 'd' || body.back() == 'D')) {
    mod = RateMod::Dotted;
    body.pop_back();
  }
  const std::size_t slash = body.find('/');
  const std::string divisionText = (slash == std::string::npos) ? body : body.substr(slash + 1);
  if (divisionText.empty()) return false;
  const int division = toInt(divisionText);
  if (division < 1) return false;
  out = Rate(division, mod);
  return true;
}

bool parseScript(const std::string& path, Script& out, std::string& error) {
  std::ifstream file(path);
  if (!file) {
    error = "cannot open script: " + path;
    return false;
  }

  std::string line;
  int lineNumber = 0;
  while (std::getline(file, line)) {
    ++lineNumber;
    const std::vector<std::string> t = tokenise(line);
    if (t.empty()) continue;

    std::string detail;
    auto fail = [&](const std::string& message) {
      error = path + ":" + std::to_string(lineNumber) + ": " + message;
      return false;
    };

    if (t[0][0] == '@') {
      if (t.size() < 2) return fail("timed line needs a command");
      ScriptCommand cmd;
      cmd.beats = toDouble(t[0].substr(1));
      cmd.kind = t[1];
      if (cmd.kind == "trigger" || cmd.kind == "release") {
        if (t.size() < 3) return fail(cmd.kind + " needs a pad index");
        cmd.index = toInt(t[2]);
        cmd.value = (t.size() > 3) ? toFloat(t[3]) : 1.0f;
      } else if (cmd.kind == "param") {
        if (t.size() < 4) return fail("param needs a path and a value");
        cmd.path = t[2];
        cmd.value = toFloat(t[3]);
      } else if (cmd.kind != "rollstart" && cmd.kind != "rollstop") {
        return fail("unknown command: " + cmd.kind);
      }
      out.commands.push_back(cmd);
      continue;
    }

    const std::string& head = t[0];
    if (head == "samplerate" && t.size() > 1) {
      out.sampleRate = toDouble(t[1]);
    } else if (head == "bpm" && t.size() > 1) {
      out.bpm = toDouble(t[1]);
    } else if (head == "beatsperbar" && t.size() > 1) {
      out.beatsPerBar = toInt(t[1]);
    } else if (head == "length" && t.size() > 1) {
      out.lengthBeats = toDouble(t[1]);
    } else if (head == "output" && t.size() > 1) {
      out.output = t[1];
    } else if (head == "sample" && t.size() > 2) {
      const int slot = toInt(t[1]);
      if (slot < 0 || slot >= kMaxSamples) return fail("sample slot out of range");
      if (out.samples.size() <= static_cast<std::size_t>(slot)) {
        out.samples.resize(static_cast<std::size_t>(slot) + 1);
      }
      out.samples[static_cast<std::size_t>(slot)] = t[2];
    } else if (head == "pad" && t.size() > 1) {
      const int index = toInt(t[1]);
      if (index < 0 || index >= kNumPads) return fail("pad index out of range");
      PadConfig pad;
      if (!parsePad(t, 2, pad, detail)) return fail(detail);
      out.pads.emplace_back(index, pad);
    } else if (head == "roll") {
      if (!parseRoll(t, 1, out.roll, detail)) return fail(detail);
    } else if (head == "param" && t.size() > 2) {
      out.initialParams.emplace_back(t[1], toFloat(t[2]));
    } else if (head == "quantise" && t.size() > 1) {
      if (t[1] == "off") {
        out.quantiseLaunch = false;
      } else if (parseRate(t[1], out.launchGrid)) {
        out.quantiseLaunch = true;
      } else {
        return fail("bad quantise grid: " + t[1]);
      }
    } else {
      return fail("unknown directive: " + head);
    }
  }
  return true;
}

}  // namespace harness
}  // namespace nekta
