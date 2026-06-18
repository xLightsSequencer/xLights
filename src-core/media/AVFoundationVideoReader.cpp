/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Apple-only `VideoReaderImpl` shell. All AVFoundation /
// VideoToolbox / CoreImage / Accelerate machinery lives behind
// `AppleAVFoundationVideoBridge` in
// `macOS/src-apple-core/media/AVFoundationVideoBridge.{h,mm}`.

#ifdef __APPLE__

#include "AVFoundationVideoReader.h"
#include "media/AVFoundationVideoBridge.h"

namespace {

VideoPixelFormat ToCorePixelFormat(AppleAVFoundationVideoBridge::PixelFormat f) {
    using BridgeFmt = AppleAVFoundationVideoBridge::PixelFormat;
    switch (f) {
        case BridgeFmt::RGB24: return VideoPixelFormat::RGB24;
        case BridgeFmt::BGR24: return VideoPixelFormat::BGR24;
        case BridgeFmt::RGBA:  return VideoPixelFormat::RGBA;
        case BridgeFmt::BGRA:  return VideoPixelFormat::BGRA;
    }
    return VideoPixelFormat::RGB24;
}

AppleAVFoundationVideoBridge::ScaleAlgorithm ToBridgeScaleAlgorithm(VideoScaleAlgorithm a) {
    using BridgeAlg = AppleAVFoundationVideoBridge::ScaleAlgorithm;
    switch (a) {
        case VideoScaleAlgorithm::Default: return BridgeAlg::Default;
        case VideoScaleAlgorithm::Bicubic: return BridgeAlg::Bicubic;
        case VideoScaleAlgorithm::Lanczos: return BridgeAlg::Lanczos;
        case VideoScaleAlgorithm::Area:    return BridgeAlg::Area;
        case VideoScaleAlgorithm::Point:   return BridgeAlg::Point;
    }
    return BridgeAlg::Default;
}

} // namespace

AVFoundationVideoReader::AVFoundationVideoReader(const std::string& filename, int maxwidth, int maxheight,
                                                  bool keepaspectratio, bool usenativeresolution,
                                                  bool wantAlpha, bool bgr, bool wantsHWType)
    : _filename(filename) {
    _bridge = AppleAVFoundationVideoBridge::CreateReader(filename, maxwidth, maxheight,
                                                          keepaspectratio, usenativeresolution,
                                                          wantAlpha, bgr, wantsHWType);
}

AVFoundationVideoReader::~AVFoundationVideoReader() {
    AppleAVFoundationVideoBridge::DestroyReader(_bridge);
    _bridge = nullptr;
}

int AVFoundationVideoReader::GetLengthMS() const {
    return AppleAVFoundationVideoBridge::GetLengthMS(_bridge);
}

bool AVFoundationVideoReader::IsValid() const {
    return AppleAVFoundationVideoBridge::IsValid(_bridge);
}

int AVFoundationVideoReader::GetWidth() const {
    return AppleAVFoundationVideoBridge::GetWidth(_bridge);
}

int AVFoundationVideoReader::GetHeight() const {
    return AppleAVFoundationVideoBridge::GetHeight(_bridge);
}

bool AVFoundationVideoReader::AtEnd() const {
    return AppleAVFoundationVideoBridge::AtEnd(_bridge);
}

int AVFoundationVideoReader::GetPos() {
    return AppleAVFoundationVideoBridge::GetPos(_bridge);
}

std::string AVFoundationVideoReader::GetFilename() const {
    return _filename;
}

int AVFoundationVideoReader::GetPixelChannels() const {
    return AppleAVFoundationVideoBridge::GetPixelChannels(_bridge);
}

bool AVFoundationVideoReader::Resize(int width, int height) {
    return AppleAVFoundationVideoBridge::Resize(_bridge, width, height);
}

void AVFoundationVideoReader::SetScaleAlgorithm(VideoScaleAlgorithm algorithm) {
    AppleAVFoundationVideoBridge::SetScaleAlgorithm(_bridge, ToBridgeScaleAlgorithm(algorithm));
}

void AVFoundationVideoReader::Seek(int timestampMS, bool readFrame) {
    AppleAVFoundationVideoBridge::Seek(_bridge, timestampMS, readFrame);
}

VideoFrame* AVFoundationVideoReader::GetNextFrame(int timestampMS, int gracetime) {
    const auto* view = AppleAVFoundationVideoBridge::GetNextFrame(_bridge, timestampMS, gracetime);
    if (!view) return nullptr;

    _frameOut.data = view->data;
    _frameOut.nativeHandle = nullptr;
    _frameOut.linesize = view->linesize;
    _frameOut.width = view->width;
    _frameOut.height = view->height;
    _frameOut.format = ToCorePixelFormat(view->format);
    return &_frameOut;
}

long AVFoundationVideoReader::GetVideoLengthStatic(const std::string& filename) {
    return AppleAVFoundationVideoBridge::GetVideoLengthStatic(filename);
}

#endif // __APPLE__
