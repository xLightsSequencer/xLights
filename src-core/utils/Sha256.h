#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

// Self-contained SHA-256 (FIPS 180-4). The bundled dependencies offer no
// portable implementation (FFmpeg's isn't linked everywhere), and the S3
// uploader needs it for signed payloads and the backup manifest.
class Sha256 {
public:
    using Digest = std::array<uint8_t, 32>;

    Sha256();
    void update(const void* data, size_t len);
    Digest finish();

    static std::string ToHex(const Digest& d);
    static std::string HexOf(const void* data, size_t len);
    static std::optional<std::string> HexOfFile(const std::filesystem::path& file);

private:
    void transform(const uint8_t* block);

    uint32_t _state[8];
    uint8_t _buffer[64];
    uint64_t _totalLen = 0;
    size_t _bufferLen = 0;
};
