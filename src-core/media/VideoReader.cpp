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
#include <vector>
#include <cstdio>
#include <mutex>
#include <atomic>

#include <log.h>

// XL_VIDEO_DUMP=<dir>: write the first few frames each reader serves as binary
// PPM, named <basename>_<decoder>_<posMS>.ppm. The point is to compare decoders
// on RAW pixels: an fseq channel is the decoded pixel after crop, scale,
// TransparentBlack, layer blending and node mapping, which is far too many
// layers to attribute a colour difference through. Zero cost when unset.
static void MaybeDumpFrame(const VideoFrame& vf, const std::string& filename, const char* decoder, int posMS, int& dumped) {
    static const char* dir = getenv("XL_VIDEO_DUMP");
    if (dir == nullptr || vf.data == nullptr || dumped >= 3 || vf.width <= 0 || vf.height <= 0) {
        return;
    }
    ++dumped;

    int ch = 3;
    bool bgr = false;
    switch (vf.format) {
        case VideoPixelFormat::RGB24: ch = 3; bgr = false; break;
        case VideoPixelFormat::BGR24: ch = 3; bgr = true;  break;
        case VideoPixelFormat::RGBA:  ch = 4; bgr = false; break;
        case VideoPixelFormat::BGRA:  ch = 4; bgr = true;  break;
        default: return;
    }

    std::string base = std::filesystem::path(filename).stem().string();
    for (auto& c : base) {
        if (c == ' ' || c == '\'' || c == '/' || c == '\\') {
            c = '_';
        }
    }
    // Size is part of the identity: the same file is opened at different
    // render sizes by different effects, and comparing across those is
    // meaningless.
    std::string out = std::string(dir) + "/" + base + "_" + std::to_string(vf.width) + "x" + std::to_string(vf.height) +
                      "_" + std::to_string(posMS) + "_" + decoder + ".ppm";
    FILE* f = fopen(out.c_str(), "wb");
    if (f == nullptr) {
        spdlog::warn("XL_VIDEO_DUMP: cannot write {}", out);
        return;
    }
    fprintf(f, "P6\n%d %d\n255\n", vf.width, vf.height);
    std::vector<uint8_t> row((size_t)vf.width * 3);
    for (int y = 0; y < vf.height; ++y) {
        const uint8_t* src = vf.data + (size_t)y * vf.linesize;
        for (int x = 0; x < vf.width; ++x) {
            uint8_t a = src[(size_t)x * ch + 0];
            uint8_t b = src[(size_t)x * ch + 1];
            uint8_t c = src[(size_t)x * ch + 2];
            row[(size_t)x * 3 + 0] = bgr ? c : a;
            row[(size_t)x * 3 + 1] = b;
            row[(size_t)x * 3 + 2] = bgr ? a : c;
        }
        fwrite(row.data(), 1, row.size(), f);
    }
    fclose(f);
    spdlog::info("XL_VIDEO_DUMP: wrote {} ({}x{}, {})", out, vf.width, vf.height, decoder);
}


// XL_VIDEO_TRACE=<file>: one line per frame served - requested timestamp, the
// position the decoder actually served, and a hash of the pixels. Diffing two
// decoders' traces answers "is each decoder returning the SAME frame for the
// same request", which pixel comparison alone cannot: two decoders can both
// look plausible while silently sitting on different frames.
static void MaybeTraceFrame(const VideoFrame& vf, const std::string& filename, const char* decoder, int requestedMS, int servedMS, int readerId) {
    static const char* path = getenv("XL_VIDEO_TRACE");
    if (path == nullptr) {
        return;
    }
    static std::mutex lk;
    static FILE* out = nullptr;
    std::lock_guard<std::mutex> guard(lk);
    if (out == nullptr) {
        out = fopen(path, "w");
        if (out == nullptr) {
            return;
        }
        fprintf(out, "# decoder\treader\tfile\twidth\theight\trequestedMS\tservedMS\thash\n");
    }
    // FNV-1a over the frame, row by row so stride padding never enters the hash.
    uint64_t h = 1469598103934665603ULL;
    if (vf.data != nullptr) {
        const int bpp = (vf.format == VideoPixelFormat::RGBA || vf.format == VideoPixelFormat::BGRA) ? 4 : 3;
        for (int y = 0; y < vf.height; ++y) {
            const uint8_t* row = vf.data + (size_t)y * vf.linesize;
            for (int i = 0; i < vf.width * bpp; ++i) {
                h = (h ^ row[i]) * 1099511628211ULL;
            }
        }
    }
    std::string base = std::filesystem::path(filename).filename().string();
    fprintf(out, "%s\t%d\t%s\t%d\t%d\t%d\t%d\t%016llx\n",
            decoder, readerId, base.c_str(), vf.width, vf.height, requestedMS, servedMS, (unsigned long long)h);
    fflush(out);
}

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
    static std::atomic<int> sNextReaderId{ 0 };
    _readerId = ++sNextReaderId;

#if TARGET_OS_IPHONE
    // iPad: AVFoundation only
    _impl = new AVFoundationVideoReader(filename, width, height, keepaspectratio,
                                        usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
    _decoderTag = "avf";
#elif defined(__APPLE__)
    // macOS: try AVFoundation first (unless hardware acceleration disabled), fall back to FFmpeg if it fails
    if (FFmpegVideoReader::IsHardwareAcceleratedVideo()) {
        _impl = new AVFoundationVideoReader(filename, width, height, keepaspectratio,
                                            usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
        _decoderTag = "avf";
        if (!_impl->IsValid()) {
            spdlog::info("AVFoundationVideoReader failed for {}, falling back to FFmpeg", filename);
            delete _impl;
            _impl = new FFmpegVideoReader(filename, width, height, keepaspectratio,
                                          usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
            _decoderTag = "ffmpeg";
        }
    } else {
        _impl = new FFmpegVideoReader(filename, width, height, keepaspectratio,
                                      usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
    }
#else
    // Linux / Windows: FFmpeg only (which may internally use the Media
    // Foundation reader - the tag says which actually ran)
    _impl = new FFmpegVideoReader(filename, width, height, keepaspectratio,
                                  usenativeresolution, wantAlpha, bgr, wantsHardwareDecoderType);
    // Renderer 0 (DIRECX11_API) selects the Media Foundation reader nested
    // inside FFmpegVideoReader; anything else is FFmpeg's own decode path.
    const bool mf = IsHardwareAcceleratedVideo() && GetHardwareRenderType() == 0;
    _decoderTag = mf ? (getenv("XL_MF_NO_D3DVP") ? "mfauto" : "mfvp") : "ffmpeg";
#endif
}

VideoReader::~VideoReader()
{
    delete _impl;
    _impl = nullptr;
}

int VideoReader::GetLengthMS() const { return _impl->GetLengthMS(); }
void VideoReader::Seek(int timestampMS, bool readFrame) { _impl->Seek(timestampMS, readFrame); }
VideoFrame* VideoReader::GetNextFrame(int timestampMS, int gracetime) {
    VideoFrame* f = _impl->GetNextFrame(timestampMS, gracetime);
    if (f != nullptr) {
        MaybeDumpFrame(*f, _impl->GetFilename(), _decoderTag, _impl->GetPos(), _dumpedFrames);
        MaybeTraceFrame(*f, _impl->GetFilename(), _decoderTag, timestampMS, _impl->GetPos(), _readerId);
    }
    return f;
}
bool VideoReader::IsValid() const { return _impl->IsValid(); }
int VideoReader::GetWidth() const { return _impl->GetWidth(); }
int VideoReader::GetHeight() const { return _impl->GetHeight(); }
bool VideoReader::AtEnd() const { return _impl->AtEnd(); }
int VideoReader::GetPos() { return _impl->GetPos(); }
std::string VideoReader::GetFilename() const { return _impl->GetFilename(); }
int VideoReader::GetPixelChannels() const { return _impl->GetPixelChannels(); }
bool VideoReader::Resize(int width, int height) { return _impl ? _impl->Resize(width, height) : false; }
bool VideoReader::SupportsFrameIndependentAccess() const { return _impl ? _impl->SupportsFrameIndependentAccess() : false; }
void VideoReader::SetStreamGroup(uint64_t group) { if (_impl) _impl->SetStreamGroup(group); }

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
