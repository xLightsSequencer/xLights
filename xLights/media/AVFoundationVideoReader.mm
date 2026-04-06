/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef __APPLE__

#include "AVFoundationVideoReader.h"
#include <log.h>

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <VideoToolbox/VideoToolbox.h>
#import <CoreImage/CoreImage.h>

#include <algorithm>

// Internal ObjC++ implementation struct
// __strong is explicit to ensure ARC retains ObjC objects stored in this C++ struct
struct AVFoundationVideoReaderImpl {
    __strong AVAsset* asset = nil;
    __strong AVAssetTrack* videoTrack = nil;
    __strong AVAssetReader* reader = nil;
    __strong AVAssetReaderTrackOutput* trackOutput = nil;
    VTPixelTransferSessionRef transferSession = nullptr;

    std::string filename;
    bool valid = false;
    bool atEnd = false;
    bool failed = false;
    bool wantAlpha = false;
    bool bgr = false;
    bool wantsHWType = false;

    int width = 0;          // output width
    int height = 0;         // output height
    int nativeWidth = 0;    // source width
    int nativeHeight = 0;   // source height
    double lengthMS = 0;
    long frames = 0;
    int frameMS = 50;
    int curPos = -1000;
    int firstFramePos = -1;

    VideoPixelFormat outputFormat = VideoPixelFormat::RGB24;

    // Double-buffered output frames (current + previous)
    uint8_t* frameBuffer1 = nullptr;
    uint8_t* frameBuffer2 = nullptr;
    int frameBufferSize = 0;

    VideoFrame videoFrame1;
    VideoFrame videoFrame2;
    bool frame1IsCurrent = true; // which buffer is the "current" frame

    // Scaled pixel buffer (reused across frames)
    CVPixelBufferRef scaledPixelBuffer = nullptr;

    VideoFrame& currentFrame() { return frame1IsCurrent ? videoFrame1 : videoFrame2; }
    VideoFrame& prevFrame() { return frame1IsCurrent ? videoFrame2 : videoFrame1; }
    uint8_t* currentBuffer() { return frame1IsCurrent ? frameBuffer1 : frameBuffer2; }
    void swapFrames() { frame1IsCurrent = !frame1IsCurrent; }

    ~AVFoundationVideoReaderImpl() {
        closeReader();
        if (transferSession) {
            VTPixelTransferSessionInvalidate(transferSession);
            CFRelease(transferSession);
            transferSession = nullptr;
        }
        if (scaledPixelBuffer) {
            CVPixelBufferRelease(scaledPixelBuffer);
            scaledPixelBuffer = nullptr;
        }
        if (frameBuffer1) { free(frameBuffer1); frameBuffer1 = nullptr; }
        if (frameBuffer2) { free(frameBuffer2); frameBuffer2 = nullptr; }
        // ARC handles ObjC objects
    }

    void closeReader() {
        if (reader) {
            [reader cancelReading];
            reader = nil;
        }
        trackOutput = nil;
    }

    bool openReader(CMTime startTime) {
        closeReader();

        NSError* error = nil;
        reader = [[AVAssetReader alloc] initWithAsset:asset error:&error];
        if (!reader || error) {
            spdlog::error("AVFoundationVideoReader: Failed to create AVAssetReader: {}",
                         error ? [[error localizedDescription] UTF8String] : "unknown error");
            return false;
        }

        // Request BGRA output — native format for Apple GPUs, minimal conversion overhead
        NSDictionary* outputSettings = @{
            (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
        };
        trackOutput = [[AVAssetReaderTrackOutput alloc] initWithTrack:videoTrack
                                                       outputSettings:outputSettings];
        trackOutput.alwaysCopiesSampleData = NO;

        if (![reader canAddOutput:trackOutput]) {
            spdlog::error("AVFoundationVideoReader: Cannot add track output to reader");
            reader = nil;
            trackOutput = nil;
            return false;
        }
        [reader addOutput:trackOutput];

        // Set time range from startTime to end
        CMTime duration = asset.duration;
        CMTime rangeStart = startTime;
        CMTime rangeEnd = CMTimeSubtract(duration, startTime);
        if (CMTimeCompare(rangeEnd, kCMTimeZero) <= 0) {
            rangeEnd = kCMTimeZero;
        }
        reader.timeRange = CMTimeRangeMake(rangeStart, rangeEnd);

        if (![reader startReading]) {
            spdlog::error("AVFoundationVideoReader: Failed to start reading: {}",
                         reader.error ? [[reader.error localizedDescription] UTF8String] : "unknown");
            reader = nil;
            trackOutput = nil;
            return false;
        }
        return true;
    }

    bool ensureTransferSession() {
        if (transferSession) return true;

        OSStatus status = VTPixelTransferSessionCreate(kCFAllocatorDefault, &transferSession);
        if (status != noErr) {
            spdlog::error("AVFoundationVideoReader: VTPixelTransferSessionCreate failed: {}", (int)status);
            return false;
        }
        return true;
    }

    bool ensureScaledPixelBuffer() {
        if (scaledPixelBuffer) {
            // Check dimensions match
            size_t w = CVPixelBufferGetWidth(scaledPixelBuffer);
            size_t h = CVPixelBufferGetHeight(scaledPixelBuffer);
            if ((int)w == width && (int)h == height) return true;
            CVPixelBufferRelease(scaledPixelBuffer);
            scaledPixelBuffer = nullptr;
        }

        NSDictionary* attrs = @{
            (NSString*)kCVPixelBufferIOSurfacePropertiesKey: @{}
        };
        CVReturn ret = CVPixelBufferCreate(kCFAllocatorDefault,
                                           width, height,
                                           kCVPixelFormatType_32BGRA,
                                           (__bridge CFDictionaryRef)attrs,
                                           &scaledPixelBuffer);
        if (ret != kCVReturnSuccess) {
            spdlog::error("AVFoundationVideoReader: CVPixelBufferCreate failed: {}", (int)ret);
            return false;
        }
        return true;
    }

    // Copy pixel data from a CVPixelBuffer into the current frame buffer,
    // performing BGRA→target format conversion
    void copyPixelBufferToFrame(CVPixelBufferRef pixelBuffer) {
        CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

        uint8_t* src = (uint8_t*)CVPixelBufferGetBaseAddress(pixelBuffer);
        size_t srcStride = CVPixelBufferGetBytesPerRow(pixelBuffer);
        size_t srcWidth = CVPixelBufferGetWidth(pixelBuffer);
        size_t srcHeight = CVPixelBufferGetHeight(pixelBuffer);

        uint8_t* dst = currentBuffer();
        int channels = wantAlpha ? 4 : 3;
        int dstStride = width * channels;

        int copyWidth = std::min((int)srcWidth, width);
        int copyHeight = std::min((int)srcHeight, height);

        if (wantAlpha) {
            if (bgr) {
                // BGRA → BGRA: direct copy
                for (int y = 0; y < copyHeight; y++) {
                    memcpy(dst + y * dstStride, src + y * srcStride, copyWidth * 4);
                }
            } else {
                // BGRA → RGBA: swap R and B
                for (int y = 0; y < copyHeight; y++) {
                    uint8_t* s = src + y * srcStride;
                    uint8_t* d = dst + y * dstStride;
                    for (int x = 0; x < copyWidth; x++) {
                        d[0] = s[2]; // R
                        d[1] = s[1]; // G
                        d[2] = s[0]; // B
                        d[3] = s[3]; // A
                        s += 4;
                        d += 4;
                    }
                }
            }
        } else {
            if (bgr) {
                // BGRA → BGR24: drop alpha
                for (int y = 0; y < copyHeight; y++) {
                    uint8_t* s = src + y * srcStride;
                    uint8_t* d = dst + y * dstStride;
                    for (int x = 0; x < copyWidth; x++) {
                        d[0] = s[0]; // B
                        d[1] = s[1]; // G
                        d[2] = s[2]; // R
                        s += 4;
                        d += 3;
                    }
                }
            } else {
                // BGRA → RGB24: swap R/B and drop alpha
                for (int y = 0; y < copyHeight; y++) {
                    uint8_t* s = src + y * srcStride;
                    uint8_t* d = dst + y * dstStride;
                    for (int x = 0; x < copyWidth; x++) {
                        d[0] = s[2]; // R
                        d[1] = s[1]; // G
                        d[2] = s[0]; // B
                        s += 4;
                        d += 3;
                    }
                }
            }
        }

        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

        // Update the current VideoFrame
        VideoFrame& vf = currentFrame();
        vf.data = dst;
        vf.linesize = dstStride;
        vf.width = width;
        vf.height = height;
        vf.format = outputFormat;
        vf.nativeHandle = nullptr;
    }

    // Decode the next sample, scale it, and populate the current frame buffer.
    // Returns true if a frame was decoded.
    bool decodeNextFrame() {
        if (!reader) {
            return false;
        }
        AVAssetReaderStatus readerStatus = reader.status;
        if (readerStatus != AVAssetReaderStatusReading) {
            if (readerStatus == AVAssetReaderStatusFailed) {
                spdlog::error("AVFoundationVideoReader: Reader failed: {}",
                             reader.error ? [[reader.error localizedDescription] UTF8String] : "unknown");
            }
            atEnd = true;
            return false;
        }

        @autoreleasepool {
            CMSampleBufferRef sampleBuffer = nullptr;

            @try {
                sampleBuffer = [trackOutput copyNextSampleBuffer];
            } @catch (NSException* exception) {
                spdlog::error("AVFoundationVideoReader: Exception in copyNextSampleBuffer: {} - {}",
                             [[exception name] UTF8String], [[exception reason] UTF8String]);
                failed = true;
                return false;
            }

            if (!sampleBuffer) {
                if (reader.status != AVAssetReaderStatusReading) {
                    atEnd = true;
                }
                return false;
            }

            // Validate the sample buffer before accessing it
            if (!CMSampleBufferIsValid(sampleBuffer)) {
                spdlog::warn("AVFoundationVideoReader: Invalid sample buffer received");
                CFRelease(sampleBuffer);
                return false;
            }

            // Get presentation timestamp
            CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
            if (CMTIME_IS_VALID(pts)) {
                curPos = (int)(CMTimeGetSeconds(pts) * 1000.0);
            }
            if (firstFramePos == -1) {
                firstFramePos = curPos;
            }

            CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
            if (!imageBuffer) {
                CFRelease(sampleBuffer);
                return false;
            }

            bool needsScale = ((int)CVPixelBufferGetWidth(imageBuffer) != width ||
                               (int)CVPixelBufferGetHeight(imageBuffer) != height);

            if (needsScale) {
                // Hardware-accelerated scaling via VTPixelTransferSession
                if (ensureTransferSession() && ensureScaledPixelBuffer()) {
                    OSStatus xferStatus = VTPixelTransferSessionTransferImage(transferSession,
                                                                              imageBuffer,
                                                                              scaledPixelBuffer);
                    if (xferStatus == noErr) {
                        swapFrames();
                        copyPixelBufferToFrame(scaledPixelBuffer);
                        CFRelease(sampleBuffer);
                        return true;
                    } else {
                        spdlog::warn("AVFoundationVideoReader: VTPixelTransferSession failed ({}), falling back to unscaled", (int)xferStatus);
                    }
                }
            }

            // No scaling needed (or scaling failed) — copy directly from decoded buffer
            swapFrames();
            copyPixelBufferToFrame(imageBuffer);
            CFRelease(sampleBuffer);
            return true;
        }
    }
};


// --- AVFoundationVideoReader public methods ---

AVFoundationVideoReader::AVFoundationVideoReader(const std::string& filename, int maxwidth, int maxheight,
                                                  bool keepaspectratio, bool usenativeresolution,
                                                  bool wantAlpha, bool bgr, bool wantsHWType)
{
    _impl = new AVFoundationVideoReaderImpl();
    _impl->filename = filename;
    _impl->wantAlpha = wantAlpha;
    _impl->bgr = bgr;
    _impl->wantsHWType = wantsHWType;

    // Determine output pixel format
    if (wantAlpha) {
        _impl->outputFormat = bgr ? VideoPixelFormat::BGRA : VideoPixelFormat::RGBA;
    } else {
        _impl->outputFormat = bgr ? VideoPixelFormat::BGR24 : VideoPixelFormat::RGB24;
    }

    {
        // Open the asset — use AVURLAsset for explicit control over loading
        NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]];
        AVURLAsset* urlAsset = [AVURLAsset URLAssetWithURL:url options:nil];
        _impl->asset = urlAsset;
        if (!_impl->asset) {
            spdlog::error("AVFoundationVideoReader: Failed to create AVAsset for {}", filename);
            return;
        }

        // Get video tracks — tracksWithMediaType: triggers synchronous loading
        NSArray<AVAssetTrack*>* tracks = [_impl->asset tracksWithMediaType:AVMediaTypeVideo];
        if (tracks.count == 0) {
            spdlog::error("AVFoundationVideoReader: No video tracks in {}", filename);
            return;
        }
        _impl->videoTrack = tracks[0];

        // Get source dimensions
        CGSize naturalSize = _impl->videoTrack.naturalSize;

        // Apply any transform (rotation) to get correct dimensions
        CGAffineTransform transform = _impl->videoTrack.preferredTransform;
        CGSize transformedSize = CGSizeApplyAffineTransform(naturalSize, transform);
        _impl->nativeWidth = (int)fabs(transformedSize.width);
        _impl->nativeHeight = (int)fabs(transformedSize.height);

        if (_impl->nativeWidth == 0 || _impl->nativeHeight == 0) {
            spdlog::error("AVFoundationVideoReader: Invalid video dimensions for {}", filename);
            return;
        }

        // Calculate output dimensions
        if (usenativeresolution) {
            _impl->width = _impl->nativeWidth;
            _impl->height = _impl->nativeHeight;
        } else if (keepaspectratio) {
            float shrink = std::min((float)maxwidth / (float)_impl->nativeWidth,
                                    (float)maxheight / (float)_impl->nativeHeight);
            _impl->width = (int)((float)_impl->nativeWidth * shrink);
            _impl->height = (int)((float)_impl->nativeHeight * shrink);
        } else {
            _impl->width = maxwidth;
            _impl->height = maxheight;
        }

        // Get duration
        CMTime duration = _impl->asset.duration;
        _impl->lengthMS = CMTimeGetSeconds(duration) * 1000.0;

        // Estimate frame count and frame duration
        float nominalFrameRate = _impl->videoTrack.nominalFrameRate;
        if (nominalFrameRate > 0) {
            _impl->frames = (long)((_impl->lengthMS / 1000.0) * nominalFrameRate);
            _impl->frameMS = (int)(1000.0 / nominalFrameRate);
        } else {
            // Fallback
            CMTime minFrameDuration = _impl->videoTrack.minFrameDuration;
            if (CMTIME_IS_VALID(minFrameDuration) && CMTimeGetSeconds(minFrameDuration) > 0) {
                double fps = 1.0 / CMTimeGetSeconds(minFrameDuration);
                _impl->frames = (long)((_impl->lengthMS / 1000.0) * fps);
                _impl->frameMS = (int)(CMTimeGetSeconds(minFrameDuration) * 1000.0);
            } else {
                _impl->frames = (long)(_impl->lengthMS / 50.0); // assume 20fps
                _impl->frameMS = 50;
            }
        }

        if (_impl->lengthMS <= 0 || _impl->frames <= 0) {
            spdlog::warn("AVFoundationVideoReader: Could not determine video length for {}", filename);
            return;
        }

        // Allocate frame buffers
        int channels = wantAlpha ? 4 : 3;
        _impl->frameBufferSize = _impl->width * _impl->height * channels;
        _impl->frameBuffer1 = (uint8_t*)calloc(1, _impl->frameBufferSize);
        _impl->frameBuffer2 = (uint8_t*)calloc(1, _impl->frameBufferSize);

        // Initialize both VideoFrames
        int stride = _impl->width * channels;
        _impl->videoFrame1 = { _impl->frameBuffer1, nullptr, stride, _impl->width, _impl->height, _impl->outputFormat };
        _impl->videoFrame2 = { _impl->frameBuffer2, nullptr, stride, _impl->width, _impl->height, _impl->outputFormat };

        // Open the reader at the beginning
        if (!_impl->openReader(kCMTimeZero)) {
            return;
        }

        _impl->valid = true;

        spdlog::info("AVFoundationVideoReader: Loaded {}", filename);
        spdlog::info("      Length MS: {}", _impl->lengthMS);
        spdlog::info("      Source size: {}x{}", _impl->nativeWidth, _impl->nativeHeight);
        spdlog::info("      Output size: {}x{}", _impl->width, _impl->height);
        spdlog::info("      Frames: {} @ {}fps", _impl->frames, nominalFrameRate);
        spdlog::info("      Frame ms: {}", _impl->frameMS);
        if (wantAlpha) spdlog::info("      Alpha: TRUE");

        // Read the first frame
        _impl->decodeNextFrame();
    }
}

AVFoundationVideoReader::~AVFoundationVideoReader()
{
    delete _impl;
    _impl = nullptr;
}

int AVFoundationVideoReader::GetLengthMS() const { return (int)_impl->lengthMS; }
bool AVFoundationVideoReader::IsValid() const { return _impl->valid && !_impl->failed; }
int AVFoundationVideoReader::GetWidth() const { return _impl->width; }
int AVFoundationVideoReader::GetHeight() const { return _impl->height; }
bool AVFoundationVideoReader::AtEnd() const { return _impl->atEnd; }
int AVFoundationVideoReader::GetPos() { return _impl->curPos; }
std::string AVFoundationVideoReader::GetFilename() const { return _impl->filename; }
int AVFoundationVideoReader::GetPixelChannels() const { return _impl->wantAlpha ? 4 : 3; }

void AVFoundationVideoReader::Seek(int timestampMS, bool readFrame)
{
    if (!_impl->valid) return;

    if (timestampMS >= _impl->lengthMS) {
        _impl->atEnd = true;
        return;
    }

    _impl->atEnd = false;

    // AVAssetReader is forward-only — must recreate at the new position
    CMTime seekTime = CMTimeMakeWithSeconds(timestampMS / 1000.0, 600);
    if (!_impl->openReader(seekTime)) {
        spdlog::error("AVFoundationVideoReader: Seek to {} failed", timestampMS);
        return;
    }

    _impl->curPos = -1000;
    if (readFrame) {
        GetNextFrame(timestampMS, 0);
    }
}

VideoFrame* AVFoundationVideoReader::GetNextFrame(int timestampMS, int gracetime)
{
    if (!_impl->valid || _impl->frames == 0) {
        return nullptr;
    }

    if (timestampMS > _impl->lengthMS) {
        _impl->atEnd = true;
        return nullptr;
    }

    int currenttime = _impl->curPos;
    int timeOfNextFrame = currenttime + _impl->frameMS;
    int timeOfPrevFrame = currenttime - _impl->frameMS;

    if (_impl->firstFramePos >= timestampMS) {
        timestampMS = _impl->firstFramePos;
    }

    // Same frame — return current
    if (timestampMS >= currenttime && timestampMS < timeOfNextFrame) {
        return &_impl->currentFrame();
    }
    // Previous frame — return prev (avoids a seek)
    if (timestampMS >= timeOfPrevFrame - 1 && timestampMS < currenttime) {
        return &_impl->prevFrame();
    }

    // Need to seek backward, or forward by more than 1 second
    if (currenttime > timestampMS + gracetime || timestampMS - currenttime > 1000) {
        Seek(timestampMS, false);
        currenttime = _impl->curPos;
    }

    // Read frames forward until we reach the target timestamp
    if (timestampMS <= _impl->lengthMS) {
        bool firstframe = (currenttime <= 0 && timestampMS == 0);

        while (firstframe || ((currenttime + (_impl->frameMS / 2.0)) < timestampMS)) {
            if (!_impl->decodeNextFrame()) {
                break;
            }
            firstframe = false;
            currenttime = _impl->curPos;
            if (currenttime > _impl->lengthMS) break;
        }
    } else {
        _impl->atEnd = true;
        return nullptr;
    }

    if (_impl->currentFrame().data == nullptr || currenttime > _impl->lengthMS) {
        _impl->atEnd = true;
        return nullptr;
    }

    if (timestampMS >= _impl->curPos) {
        return &_impl->currentFrame();
    } else {
        return &_impl->prevFrame();
    }
}

long AVFoundationVideoReader::GetVideoLengthStatic(const std::string& filename)
{
    @autoreleasepool {
        NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]];
        AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];
        if (!asset) return 0;

        // Accessing .duration triggers synchronous loading
        CMTime duration = asset.duration;
        if (!CMTIME_IS_VALID(duration)) return 0;

        return (long)(CMTimeGetSeconds(duration) * 1000.0);
    }
}

#endif // __APPLE__
