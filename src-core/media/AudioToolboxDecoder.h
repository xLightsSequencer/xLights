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

// AudioToolbox-based audio decoder/encoder for Apple platforms (macOS + iPad).
// Uses ExtAudioFile for decoding (handles all Core Audio formats) and
// ExtAudioFile for encoding.

#include "IAudioDecoder.h"

class AudioToolboxDecoder : public IAudioDecoder {
public:
    AudioToolboxDecoder() = default;
    ~AudioToolboxDecoder() override = default;

    bool DecodeFile(const std::string& path,
                    long targetRate,
                    int extra,
                    DecodedAudioInfo& info,
                    uint8_t*& pcmData, long& pcmDataSize,
                    float*& leftData, float*& rightData,
                    long& trackSize,
                    std::function<void(int pct)> progress = nullptr) override;

    bool EncodeToFile(const std::vector<float>& left,
                      const std::vector<float>& right,
                      size_t sampleRate,
                      const std::string& filename) override;

    size_t GetAudioFileLength(const std::string& filename) override;
};
