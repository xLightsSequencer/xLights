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
#import <Accelerate/Accelerate.h>

#include <algorithm>
#include <mutex>
#include <queue>
#include <vector>

namespace {
    // Mirror of FFmpeg's videotoolbox hwaccel init probe. Some H.264 streams
    // (notably High@L1.0 with B-frames at very small resolutions) pass
    // AVAssetReader's setup but cause its internal HW decoder to silently
    // wedge mid-stream — status stays "Reading" but copyNextSampleBuffer
    // never returns. This probe asks VideoToolbox if it can hardware-decode
    // this format description; if not, we route the file through a manual
    // VTDecompressionSession with HW disabled.
    bool probeHardwareDecoderSupport(AVAssetTrack* track) {
        if (!track) return false;
        NSArray* formatDescs = nil;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        formatDescs = track.formatDescriptions;
#pragma clang diagnostic pop
        if (formatDescs.count == 0) return true;

        CMVideoFormatDescriptionRef formatDesc = (__bridge CMVideoFormatDescriptionRef)formatDescs[0];

        NSDictionary* spec = @{
            (NSString*)kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder: @YES
        };
        NSDictionary* dstAttrs = @{
            (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
            (NSString*)kCVPixelBufferIOSurfacePropertiesKey: @{}
        };

        VTDecompressionSessionRef session = nullptr;
        OSStatus status = VTDecompressionSessionCreate(
            kCFAllocatorDefault,
            formatDesc,
            (__bridge CFDictionaryRef)spec,
            (__bridge CFDictionaryRef)dstAttrs,
            nullptr,
            &session);

        if (session) {
            VTDecompressionSessionInvalidate(session);
            CFRelease(session);
        }
        return status == noErr;
    }
}

// Internal ObjC++ implementation struct
// __strong is explicit to ensure ARC retains ObjC objects stored in this C++ struct
struct AVFoundationVideoReaderImpl {
    __strong AVAsset* asset = nil;
    __strong AVAssetTrack* videoTrack = nil;
    __strong AVAssetReader* reader = nil;
    __strong AVAssetReaderTrackOutput* trackOutput = nil;
    VTPixelTransferSessionRef transferSession = nullptr;
    __strong CIContext* ciContext = nil;

    // Software-decode path: AVAssetReader is used for demux only (raw H.264
    // sample buffers), and we run our own VTDecompressionSession with HW
    // disabled. Used for streams that fail probeHardwareDecoderSupport().
    bool useSoftwareDecode = false;
    VTDecompressionSessionRef vtSession = nullptr;
    CMVideoFormatDescriptionRef cachedFormatDesc = nullptr; // retained
    int maxBFrameDelay = 2;                                  // queue depth = this + 1
    bool demuxAtEnd = false;
    std::mutex queueMutex;
    struct DecodedEntry {
        CVImageBufferRef image; // retained
        int64_t ptsMs;
        bool operator<(const DecodedEntry& o) const { return ptsMs > o.ptsMs; } // min-heap
    };
    std::priority_queue<DecodedEntry, std::vector<DecodedEntry>> ptsQueue;

    std::string filename;
    bool valid = false;
    bool atEnd = false;
    bool failed = false;
    bool wantAlpha = false;
    bool bgr = false;
    bool wantsHWType = false;
    VideoScaleAlgorithm scaleAlgorithm = VideoScaleAlgorithm::Default;

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
        if (cachedFormatDesc) {
            CFRelease(cachedFormatDesc);
            cachedFormatDesc = nullptr;
        }
        if (frameBuffer1) { free(frameBuffer1); frameBuffer1 = nullptr; }
        if (frameBuffer2) { free(frameBuffer2); frameBuffer2 = nullptr; }
        // ARC handles ObjC objects
    }

    void closeReader() {
        closeVTSession();
        if (reader) {
            [reader cancelReading];
            reader = nil;
        }
        trackOutput = nil;
        demuxAtEnd = false;
    }

    void closeVTSession() {
        if (vtSession) {
            VTDecompressionSessionInvalidate(vtSession);
            CFRelease(vtSession);
            vtSession = nullptr;
        }
        std::lock_guard<std::mutex> lk(queueMutex);
        while (!ptsQueue.empty()) {
            CVBufferRelease(ptsQueue.top().image);
            ptsQueue.pop();
        }
    }

    static void vtOutputCallback(void* refCon, void* /*sourceFrameRefCon*/,
                                 OSStatus status, VTDecodeInfoFlags /*infoFlags*/,
                                 CVImageBufferRef imageBuffer,
                                 CMTime pts, CMTime /*duration*/) {
        if (status != noErr || imageBuffer == nullptr) return;
        AVFoundationVideoReaderImpl* self = static_cast<AVFoundationVideoReaderImpl*>(refCon);
        DecodedEntry entry;
        entry.image = (CVImageBufferRef)CVBufferRetain(imageBuffer);
        entry.ptsMs = CMTIME_IS_VALID(pts) ? (int64_t)(CMTimeGetSeconds(pts) * 1000.0) : 0;
        std::lock_guard<std::mutex> lk(self->queueMutex);
        self->ptsQueue.push(entry);
    }

    bool openReader(CMTime startTime) {
        closeReader();

        // [AVAssetReader addOutput:] starts internal KVO observers that
        // autorelease NSStrings via -[NSString initWithFormat:]. Drain
        // those locally instead of letting them pile up on the render
        // thread's job-scoped pool, which only flushes when the whole
        // render finishes.
        @autoreleasepool {
            NSError* error = nil;
            reader = [[AVAssetReader alloc] initWithAsset:asset error:&error];
            if (!reader || error) {
                spdlog::error("AVFoundationVideoReader: Failed to create AVAssetReader: {}",
                             error ? [[error localizedDescription] UTF8String] : "unknown error");
                return false;
            }

            // HW path: ask AVAssetReader to deliver decoded BGRA pixel buffers.
            // SW path: pass nil outputSettings so AVAssetReader becomes a pure
            // demuxer; we run our own VTDecompressionSession with HW disabled
            // for streams that fail the HW probe.
            NSDictionary* outputSettings = nil;
            if (!useSoftwareDecode) {
                outputSettings = @{
                    (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
                };
            }
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
    }

    // Build a fresh software-mode VTDecompressionSession bound to the given
    // format description. AVAssetReader can deliver samples whose format
    // description differs from the track's published one (or there can be
    // multiple in a track); the only way to be sure the session matches is
    // to construct it from the sample's own format description on first use,
    // and rebuild if it ever changes.
    bool ensureVTSession(CMVideoFormatDescriptionRef formatDesc) {
        if (!formatDesc) return false;
        if (vtSession && cachedFormatDesc &&
            CMFormatDescriptionEqual((CMFormatDescriptionRef)cachedFormatDesc,
                                     (CMFormatDescriptionRef)formatDesc)) {
            return true;
        }

        // Format-desc mismatch (or first call) — tear the old session down.
        if (vtSession) {
            VTDecompressionSessionInvalidate(vtSession);
            CFRelease(vtSession);
            vtSession = nullptr;
        }
        if (cachedFormatDesc) {
            CFRelease(cachedFormatDesc);
            cachedFormatDesc = nullptr;
        }
        cachedFormatDesc = (CMVideoFormatDescriptionRef)CFRetain(formatDesc);

        NSDictionary* spec = @{
            (NSString*)kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder: @NO,
            (NSString*)kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder: @NO
        };
        NSDictionary* dstAttrs = @{
            (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
            (NSString*)kCVPixelBufferIOSurfacePropertiesKey: @{}
        };

        VTDecompressionOutputCallbackRecord cb = {
            .decompressionOutputCallback = &AVFoundationVideoReaderImpl::vtOutputCallback,
            .decompressionOutputRefCon = this
        };

        OSStatus status = VTDecompressionSessionCreate(
            kCFAllocatorDefault,
            cachedFormatDesc,
            (__bridge CFDictionaryRef)spec,
            (__bridge CFDictionaryRef)dstAttrs,
            &cb,
            &vtSession);

        if (status != noErr) {
            spdlog::error("AVFoundationVideoReader: VTDecompressionSessionCreate (SW) failed: {}", (int)status);
            vtSession = nullptr;
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
    // performing BGRA→target format conversion using Accelerate.framework
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

        vImage_Buffer srcBuf = { src, (vImagePixelCount)copyHeight, (vImagePixelCount)copyWidth, srcStride };
        vImage_Buffer dstBuf = { dst, (vImagePixelCount)copyHeight, (vImagePixelCount)copyWidth, (size_t)dstStride };

        if (wantAlpha) {
            if (bgr) {
                // BGRA → BGRA: direct copy
                for (int y = 0; y < copyHeight; y++) {
                    memcpy(dst + y * dstStride, src + y * srcStride, copyWidth * 4);
                }
            } else {
                // BGRA → RGBA: permute channels using NEON-accelerated vImage
                const uint8_t permuteMap[4] = { 2, 1, 0, 3 }; // B,G,R,A → R,G,B,A
                vImagePermuteChannels_ARGB8888(&srcBuf, &dstBuf, permuteMap, kvImageNoFlags);
            }
        } else {
            if (bgr) {
                // BGRA → BGR24: drop alpha, keep channel order
                // Permute BGRA → XBGR {3,0,1,2}, then vImageConvert_ARGB8888toRGB888 drops X
                size_t tmpStride = copyWidth * 4;
                size_t tmpSize = tmpStride * copyHeight;
                uint8_t stackBuf[64 * 1024];
                uint8_t* tmpData = (tmpSize <= sizeof(stackBuf)) ? stackBuf : (uint8_t*)malloc(tmpSize);
                vImage_Buffer tmpBuf = { tmpData, (vImagePixelCount)copyHeight, (vImagePixelCount)copyWidth, tmpStride };
                const uint8_t permuteMap[4] = { 3, 0, 1, 2 }; // BGRA → XBGR
                vImagePermuteChannels_ARGB8888(&srcBuf, &tmpBuf, permuteMap, kvImageNoFlags);
                vImageConvert_ARGB8888toRGB888(&tmpBuf, &dstBuf, kvImageNoFlags);
                if (tmpData != stackBuf) free(tmpData);
            } else {
                // BGRA → RGB24: permute + drop alpha
                // Permute BGRA → XRGB {3,2,1,0}, then vImageConvert_ARGB8888toRGB888 drops X
                size_t tmpStride = copyWidth * 4;
                size_t tmpSize = tmpStride * copyHeight;
                uint8_t stackBuf[64 * 1024];
                uint8_t* tmpData = (tmpSize <= sizeof(stackBuf)) ? stackBuf : (uint8_t*)malloc(tmpSize);
                vImage_Buffer tmpBuf = { tmpData, (vImagePixelCount)copyHeight, (vImagePixelCount)copyWidth, tmpStride };
                const uint8_t permuteMap[4] = { 3, 2, 1, 0 }; // BGRA → XRGB
                vImagePermuteChannels_ARGB8888(&srcBuf, &tmpBuf, permuteMap, kvImageNoFlags);
                vImageConvert_ARGB8888toRGB888(&tmpBuf, &dstBuf, kvImageNoFlags);
                if (tmpData != stackBuf) free(tmpData);
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

    // CIFilter-based scaling for specific algorithms (bicubic, lanczos, area, point).
    // Returns true if scaling succeeded and frame was populated.
    bool ciFilterScale(CVImageBufferRef imageBuffer) {
        if (!ciContext) {
            ciContext = [[CIContext alloc] initWithOptions:@{
                (id)kCIContextUseSoftwareRenderer: @NO,
                (id)kCIContextOutputPremultiplied: @NO,
                (id)kCIContextHighQualityDownsample: @YES,
                (id)kCIContextCacheIntermediates: @NO,
                (id)kCIContextAllowLowPower: @YES,
            }];
            if (!ciContext) return false;
        }
        if (!ensureScaledPixelBuffer()) return false;

        @autoreleasepool {
            CIImage* image = [CIImage imageWithCVImageBuffer:imageBuffer];
            if (!image) return false;

            float w = (float)width / (float)CVPixelBufferGetWidth(imageBuffer);
            float h = (float)height / (float)CVPixelBufferGetHeight(imageBuffer);

            CIImage* scaled = nil;
            switch (scaleAlgorithm) {
            case VideoScaleAlgorithm::Bicubic: {
                CIFilter* f = [CIFilter filterWithName:@"CIBicubicScaleTransform"];
                [f setValue:@(h) forKey:@"inputScale"];
                [f setValue:@(w / h) forKey:@"inputAspectRatio"];
                [f setValue:@(0.0f) forKey:@"inputB"];
                [f setValue:@(0.75f) forKey:@"inputC"];
                [f setValue:image forKey:@"inputImage"];
                scaled = [f valueForKey:@"outputImage"];
                break;
            }
            case VideoScaleAlgorithm::Lanczos: {
                CIFilter* f = [CIFilter filterWithName:@"CILanczosScaleTransform"];
                [f setValue:@(h) forKey:@"inputScale"];
                [f setValue:@(w / h) forKey:@"inputAspectRatio"];
                [f setValue:image forKey:@"inputImage"];
                scaled = [f valueForKey:@"outputImage"];
                break;
            }
            case VideoScaleAlgorithm::Area:
                scaled = [image imageByApplyingTransform:CGAffineTransformMakeScale(w, h)
                                   highQualityDownsample:YES];
                break;
            case VideoScaleAlgorithm::Point:
                scaled = [image imageByApplyingTransform:CGAffineTransformMakeScale(w, h)
                                   highQualityDownsample:NO];
                break;
            default:
                break;
            }
            if (!scaled) return false;

            [ciContext render:scaled toCVPixelBuffer:scaledPixelBuffer];
        }

        swapFrames();
        copyPixelBufferToFrame(scaledPixelBuffer);
        return true;
    }

    // Scale (if needed) and copy a decoded image buffer into the current
    // frame slot. Caller retains ownership of `imageBuffer`.
    void emitDecodedImage(CVImageBufferRef imageBuffer) {
        bool needsScale = ((int)CVPixelBufferGetWidth(imageBuffer) != width ||
                           (int)CVPixelBufferGetHeight(imageBuffer) != height);

        if (needsScale) {
            if (scaleAlgorithm != VideoScaleAlgorithm::Default) {
                if (ciFilterScale(imageBuffer)) return;
            } else {
                if (ensureTransferSession() && ensureScaledPixelBuffer()) {
                    OSStatus xferStatus = VTPixelTransferSessionTransferImage(transferSession,
                                                                              imageBuffer,
                                                                              scaledPixelBuffer);
                    if (xferStatus == noErr) {
                        swapFrames();
                        copyPixelBufferToFrame(scaledPixelBuffer);
                        return;
                    } else {
                        spdlog::warn("AVFoundationVideoReader: VTPixelTransferSession failed ({}), falling back to unscaled", (int)xferStatus);
                    }
                }
            }
        }

        // No scaling needed (or scaling failed) — copy directly
        swapFrames();
        copyPixelBufferToFrame(imageBuffer);
    }

    // HW path: AVAssetReader decodes via VideoToolbox internally and delivers
    // CVPixelBuffers. We just consume them.
    bool decodeNextFrameHW() {
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

            if (!CMSampleBufferIsValid(sampleBuffer)) {
                spdlog::warn("AVFoundationVideoReader: Invalid sample buffer received");
                CFRelease(sampleBuffer);
                return false;
            }

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

            emitDecodedImage(imageBuffer);
            CFRelease(sampleBuffer);
            return true;
        }
    }

    // SW path: AVAssetReader is a pure demuxer (delivers raw H.264 sample
    // buffers); we feed them through our own VTDecompressionSession with HW
    // disabled and pull decoded pictures out of the PTS-ordered queue. A
    // small lookahead is required to honor B-frame display order.
    bool decodeNextFrameSW() {
        const int threshold = maxBFrameDelay + 1;

        // Pump compressed samples through VT until we have enough decoded
        // frames in the queue to safely emit the next-display-order one.
        while (!demuxAtEnd && queueSize() < threshold) {
            if (reader.status != AVAssetReaderStatusReading) {
                demuxAtEnd = true;
                break;
            }
            @autoreleasepool {
                CMSampleBufferRef sample = nullptr;
                @try {
                    sample = [trackOutput copyNextSampleBuffer];
                } @catch (NSException* exception) {
                    spdlog::error("AVFoundationVideoReader: Exception in copyNextSampleBuffer (SW): {} - {}",
                                 [[exception name] UTF8String], [[exception reason] UTF8String]);
                    failed = true;
                    return false;
                }
                if (!sample) {
                    demuxAtEnd = true;
                    break;
                }

                CMVideoFormatDescriptionRef sampleFormat =
                    (CMVideoFormatDescriptionRef)CMSampleBufferGetFormatDescription(sample);
                if (!sampleFormat) {
                    // Discontinuity / metadata sample with no format desc — skip it.
                    spdlog::debug("AVFoundationVideoReader: SW sample with no format description, skipping");
                    CFRelease(sample);
                    continue;
                }
                if (!ensureVTSession(sampleFormat)) {
                    spdlog::error("AVFoundationVideoReader: ensureVTSession failed for {} (sample format desc {}); aborting SW reader",
                                 filename, (void*)sampleFormat);
                    CFRelease(sample);
                    failed = true;
                    return false;
                }

                VTDecodeFrameFlags flags = 0;
                VTDecodeInfoFlags info = 0;
                OSStatus status = VTDecompressionSessionDecodeFrame(vtSession, sample, flags, nullptr, &info);
                CFRelease(sample);
                if (status != noErr) {
                    spdlog::warn("AVFoundationVideoReader: VTDecompressionSessionDecodeFrame failed: {}", (int)status);
                }
            }
        }

        // At end-of-stream, drain anything VT still has buffered for B-frame
        // reordering before declaring the queue authoritative.
        if (demuxAtEnd && vtSession && queueSize() < threshold) {
            VTDecompressionSessionFinishDelayedFrames(vtSession);
            VTDecompressionSessionWaitForAsynchronousFrames(vtSession);
        }

        DecodedEntry entry{};
        bool got = false;
        {
            std::lock_guard<std::mutex> lk(queueMutex);
            if (!ptsQueue.empty()) {
                entry = ptsQueue.top();
                ptsQueue.pop();
                got = true;
            }
        }
        if (!got) {
            atEnd = true;
            return false;
        }

        curPos = (int)entry.ptsMs;
        if (firstFramePos == -1) firstFramePos = curPos;

        emitDecodedImage(entry.image);
        CVBufferRelease(entry.image);
        return true;
    }

    int queueSize() {
        std::lock_guard<std::mutex> lk(queueMutex);
        return (int)ptsQueue.size();
    }

    // Decode the next sample, scale it, and populate the current frame buffer.
    // Returns true if a frame was decoded.
    bool decodeNextFrame() {
        if (!reader) {
            return false;
        }
        if (!useSoftwareDecode) {
            AVAssetReaderStatus readerStatus = reader.status;
            if (readerStatus != AVAssetReaderStatusReading) {
                if (readerStatus == AVAssetReaderStatusFailed) {
                    spdlog::error("AVFoundationVideoReader: Reader failed: {}",
                                 reader.error ? [[reader.error localizedDescription] UTF8String] : "unknown");
                }
                atEnd = true;
                return false;
            }
            return decodeNextFrameHW();
        }
        return decodeNextFrameSW();
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

    // [AVURLAsset URLAssetWithURL:] internally autoreleases NSArrays/NSStrings
    // via AVCMNotificationDispatcher when registering FigAsset notification
    // listeners. Drain locally so they don't accumulate on the render-job pool.
    @autoreleasepool {
        // Open the asset — use AVURLAsset for explicit control over loading
        NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]];
        AVURLAsset* urlAsset = [AVURLAsset URLAssetWithURL:url options:nil];
        _impl->asset = urlAsset;
        if (!_impl->asset) {
            spdlog::error("AVFoundationVideoReader: Failed to create AVAsset for {}", filename);
            return;
        }

        // Get video tracks — tracksWithMediaType: deprecated in macOS 15 but replacement requires macOS 15+
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        NSArray<AVAssetTrack*>* tracks = [_impl->asset tracksWithMediaType:AVMediaTypeVideo];
#pragma clang diagnostic pop
        if (tracks.count == 0) {
            spdlog::error("AVFoundationVideoReader: No video tracks in {}", filename);
            return;
        }
        _impl->videoTrack = tracks[0];

        // Probe HW decode capability (matches FFmpeg's videotoolbox hwaccel
        // init check). Files that fail this — typically H.264 High@L1.0 with
        // B-frames at very small resolutions — cause AVAssetReader's HW
        // decoder to silently wedge mid-stream. Route them to our manual
        // VTDecompressionSession with HW disabled instead.
        _impl->useSoftwareDecode = !probeHardwareDecoderSupport(_impl->videoTrack);
        if (_impl->useSoftwareDecode) {
            spdlog::info("AVFoundationVideoReader: HW decode unsupported for {}; using software VTDecompressionSession path", filename);
        }

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

void AVFoundationVideoReader::SetScaleAlgorithm(VideoScaleAlgorithm algorithm) {
    _impl->scaleAlgorithm = algorithm;
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

bool AVFoundationVideoReader::Resize(int width, int height)
{
    if (!_impl || !_impl->valid) return false;
    if (width <= 0 || height <= 0) return false;
    if (_impl->width == width && _impl->height == height) return true;

    int channels = _impl->wantAlpha ? 4 : 3;
    int newSize = width * height * channels;

    if (_impl->frameBuffer1) { free(_impl->frameBuffer1); _impl->frameBuffer1 = nullptr; }
    if (_impl->frameBuffer2) { free(_impl->frameBuffer2); _impl->frameBuffer2 = nullptr; }
    _impl->frameBuffer1 = (uint8_t*)calloc(1, newSize);
    _impl->frameBuffer2 = (uint8_t*)calloc(1, newSize);
    if (!_impl->frameBuffer1 || !_impl->frameBuffer2) return false;

    _impl->frameBufferSize = newSize;
    _impl->width = width;
    _impl->height = height;

    int stride = width * channels;
    _impl->videoFrame1 = { _impl->frameBuffer1, nullptr, stride, width, height, _impl->outputFormat };
    _impl->videoFrame2 = { _impl->frameBuffer2, nullptr, stride, width, height, _impl->outputFormat };

    // The cached scaled pixel buffer is sized to the old dimensions; ensureScaledPixelBuffer()
    // will lazily reallocate to match the new width/height on the next decode.
    if (_impl->scaledPixelBuffer) {
        CVPixelBufferRelease(_impl->scaledPixelBuffer);
        _impl->scaledPixelBuffer = nullptr;
    }

    // Force the next GetNextFrame() to actually decode rather than returning the
    // current/prev frame, whose backing storage is the freshly-allocated (zeroed) buffers.
    _impl->curPos = -1000;

    return true;
}

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
