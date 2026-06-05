/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MediaCompatibility.h"

#ifdef __APPLE__
#include "media/MediaCompatibilityBridge.h"
#else
// Non-Apple platforms: approximate AVFoundation's compatibility rules using
// FFmpeg's demuxer/codec metadata. AVFoundation only reliably reads MOV/MP4
// containers with a small set of video codecs; this catches the common
// Mac-incompatible cases (AVI, WMV, legacy QuickTime codecs, animated GIFs,
// Matroska/WebM) without needing AVFoundation itself.
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <set>
#endif

std::string MediaCompatibility::CheckVideoFile(const std::string& filePath)
{
#ifdef __APPLE__
    return AppleMediaCompatibility::CheckVideoFile(filePath);
#else
    if (filePath.empty()) return "";

    AVFormatContext* fmt = nullptr;
    if (avformat_open_input(&fmt, filePath.c_str(), nullptr, nullptr) < 0 || fmt == nullptr) {
        return "Cannot open file";
    }
    struct CtxGuard {
        AVFormatContext** p;
        ~CtxGuard() { if (*p) avformat_close_input(p); }
    } guard{&fmt};

    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        return "Cannot read stream info";
    }

    // Container check — FFmpeg's demuxer name is often a comma-separated
    // list (e.g. "mov,mp4,m4a,3gp,3g2,mj2"), so a substring search is enough.
    std::string names = (fmt->iformat && fmt->iformat->name) ? fmt->iformat->name : "";
    bool containerOK = false;
    for (const char* n : {"mov", "mp4", "m4v"}) {
        if (names.find(n) != std::string::npos) {
            containerOK = true;
            break;
        }
    }
    if (!containerOK) {
        return "Container '" + names + "' not supported";
    }

    int vIdx = -1;
    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
        if (fmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vIdx = static_cast<int>(i);
            break;
        }
    }
    if (vIdx < 0) return "No video tracks found";

    AVCodecID id = fmt->streams[vIdx]->codecpar->codec_id;
    static const std::set<int> kSupported = {
        AV_CODEC_ID_H264,
        AV_CODEC_ID_HEVC,
        AV_CODEC_ID_PRORES,
        AV_CODEC_ID_MJPEG,
        AV_CODEC_ID_RAWVIDEO,
        AV_CODEC_ID_MPEG4,
        AV_CODEC_ID_VP9,
        AV_CODEC_ID_AV1,
    };
    if (kSupported.find(static_cast<int>(id)) == kSupported.end()) {
        const char* cname = avcodec_get_name(id);
        return std::string("Video codec '") + (cname ? cname : "unknown") +
               "' not supported";
    }
    return "";
#endif
}

std::string MediaCompatibility::CheckAudioFile(const std::string& filePath)
{
#ifdef __APPLE__
    return AppleMediaCompatibility::CheckAudioFile(filePath);
#else
    (void)filePath;
    return "";
#endif
}

std::vector<MediaCompatibilityIssue> MediaCompatibility::CheckSequenceMedia(
    const std::string& audioFile,
    const std::vector<std::string>& videoFiles)
{
    std::vector<MediaCompatibilityIssue> issues;

    if (!audioFile.empty()) {
        std::string reason = CheckAudioFile(audioFile);
        if (!reason.empty()) {
            issues.push_back({audioFile, reason, false});
        }
    }

    for (const auto& videoFile : videoFiles) {
        if (videoFile.empty()) continue;
        std::string reason = CheckVideoFile(videoFile);
        if (!reason.empty()) {
            issues.push_back({videoFile, reason, true});
        }
    }

    return issues;
}
