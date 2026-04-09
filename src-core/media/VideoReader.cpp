/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoReader.h"

#if TARGET_OS_IPHONE
// iPad: AVFoundation only, no FFmpeg
#include "AVFoundationVideoReader.h"
#else
// macOS / Linux / Windows: FFmpeg always available
#include "FFmpegVideoReader.h"
#if defined(__APPLE__)
// macOS: also compile AVFoundation path for runtime selection
#include "AVFoundationVideoReader.h"
#endif
#endif

#include <algorithm>
#include <filesystem>

#include <log.h>

bool VideoReader::IsVideoFile(const std::string& filename)
{
    auto ext = std::filesystem::path(filename).extension().string();
    if (!ext.empty() && ext[0] == '.') ext.erase(0, 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (ext == "avi" ||
        ext == "mp4" ||
        ext == "mkv" ||
        ext == "mov" ||
        ext == "asf" ||
        ext == "flv" ||
        ext == "mpg" ||
        ext == "wmv" ||
        ext == "mpeg" ||
        ext == "m4v"
        )
    {
        return true;
    }

    return false;
}

long VideoReader::GetVideoLength(const std::string& filename)
{
#if TARGET_OS_IPHONE
    return AVFoundationVideoReader::GetVideoLengthStatic(filename);
#elif defined(__APPLE__)
    // On macOS, try AVFoundation first (faster, no FFmpeg init overhead)
    // unless hardware acceleration is disabled
    if (FFmpegVideoReader::IsHardwareAcceleratedVideo()) {
        long len = AVFoundationVideoReader::GetVideoLengthStatic(filename);
        if (len > 0) return len;
    }
    return FFmpegVideoReader::GetVideoLengthStatic(filename);
#else
    return FFmpegVideoReader::GetVideoLengthStatic(filename);
#endif
}

VideoReader::VideoReader(const std::string& filename, int width, int height, bool keepaspectratio,
                         bool usenativeresolution, bool wantAlpha, bool bgr, bool wantsHardwareDecoderType)
{
#if TARGET_OS_IPHONE
    // iPad: AVFoundation only
    _impl = new AVFoundationVideoReader(filename, width, height, keepaspectratio,
                                        usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
#elif defined(__APPLE__)
    // macOS: try AVFoundation first (unless hardware acceleration disabled), fall back to FFmpeg if it fails
    if (FFmpegVideoReader::IsHardwareAcceleratedVideo()) {
        _impl = new AVFoundationVideoReader(filename, width, height, keepaspectratio,
                                            usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
        if (!_impl->IsValid()) {
            spdlog::info("AVFoundationVideoReader failed for {}, falling back to FFmpeg", filename);
            delete _impl;
            _impl = new FFmpegVideoReader(filename, width, height, keepaspectratio,
                                          usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
        }
    } else {
        _impl = new FFmpegVideoReader(filename, width, height, keepaspectratio,
                                      usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
    }
#else
    // Linux / Windows: FFmpeg only
    _impl = new FFmpegVideoReader(filename, width, height, keepaspectratio,
                                  usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
#endif
}

VideoReader::~VideoReader()
{
    delete _impl;
    _impl = nullptr;
}

int VideoReader::GetLengthMS() const { return _impl->GetLengthMS(); }
void VideoReader::Seek(int timestampMS, bool readFrame) { _impl->Seek(timestampMS, readFrame); }
VideoFrame* VideoReader::GetNextFrame(int timestampMS, int gracetime) { return _impl->GetNextFrame(timestampMS, gracetime); }
bool VideoReader::IsValid() const { return _impl->IsValid(); }
int VideoReader::GetWidth() const { return _impl->GetWidth(); }
int VideoReader::GetHeight() const { return _impl->GetHeight(); }
bool VideoReader::AtEnd() const { return _impl->AtEnd(); }
int VideoReader::GetPos() { return _impl->GetPos(); }
std::string VideoReader::GetFilename() const { return _impl->GetFilename(); }
int VideoReader::GetPixelChannels() const { return _impl->GetPixelChannels(); }

// Static methods delegate to FFmpeg on platforms that have it, no-ops on iPad
#if TARGET_OS_IPHONE
void VideoReader::SetHardwareAcceleratedVideo(bool) {}
void VideoReader::SetHardwareRenderType(int) {}
bool VideoReader::IsHardwareAcceleratedVideo() { return true; }
int VideoReader::GetHardwareRenderType() { return 0; }
void VideoReader::InitHWAcceleration() {}
#else
void VideoReader::SetHardwareAcceleratedVideo(bool accel) { FFmpegVideoReader::SetHardwareAcceleratedVideo(accel); }
void VideoReader::SetHardwareRenderType(int type) { FFmpegVideoReader::SetHardwareRenderType(type); }
bool VideoReader::IsHardwareAcceleratedVideo() { return FFmpegVideoReader::IsHardwareAcceleratedVideo(); }
int VideoReader::GetHardwareRenderType() { return FFmpegVideoReader::GetHardwareRenderType(); }
void VideoReader::InitHWAcceleration() { FFmpegVideoReader::InitHWAcceleration(); }
#endif
