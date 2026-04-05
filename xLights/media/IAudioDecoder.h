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

// Abstract audio decoder/encoder interface.
// AudioToolboxDecoder (Apple) and FFmpegAudioDecoder (Linux/Windows) implement this.

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

struct DecodedAudioInfo {
    long sampleRate = 0;       // original file sample rate
    int channels = 0;          // original channel count
    long trackSize = 0;        // total samples per channel at target rate
    long lengthMS = 0;         // duration in milliseconds
    long bitRate = 0;
    int bitsPerSample = 0;
    std::string title;
    std::string artist;
    std::string album;
    std::map<std::string, std::string> metadata;
};

class IAudioDecoder {
public:
    virtual ~IAudioDecoder() = default;

    // Decode an audio file to 16-bit stereo interleaved PCM at targetRate.
    //
    // On success, returns true and populates:
    //   info        - track metadata and metrics
    //   pcmData     - raw interleaved 16-bit stereo PCM (caller takes ownership via free())
    //   pcmDataSize - byte size of pcmData
    //   leftData    - normalized float samples for left channel (caller takes ownership via free())
    //   rightData   - normalized float samples for right channel (may == leftData for mono; caller
    //                 frees only if != leftData)
    //   trackSize   - samples per channel at targetRate
    //
    // The extra parameter adds padding samples to the allocated float buffers (for analysis
    // functions that read past end).
    //
    // progress callback receives percentage 0-100.
    virtual bool DecodeFile(const std::string& path,
                            long targetRate,
                            int extra,
                            DecodedAudioInfo& info,
                            uint8_t*& pcmData, long& pcmDataSize,
                            float*& leftData, float*& rightData,
                            long& trackSize,
                            std::function<void(int pct)> progress = nullptr) = 0;

    // Encode float PCM to an audio file (format determined by filename extension).
    // left/right are normalized float samples. sampleRate is the output rate.
    virtual bool EncodeToFile(const std::vector<float>& left,
                              const std::vector<float>& right,
                              size_t sampleRate,
                              const std::string& filename) = 0;

    // Get the length of an audio file in bytes without fully decoding it.
    virtual size_t GetAudioFileLength(const std::string& filename) = 0;
};
