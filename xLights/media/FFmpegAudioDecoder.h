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

// FFmpeg-based audio decoder/encoder.  Used on Linux and Windows.
// On Apple platforms, AudioToolboxDecoder is preferred.

#include "IAudioDecoder.h"

class FFmpegAudioDecoder : public IAudioDecoder {
public:
    FFmpegAudioDecoder() = default;
    ~FFmpegAudioDecoder() override = default;

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
