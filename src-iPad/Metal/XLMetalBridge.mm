/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLMetalBridge.h"
#import "../Bridge/XLSequenceDocument.h"
#include "xlStandaloneMetalCanvas.h"
#include "iPadModelPreview.h"
#include "../Bridge/iPadRenderContext.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "render/ViewpointMgr.h"
#include "graphics/xlGraphicsContext.h"
#include "utils/xlImage.h"
#include "graphics/metal/xlMetalGraphicsContext.h"

#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>

#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <spdlog/spdlog.h>

#define PIXEL_SIZE_ON_DIALOGS 2.0

@interface XLMetalBridge ()
- (void)drawBackgroundWithContext:(iPadRenderContext*)ctx
                      graphicsCtx:(xlGraphicsContext*)graphicsCtx
                        solidProg:(xlGraphicsProgram*)solidProg;
@end

@implementation XLMetalBridge {
    std::unique_ptr<xlStandaloneMetalCanvas> _canvas;
    std::unique_ptr<iPadModelPreview> _preview;
    std::string _previewModel;   // set via setPreviewModel:
    BOOL _isModelPreview;        // YES = single-model pane; NO = full house
    BOOL _showViewObjects;       // House Preview view-object visibility toggle
    // Cached background image — loaded once per path change, reused across
    // frames. Texture ownership is manual because xlTexture has no
    // unique_ptr-friendly deleter in the public header; released in
    // dealloc. The raw xlImage bytes are kept so we can re-upload into a
    // fresh texture if we ever need to invalidate without re-decoding.
    xlTexture* _bgTexture;
    std::string _bgLoadedPath;
    int _bgImageWidth;
    int _bgImageHeight;
    // Diagnostics — most-recent silent-fail reason, set on every draw
    // attempt and cleared on success. `_loggedReasons` ensures we only
    // log each unique failure once per bridge lifetime so a per-frame
    // failure doesn't flood the log; the field stays current even
    // when nothing's logging.
    NSString* _errorReason;
    BOOL _hasRenderedSuccessfully;
    std::set<std::string> _loggedReasons;
}

- (instancetype)initWithName:(NSString*)name {
    self = [super init];
    if (self) {
        // is3d=true so the canvas allocates a depth buffer + MSAA target.
        // MeshObject (and other view-object renderers) enable depth testing
        // in drawMeshSolids, which crashes validation if no depth attachment
        // is bound. Depth is harmless for existing 2D model rendering.
        std::string nameStr = std::string([name UTF8String]);
        _canvas = std::make_unique<xlStandaloneMetalCanvas>(nameStr, true);
        _preview = std::make_unique<iPadModelPreview>(_canvas.get());
        _preview->SetName(nameStr);
        _isModelPreview = [name isEqualToString:@"ModelPreview"];
        _showViewObjects = YES;
        _bgTexture = nullptr;
        _bgImageWidth = 0;
        _bgImageHeight = 0;
        _errorReason = nil;
        _hasRenderedSuccessfully = NO;
        // Model preview defaults to 2D (fit-to-window single-model view); the
        // House preview keeps the 3D default.
        if (_isModelPreview) {
            _preview->SetIs3D(false);
        }
    }
    return self;
}

- (void)dealloc {
    delete _bgTexture;
    _bgTexture = nullptr;
    [_errorReason release];
    [super dealloc];
}

- (NSString*)errorReason {
    return _errorReason;
}

- (BOOL)hasRenderedSuccessfully {
    return _hasRenderedSuccessfully;
}

/// Note a silent-fail reason so SwiftUI can surface it. Logs once per
/// unique reason per bridge lifetime so a per-frame failure (eg.
/// "drawable size 0×0" while the layer is being sized) doesn't spam
/// the log; the field stays current regardless so the SwiftUI
/// banner reflects the latest state.
- (void)setErrorReasonInternal:(NSString*)reason {
    NSString* canonical = reason ?: @"";
    if (_errorReason && [_errorReason isEqualToString:canonical]) return;
    [_errorReason release];
    _errorReason = [canonical copy];
    if (canonical.length > 0) {
        std::string s = std::string([canonical UTF8String]);
        if (_loggedReasons.insert(s).second) {
            std::string n = _preview ? _preview->GetName() : "Preview";
            spdlog::warn("XLMetalBridge[{}]: {}", n, s);
        }
    }
}

- (void)clearErrorReason {
    if (_errorReason && _errorReason.length == 0) return;
    [_errorReason release];
    _errorReason = nil;
}

/// Load an image file into an xlImage using ImageIO (CGImageSource). The
/// returned pointer is nullptr on failure. Mirrors the
/// CGImageSourceCreateWithURL path in XLiPadInit.mm — RGBA interleaved
/// bytes so they copy straight into an xlImage buffer.
static std::unique_ptr<xlImage> LoadImageFile(const std::string& path, int& outW, int& outH) {
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
    CGContextRef ctx = CGBitmapContextCreate(rgba.get(), w, h, 8, w * 4, cs,
                                             (uint32_t)kCGImageAlphaPremultipliedLast
                                             | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(cs);
    if (!ctx) {
        CGImageRelease(cgImg);
        return nullptr;
    }
    CGContextSetBlendMode(ctx, kCGBlendModeCopy);
    CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), cgImg);
    CGContextRelease(ctx);
    CGImageRelease(cgImg);

    auto img = std::make_unique<xlImage>(w, h);
    std::memcpy(img->GetData(), rgba.get(), (size_t)w * h * 4);
    outW = w;
    outH = h;
    return img;
}

- (void)attachLayer:(CAMetalLayer*)layer {
    _canvas->setMetalLayer(layer);
}

- (void)setDrawableSize:(CGSize)size scale:(CGFloat)scale {
    _canvas->setSize((int)size.width, (int)size.height);
    _canvas->setScaleFactor(scale);
}

- (void)setPreviewModel:(NSString*)modelName {
    if (modelName == nil || modelName.length == 0) {
        _previewModel.clear();
    } else {
        _previewModel = std::string([modelName UTF8String]);
    }
    if (_preview) {
        _preview->SetCurrentModel(_previewModel);
    }
}

// Visual zoom factor — > 1 = zoomed in (scene appears larger), < 1 = zoomed out.
// The underlying PreviewCamera::zoom is a raw multiplier on the view-matrix
// distance (3D) or an inverse on the ortho half-width (2D), with opposite
// directions in the two modes (see ViewpointMgr.cpp GetViewMatrix and
// ModelPreview::SetZoomDelta). Normalising through a visual factor here lets
// the Swift gesture code stay mode-agnostic.
- (void)setCameraZoom:(float)zoom {
    if (!_preview) return;
    float factor = zoom <= 0.0f ? 1.0f : zoom;
    float raw = _preview->Is3D() ? (1.0f / factor) : factor;
    _preview->ActiveCamera().SetZoom(raw);
}

- (float)cameraZoom {
    if (!_preview) return 1.0f;
    float raw = _preview->ActiveCamera().GetZoom();
    if (raw <= 0.0f) return 1.0f;
    return _preview->Is3D() ? (1.0f / raw) : raw;
}

- (void)setCameraPanX:(float)x panY:(float)y {
    if (!_preview) return;
    _preview->ActiveCamera().SetPanX(x);
    _preview->ActiveCamera().SetPanY(y);
}

- (void)offsetCameraPanX:(float)dx panY:(float)dy {
    if (!_preview) return;
    auto& cam = _preview->ActiveCamera();
    cam.SetPanX(cam.GetPanX() + dx);
    cam.SetPanY(cam.GetPanY() + dy);
}

- (float)cameraPanX {
    return _preview ? _preview->ActiveCamera().GetPanX() : 0.0f;
}

- (float)cameraPanY {
    return _preview ? _preview->ActiveCamera().GetPanY() : 0.0f;
}

- (void)setCameraAngleX:(float)ax angleY:(float)ay {
    if (!_preview) return;
    _preview->ActiveCamera().SetAngleX(ax);
    _preview->ActiveCamera().SetAngleY(ay);
}

- (void)offsetCameraAngleX:(float)dx angleY:(float)dy {
    if (!_preview) return;
    auto& cam = _preview->ActiveCamera();
    cam.SetAngleX(cam.GetAngleX() + dx);
    cam.SetAngleY(cam.GetAngleY() + dy);
}

- (float)cameraAngleX {
    return _preview ? _preview->ActiveCamera().GetAngleX() : 0.0f;
}

- (float)cameraAngleY {
    return _preview ? _preview->ActiveCamera().GetAngleY() : 0.0f;
}

- (void)resetCamera {
    if (_preview) _preview->ResetCamera();
}

- (void)setIs3D:(BOOL)is3d {
    if (_preview) _preview->SetIs3D(is3d ? true : false);
}

- (BOOL)is3D {
    return (_preview && _preview->Is3D()) ? YES : NO;
}

- (void)setShowViewObjects:(BOOL)show {
    _showViewObjects = show;
}

- (BOOL)showViewObjects {
    return _showViewObjects;
}

- (void)invalidateBackgroundCache {
    if (_bgTexture) {
        delete _bgTexture;
        _bgTexture = nullptr;
        _bgLoadedPath.clear();
        _bgImageWidth = 0;
        _bgImageHeight = 0;
    }
}

// Helper: look up the iPadRenderContext from the document. Returns
// nullptr if the document has no render context yet.
static iPadRenderContext* ContextFromDoc(XLSequenceDocument* doc) {
    if (!doc) return nullptr;
    return static_cast<iPadRenderContext*>([doc renderContext]);
}

- (NSArray<NSString*>*)viewpointNamesForDocument:(XLSequenceDocument*)doc {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !_preview) return out;
    // "Default" always leads the list — every preview has a default
    // viewpoint (either the user's saved DefaultCamera2D/3D, or the
    // built-in PreviewCamera::Reset state as a fallback). Treating it
    // as a normal list entry lets the overlay show "Default" next to
    // the camera icon once the user picks it, matching the named-
    // viewpoint flow.
    [out addObject:@"Default"];
    ViewpointMgr& vm = rctx->GetViewpointMgr();
    const bool wantIs3D = _preview->Is3D();
    const int n = wantIs3D ? vm.GetNum3DCameras() : vm.GetNum2DCameras();
    for (int i = 0; i < n; i++) {
        PreviewCamera* c = wantIs3D ? vm.GetCamera3D(i) : vm.GetCamera2D(i);
        if (c) {
            [out addObject:[NSString stringWithUTF8String:c->GetName().c_str()]];
        }
    }
    return out;
}

- (BOOL)applyViewpointNamed:(NSString*)name
                forDocument:(XLSequenceDocument*)doc {
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !_preview || !name) return NO;
    ViewpointMgr& vm = rctx->GetViewpointMgr();
    std::string wanted = std::string([name UTF8String]);
    const bool is3d = _preview->Is3D();

    // "Default" is a virtual entry — the saved user-default camera if
    // one exists, else the built-in PreviewCamera::Reset state.
    // Matches desktop "Restore Default Viewpoint" semantics and lets
    // users see "Default" reflected in the overlay label.
    if (wanted == "Default") {
        PreviewCamera* def = is3d ? vm.GetDefaultCamera3D()
                                  : vm.GetDefaultCamera2D();
        if (def) {
            _preview->ActiveCamera() = *def;
        } else {
            _preview->ResetCamera();
        }
        return YES;
    }

    const int n = is3d ? vm.GetNum3DCameras() : vm.GetNum2DCameras();
    for (int i = 0; i < n; i++) {
        PreviewCamera* c = is3d ? vm.GetCamera3D(i) : vm.GetCamera2D(i);
        if (c && c->GetName() == wanted) {
            // Copy the saved camera state into the preview's active
            // camera. PreviewCamera::operator= mirrors every field and
            // invalidates the cached view matrix so the next draw
            // rebuilds with the new position/angles.
            _preview->ActiveCamera() = *c;
            return YES;
        }
    }
    return NO;
}

- (BOOL)saveCurrentViewAs:(NSString*)name
              forDocument:(XLSequenceDocument*)doc {
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !_preview || !name || name.length == 0) return NO;
    // "Default" is a reserved virtual entry — don't let a user save
    // over it. Desktop exposes a separate "Save as default viewpoint"
    // action for that; we can add it later if needed.
    if ([name isEqualToString:@"Default"]) return NO;
    ViewpointMgr& vm = rctx->GetViewpointMgr();
    const bool is3d = _preview->Is3D();
    std::string n = std::string([name UTF8String]);
    if (!vm.IsNameUnique(n, is3d)) return NO;
    vm.AddCamera(n, &_preview->ActiveCamera(), is3d);
    return rctx->SaveViewpoints();
}

- (BOOL)deleteViewpointNamed:(NSString*)name
                 forDocument:(XLSequenceDocument*)doc {
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !_preview || !name) return NO;
    if ([name isEqualToString:@"Default"]) return NO;  // can't delete virtual
    ViewpointMgr& vm = rctx->GetViewpointMgr();
    const bool is3d = _preview->Is3D();
    std::string wanted = std::string([name UTF8String]);
    const int n = is3d ? vm.GetNum3DCameras() : vm.GetNum2DCameras();
    for (int i = 0; i < n; i++) {
        PreviewCamera* c = is3d ? vm.GetCamera3D(i) : vm.GetCamera2D(i);
        if (c && c->GetName() == wanted) {
            if (is3d) vm.DeleteCamera3D(i);
            else vm.DeleteCamera2D(i);
            return rctx->SaveViewpoints();
        }
    }
    return NO;
}

- (void)restoreDefaultViewpointForDocument:(XLSequenceDocument*)doc {
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!_preview) return;
    // Prefer the user's saved Default if one exists; otherwise fall
    // back to PreviewCamera::Reset() defaults. Matches desktop
    // "Restore Default Viewpoint" semantics.
    if (rctx) {
        ViewpointMgr& vm = rctx->GetViewpointMgr();
        PreviewCamera* def = _preview->Is3D()
            ? vm.GetDefaultCamera3D()
            : vm.GetDefaultCamera2D();
        if (def) {
            _preview->ActiveCamera() = *def;
            return;
        }
    }
    _preview->ResetCamera();
}

/// Accumulate `m`'s world-coord bounding box into the running min/max.
/// Uses ModelScreenLocation's center + render size, since UpdateBoundingBox
/// needs the Node list populated via PrepareToDraw — we want a cheap
/// approximation that works for any loaded model. Returns true if `m`
/// contributed (i.e. had valid dimensions).
static bool AccumulateModelBounds(Model* m, float& minX, float& minY,
                                   float& maxX, float& maxY) {
    if (!m) return false;
    auto& loc = m->GetModelScreenLocation();
    float cx = loc.GetHcenterPos();
    float cy = loc.GetVcenterPos();
    // GetRestorableMWidth / GetRestorableMHeight return post-scale world
    // extents for most model types. Fall back to a small radius if the
    // model doesn't report a meaningful size.
    float halfW = std::max(1.0f, loc.GetMWidth() * 0.5f);
    float halfH = std::max(1.0f, loc.GetMHeight() * 0.5f);
    minX = std::min(minX, cx - halfW);
    maxX = std::max(maxX, cx + halfW);
    minY = std::min(minY, cy - halfH);
    maxY = std::max(maxY, cy + halfH);
    return true;
}

/// Apply a bounding-box-to-viewport fit to the active PreviewCamera.
/// 2D sets zoom + pan so the bbox fills the virtual canvas area with
/// a small margin; 3D keeps the current rotation but adjusts pan so
/// the bbox centre is at world origin and distance so the larger of
/// the bbox dimensions fits the 45° vertical FOV. Called after the
/// caller has accumulated bounds from one or more models.
- (void)fitToBoundingBoxMinX:(float)minX minY:(float)minY
                        maxX:(float)maxX maxY:(float)maxY
                         ctx:(iPadRenderContext*)rctx {
    if (!_preview) return;
    if (maxX <= minX || maxY <= minY) return;

    const float bboxW = maxX - minX;
    const float bboxH = maxY - minY;
    const float cx = (minX + maxX) * 0.5f;
    const float cy = (minY + maxY) * 0.5f;
    // Leave ~5% margin around the fitted content so models at the
    // bounding box edge don't render flush with the pane border.
    constexpr float margin = 0.95f;

    PreviewCamera& cam = _preview->ActiveCamera();
    if (_preview->Is3D()) {
        // 3D: pan moves the scene so its centre sits at world origin
        // (the rotation / distance pivot), then distance is chosen to
        // fit the larger of the bbox dimensions in the 45° FOV at the
        // current aspect ratio. We don't touch angleX/Y/Z — this is a
        // "frame the bbox from the current viewpoint" operation.
        cam.SetPanX(-cx);
        cam.SetPanY(-cy);
        cam.SetPanZ(0.0f);

        const int paneW = _canvas ? _canvas->getWidth() : 0;
        const int paneH = _canvas ? _canvas->getHeight() : 0;
        const float aspect = (paneH > 0) ? (float)paneW / (float)paneH : 1.0f;
        // Vertical half-FOV at 45° total. For the horizontal axis we
        // divide by aspect to pick up pane-width-constrained scenes.
        const float tanHalfFov = std::tan(glm::radians(22.5f));
        const float neededYDist = (bboxH * 0.5f) / tanHalfFov;
        const float neededXDist = (bboxW * 0.5f) / (tanHalfFov * aspect);
        float dist = std::max(neededYDist, neededXDist) / margin;
        if (dist < 100.0f) dist = 100.0f;                 // sanity floor
        cam.SetDistance(-dist);
        cam.SetZoom(1.0f);
    } else {
        // 2D: the pane's ortho mapping applies scale2d = min(h/vH, w/vW)
        // automatically, so we just choose a PreviewCamera zoom that
        // makes the bbox fill the virtual canvas (after scale2d the
        // bbox lands in the pane). Pan is in scaled pixel space
        // pre-translation, and the 2D view matrix adds a
        // `virtualW/2` shift when Display2DCenter0 is on — handle that
        // here so fit is correct for both layouts.
        int virtualW = 0;
        int virtualH = 0;
        _preview->GetVirtualCanvasSize(virtualW, virtualH);
        if (virtualW <= 0 || virtualH <= 0) {
            // Fallback when the pane hasn't been sized yet — nothing
            // meaningful we can compute; reset instead.
            cam.Reset();
            return;
        }
        float zoom = std::min((float)virtualW / bboxW,
                              (float)virtualH / bboxH) * margin;
        if (zoom < 0.01f) zoom = 0.01f;
        cam.SetZoom(zoom);
        cam.SetZoomCorrX(0.0f);
        cam.SetZoomCorrY(0.0f);
        // Place bbox centre at virtual canvas centre. With center2D0
        // on, the view matrix already shifts world-X by +virtualW/2,
        // so the pan target is -cx; otherwise we need to also recentre
        // the virtual canvas on (virtualW/2) → pan target is
        // (virtualW/2 - cx).
        const bool c0 = rctx ? rctx->GetDisplay2DCenter0() : false;
        const float vhx = (float)virtualW * 0.5f;
        const float vhy = (float)virtualH * 0.5f;
        float panX = c0 ? (-cx) : (vhx - cx);
        float panY = vhy - cy;
        cam.SetPanX(panX);
        cam.SetPanY(panY);
        cam.SetPanZ(0.0f);
    }
}

- (BOOL)fitAllModelsForDocument:(XLSequenceDocument*)doc {
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !_preview) return NO;

    float minX = std::numeric_limits<float>::infinity();
    float minY = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    bool any = false;
    // Respect the active layout-group filter so Fit All matches what
    // the user actually sees — not every model in the show.
    for (Model* m : rctx->GetModelsForActivePreview()) {
        if (AccumulateModelBounds(m, minX, minY, maxX, maxY)) any = true;
    }
    if (!any) return NO;
    [self fitToBoundingBoxMinX:minX minY:minY maxX:maxX maxY:maxY ctx:rctx];
    return YES;
}

- (BOOL)fitModelNamed:(NSString*)name
          forDocument:(XLSequenceDocument*)doc {
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !_preview || !name || name.length == 0) return NO;

    std::string wanted = std::string([name UTF8String]);
    Model* target = rctx->GetModelManager()[wanted];
    if (!target) return NO;

    // Only fit if the model is actually visible in the current layout
    // group — otherwise fitting to an offscreen model would leave the
    // user staring at empty space.
    bool visible = false;
    for (Model* m : rctx->GetModelsForActivePreview()) {
        if (m == target) { visible = true; break; }
    }
    if (!visible) return NO;

    float minX = std::numeric_limits<float>::infinity();
    float minY = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    if (!AccumulateModelBounds(target, minX, minY, maxX, maxY)) return NO;
    [self fitToBoundingBoxMinX:minX minY:minY maxX:maxX maxY:maxY ctx:rctx];
    return YES;
}

- (void)drawModelsForDocument:(XLSequenceDocument*)doc atMS:(int)frameMS pointSize:(float)pointSize {
    // Reset any stale banner from the previous frame; each path below
    // (early return, info-banner, or successful draw) sets the correct
    // state for this frame.
    [self clearErrorReason];

    if (_canvas->getMetalLayer() == nil) {
        [self setErrorReasonInternal:@"No Metal layer attached"];
        return;
    }
    if (_canvas->getWidth() == 0 || _canvas->getHeight() == 0) {
        [self setErrorReasonInternal:
            [NSString stringWithFormat:@"Drawable size is %dx%d (waiting for layout)",
                _canvas->getWidth(), _canvas->getHeight()]];
        return;
    }

    iPadRenderContext* ctx = static_cast<iPadRenderContext*>([doc renderContext]);
    if (!ctx) {
        [self setErrorReasonInternal:@"No render context (no document loaded)"];
        return;
    }

    // Set channel data on all models for this frame
    ctx->SetModelColors(frameMS);

    // Set current frame time so models can query it
    _preview->SetCurrentFrameTime(frameMS);

    // House Preview needs a virtual canvas so the 2D ortho projection in
    // iPadModelPreview::StartDrawing maps world coords (model positions
    // saved in xlights_rgbeffects.xml against previewWidth × previewHeight)
    // onto pixel coords at the current pane size. Without this, scale2d
    // stays 1 and models at e.g. (600, 400) land off-screen in 2D mode.
    // Model Preview stays at 0×0 so DisplayEffectOnWindow's own
    // fit-to-window scaling still applies.
    if (_isModelPreview) {
        _preview->SetVirtualCanvasSize(0, 0);
        _preview->SetCenter2D0(false);
    } else {
        _preview->SetVirtualCanvasSize(ctx->GetPreviewWidth(),
                                       ctx->GetPreviewHeight());
        _preview->SetCenter2D0(ctx->GetDisplay2DCenter0());
    }

    // Start a single drawing pass — acquires one drawable
    if (!_preview->StartDrawing(pointSize)) {
        [self setErrorReasonInternal:
            [NSString stringWithUTF8String:_preview->GetLastStartDrawingFailure().c_str()]];
        return;
    }

    auto* graphicsCtx = _preview->getCurrentGraphicsContext();
    auto* solidProg = _preview->getCurrentSolidProgram();
    auto* transparentProg = _preview->getCurrentTransparentProgram();
    auto* solidVOProg = _preview->getCurrentSolidViewObjectProgram();
    auto* transparentVOProg = _preview->getCurrentTransparentViewObjectProgram();

    if (_isModelPreview) {
        // Model Preview pane: draw ONLY the selected model (or group), fit-to-window,
        // ignoring its world placement. Mirrors desktop ModelPreview's 2D
        // single-model path (ModelPreview.cpp:538): DisplayEffectOnWindow
        // applies its own scale + centering to fit the model to the preview
        // dimensions rather than using ModelScreenLocation. ModelGroups build
        // pseudo-nodes sized to their default buffer style, so the same call
        // works for groups — constituent models appear at their group-buffer
        // positions, not their world positions. If nothing is selected, the
        // pane stays black (clear-only) — we intentionally do NOT fall through
        // to the full-house path.
        if (_previewModel.empty()) {
            [self setErrorReasonInternal:@"No model selected"];
        } else {
            auto& models = ctx->GetModelManager();
            Model* m = models[_previewModel];
            if (m) {
                m->DisplayEffectOnWindow(_preview.get(), pointSize);
            } else {
                [self setErrorReasonInternal:
                    [NSString stringWithFormat:@"Model '%s' not found in layout",
                        _previewModel.c_str()]];
            }
        }
    } else {
        // House Preview: models of the active layout group at their
        // world positions, view objects on top (Default group only).
        // Sort models back-to-front by camera-space Z of their world
        // centre so alpha-blended pixels from one model composite over
        // models behind them. Matches ModelPreview::RenderModels on
        // desktop.

        // Background image — only rendered in 2D mode, matching desktop
        // (ModelPreview.cpp:1411). Brightness/alpha/scale come from the
        // active layout group (Default or named); iPad never edits
        // them. The texture is cached between frames and re-fetched
        // only when the path changes. Gated behind the same "View
        // Objects" toggle as the house-mesh/ground/terrain loop below,
        // so users have one switch that hides every non-pixel scene
        // element (background, view objects, and once Phase D-8 lands,
        // the 2D grid and bounding-box overlays too).
        if (!_preview->Is3D() && _showViewObjects) {
            [self drawBackgroundWithContext:ctx graphicsCtx:graphicsCtx solidProg:solidProg];
        }

        std::vector<Model*> models = ctx->GetModelsForActivePreview();
        if (models.empty()) {
            // Could be a freshly-loaded show with no models, an empty
            // layout group, or — most often — the show folder load
            // hasn't populated models yet. SwiftUI surfaces the
            // reason; the actual draw still completes (cleared
            // background) so we don't paint over the message.
            [self setErrorReasonInternal:@"No models in active preview"];
        }
        const glm::mat4& viewMatrix = _preview->GetViewMatrix();
        std::vector<std::pair<Model*, float>> keyed;
        keyed.reserve(models.size());
        for (Model* model : models) {
            if (!model) continue;
            glm::vec3 c = model->GetModelScreenLocation().GetCenterPosition();
            float z = (viewMatrix * glm::vec4(c, 1.0f)).z;
            keyed.emplace_back(model, z);
        }
        std::stable_sort(keyed.begin(), keyed.end(),
                         [](const std::pair<Model*, float>& a, const std::pair<Model*, float>& b) {
                             return a.second < b.second;
                         });
        const bool is3d = _preview->Is3D();
        for (const auto& [model, z] : keyed) {
            // Pass the current 2D/3D state rather than a hardcoded true —
            // it drives PrepareToDraw's draw_3d flag (which controls
            // worldPos_z usage and the 2D perspective rotation) and the
            // uiCaches key. Telling a 2D render "we're 3D" made
            // BoxedScreenLocation place models at their saved Z, and with
            // ortho left-handed near=1/far=0 any non-zero Z put geometry
            // outside the frustum.
            model->DisplayModelOnWindow(_preview.get(), graphicsCtx, solidProg, transparentProg,
                                         is3d, nullptr, false, false, false, 0, nullptr);
        }

        // View objects (house meshes, ground images, gridlines, terrain).
        // Only the Default layout group owns view objects (desktop hard-
        // codes their layout_group to "Default"); named groups skip the
        // loop entirely. Also gated on the "View Objects" toggle.
        if (_showViewObjects && ctx->ActivePreviewShowsViewObjects()) {
            auto& allObjects = ctx->GetAllObjects();
            for (auto it = allObjects.begin(); it != allObjects.end(); ++it) {
                ViewObject* vo = it->second;
                if (vo) {
                    vo->Draw(_preview.get(), graphicsCtx, solidVOProg, transparentVOProg, false);
                }
            }
        }
    }

    // Finish and present. `_errorReason` reflects this frame's state:
    // nil for a normal successful draw, set to an info banner for
    // "No model selected" / "No models in active preview" (which are
    // technically successful clear-to-background draws — SwiftUI
    // surfaces them as informational rather than failure).
    _preview->EndDrawing(true);
    _hasRenderedSuccessfully = YES;
}

// Lazy-load + enqueue the 2D background draw. No-op when no path is
// configured or the texture fails to load. Draw math mirrors
// ModelPreview.cpp:1431 — image sits in world coords 0..virtualW by
// 0..virtualH (optionally scaled to preserve aspect when !scaleImage),
// shifted by -virtualW/2 in X when Display2DCenter0 is on. Brightness
// stays 0..100; alpha is percent → 0..255.
- (void)drawBackgroundWithContext:(iPadRenderContext*)rctx
                      graphicsCtx:(xlGraphicsContext*)gctx
                        solidProg:(xlGraphicsProgram*)solidProg {
    const std::string& path = rctx->GetActiveBackgroundImage();
    if (path.empty() || !gctx || !solidProg) return;

    if (_bgTexture == nullptr || path != _bgLoadedPath) {
        int w = 0, h = 0;
        auto img = LoadImageFile(path, w, h);
        if (!img || w <= 0 || h <= 0) {
            return;
        }
        delete _bgTexture;
        _bgTexture = gctx->createTexture(*img, path, /* finalize */ true);
        _bgLoadedPath = path;
        _bgImageWidth = w;
        _bgImageHeight = h;
    }
    if (!_bgTexture) return;

    const int virtualW = rctx->GetPreviewWidth();
    const int virtualH = rctx->GetPreviewHeight();
    const bool scaleImage = rctx->GetActiveScaleBackgroundImage();
    const bool center0 = rctx->GetDisplay2DCenter0();
    const int brightness = rctx->GetActiveBackgroundBrightness();
    const int alpha = (int)((rctx->GetActiveBackgroundAlpha() * 255) / 100);

    float scaleh = 1.0f;
    float scalew = 1.0f;
    if (!scaleImage && virtualW > 0 && virtualH > 0 && _bgImageWidth > 0 && _bgImageHeight > 0) {
        // Preserve the image's aspect ratio inside the virtual preview
        // rectangle — the axis that would overflow gets pulled back in.
        float nscaleh = (float)_bgImageHeight / (float)virtualH;
        float nscalew = (float)_bgImageWidth / (float)virtualW;
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
    float x = 0.0f;
    if (center0) {
        x = -(float)virtualW / 2.0f;
    }
    const float x2 = x + (float)virtualW * scalew;
    const float y2 = (float)virtualH * scaleh;

    xlTexture* tex = _bgTexture;
    solidProg->addStep([tex, x, y2, x2, brightness, alpha](xlGraphicsContext* c) {
        c->drawTexture(tex, x, y2, x2, 0.0f,
                       0.0f, 0.0f, 1.0f, 1.0f,
                       /* smoothScale */ true,
                       brightness, alpha);
    });
}

@end
