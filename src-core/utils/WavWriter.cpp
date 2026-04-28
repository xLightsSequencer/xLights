/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "WavWriter.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace xlights::wav {

namespace {

inline void put_u32_le(uint8_t* p, uint32_t v) {
    p[0] = uint8_t(v & 0xFF);
    p[1] = uint8_t((v >> 8) & 0xFF);
    p[2] = uint8_t((v >> 16) & 0xFF);
    p[3] = uint8_t((v >> 24) & 0xFF);
}

inline void put_u16_le(uint8_t* p, uint16_t v) {
    p[0] = uint8_t(v & 0xFF);
    p[1] = uint8_t((v >> 8) & 0xFF);
}

}

bool WriteStereoFloatWav(const std::string& path,
                         const std::vector<float>& left,
                         const std::vector<float>& right,
                         uint32_t sampleRate)
{
    if (left.size() != right.size()) return false;

    const uint16_t channels = 2;
    const uint16_t bitsPerSample = 32;
    const uint16_t formatTag = 3;  // WAVE_FORMAT_IEEE_FLOAT
    const uint16_t blockAlign = channels * (bitsPerSample / 8);
    const uint32_t byteRate = sampleRate * blockAlign;

    // RIFF/WAVE chunk sizes are 32-bit. Reject buffers that would overflow
    // — caller would need >27h of stereo audio at 44.1kHz to hit this.
    if (left.size() > UINT32_MAX / blockAlign - 36 / blockAlign) return false;

    const uint32_t numFrames = uint32_t(left.size());
    const uint32_t dataBytes = numFrames * blockAlign;

    // RIFF header (12) + fmt chunk (8 + 16) + data chunk header (8) = 44.
    uint8_t hdr[44];
    std::memcpy(hdr + 0,  "RIFF", 4);
    put_u32_le(hdr + 4,   36 + dataBytes);
    std::memcpy(hdr + 8,  "WAVE", 4);
    std::memcpy(hdr + 12, "fmt ", 4);
    put_u32_le(hdr + 16,  16);            // PCM/float fmt subchunk size
    put_u16_le(hdr + 20,  formatTag);
    put_u16_le(hdr + 22,  channels);
    put_u32_le(hdr + 24,  sampleRate);
    put_u32_le(hdr + 28,  byteRate);
    put_u16_le(hdr + 32,  blockAlign);
    put_u16_le(hdr + 34,  bitsPerSample);
    std::memcpy(hdr + 36, "data", 4);
    put_u32_le(hdr + 40,  dataBytes);

    FILE* f = std::fopen(path.c_str(), "wb");
    if (f == nullptr) return false;

    if (std::fwrite(hdr, 1, sizeof(hdr), f) != sizeof(hdr)) {
        std::fclose(f);
        return false;
    }

    // Interleave L/R per frame and write in chunks to keep memory bounded.
    constexpr size_t kFramesPerChunk = 4096;
    float chunk[kFramesPerChunk * 2];
    size_t i = 0;
    while (i < numFrames) {
        size_t n = std::min<size_t>(kFramesPerChunk, numFrames - i);
        for (size_t j = 0; j < n; j++) {
            chunk[j * 2 + 0] = left[i + j];
            chunk[j * 2 + 1] = right[i + j];
        }
        size_t bytes = n * blockAlign;
        if (std::fwrite(chunk, 1, bytes, f) != bytes) {
            std::fclose(f);
            return false;
        }
        i += n;
    }

    std::fclose(f);
    return true;
}

}
