//
//  xlVideoToolboxUtils.m
//  xLights


#include "CoreImage/CIImage.h"
#include "CoreImage/CIContext.h"
#include "CoreImage/CIKernel.h"
#include "CoreImage/CIFilter.h"
#include "CoreImage/CISampler.h"

extern "C" {
#include "libavcodec/videotoolbox.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
}

#include <log4cpp/Category.hh>

#include "TraceLog.h"
using namespace TraceLog;

static CIContext *ciContext = nullptr;
static CIColorKernel *rbFlipKernel = nullptr;




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
void InitVideoToolboxAcceleration() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    NSDictionary *dict = @{
        kCIContextHighQualityDownsample: @YES,
        kCIContextCacheIntermediates: @NO,
        kCIContextOutputPremultiplied: @NO,
        kCIContextUseSoftwareRenderer: @NO,
        kCIContextAllowLowPower: @YES
    };
    ciContext = [[CIContext alloc] initWithOptions:dict];

    if (ciContext == nullptr) {
        logger_base.info("Could not create hardware context for scaling.");
        // wasn't able to create the context, let's try
        // with allowing the software renderer
        dict = @{
            kCIContextHighQualityDownsample: @YES,
            kCIContextCacheIntermediates: @NO,
            kCIContextOutputPremultiplied: @NO,
            kCIContextUseSoftwareRenderer: @YES,
            kCIContextAllowLowPower: @YES
        };
        ciContext = [[CIContext alloc] initWithOptions:dict];
    }
    if (ciContext == nullptr) {
        logger_base.info("Could not create context for scaling.");
    } else {
        [ciContext retain];
    }

    rbFlipKernel = [CIColorKernel kernelWithString: @"kernel vec4 swapRedAndGreenAmount(__sample s) { return s.bgra; }" ];
    [rbFlipKernel retain];
}


bool IsVideoToolboxAcceleratedFrame(AVFrame *frame) {
    for (int x = 0; x < 3; x++) {
        if (frame->data[x]) return false;
    }
    return frame->data[3] != nullptr;
}

@interface CIRBFlipFilter: CIFilter {
    @public CIImage *inputImage;
}
@end

@implementation CIRBFlipFilter


- (CIImage *)outputImage
{
    CIImage *ci = inputImage;
    return [rbFlipKernel applyWithExtent:ci.extent arguments:@[ci] ];
}
@end


bool VideoToolboxScaleImage(AVCodecContext *codecContext, AVFrame *frame, AVFrame *dstFrame) {
    CVPixelBufferRef pixbuf = (CVPixelBufferRef)frame->data[3];
    float w = dstFrame->width;
    w /= (float)frame->width;
    float h = dstFrame->height;
    h /= (float)frame->height;
    
    if (pixbuf == nullptr) {
        memset(dstFrame->data[0], 0, dstFrame->height * dstFrame->linesize[0]);
        return false;
    }
    if (ciContext == nullptr) {
        //cannot use a hardware scaler
        return false;
    }

    PushTraceContext();
    @autoreleasepool {
        CVPixelBufferRef scaledBuf = NULL;
        
        //BGRA is the pixel type that works best for us, there isn't an accelerated RGBA or just RGB
        //so we'll do BGRA and map while copying
        AddTraceMessage("VideoToolbox - creating RGBA destination buffer");
        CVPixelBufferCreate(kCFAllocatorDefault,
                          dstFrame->width,
                          dstFrame->height,
                          kCVPixelFormatType_32BGRA,
                          (__bridge CFDictionaryRef) @{(__bridge NSString *) kCVPixelBufferIOSurfacePropertiesKey: @{}},
                          &scaledBuf);
        if (scaledBuf == nullptr) {
            PopTraceContext();
            return false;
        }
                
        AddTraceMessage("VideoToolbox - creating CIImage from pixbuf");
        CIImage *image = [CIImage imageWithCVImageBuffer:pixbuf];
        if (image == nullptr) {
            PopTraceContext();
            return false;
        }

        
        AddTraceMessage("VideoToolbox - Scaling image");
        CIImage *scaledimage = [image imageByApplyingTransform:CGAffineTransformMakeScale(w, h) highQualityDownsample:TRUE];
        if (scaledimage == nullptr) {
            PopTraceContext();
            return false;
        }

        AddTraceMessage("VideoToolbox - Applying BGRA -> RGBA filter");
        CIRBFlipFilter *filter = [[CIRBFlipFilter alloc] init];
        filter->inputImage = scaledimage;
        CIImage *swappedImage =  [filter outputImage];
        if (swappedImage == nullptr) {
            PopTraceContext();
            return false;
        }

        AddTraceMessage("VideoToolbox - Rendering image to buffer");
        [ciContext render:swappedImage toCVPixelBuffer:scaledBuf];
        pixbuf = nil;
        
        AddTraceMessage("VideoToolbox - Copy data from buffer to dstFrame");
        CVPixelBufferLockBaseAddress(scaledBuf, kCVPixelBufferLock_ReadOnly);
        uint8_t *data = (uint8_t *)CVPixelBufferGetBaseAddress(scaledBuf);
        int linesize = CVPixelBufferGetBytesPerRow(scaledBuf);
        if (data == nullptr) {
            AddTraceMessage("VideoToolbox - No data returned");
        }
        if (dstFrame->data[0] == nullptr) {
            AddTraceMessage("VideoToolbox - No data in target frame");
        }
        if (CVPixelBufferGetHeight(scaledBuf) != dstFrame->height) {
            AddTraceMessage("VideoToolbox - Heights don't match " + std::to_string(CVPixelBufferGetHeight(scaledBuf)) + ":" + std::to_string(dstFrame->height));
        }
        if (CVPixelBufferGetWidth(scaledBuf) != dstFrame->width) {
            AddTraceMessage("VideoToolbox - Widths don't match " + std::to_string(CVPixelBufferGetWidth(scaledBuf)) + ":" + std::to_string(dstFrame->width));
        }

        //copy data to dest frame
        if (linesize) {
            if (dstFrame->format == AV_PIX_FMT_RGBA) {
                memcpy(dstFrame->data[0], data, linesize*dstFrame->height);
            } else {
                uint8_t *dst = (uint8_t*)dstFrame->data[0];
                uint8_t *src = (uint8_t*)data;

                int total = dstFrame->height * linesize;
                for (int w = 0, rgbLoc = 0; w < total; w += 4, rgbLoc += 3) {
                    dst[rgbLoc] = src[w];
                    dst[rgbLoc + 1] = src[w + 1];
                    dst[rgbLoc + 2] = src[w + 2];
                }
            }
        }
        AddTraceMessage("VideoToolbox - Releasing buffer");
        CVPixelBufferUnlockBaseAddress(scaledBuf, kCVPixelBufferLock_ReadOnly);
        CVPixelBufferRelease(scaledBuf);
        scaledBuf = nil;
    }

    av_frame_copy_props(dstFrame, frame);
    PopTraceContext();
    return true;
}
