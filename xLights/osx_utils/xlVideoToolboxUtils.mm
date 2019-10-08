//
//  xlVideoToolboxUtils.m
//  xLights


#include "CoreImage/CIImage.h"
#include "CoreImage/CIContext.h"

extern "C" {
#include "libavcodec/videotoolbox.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
}


static AVPixelFormat negotiate_pixel_format(AVCodecContext *s, const AVPixelFormat *fmt) {
    const enum AVPixelFormat *p;
    for (p = fmt; *p != AV_PIX_FMT_NONE; p++) {
        if (*p == AV_PIX_FMT_VIDEOTOOLBOX) {
            if (s->hwaccel_context == NULL) {
                AVVideotoolboxContext * vtctx = av_videotoolbox_alloc_context();
                int result = av_videotoolbox_default_init2(s, vtctx);
                if (result < 0) {
                    return s->pix_fmt;
                }
            }
            return *p;
        }
    }
    return s->pix_fmt;
}


bool SetupVideoToolboxAcceleration(AVCodecContext *s, bool enabled) {
    if (enabled) {
        s->thread_count = 2;
        s->get_format = negotiate_pixel_format;
        return true;
    }
    return false;
}
void CleanupVideoToolbox(AVCodecContext *s) {
    if (s->hwaccel_context == NULL) {
        av_videotoolbox_default_free(s);
    }
}
static CIContext *ciContext = nullptr;
void InitVideoToolboxAcceleration() {
    NSDictionary *dict = @{
        kCIContextHighQualityDownsample: @YES,
        kCIContextCacheIntermediates: @NO,
        kCIContextOutputPremultiplied: @NO,
        kCIContextUseSoftwareRenderer: @NO,
        kCIContextWorkingFormat: @"RGBA8",
        kCIContextAllowLowPower: @YES
    };
    ciContext = [[CIContext alloc] initWithOptions:dict];
}


bool IsVideoToolboxAcceleratedFrame(AVFrame *frame) {
    for (int x = 0; x < 3; x++) {
        if (frame->data[x]) return false;
    }
    return frame->data[3] != nullptr;
}

void VideoToolboxScaleImage(AVCodecContext *codecContext, AVFrame *frame, AVFrame *dstFrame) {
    CVPixelBufferRef pixbuf = (CVPixelBufferRef)frame->data[3];
    float w = dstFrame->width;
    w /= (float)frame->width;
    float h = dstFrame->height;
    h /= (float)frame->height;
    
    if (pixbuf == nullptr) {
        memset(dstFrame->data[0], 0, dstFrame->height * dstFrame->linesize[0]);
        return;
    }
        
    @autoreleasepool {
        CVPixelBufferRef scaledBuf = NULL;
        
        //BGRA is the pixel type that works best for us, there isn't an accelerated RGBA or just RGB
        //so we'll do BGRA and map while copying
        CVPixelBufferCreate(kCFAllocatorDefault,
                          dstFrame->width,
                          dstFrame->height,
                          kCVPixelFormatType_32BGRA,
                          (__bridge CFDictionaryRef) @{(__bridge NSString *) kCVPixelBufferIOSurfacePropertiesKey: @{}},
                          &scaledBuf);
    
        CIImage *image = [CIImage imageWithCVImageBuffer:pixbuf];
        CIImage *scaledimage = [image imageByApplyingTransform:CGAffineTransformMakeScale(w, h) highQualityDownsample:TRUE];
        [ciContext render:scaledimage toCVPixelBuffer:scaledBuf];
        pixbuf = nil;
        
        CVPixelBufferLockBaseAddress(scaledBuf, kCVPixelBufferLock_ReadOnly);
        uint8_t *data = (uint8_t *)CVPixelBufferGetBaseAddress(scaledBuf);
        int linesize = CVPixelBufferGetBytesPerRow(scaledBuf);
        
        //copy data to dest frame
        if (linesize) {
            uint8_t *dst = (uint8_t*)dstFrame->data[0];
            uint8_t *src = (uint8_t*)data;
            
            int rgbLoc = 0;
            for (int y = 0; y < dstFrame->height; y++) {
                for (int w = 0; w < linesize; w += 4) {
                    dst[rgbLoc] = src[w + 2];
                    dst[rgbLoc + 1] = src[w + 1];
                    dst[rgbLoc + 2] = src[w];
                    if (dstFrame->format == AV_PIX_FMT_RGBA) {
                        dst[rgbLoc + 3] = src[w + 3];
                        rgbLoc += 4;
                    } else {
                        rgbLoc += 3;
                    }
                }
                src += linesize;
            }
        }
        CVPixelBufferUnlockBaseAddress(scaledBuf, kCVPixelBufferLock_ReadOnly);
        CVPixelBufferRelease(scaledBuf);
        scaledBuf = nil;
    }

    av_frame_copy_props(dstFrame, frame);
}
