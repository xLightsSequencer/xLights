/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLHousePreviewVideoExporter.h"

#import <Metal/Metal.h>
#import <CoreImage/CoreImage.h>

#include "iPadModelPreview.h"
#include "xlStandaloneMetalCanvas.h"
#include "iPadRenderContext.h"
#include "graphics/metal/xlMetalGraphicsContext.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "media/VideoWriter.h"
#include "media/AudioManager.h"
#include "models/Model.h"
#include "models/ModelScreenLocation.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "render/SequenceData.h"
#include "graphics/xlGraphicsContext.h"
#include "utils/xlImage.h"
#include "osxUtils/MetalDeviceManager.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

// Load an image file into an xlImage using ImageIO. Mirrors
// XLMetalBridge.mm's private LoadImageFile so the exported 2D
// background decodes byte-for-byte the same way the on-screen house
// preview does (same RGBA layout the createTexture path expects).
static std::unique_ptr<xlImage> LoadBackgroundImageFile(const std::string& path, int& outW, int& outH) {
    outW = outH = 0;
    if (path.empty()) return nullptr;

    NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
    NSURL* url = [NSURL fileURLWithPath:nsPath];
    CGImageSourceRef src = CGImageSourceCreateWithURL((__bridge CFURLRef)url, nullptr);
    if (!src) return nullptr;
    CGImageRef cgImg = CGImageSourceCreateImageAtIndex(src, 0, nullptr);
    CFRelease(src);
    if (!cgImg) return nullptr;

    int w = (int)CGImageGetWidth(cgImg);
    int h = (int)CGImageGetHeight(cgImg);
    if (w <= 0 || h <= 0) {
        CGImageRelease(cgImg);
        return nullptr;
    }
    std::unique_ptr<uint8_t[]> rgba(new uint8_t[(size_t)w * h * 4]());
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cgctx = CGBitmapContextCreate(rgba.get(), w, h, 8, w * 4, cs,
                                               (uint32_t)kCGImageAlphaPremultipliedLast
                                               | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(cs);
    if (!cgctx) {
        CGImageRelease(cgImg);
        return nullptr;
    }
    CGContextSetBlendMode(cgctx, kCGBlendModeCopy);
    CGContextDrawImage(cgctx, CGRectMake(0, 0, w, h), cgImg);
    CGContextRelease(cgctx);
    CGImageRelease(cgImg);

    auto img = std::make_unique<xlImage>(w, h);
    std::memcpy(img->GetData(), rgba.get(), (size_t)w * h * 4);
    outW = w;
    outH = h;
    return img;
}

@implementation XLHousePreviewVideoExporter

+ (BOOL)exportToPath:(NSString*)path
       renderContext:(void*)renderContext
               width:(int)width
              height:(int)height
                is3d:(BOOL)is3d
         highQuality:(BOOL)highQuality
          startFrame:(int)startFrame
            endFrame:(int)endFrame
            progress:(void (^)(double))progress {
    if (!path || path.length == 0 || renderContext == nullptr) return NO;
    if (width <= 0 || height <= 0 || endFrame <= startFrame) return NO;

    auto* ctx = static_cast<iPadRenderContext*>(renderContext);
    SequenceData& seqData = ctx->GetSequenceData();
    if (seqData.NumFrames() == 0 || seqData.NumChannels() == 0) return NO;
    const int frameTime = seqData.FrameTime() > 0 ? seqData.FrameTime() : 50;
    const int fps = std::max(1, 1000 / frameTime);

    // Snapshot of the live house-preview camera (pan / rotation / 2D-3D mode +
    // the pane's drawable size). Published by the on-screen bridge on each draw;
    // absent only if the house preview has never rendered, in which case we fall
    // back to the caller-requested mode and a reset/fit camera. The live mode
    // wins over the `is3d:` argument so the movie matches what the user sees.
    PreviewCamera liveCam2d(false), liveCam3d(true);
    bool liveIs3d = (is3d ? true : false);
    int liveCanvasW = 0, liveCanvasH = 0;
    const bool haveLiveCam = ctx->GetHousePreviewCamera(liveCam2d, liveCam3d, liveIs3d,
                                                        liveCanvasW, liveCanvasH);
    const bool is3dCpp = haveLiveCam ? liveIs3d : (is3d ? true : false);

    // Audio (if the sequence has media loaded). Mirrors the desktop House
    // Preview export — the writer pulls stereo float samples in step with video.
    AudioManager* audioMgr = ctx->GetCurrentMediaManager();
    const int audioRate = (audioMgr != nullptr) ? (int)audioMgr->GetRate() : 0;
    const bool hasAudio = audioMgr != nullptr && audioRate > 0 && audioMgr->GetTrackSize() > 0;

    // Layer-less canvas + preview that render straight into our own texture.
    xlStandaloneMetalCanvas canvas("HousePreviewExport", is3dCpp);
    canvas.setSize(width, height);
    canvas.setScaleFactor(1.0);
    iPadModelPreview preview(&canvas);
    preview.SetName("HousePreviewExport");
    preview.SetIs3D(is3dCpp);
    // Map the layout's virtual coordinate space onto the target frame (the
    // 2D ortho path fits virtualW×virtualH into width×height preserving
    // aspect). A reset camera frames the whole layout, scaled to fill.
    preview.SetVirtualCanvasSize(ctx->GetPreviewWidth(), ctx->GetPreviewHeight());
    preview.SetCenter2D0(ctx->GetDisplay2DCenter0());
    if (haveLiveCam) {
        if (is3dCpp) {
            // 3D view is world-space (pos/pan/distance/angles); the perspective
            // aspect recomputes for the export size, so copy the camera as-is.
            preview.Get3DCamera() = liveCam3d;
        } else {
            // 2D pan + zoom-correction are window-pixel translations. The live
            // pane fits the layout into liveCanvas via scale2d = min(H/vH, W/vW);
            // the export fits into width×height with its own scale2d. Rescale the
            // pixel terms by the ratio so the same fraction of the layout stays
            // framed (the relative zoom copies directly — scale2d handles the
            // absolute fit at the export resolution).
            PreviewCamera cam = liveCam2d;
            const int vW = ctx->GetPreviewWidth();
            const int vH = ctx->GetPreviewHeight();
            if (vW > 0 && vH > 0 && liveCanvasW > 0 && liveCanvasH > 0) {
                float liveScale = std::min((float)liveCanvasH / vH, (float)liveCanvasW / vW);
                float expScale = std::min((float)height / vH, (float)width / vW);
                if (liveScale > 0.0f) {
                    float f = expScale / liveScale;
                    cam.SetPanX(cam.GetPanX() * f);
                    cam.SetPanY(cam.GetPanY() * f);
                    cam.SetZoomCorrX(cam.GetZoomCorrX() * f);
                    cam.SetZoomCorrY(cam.GetZoomCorrY() * f);
                }
            }
            preview.Get2DCamera() = cam;
        }
    } else {
        preview.ResetCamera();
    }

    id<MTLDevice> device = MetalDeviceManager::instance().getMTLDevice();
    if (device == nil) return NO;

    // Offscreen render target. No CPU capture buffer — we feed the rendered
    // texture straight to the encoder's pixel buffer via CoreImage (GPU), which
    // avoids a per-frame CPU readback + scalar BGRA->RGB->NV12 conversion that
    // made 4K exports crawl.
    MTLTextureDescriptor* td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                  width:width
                                                                                 height:height
                                                                              mipmapped:NO];
    td.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    td.storageMode = MTLStorageModePrivate;
    id<MTLTexture> target = [device newTextureWithDescriptor:td];
    if (target == nil) return NO;
    preview.SetOffscreenTarget((__bridge void*)target, nullptr);  // nil capture: keep the texture
    CIContext* ciCtx = [CIContext contextWithMTLDevice:device];
    CGColorSpaceRef exportColorSpace = CGColorSpaceCreateDeviceRGB();

    VideoWriterParams params;
    params.width = width;
    params.height = height;
    params.fps = fps;
    params.audioSampleRate = hasAudio ? audioRate : 0;
    params.cpuFrames = false;     // GPU path: fill the encoder's CVPixelBuffer directly
    // Pick the codec only; leave videoQuality/bitrate at 0 so the writer's "Auto"
    // path applies its content-adaptive constant-quality default (~0.80) AND
    // PrioritizeEncodingSpeedOverQuality. Setting videoQuality here explicitly
    // would bypass that speed bias and encode much slower for the same quality.
    params.videoCodec = highQuality ? "H.265" : "H.264";

    // 2D background-image texture, lazily created on the first frame and
    // reused across the export (createTexture returns a device texture not
    // tied to a single frame's context — same lifetime trick the live
    // bridge uses for _bgTexture). Freed after the encode completes.
    xlTexture* bgTex = nullptr;
    std::string bgLoadedPath;
    int bgImgW = 0, bgImgH = 0;

    const int totalFrames = endFrame - startFrame;
    bool ok = true;
    try {
        VideoWriter writer([path UTF8String], params, /*videoOnly*/ !hasAudio);

        writer.setGetVideoCallback([&](VideoWriterFrame& vf, unsigned frameIndex) -> bool {
            @autoreleasepool {
                const int dataFrame = std::min(endFrame - 1, startFrame + (int)frameIndex);
                const int frameMS = dataFrame * frameTime;
                ctx->SetModelColors(frameMS);
                preview.SetCurrentFrameTime((uint32_t)frameMS);

                if (preview.StartDrawing(2.0)) {
                    xlGraphicsContext* gctx = preview.getCurrentGraphicsContext();
                    xlGraphicsProgram* solid = preview.getCurrentSolidProgram();
                    xlGraphicsProgram* transp = preview.getCurrentTransparentProgram();
                    xlGraphicsProgram* solidVO = preview.getCurrentSolidViewObjectProgram();
                    xlGraphicsProgram* transpVO = preview.getCurrentTransparentViewObjectProgram();

                    // 2D background image — only in 2D mode, matching the live
                    // house preview (ModelPreview.cpp:1411 / XLMetalBridge
                    // drawBackgroundWithContext). The texture is cached across
                    // frames and re-decoded only if the configured path changes.
                    if (!is3dCpp) {
                        const std::string& bgPath = ctx->GetActiveBackgroundImage();
                        if (!bgPath.empty()) {
                            if (bgTex == nullptr || bgPath != bgLoadedPath) {
                                int iw = 0, ih = 0;
                                auto bgImg = LoadBackgroundImageFile(bgPath, iw, ih);
                                if (bgImg && iw > 0 && ih > 0) {
                                    delete bgTex;
                                    bgTex = gctx->createTexture(*bgImg, bgPath, /*finalize*/ true);
                                    bgLoadedPath = bgPath;
                                    bgImgW = iw;
                                    bgImgH = ih;
                                }
                            }
                            if (bgTex != nullptr) {
                                const int virtualW = ctx->GetPreviewWidth();
                                const int virtualH = ctx->GetPreviewHeight();
                                const bool scaleImage = ctx->GetActiveScaleBackgroundImage();
                                const bool center0 = ctx->GetDisplay2DCenter0();
                                const int brightness = ctx->GetActiveBackgroundBrightness();
                                const int alpha = (int)((ctx->GetActiveBackgroundAlpha() * 255) / 100);
                                float scaleh = 1.0f, scalew = 1.0f;
                                if (!scaleImage && virtualW > 0 && virtualH > 0 && bgImgW > 0 && bgImgH > 0) {
                                    float nscaleh = (float)bgImgH / (float)virtualH;
                                    float nscalew = (float)bgImgW / (float)virtualW;
                                    if (nscaleh == 0) nscaleh = 1.0f;
                                    if (nscalew == 0) nscalew = 1.0f;
                                    if (nscalew < nscaleh) {
                                        scaleh = 1.0f;
                                        scalew = nscalew / nscaleh;
                                    } else {
                                        scaleh = nscaleh / nscalew;
                                        scalew = 1.0f;
                                    }
                                }
                                float bx = center0 ? -(float)virtualW / 2.0f : 0.0f;
                                const float bx2 = bx + (float)virtualW * scalew;
                                const float by2 = (float)virtualH * scaleh;
                                xlTexture* tex = bgTex;
                                solid->addStep([tex, bx, by2, bx2, brightness, alpha](xlGraphicsContext* c) {
                                    c->drawTexture(tex, bx, by2, bx2, 0.0f,
                                                   0.0f, 0.0f, 1.0f, 1.0f,
                                                   /*smoothScale*/ true, brightness, alpha);
                                });
                            }
                        }
                    }

                    // Back-to-front by camera-space Z of each model's world
                    // centre, matching the live house preview's draw order so
                    // alpha-blended pixels composite correctly.
                    std::vector<Model*> models = ctx->GetModelsForActivePreview();
                    const glm::mat4& vm = preview.GetViewMatrix();
                    std::vector<std::pair<Model*, float>> keyed;
                    keyed.reserve(models.size());
                    for (Model* m : models) {
                        if (!m) continue;
                        glm::vec3 c = m->GetModelScreenLocation().GetCenterPosition();
                        float z = (vm * glm::vec4(c, 1.0f)).z;
                        keyed.emplace_back(m, z);
                    }
                    std::stable_sort(keyed.begin(), keyed.end(),
                                     [](const std::pair<Model*, float>& a,
                                        const std::pair<Model*, float>& b) {
                                         return a.second < b.second;
                                     });
                    for (const auto& mz : keyed) {
                        mz.first->DisplayModelOnWindow(&preview, gctx, solid, transp, is3dCpp,
                                                       /*color*/ nullptr, /*allowSelected*/ false,
                                                       /*wiring*/ false, /*highlightFirst*/ false,
                                                       0, nullptr);
                    }

                    // View objects (house meshes, ground images, terrain).
                    // Only the Default layout group owns them; the VO programs
                    // composite at the right depth in EndDrawing regardless of
                    // submit order. Matches XLMetalBridge's house-preview loop.
                    if (ctx->ActivePreviewShowsViewObjects() && ctx->HasViewObjectManager()) {
                        auto& allObjects = ctx->GetAllObjects();
                        for (auto it = allObjects.begin(); it != allObjects.end(); ++it) {
                            ViewObject* vo = it->second;
                            if (!vo) continue;
                            vo->Selected(false);
                            vo->Draw(&preview, gctx, solidVO, transpVO, /*allowSelected*/ false);
                        }
                    }

                    preview.EndDrawing(false);  // commits + waits: `target` is finished
                }

                // GPU fill: CoreImage the rendered texture straight into the
                // encoder's NV12 pixel buffer, flipped to the video (top-down)
                // convention. Matches the desktop Metal export path.
                CVPixelBufferRef dst = (CVPixelBufferRef)vf.nativeSurface;
                if (dst != nullptr) {
                    // Tag the source texture as DeviceRGB so CoreImage does a 1:1
                    // value passthrough. With options:nil CoreImage assumes the
                    // texture is linear and re-encodes it to the output's gamma,
                    // which lifts dark (low-brightness) content and makes dimmed
                    // meshes/ground render near full brightness. Mirrors the
                    // desktop export path (xlMetalCanvas::getFrameForExport).
                    NSDictionary* ciOpts = @{ (NSString*)kCIImageColorSpace : (__bridge id)exportColorSpace };
                    CIImage* img = [[CIImage imageWithMTLTexture:target options:ciOpts]
                                    imageByApplyingCGOrientation:kCGImagePropertyOrientationDownMirrored];
                    [ciCtx render:img toCVPixelBuffer:dst];
                }

                if (progress && totalFrames > 0) {
                    progress((double)(frameIndex + 1) / (double)totalFrames);
                }
            }
            return false;  // nativeSurface (CVPixelBuffer) filled, not rgbBuffer
        });

        if (hasAudio) {
            // Pull stereo float samples aligned to the export's start frame.
            long audioOffset = (long)startFrame * audioRate / fps;
            writer.setGetAudioCallback([audioMgr, audioOffset](float* left, float* right, int frameSize) mutable -> bool {
                const long trackSize = audioMgr->GetTrackSize();
                const int n = (int)std::min<long>(frameSize, trackSize - audioOffset);
                if (n > 0) {
                    const float* lp = audioMgr->GetRawLeftDataPtr(audioOffset);
                    const float* rp = audioMgr->GetRawRightDataPtr(audioOffset);
                    if (lp != nullptr) {
                        std::memcpy(left, lp, n * sizeof(float));
                        std::memcpy(right, rp != nullptr ? rp : lp, n * sizeof(float));
                    } else {
                        std::memset(left, 0, frameSize * sizeof(float));
                        std::memset(right, 0, frameSize * sizeof(float));
                    }
                    if (n < frameSize) {
                        std::memset(left + n, 0, (frameSize - n) * sizeof(float));
                        std::memset(right + n, 0, (frameSize - n) * sizeof(float));
                    }
                } else {
                    std::memset(left, 0, frameSize * sizeof(float));
                    std::memset(right, 0, frameSize * sizeof(float));
                }
                audioOffset += frameSize;
                return true;
            });
        }

        writer.initialize();
        writer.exportFrames(totalFrames);
        writer.completeExport();
    } catch (const std::exception& e) {
        NSLog(@"House preview video export failed: %s", e.what());
        ok = false;
    }

    preview.SetOffscreenTarget(nullptr, nullptr);
    delete bgTex;
    CGColorSpaceRelease(exportColorSpace);
    return ok ? YES : NO;
}

@end
