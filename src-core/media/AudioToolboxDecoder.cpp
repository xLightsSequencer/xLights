/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Apple-only `IAudioDecoder` shell. The actual AVFoundation /
// AudioToolbox calls live behind `AppleAudioToolboxBridge` in
// `macOS/src-apple-core/media/AudioToolboxBridge.{h,mm}`. The bridge
// tries ExtAudioFile then AVAssetReader; this file owns the FFmpeg
// fallback decision (desktop only — iOS has no FFmpeg linked in).

#ifdef __APPLE__

#include "AudioToolboxDecoder.h"
#include "media/AudioToolboxBridge.h"

#include <TargetConditionals.h>
#if !TARGET_OS_IPHONE
#include "FFmpegAudioDecoder.h"
#endif

#include <log.h>

namespace {

void CopyInfo(const AppleAudioToolboxBridge::DecodedInfo& src, DecodedAudioInfo& dst) {
    dst.sampleRate = src.sampleRate;
    dst.channels = src.channels;
    dst.trackSize = src.trackSize;
    dst.lengthMS = src.lengthMS;
    dst.bitRate = src.bitRate;
    dst.bitsPerSample = src.bitsPerSample;
    dst.title = src.title;
    dst.artist = src.artist;
    dst.album = src.album;
    dst.metadata = src.metadata;
}

} // namespace

bool AudioToolboxDecoder::DecodeFile(const std::string& path,
                                      long targetRate,
                                      int extra,
                                      DecodedAudioInfo& info,
                                      uint8_t*& pcmData, long& pcmDataSize,
                                      float*& leftData, float*& rightData,
                                      long& trackSize,
                                      std::function<void(int pct)> progress) {
    AppleAudioToolboxBridge::DecodedInfo bridgeInfo;
    if (AppleAudioToolboxBridge::DecodeFile(path, targetRate, extra, bridgeInfo,
                                             pcmData, pcmDataSize,
                                             leftData, rightData, trackSize, progress)) {
        CopyInfo(bridgeInfo, info);
        return true;
    }

#if !TARGET_OS_IPHONE
    // Apple decoders couldn't open the file (e.g. AVI). Fall through to FFmpeg
    // on desktop; iOS doesn't link FFmpeg.
    spdlog::debug("AudioToolboxDecoder: Apple decoders failed for {}, falling back to FFmpeg", path);
    FFmpegAudioDecoder ffmpeg;
    return ffmpeg.DecodeFile(path, targetRate, extra, info,
                             pcmData, pcmDataSize,
                             leftData, rightData, trackSize, progress);
#else
    spdlog::warn("AudioToolboxDecoder: Apple decoders failed for {}, no FFmpeg fallback on iOS", path);
    return false;
#endif
}

bool AudioToolboxDecoder::EncodeToFile(const std::vector<float>& left,
                                        const std::vector<float>& right,
                                        size_t sampleRate,
                                        const std::string& filename) {
    return AppleAudioToolboxBridge::EncodeToFile(left, right, sampleRate, filename);
}

size_t AudioToolboxDecoder::GetAudioFileLength(const std::string& filename) {
    return AppleAudioToolboxBridge::GetAudioFileLength(filename);
}

#endif // __APPLE__
