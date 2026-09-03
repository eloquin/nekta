#include "io/WavFile.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace nekta {
namespace wav {
namespace {

struct Reader {
  const std::vector<unsigned char>& bytes;
  std::size_t pos = 0;

  bool have(std::size_t n) const { return pos + n <= bytes.size(); }
  std::uint32_t u32() {
    const std::uint32_t v = static_cast<std::uint32_t>(bytes[pos]) |
                            (static_cast<std::uint32_t>(bytes[pos + 1]) << 8) |
                            (static_cast<std::uint32_t>(bytes[pos + 2]) << 16) |
                            (static_cast<std::uint32_t>(bytes[pos + 3]) << 24);
    pos += 4;
    return v;
  }
  std::uint16_t u16() {
    const std::uint16_t v = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[pos]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[pos + 1]) << 8));
    pos += 2;
    return v;
  }
};

bool readFile(const char* path, std::vector<unsigned char>& out, std::string& error) {
  std::FILE* f = std::fopen(path, "rb");
  if (f == nullptr) {
    error = std::string("cannot open ") + path;
    return false;
  }
  std::fseek(f, 0, SEEK_END);
  const long size = std::ftell(f);
  std::fseek(f, 0, SEEK_SET);
  if (size <= 0) {
    std::fclose(f);
    error = std::string("empty file ") + path;
    return false;
  }
  out.resize(static_cast<std::size_t>(size));
  const std::size_t got = std::fread(out.data(), 1, out.size(), f);
  std::fclose(f);
  if (got != out.size()) {
    error = std::string("short read on ") + path;
    return false;
  }
  return true;
}

float sampleFromBits(const unsigned char* p, int bits, bool isFloat) {
  if (isFloat) {
    if (bits == 32) {
      float v = 0.0f;
      std::memcpy(&v, p, 4);
      return v;
    }
    double v = 0.0;
    std::memcpy(&v, p, 8);
    return static_cast<float>(v);
  }
  switch (bits) {
    case 8:
      return (static_cast<float>(p[0]) - 128.0f) / 128.0f;
    case 16: {
      std::int16_t v = 0;
      std::memcpy(&v, p, 2);
      return static_cast<float>(v) / 32768.0f;
    }
    case 24: {
      std::int32_t v = (static_cast<std::int32_t>(p[0]) << 8) |
                       (static_cast<std::int32_t>(p[1]) << 16) |
                       (static_cast<std::int32_t>(p[2]) << 24);
      return static_cast<float>(v >> 8) / 8388608.0f;
    }
    case 32: {
      std::int32_t v = 0;
      std::memcpy(&v, p, 4);
      return static_cast<float>(static_cast<double>(v) / 2147483648.0);
    }
    default:
      return 0.0f;
  }
}

void putU32(std::vector<unsigned char>& out, std::uint32_t v) {
  out.push_back(static_cast<unsigned char>(v & 0xff));
  out.push_back(static_cast<unsigned char>((v >> 8) & 0xff));
  out.push_back(static_cast<unsigned char>((v >> 16) & 0xff));
  out.push_back(static_cast<unsigned char>((v >> 24) & 0xff));
}

void putU16(std::vector<unsigned char>& out, std::uint16_t v) {
  out.push_back(static_cast<unsigned char>(v & 0xff));
  out.push_back(static_cast<unsigned char>((v >> 8) & 0xff));
}

void putTag(std::vector<unsigned char>& out, const char* tag) {
  for (int i = 0; i < 4; ++i) out.push_back(static_cast<unsigned char>(tag[i]));
}

}  // namespace

bool read(const char* path, SampleBuffer& out, std::string& error) {
  std::vector<unsigned char> bytes;
  if (!readFile(path, bytes, error)) return false;
  if (bytes.size() < 44 || std::memcmp(bytes.data(), "RIFF", 4) != 0 ||
      std::memcmp(bytes.data() + 8, "WAVE", 4) != 0) {
    error = std::string("not a RIFF/WAVE file: ") + path;
    return false;
  }

  Reader r{bytes, 12};
  int channels = 0;
  int bits = 0;
  bool isFloat = false;
  double sampleRate = 48000.0;
  std::size_t dataOffset = 0;
  std::size_t dataSize = 0;

  while (r.have(8)) {
    const std::size_t chunkStart = r.pos;
    char tag[5] = {0};
    std::memcpy(tag, bytes.data() + r.pos, 4);
    r.pos += 4;
    const std::uint32_t size = r.u32();
    const std::size_t next = r.pos + size + (size & 1u);
    if (std::memcmp(tag, "fmt ", 4) == 0 && size >= 16) {
      const std::size_t fmtStart = r.pos;
      std::uint16_t format = r.u16();
      channels = r.u16();
      sampleRate = static_cast<double>(r.u32());
      r.u32();  // byte rate
      r.u16();  // block align
      bits = r.u16();
      if (format == 0xFFFE && size >= 40) {  // WAVE_FORMAT_EXTENSIBLE
        r.pos = fmtStart + 24;              // sub-format GUID starts here
        format = r.u16();
      }
      isFloat = (format == 3);
      if (format != 1 && format != 3) {
        error = "unsupported WAV encoding (only PCM and IEEE float)";
        return false;
      }
    } else if (std::memcmp(tag, "data", 4) == 0) {
      dataOffset = r.pos;
      dataSize = size;
      if (dataOffset + dataSize > bytes.size()) dataSize = bytes.size() - dataOffset;
    }
    // Always advance by the chunk header plus its payload; a chunk consumed
    // exactly (a 16-byte "fmt ") must not end the walk before "data".
    if (next <= chunkStart || next > bytes.size()) break;
    r.pos = next;
  }

  if (channels <= 0 || bits <= 0 || dataSize == 0) {
    error = std::string("no usable audio in ") + path;
    return false;
  }

  const int bytesPerSample = bits / 8;
  const int frames = static_cast<int>(dataSize / static_cast<std::size_t>(bytesPerSample * channels));
  out.setSize(channels, frames);
  out.setSampleRate(sampleRate);
  for (int c = 0; c < channels; ++c) {
    float* dst = out.writePtr(c);
    for (int i = 0; i < frames; ++i) {
      const std::size_t offset =
          dataOffset + (static_cast<std::size_t>(i) * static_cast<std::size_t>(channels) +
                        static_cast<std::size_t>(c)) *
                           static_cast<std::size_t>(bytesPerSample);
      dst[i] = sampleFromBits(bytes.data() + offset, bits, isFloat);
    }
  }
  return true;
}

bool writeFloat32(const char* path, const float* left, const float* right, int frames,
                  double sampleRate, std::string& error) {
  if (frames < 0) frames = 0;
  const std::uint32_t dataBytes = static_cast<std::uint32_t>(frames) * 2u * 4u;
  std::vector<unsigned char> out;
  out.reserve(44 + dataBytes);
  putTag(out, "RIFF");
  putU32(out, 36u + dataBytes);
  putTag(out, "WAVE");
  putTag(out, "fmt ");
  putU32(out, 16);
  putU16(out, 3);  // IEEE float
  putU16(out, 2);  // stereo
  putU32(out, static_cast<std::uint32_t>(sampleRate));
  putU32(out, static_cast<std::uint32_t>(sampleRate) * 2u * 4u);
  putU16(out, 8);   // block align
  putU16(out, 32);  // bits
  putTag(out, "data");
  putU32(out, dataBytes);
  for (int i = 0; i < frames; ++i) {
    const float l = left[i];
    const float r = right[i];
    unsigned char buf[4];
    std::memcpy(buf, &l, 4);
    out.insert(out.end(), buf, buf + 4);
    std::memcpy(buf, &r, 4);
    out.insert(out.end(), buf, buf + 4);
  }

  std::FILE* f = std::fopen(path, "wb");
  if (f == nullptr) {
    error = std::string("cannot write ") + path;
    return false;
  }
  const std::size_t wrote = std::fwrite(out.data(), 1, out.size(), f);
  std::fclose(f);
  if (wrote != out.size()) {
    error = std::string("short write on ") + path;
    return false;
  }
  return true;
}

}  // namespace wav
}  // namespace nekta
