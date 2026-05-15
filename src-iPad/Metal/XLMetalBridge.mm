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
#include "models/ModelGroup.h"
#include "models/SubModel.h"
#include "models/TerrainObject.h"
#include "models/TerrainScreenLocation.h"
#include "XmlSerializer/XmlSerializer.h"
#include "models/Node.h"
#include "models/RulerObject.h"
#include <pugixml.hpp>
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "render/ViewpointMgr.h"
#include "models/ModelScreenLocation.h"
#include "models/PolyPointScreenLocation.h"
#include "models/PolyLineModel.h"
#include "utils/VectorMath.h"
#include "graphics/xlGraphicsContext.h"
#include "models/handles/Handles.h"
#include "models/handles/DragSession.h"
#include "models/handles/HitTest.h"
#include "utils/xlImage.h"
#include "graphics/metal/xlMetalGraphicsContext.h"

#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>
#import <UIKit/UIKit.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <vector>
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
    BOOL _isLayoutEditor;        // YES = LayoutEditor pane (selection / handles enabled)
    BOOL _showFirstPixel;        // J-2 — `highlightFirst` arg to DisplayModelOnWindow
    BOOL _showViewObjects;       // House Preview view-object visibility toggle
    std::string _selectedModelName;  // J-2 — Layout Editor selection ring (primary)
    std::set<std::string> _extraSelectedModels;  // J-4 — multi-select secondary set
    std::string _selectedGroupName;        // J-6 — sidebar group sync (members tinted)
    std::string _selectedViewObjectName;   // J-6 — sidebar object sync (handles drawn)
    BOOL _showLayoutGrid;            // J-2 — Layout Editor 2D grid overlay
    BOOL _showLayoutBoundingBox;     // J-2 — Layout Editor canvas bbox
    BOOL _layoutOverlaysSeeded;      // first draw seeds from rgbeffects state
    BOOL _snapToGrid;                // J-2 — drag-to-move snap toggle
    BOOL _uniformModifier;           // J-2 UX — toolbar Uniform toggle
    NSInteger _lockAxis;             // J-2 UX — toolbar Lock Axis (0=Free, 1=X, 2=Y, 3=Z)
    BOOL _handleDragNeedsLatch;      // J-2 — true on first dragHandle call after a pick
    // J-2 UX — 3D body-drag anchor. Latched on
    // `beginBodyDrag3DForModel:` so subsequent drag updates can
    // compute the world delta on a fixed plane chosen at drag-
    // begin time. The plane is whichever of XY / XZ / YZ best
    // matches the current camera (top-down → XZ, side → YZ,
    // front → XY), so a drag from above slides the model along
    // the floor (X+Z) instead of fighting the user's view.
    // J-15 — three target-is-VO flags. When YES, the
    // corresponding `_…ModelName` field is actually a VO name
    // and the apply/end paths route to ViewObjectManager + mark
    // the VO dirty rather than a Model.
    BOOL _bodyDrag3DTargetIsVO;
    BOOL _pinchScaleTargetIsVO;
    BOOL _twistRotateTargetIsVO;
    BOOL _bodyDrag3DActive;
    glm::vec3 _bodyDrag3DSavedCenter;
    glm::vec3 _bodyDrag3DAnchor;
    glm::vec3 _bodyDrag3DPlanePoint;
    glm::vec3 _bodyDrag3DPlaneNormal;
    ModelScreenLocation::MSLPLANE _bodyDrag3DPlane;
    std::string _bodyDrag3DModelName;
    // J-2 UX — pinch-on-model = uniform scale.
    BOOL _pinchScaleActive;
    glm::vec3 _pinchScaleSavedScale;
    std::string _pinchScaleModelName;
    // J-2 UX — two-finger twist on model = rotate Z.
    BOOL _twistRotateActive;
    glm::vec3 _twistRotateSavedRotation;
    std::string _twistRotateModelName;
    // When non-null, handle drags route through the descriptor
    // DragSession API. Applies to descriptor-implemented paths
    // (e.g. 3D-translate via axis arrows on Boxed); other paths
    // still consult the `_handleDragNeedsLatch` flag.
    std::unique_ptr<handles::DragSession> _dragSession;
    // J-14 — when non-empty, the active drag session is on a
    // ViewObject's screen location rather than a Model's. End-
    // of-drag commits this name to the VO dirty set so save
    // picks up the change. Cleared on session end.
    std::string _dragSessionViewObjectName;
    handles::Id _dragSessionStartId;
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
        _isLayoutEditor = [name isEqualToString:@"LayoutEditor"];
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

- (void)setSelectedModel:(NSString*)name {
    if (name == nil || name.length == 0) {
        _selectedModelName.clear();
    } else {
        _selectedModelName = std::string([name UTF8String]);
    }
}

- (void)setSelectedGroup:(NSString*)name {
    if (name == nil || name.length == 0) {
        _selectedGroupName.clear();
    } else {
        _selectedGroupName = name.UTF8String;
    }
}

- (void)setSelectedViewObject:(NSString*)name {
    if (name == nil || name.length == 0) {
        _selectedViewObjectName.clear();
    } else {
        _selectedViewObjectName = name.UTF8String;
    }
}

- (void)setExtraSelectedModels:(NSArray<NSString*>*)names {
    _extraSelectedModels.clear();
    if (names == nil) return;
    for (NSString* n in names) {
        if (n.length > 0) {
            _extraSelectedModels.insert(std::string([n UTF8String]));
        }
    }
}

- (void)setShowLayoutGrid:(BOOL)show {
    _showLayoutGrid = show;
    _layoutOverlaysSeeded = YES; // explicit set wins over the rgbeffects seed
}

- (BOOL)showLayoutGrid {
    return _showLayoutGrid;
}

- (void)setShowLayoutBoundingBox:(BOOL)show {
    _showLayoutBoundingBox = show;
    _layoutOverlaysSeeded = YES;
}

- (BOOL)showLayoutBoundingBox {
    return _showLayoutBoundingBox;
}

- (void)setSnapToGrid:(BOOL)snap {
    _snapToGrid = snap;
}

- (BOOL)snapToGrid {
    return _snapToGrid;
}

- (void)setUniformModifier:(BOOL)uniform {
    _uniformModifier = uniform;
}

- (BOOL)uniformModifier {
    return _uniformModifier;
}

- (void)setLockAxis:(NSInteger)axis {
    // Clamp to valid range. Unknown values fall back to Free so
    // the toolbar can't accidentally pin drags to an invalid axis.
    if (axis < 0 || axis > 3) axis = 0;
    _lockAxis = axis;
}

- (NSInteger)lockAxis {
    return _lockAxis;
}


- (void)setShowFirstPixel:(BOOL)show {
    _showFirstPixel = show;
}

- (BOOL)showFirstPixel {
    return _showFirstPixel;
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

    // -ffast-math implies -ffinite-math-only on the iPadLib Release build
    // (inherited from the project-level OTHER_CFLAGS). infinity() would
    // fold to 0 and break the min/max accumulation. Use finite sentinels.
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
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

    // Finite sentinels — see fitAllModelsForDocument above.
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    if (!AccumulateModelBounds(target, minX, minY, maxX, maxY)) return NO;
    [self fitToBoundingBoxMinX:minX minY:minY maxX:maxX maxY:maxY ctx:rctx];
    return YES;
}

// Inverse of `iPadModelPreview::StartDrawing`'s 2D View matrix.
// Returns world (X, Y) for a touch in MTKView point coordinates.
// Returns NO if the canvas hasn't been sized yet or the
// projection denominator is zero. `worldDenom` (out, optional)
// receives `zoom * scale2d` so callers can convert screen-space
// distances (handle radius, etc.) to world units.
- (BOOL)unprojectScreenPoint:(CGPoint)point
                  toWorldX:(float*)outWorldX
                   worldY:(float*)outWorldY
                worldDenom:(float*)outDenom
                forContext:(iPadRenderContext*)rctx {
    int canvasW = _canvas->getWidth();
    int canvasH = _canvas->getHeight();
    if (canvasW <= 0 || canvasH <= 0) return NO;

    double scale = _canvas->getScaleFactor();
    if (scale <= 0) scale = 1.0;
    double touchPxX = (double)point.x * scale;
    double touchPxY = (double)point.y * scale;
    double windowPxX = touchPxX;
    double windowPxY = (double)canvasH - touchPxY;

    PreviewCamera& cam = _preview->Get2DCamera();
    int virtualW = 0, virtualH = 0;
    _preview->GetVirtualCanvasSize(virtualW, virtualH);

    float scale2d = 1.0f, scaleCorrX = 0.0f, scaleCorrY = 0.0f;
    if (virtualW != 0 && virtualH != 0) {
        float scale2dh = (float)canvasH / (float)virtualH;
        float scale2dw = (float)canvasW / (float)virtualW;
        if (scale2dh < scale2dw) {
            scale2d = scale2dh;
            scaleCorrX = ((scale2dw * (float)virtualW - (scale2d * (float)virtualW)) * cam.GetZoom()) / 2.0f;
        } else {
            scale2d = scale2dw;
            scaleCorrY = ((scale2dh * (float)virtualH - (scale2d * (float)virtualH)) * cam.GetZoom()) / 2.0f;
        }
    }

    float zoom = cam.GetZoom();
    float denom = zoom * scale2d;
    if (denom == 0.0f) return NO;

    float worldX = ((float)windowPxX
                    - cam.GetPanX() * zoom + cam.GetZoomCorrX() - scaleCorrX) / denom;
    float worldY = ((float)windowPxY
                    - cam.GetPanY() * zoom + cam.GetZoomCorrY() - scaleCorrY) / denom;
    if (rctx->GetDisplay2DCenter0() && virtualW != 0) {
        worldX -= ((float)virtualW) / 2.0f;
    }
    if (outWorldX) *outWorldX = worldX;
    if (outWorldY) *outWorldY = worldY;
    if (outDenom)  *outDenom  = denom;
    return YES;
}

// Convert a touch point (UIKit Y-down points) into the window-pixel
// coordinates the desktop ScreenLocation API expects. The
// ScreenLocation methods themselves flip Y internally before
// calling VectorMath::ScreenPosToWorldRay, matching desktop's
// "mouse coords are Y-down" convention.
static void PointToWindowPixels(const CGPoint& p, double scale,
                                int* outX, int* outY) {
    double sf = (scale > 0) ? scale : 1.0;
    *outX = (int)std::round((double)p.x * sf);
    *outY = (int)std::round((double)p.y * sf);
}

// Build a world-space ray from a touch point, using the preview's
// ProjView matrix. Used by `Model::HitTest3D` for model-body picks.
static void TouchPointToWorldRay(const CGPoint& p, double scale,
                                  iPadModelPreview* preview,
                                  glm::vec3& outOrigin,
                                  glm::vec3& outDirection) {
    int mx = 0, my = 0;
    PointToWindowPixels(p, scale, &mx, &my);
    VectorMath::ScreenPosToWorldRay(
        mx, preview->getHeight() - my,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        outOrigin, outDirection);
}

- (nullable NSString*)pickModelAtScreenPoint:(CGPoint)point
                                    viewSize:(CGSize)viewSize
                                 forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return nil;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return nil;

    // SubModels share their parent's screenLocation (SubModel.h:29)
    // and aren't standalone layout entities — desktop's
    // LayoutPanel doesn't expose them as selectable. Filter them
    // out of pick/draw so a tap on a submodel's pixels selects
    // the parent.
    auto isSubModel = [](Model* m) {
        return m && m->GetDisplayAs() == DisplayAsType::SubModel;
    };

    // 3D path uses the existing ray-cast against each model's
    // ScreenLocation. Returns the closest hit (smallest
    // intersection distance).
    if (_preview->Is3D()) {
        glm::vec3 ray_origin, ray_direction;
        TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                              ray_origin, ray_direction);
        Model* best = nullptr;
        // Finite sentinel — see fitAllModelsForDocument. infinity() folds
        // to 0 under -ffast-math and the `dist < bestDist` check would
        // reject every 3D hit, breaking tap-to-select on iPad Release.
        float bestDist = std::numeric_limits<float>::max();
        for (Model* m : rctx->GetModelsForActivePreview()) {
            if (!m || isSubModel(m)) continue;
            float dist = 0.0f;
            if (m->GetModelScreenLocation().HitTest3D(
                    ray_origin, ray_direction, dist) && dist < bestDist) {
                best = m;
                bestDist = dist;
            }
        }
        return best ? [NSString stringWithUTF8String:best->GetName().c_str()] : nil;
    }

    // 2D path: inverse-project the touch into world coords and
    // box-test each model in reverse draw order.
    float worldX = 0, worldY = 0, denom = 0;
    if (![self unprojectScreenPoint:point toWorldX:&worldX worldY:&worldY
                          worldDenom:&denom forContext:rctx]) {
        return nil;
    }
    auto models = rctx->GetModelsForActivePreview();
    for (auto it = models.rbegin(); it != models.rend(); ++it) {
        Model* m = *it;
        if (!m || isSubModel(m)) continue;
        auto& loc = m->GetModelScreenLocation();
        float cx = loc.GetHcenterPos();
        float cy = loc.GetVcenterPos();
        float halfW = std::max(1.0f, loc.GetMWidth() * 0.5f);
        float halfH = std::max(1.0f, loc.GetMHeight() * 0.5f);
        if (worldX >= cx - halfW && worldX <= cx + halfW &&
            worldY >= cy - halfH && worldY <= cy + halfH) {
            return [NSString stringWithUTF8String:m->GetName().c_str()];
        }
    }
    return nil;
}

- (NSInteger)pickHandleAtScreenPoint:(CGPoint)point
                            viewSize:(CGSize)viewSize
                         forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return -1;
    if (_selectedModelName.empty()) return -1;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return -1;
    Model* m = rctx->GetModelManager()[_selectedModelName];
    if (!m) return -1;
    auto& loc = m->GetModelScreenLocation();
    // Exit early on locked / from-base models so a drag gesture never
    // engages on a model that can't be edited (the user then gets the
    // camera-orbit fallback signalling "can't edit" instead of an
    // axis highlight followed by silent no-op).
    if (loc.IsLocked()) return -1;
    if (m->IsFromBase()) return -1;

    if (_preview->Is3D()) {
        handles::Tool newApiTool = handles::Tool::Translate;
        bool toolSupportedByNewApi = false;
        switch (loc.GetAxisTool()) {
            case ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE:
                newApiTool = handles::Tool::Translate;
                toolSupportedByNewApi = true;
                break;
            case ModelScreenLocation::MSLTOOL::TOOL_SCALE:
                newApiTool = handles::Tool::Scale;
                toolSupportedByNewApi = true;
                break;
            case ModelScreenLocation::MSLTOOL::TOOL_ROTATE:
                newApiTool = handles::Tool::Rotate;
                toolSupportedByNewApi = true;
                break;
            default:
                break;
        }
        const float zoom = _preview->GetCameraZoomForHandles();
        const int hscale = _preview->GetHandleScale();
        handles::ViewParams view;
        view.axisArrowLength = loc.GetAxisArrowLength(zoom, hscale);
        view.axisHeadLength  = loc.GetAxisHeadLength(zoom, hscale);
        view.axisRadius      = loc.GetAxisRadius(zoom, hscale);
        auto descriptors = toolSupportedByNewApi
            ? m->GetHandles(handles::ViewMode::ThreeD, newApiTool, view)
            : std::vector<handles::Descriptor>{};
        if (!descriptors.empty()) {
            handles::ScreenProjection proj;
            proj.projViewMatrix = _preview->GetProjViewMatrix();
            proj.viewportWidth  = _canvas->getWidth();
            proj.viewportHeight = _canvas->getHeight();
            double scaleFactor = _canvas->getScaleFactor();
            if (scaleFactor <= 0) scaleFactor = 1.0;
            glm::vec2 touchPx{
                static_cast<float>(point.x * scaleFactor),
                static_cast<float>(point.y * scaleFactor)
            };
            handles::HitTestOptions opts;
            opts.handleTolerance     = 60.0f;  // touch slop in pixels
            opts.axisHandleTolerance = 28.0f;  // tighter on the X/Y/Z gizmos
                                                // — their heads project near the
                                                // model body in top-down / side
                                                // views and a 60pt halo would
                                                // swallow body-drag taps.
            opts.preferAxisHandles   = true;
            opts.ignoreNonEditable   = true;
            if (auto hit = handles::HitTest(descriptors, proj, touchPx, opts)) {
                handles::WorldRay startRay;
                TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                                      startRay.origin, startRay.direction);
                // Refresh `active_plane` from the current camera
                // angles for PolyPoint vertex/segment drags. The
                // session reads this on construction to pick which
                // world plane to project drags onto — without the
                // refresh the plane stays at whatever
                // `InitializeLocation` (or a prior camera) seeded,
                // and post-orbit drags slide along the wrong axis.
                if (dynamic_cast<PolyPointScreenLocation*>(&loc)) {
                    loc.RefreshActivePlaneFromCamera(_preview.get());
                }
                auto session = m->BeginDrag(hit->id, startRay);
                if (session) {
                    _dragSession = std::move(session);
                    _dragSessionStartId = hit->id;
                    // Return any non-negative value — the value
                    // itself is unused for new-API drags (dragHandle
                    // routes via `_dragSession` regardless of the
                    // handleIndex Swift stores).
                    return 0;
                }
            }
        }
        // If the descriptor HitTest missed, the touch is treated as
        // no-hit.
        return -1;
    }
    // 2D path: descriptor hit-test against the model's TwoD handle set
    // (Endpoint / Vertex / ResizeCorner / Rotate / Segment / etc.).
    handles::ViewParams view2d;
    const auto descs2d = m->GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate, view2d);
    if (descs2d.empty()) return -1;
    handles::ScreenProjection proj2d;
    proj2d.projViewMatrix = _preview->GetProjViewMatrix();
    proj2d.viewportWidth  = _canvas->getWidth();
    proj2d.viewportHeight = _canvas->getHeight();
    double sf2d = _canvas->getScaleFactor();
    if (sf2d <= 0) sf2d = 1.0;
    glm::vec2 touchPx2d{
        static_cast<float>(point.x * sf2d),
        static_cast<float>(point.y * sf2d)
    };
    handles::HitTestOptions opts2d;
    opts2d.handleTolerance     = 60.0f;
    opts2d.preferAxisHandles   = true;
    opts2d.ignoreNonEditable   = true;
    auto hit2d = handles::HitTest(descs2d, proj2d, touchPx2d, opts2d);
    if (!hit2d) return -1;
    handles::WorldRay startRay2d;
    TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                          startRay2d.origin, startRay2d.direction);
    auto session2d = m->BeginDrag(hit2d->id, startRay2d);
    if (session2d) {
        _dragSession = std::move(session2d);
        _dragSessionStartId = hit2d->id;
        return 0;
    }
    return -1;
}

- (BOOL)handleCenterHandleTapAtScreenPoint:(CGPoint)point
                                  viewSize:(CGSize)viewSize
                               forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return NO;
    if (!_preview->Is3D()) return NO;
    if (_selectedModelName.empty()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[_selectedModelName];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();
    if (loc.IsLocked()) return NO;

    // descriptor hit-test for the centre-cycle tap. Only the
    // CentreCycle / non-axis selectionOnly descriptors trigger the
    // tool cycle; axis / segment / draggable descriptors are drag
    // targets and a tap on them is a no-op.
    handles::Tool centerTool = handles::Tool::Translate;
    switch (loc.GetAxisTool()) {
        case ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE: centerTool = handles::Tool::Translate;   break;
        case ModelScreenLocation::MSLTOOL::TOOL_SCALE:     centerTool = handles::Tool::Scale;       break;
        case ModelScreenLocation::MSLTOOL::TOOL_ROTATE:    centerTool = handles::Tool::Rotate;      break;
        case ModelScreenLocation::MSLTOOL::TOOL_XY_TRANS:  centerTool = handles::Tool::XYTranslate; break;
        case ModelScreenLocation::MSLTOOL::TOOL_ELEVATE:   centerTool = handles::Tool::Elevate;     break;
        default: return NO;
    }
    const float zoom = _preview->GetCameraZoomForHandles();
    const int hscale = _preview->GetHandleScale();
    handles::ViewParams view;
    view.axisArrowLength = loc.GetAxisArrowLength(zoom, hscale);
    view.axisHeadLength  = loc.GetAxisHeadLength(zoom, hscale);
    view.axisRadius      = loc.GetAxisRadius(zoom, hscale);
    const auto descs = m->GetHandles(handles::ViewMode::ThreeD, centerTool, view);
    if (descs.empty()) return NO;
    handles::ScreenProjection proj;
    proj.projViewMatrix = _preview->GetProjViewMatrix();
    proj.viewportWidth  = _canvas->getWidth();
    proj.viewportHeight = _canvas->getHeight();
    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    glm::vec2 touchPx{ static_cast<float>(point.x * scaleFactor),
                       static_cast<float>(point.y * scaleFactor) };
    handles::HitTestOptions opts;
    opts.handleTolerance   = 60.0f;
    opts.preferAxisHandles = true;
    auto hit = handles::HitTest(descs, proj, touchPx, opts);
    if (!hit) return NO;
    // Axis-style + segment hits are drag targets; tap doesn't cycle.
    if (hit->id.role == handles::Role::AxisArrow ||
        hit->id.role == handles::Role::AxisCube  ||
        hit->id.role == handles::Role::AxisRing  ||
        hit->id.role == handles::Role::Segment) return NO;
    // Only cycle when the tap landed on the currently-active sub-
    // handle (mirrors desktop's "click already-active handle to
    // advance tool" behavior).
    if (loc.GetActiveHandleId() != std::optional<handles::Id>(hit->id)) return NO;
    loc.AdvanceAxisTool();
    return YES;
}

- (BOOL)cycleAxisToolForSelectedModelForDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return NO;
    if (!_preview->Is3D()) return NO;
    if (_selectedModelName.empty()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[_selectedModelName];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();
    if (loc.IsLocked()) return NO;
    loc.AdvanceAxisTool();
    return YES;
}

#pragma mark - J-4 multi-select align / distribute / match

namespace {
// Translate `model` so the named edge / centre matches `target`.
// Returns true if the move actually shifted anything (so the
// caller can avoid marking pristine models dirty).
bool ApplyAlign(Model* model, const std::string& edge, float target) {
    auto& loc = model->GetModelScreenLocation();
    if (loc.IsLocked()) return false;
    if (model->IsFromBase()) return false;
    float current = 0.0f;
    bool isH = false, isV = false, isD = false;
    if (edge == "left")        { current = loc.GetLeft();        isH = true; }
    else if (edge == "right")  { current = loc.GetRight();       isH = true; }
    else if (edge == "centerH"){ current = loc.GetHcenterPos();  isH = true; }
    else if (edge == "top")    { current = loc.GetTop();         isV = true; }
    else if (edge == "bottom") { current = loc.GetBottom();      isV = true; }
    else if (edge == "centerV"){ current = loc.GetVcenterPos();  isV = true; }
    else if (edge == "front")  { current = loc.GetFront();       isD = true; }
    else if (edge == "back")   { current = loc.GetBack();        isD = true; }
    else if (edge == "centerD"){ current = loc.GetDcenterPos();  isD = true; }
    else return false;
    const float delta = target - current;
    if (std::fabs(delta) < 1e-4f) return false;
    if (isH) loc.SetHcenterPos(loc.GetHcenterPos() + delta);
    if (isV) loc.SetVcenterPos(loc.GetVcenterPos() + delta);
    if (isD) loc.SetDcenterPos(loc.GetDcenterPos() + delta);
    return true;
}

float ReadAlignReference(Model* model, const std::string& edge) {
    auto& loc = model->GetModelScreenLocation();
    if (edge == "left")        return loc.GetLeft();
    if (edge == "right")       return loc.GetRight();
    if (edge == "centerH")     return loc.GetHcenterPos();
    if (edge == "top")         return loc.GetTop();
    if (edge == "bottom")      return loc.GetBottom();
    if (edge == "centerV")     return loc.GetVcenterPos();
    if (edge == "front")       return loc.GetFront();
    if (edge == "back")        return loc.GetBack();
    if (edge == "centerD")     return loc.GetDcenterPos();
    return 0.0f;
}
} // namespace

- (BOOL)alignModels:(NSArray<NSString*>*)names
            toLeader:(NSString*)leader
                  by:(NSString*)edge
         forDocument:(XLSequenceDocument*)doc {
    if (!doc || names.count == 0 || leader.length == 0 || edge.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* leaderModel = rctx->GetModelManager()[leader.UTF8String];
    if (!leaderModel) return NO;
    rctx->AbortRender(5000);
    const std::string edgeStr = edge.UTF8String;
    const float target = ReadAlignReference(leaderModel, edgeStr);
    BOOL anyMoved = NO;
    const std::string leaderStd = leader.UTF8String;
    for (NSString* n in names) {
        if (n.length == 0) continue;
        const std::string nm = n.UTF8String;
        if (nm == leaderStd) continue;
        Model* m = rctx->GetModelManager()[nm];
        if (!m) continue;
        if (ApplyAlign(m, edgeStr, target)) {
            rctx->MarkLayoutModelDirty(nm);
            anyMoved = YES;
        }
    }
    return anyMoved;
}

- (BOOL)distributeModels:(NSArray<NSString*>*)names
                     axis:(NSString*)axis
              forDocument:(XLSequenceDocument*)doc {
    if (!doc || names.count < 3 || axis.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    rctx->AbortRender(5000);
    const std::string axisStr = axis.UTF8String;
    enum class A { H, V, D } which;
    if      (axisStr == "horizontal") which = A::H;
    else if (axisStr == "vertical")   which = A::V;
    else if (axisStr == "depth")      which = A::D;
    else return NO;

    // Collect editable models with their centre on the chosen axis.
    struct Entry { Model* m; std::string name; float pos; };
    std::vector<Entry> entries;
    entries.reserve(names.count);
    for (NSString* n in names) {
        if (n.length == 0) continue;
        const std::string nm = n.UTF8String;
        Model* m = rctx->GetModelManager()[nm];
        if (!m) continue;
        auto& loc = m->GetModelScreenLocation();
        if (loc.IsLocked() || m->IsFromBase()) continue;
        float pos = 0.0f;
        switch (which) {
            case A::H: pos = loc.GetHcenterPos(); break;
            case A::V: pos = loc.GetVcenterPos(); break;
            case A::D: pos = loc.GetDcenterPos(); break;
        }
        entries.push_back({m, nm, pos});
    }
    if (entries.size() < 3) return NO;
    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b) { return a.pos < b.pos; });

    const float lo = entries.front().pos;
    const float hi = entries.back().pos;
    const float step = (hi - lo) / static_cast<float>(entries.size() - 1);
    BOOL anyMoved = NO;
    for (size_t i = 1; i + 1 < entries.size(); ++i) {
        const float target = lo + step * static_cast<float>(i);
        const float delta  = target - entries[i].pos;
        if (std::fabs(delta) < 1e-4f) continue;
        auto& loc = entries[i].m->GetModelScreenLocation();
        switch (which) {
            case A::H: loc.SetHcenterPos(loc.GetHcenterPos() + delta); break;
            case A::V: loc.SetVcenterPos(loc.GetVcenterPos() + delta); break;
            case A::D: loc.SetDcenterPos(loc.GetDcenterPos() + delta); break;
        }
        rctx->MarkLayoutModelDirty(entries[i].name);
        anyMoved = YES;
    }
    return anyMoved;
}

- (BOOL)matchSizeOfModels:(NSArray<NSString*>*)names
                  toLeader:(NSString*)leader
                 dimension:(NSString*)dim
               forDocument:(XLSequenceDocument*)doc {
    if (!doc || names.count == 0 || leader.length == 0 || dim.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* leaderModel = rctx->GetModelManager()[leader.UTF8String];
    if (!leaderModel) return NO;
    rctx->AbortRender(5000);
    auto& leaderLoc = leaderModel->GetModelScreenLocation();
    const float tw = leaderLoc.GetMWidth();
    const float th = leaderLoc.GetMHeight();
    const float td = leaderLoc.GetMDepth();
    const std::string dimStr = dim.UTF8String;
    const bool wantW = (dimStr == "width"  || dimStr == "all");
    const bool wantH = (dimStr == "height" || dimStr == "all");
    const bool wantD = (dimStr == "depth"  || dimStr == "all");
    if (!wantW && !wantH && !wantD) return NO;
    BOOL anyResized = NO;
    const std::string leaderStd = leader.UTF8String;
    for (NSString* n in names) {
        if (n.length == 0) continue;
        const std::string nm = n.UTF8String;
        if (nm == leaderStd) continue;
        Model* m = rctx->GetModelManager()[nm];
        if (!m) continue;
        auto& loc = m->GetModelScreenLocation();
        if (loc.IsLocked() || m->IsFromBase()) continue;
        bool changed = false;
        if (wantW && std::fabs(loc.GetMWidth() - tw)  > 1e-4f) { loc.SetMWidth(tw);  changed = true; }
        if (wantH && std::fabs(loc.GetMHeight() - th) > 1e-4f) { loc.SetMHeight(th); changed = true; }
        if (wantD && std::fabs(loc.GetMDepth() - td)  > 1e-4f) { loc.SetMDepth(td);  changed = true; }
        if (changed) {
            rctx->MarkLayoutModelDirty(nm);
            anyResized = YES;
        }
    }
    return anyResized;
}

- (BOOL)flipModels:(NSArray<NSString*>*)names
              axis:(NSString*)axis
       forDocument:(XLSequenceDocument*)doc {
    if (!doc || names.count == 0 || axis.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasModelManager()) return NO;
    rctx->AbortRender(5000);
    const std::string axisStr = axis.UTF8String;
    const bool horizontal = (axisStr == "horizontal");
    const bool vertical   = (axisStr == "vertical");
    if (!horizontal && !vertical) return NO;
    BOOL anyFlipped = NO;
    for (NSString* n in names) {
        if (n.length == 0) continue;
        const std::string nm = n.UTF8String;
        Model* m = rctx->GetModelManager()[nm];
        if (!m) continue;
        if (m->GetBaseObjectScreenLocation().IsLocked() || m->IsFromBase()) continue;
        if (horizontal) {
            m->FlipHorizontal(false);
        } else {
            m->FlipVertical(false);
        }
        rctx->MarkLayoutModelDirty(nm);
        anyFlipped = YES;
    }
    return anyFlipped;
}

- (NSArray<NSString*>*)duplicateModels:(NSArray<NSString*>*)names
                           forDocument:(XLSequenceDocument*)doc {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!doc || names.count == 0) return out;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasModelManager()) return out;
    rctx->AbortRender(5000);
    auto& mgr = rctx->GetModelManager();

    for (NSString* n in names) {
        if (n.length == 0) continue;
        Model* src = mgr[n.UTF8String];
        if (!src) continue;
        // Skip ModelGroup duplicates — their member references are
        // ambiguous (do they share members with the source, or do
        // we deep-copy?). Defer until the use case is concrete.
        if (src->GetDisplayAs() == DisplayAsType::ModelGroup) continue;

        // Round-trip the source through the same serializer the
        // save path uses so per-type attributes are preserved.
        XmlSerializer serializer;
        pugi::xml_document srcDoc = serializer.SerializeModel(src);
        pugi::xml_node docRoot = srcDoc.document_element();
        if (!docRoot) continue;
        pugi::xml_node modelNode = docRoot.first_child();
        if (!modelNode) continue;

        const std::string newName = mgr.GenerateModelName(src->GetName());
        if (modelNode.attribute("name")) modelNode.remove_attribute("name");
        modelNode.append_attribute("name") = newName.c_str();
        // Strip controller mapping so the duplicate doesn't fight
        // for channels — desktop's Paste flow does the same.
        if (modelNode.attribute("Controller")) modelNode.remove_attribute("Controller");
        if (modelNode.attribute("StartChannel")) modelNode.remove_attribute("StartChannel");
        if (auto cc = modelNode.child("ControllerConnection")) modelNode.remove_child(cc);

        Model* dup = mgr.CreateModel(modelNode);
        if (!dup) continue;
        dup->SetControllerName("");
        dup->SetStartChannel("");
        dup->name = newName;
        dup->Lock(false);
        dup->AddOffset(50.0, 50.0, 0.0);
        mgr.AddModel(dup);
        rctx->MarkLayoutModelDirty(newName);
        [out addObject:[NSString stringWithUTF8String:newName.c_str()]];
    }
    return out;
}

- (void)endHandleDragForDocument:(XLSequenceDocument*)doc {
    _handleDragNeedsLatch = NO;
    if (iPadRenderContext* abortCtx = ContextFromDoc(doc)) abortCtx->AbortRender(5000);

    // If a descriptor session is active, commit + drop. Maps
    // DirtyField bits onto the dirty layout-models set so save
    // picks up the change.
    if (_dragSession) {
        auto result = _dragSession->Commit();
        const bool geometryDirty =
            handles::HasDirty(result.dirty, handles::DirtyField::Position) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Dimensions) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Rotation) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Endpoint) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Vertex) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Curve) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Shear);
        if (geometryDirty) {
            iPadRenderContext* rctx = ContextFromDoc(doc);
            // J-14 — view-object handle drag commits to the VO
            // dirty set. The screen-location's CreateDragSession
            // populates `result.modelName` with the VO name, but
            // we tracked it separately at session start so the
            // dispatch is unambiguous.
            if (rctx && !_dragSessionViewObjectName.empty()) {
                rctx->MarkLayoutViewObjectDirty(_dragSessionViewObjectName);
                if (ViewObject* vo = rctx->GetAllObjects().GetViewObject(_dragSessionViewObjectName)) {
                    vo->IncrementChangeCount();
                    vo->ReloadModel();
                }
            } else if (rctx && !result.modelName.empty()) {
                rctx->MarkLayoutModelDirty(result.modelName);
                // PolyPoint-style models (Poly Line, MultiPoint)
                // recompute node positions in `InitModel` from
                // per-segment counts and mPos[]. Without an
                // explicit Reinitialize the nodes stay distributed
                // along the pre-drag geometry — visible as lights
                // not following a moved vertex. Desktop achieves
                // the same via WORK_MODELS_CHANGE_REQUIRING_RERENDER
                // queued from LayoutPanel; the iPad bridge runs the
                // re-init directly since we have no work queue.
                Model* m = rctx->GetModelManager()[result.modelName];
                if (m && dynamic_cast<PolyPointScreenLocation*>(&m->GetModelScreenLocation())) {
                    m->Reinitialize();
                }
            }
        }
        _dragSession.reset();
        _dragSessionStartId = handles::Id{};
        _dragSessionViewObjectName.clear();
        return;
    }

    // Clear active_axis so the next pick starts fresh, but keep
    // active_handle at CentreCycle so the still-selected model
    // continues to show its gizmo.
    if (!_preview) return;
    if (_selectedModelName.empty()) return;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return;
    Model* m = rctx->GetModelManager()[_selectedModelName];
    if (!m) return;
    auto& loc = m->GetModelScreenLocation();
    loc.SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
}

- (BOOL)dragHandle:(NSInteger)handleIndex
   toScreenPoint:(CGPoint)point
        viewSize:(CGSize)viewSize
     forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return NO;
    if (iPadRenderContext* abortCtx = ContextFromDoc(doc)) abortCtx->AbortRender(5000);

    // if a new-API session is active, route through it.
    // Ignores `handleIndex` (the new path identifies the handle
    // via the session's stored `Id`).
    if (_dragSession) {
        handles::WorldRay ray;
        TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                              ray.origin, ray.direction);

        // Lock Axis: project the world ray onto an axis-aligned
        // line through the model's centre. The underlying session
        // then sees a constrained cursor, no per-session
        // awareness needed.
        if (_lockAxis != 0 && !_selectedModelName.empty()) {
            iPadRenderContext* rctx = ContextFromDoc(doc);
            if (rctx) {
                Model* m = rctx->GetModelManager()[_selectedModelName];
                if (m) {
                    auto& loc = m->GetModelScreenLocation();
                    const glm::vec3 c(loc.GetHcenterPos(), loc.GetVcenterPos(), loc.GetDcenterPos());
                    switch (_lockAxis) {
                        case 1: ray.origin.y = c.y; ray.origin.z = c.z; break;  // X
                        case 2: ray.origin.x = c.x; ray.origin.z = c.z; break;  // Y
                        case 3: ray.origin.x = c.x; ray.origin.y = c.y; break;  // Z
                        default: break;
                    }
                }
            }
        }

        // Uniform: OR Shift into the modifier so existing
        // session classes (which already interpret Shift as
        // "uniform scale" / "aspect lock") work unchanged.
        handles::Modifier mods = handles::Modifier::None;
        if (_uniformModifier) mods = mods | handles::Modifier::Shift;

        auto result = _dragSession->Update(ray, mods);
        if (result == handles::UpdateResult::Updated ||
            result == handles::UpdateResult::NeedsInit) {
            iPadRenderContext* rctx = ContextFromDoc(doc);
            if (rctx) rctx->MarkLayoutModelDirty(_selectedModelName);
            // PolyPoint-style models keep their light positions in
            // `Nodes[]` and only recompute them in `InitModel`. Live
            // dragging a vertex without re-init leaves the lights
            // anchored to the pre-drag mPos[] until the user
            // releases. Mirrors desktop's per-frame
            // WORK_MODELS_CHANGE_REQUIRING_RERENDER queueing.
            Model* m = rctx ? rctx->GetModelManager()[_selectedModelName] : nullptr;
            if (m && dynamic_cast<PolyPointScreenLocation*>(&m->GetModelScreenLocation())) {
                m->Reinitialize();
            }
        }
        return YES;
    }

    // No-op without an active descriptor session.
    (void)handleIndex;
    return NO;
}

// J-13 — view-object hit-test. Mirrors `pickModelAtScreenPoint`
// but searches `ViewObjectManager`. Returns the topmost hit
// (last-drawn = visually on top).
- (nullable NSString*)pickViewObjectAtScreenPoint:(CGPoint)point
                                          viewSize:(CGSize)viewSize
                                       forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return nil;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasViewObjectManager()) return nil;
    ViewObjectManager& vm = rctx->GetAllObjects();

    if (_preview->Is3D()) {
        glm::vec3 ray_origin, ray_direction;
        TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                              ray_origin, ray_direction);
        ViewObject* best = nullptr;
        // Finite sentinel — see hitTestModelAtPoint above. Same -ffast-math
        // hazard breaks 3D view-object hit-test in Release.
        float bestDist = std::numeric_limits<float>::max();
        for (auto it = vm.begin(); it != vm.end(); ++it) {
            ViewObject* vo = it->second;
            if (!vo) continue;
            float dist = 0.0f;
            if (vo->GetObjectScreenLocation().HitTest3D(
                    ray_origin, ray_direction, dist) && dist < bestDist) {
                best = vo;
                bestDist = dist;
            }
        }
        return best ? [NSString stringWithUTF8String:best->GetName().c_str()] : nil;
    }

    // 2D path — box-test the unprojected world point.
    float worldX = 0, worldY = 0, denom = 0;
    if (![self unprojectScreenPoint:point toWorldX:&worldX worldY:&worldY
                          worldDenom:&denom forContext:rctx]) {
        return nil;
    }
    ViewObject* topMost = nullptr;
    for (auto it = vm.begin(); it != vm.end(); ++it) {
        ViewObject* vo = it->second;
        if (!vo) continue;
        auto& loc = vo->GetObjectScreenLocation();
        float cx = loc.GetHcenterPos();
        float cy = loc.GetVcenterPos();
        float halfW = std::max(1.0f, loc.GetMWidth() * 0.5f);
        float halfH = std::max(1.0f, loc.GetMHeight() * 0.5f);
        if (worldX >= cx - halfW && worldX <= cx + halfW &&
            worldY >= cy - halfH && worldY <= cy + halfH) {
            topMost = vo;  // keep walking — later entries draw later (on top)
        }
    }
    return topMost ? [NSString stringWithUTF8String:topMost->GetName().c_str()] : nil;
}

// J-14 — handle-based endpoint drag for view objects.
// Descriptor-pipeline hit-test against the SELECTED VO's
// screen-location handles. On a hit, opens a `_dragSession`
// via the screen location's `CreateDragSession` factory,
// stashes the VO name in `_dragSessionViewObjectName` so
// `endHandleDragForDocument:` knows to mark the VO dirty
// instead of a model. Returns 0 on hit (the legacy index is
// unused by the new descriptor path), -1 on miss.
- (NSInteger)pickViewObjectHandleAtScreenPoint:(CGPoint)point
                                       viewSize:(CGSize)viewSize
                                    forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return -1;
    if (_selectedViewObjectName.empty()) return -1;
    if (_selectedViewObjectName == "2D Background") return -1;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasViewObjectManager()) return -1;
    ViewObject* vo = rctx->GetAllObjects().GetViewObject(_selectedViewObjectName);
    if (!vo) return -1;
    auto& loc = vo->GetObjectScreenLocation();
    if (loc.IsLocked() || vo->IsFromBase()) return -1;

    // Descriptor mode used both for 2D and 3D (the screen
    // location decides which set of handles to emit). Translate
    // tool is the only one meaningful for view-object endpoints
    // and bbox handles — Rotate / Scale ride a separate gizmo
    // we don't expose on the VO surface yet.
    const float zoom = _preview->GetCameraZoomForHandles();
    const int hscale = _preview->GetHandleScale();
    handles::ViewParams view;
    view.axisArrowLength = loc.GetAxisArrowLength(zoom, hscale);
    view.axisHeadLength  = loc.GetAxisHeadLength(zoom, hscale);
    view.axisRadius      = loc.GetAxisRadius(zoom, hscale);

    auto mode = _preview->Is3D() ? handles::ViewMode::ThreeD
                                   : handles::ViewMode::TwoD;
    auto descriptors = loc.GetHandles(mode, handles::Tool::Translate, view);
    if (descriptors.empty()) return -1;

    handles::ScreenProjection proj;
    proj.projViewMatrix = _preview->GetProjViewMatrix();
    proj.viewportWidth  = _canvas->getWidth();
    proj.viewportHeight = _canvas->getHeight();
    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    glm::vec2 touchPx{
        static_cast<float>(point.x * scaleFactor),
        static_cast<float>(point.y * scaleFactor)
    };
    handles::HitTestOptions opts;
    opts.handleTolerance     = 60.0f;
    opts.axisHandleTolerance = 28.0f;
    opts.preferAxisHandles   = true;
    opts.ignoreNonEditable   = true;

    auto hit = handles::HitTest(descriptors, proj, touchPx, opts);
    if (!hit) return -1;

    handles::WorldRay startRay;
    TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                          startRay.origin, startRay.direction);
    auto session = loc.CreateDragSession(_selectedViewObjectName, hit->id, startRay);
    if (!session) return -1;
    _dragSession = std::move(session);
    _dragSessionStartId = hit->id;
    _dragSessionViewObjectName = _selectedViewObjectName;
    return 0;
}

// J-13 — 2D drag-to-move for view objects. Math mirrors
// `moveModel:byDeltaDX:dY:viewSize:forDocument:` exactly; just
// targets `ViewObjectManager` and marks the VO dirty rather
// than the model.
- (BOOL)moveViewObject:(NSString*)name
              byDeltaDX:(CGFloat)dx
                     dY:(CGFloat)dy
               viewSize:(CGSize)viewSize
            forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasViewObjectManager()) return NO;
    if (_preview->Is3D()) return NO;

    ViewObject* vo = rctx->GetAllObjects().GetViewObject(name.UTF8String);
    if (!vo) return NO;
    auto& loc = vo->GetObjectScreenLocation();
    if (loc.IsLocked() || vo->IsFromBase()) return NO;

    rctx->AbortRender(5000);
    int canvasW = _canvas->getWidth();
    int canvasH = _canvas->getHeight();
    if (canvasW <= 0 || canvasH <= 0) return NO;

    double scale = _canvas->getScaleFactor();
    if (scale <= 0) scale = 1.0;
    double dxPx = (double)dx * scale;
    double dyPx = -(double)dy * scale;

    PreviewCamera& cam = _preview->Get2DCamera();
    int virtualW = 0, virtualH = 0;
    _preview->GetVirtualCanvasSize(virtualW, virtualH);
    float scale2d = 1.0f;
    if (virtualW != 0 && virtualH != 0) {
        float scale2dh = (float)canvasH / (float)virtualH;
        float scale2dw = (float)canvasW / (float)virtualW;
        scale2d = std::min(scale2dh, scale2dw);
    }
    float denom = cam.GetZoom() * scale2d;
    if (denom == 0.0f) return NO;
    float worldDX = (float)dxPx / denom;
    float worldDY = (float)dyPx / denom;

    float newH = loc.GetHcenterPos() + worldDX;
    float newV = loc.GetVcenterPos() + worldDY;

    if (_snapToGrid) {
        float spacing = (float)std::max((long)1, rctx->GetDisplay2DGridSpacing());
        newH = std::round(newH / spacing) * spacing;
        newV = std::round(newV / spacing) * spacing;
    }

    vo->SetHcenterPos(newH);
    vo->SetVcenterPos(newV);
    vo->IncrementChangeCount();
    rctx->MarkLayoutViewObjectDirty(name.UTF8String);
    return YES;
}

// J-13 — Terrain heightmap edit. Unproject the touch into world
// XZ space, find the nearest grid point in the terrain's
// (u,v) coordinate system, then raise/lower it by `delta`.
// `brushRadiusPoints > 0` applies a cosine falloff to neighbours
// within the radius for a smoother deformation.
- (BOOL)editTerrainHeight:(NSString*)terrainName
              atScreenPoint:(CGPoint)point
                  viewSize:(CGSize)viewSize
                     delta:(float)delta
        brushRadiusPoints:(CGFloat)brushRadiusPoints
               forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !terrainName || terrainName.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasViewObjectManager()) return NO;
    ViewObject* vo = rctx->GetAllObjects().GetViewObject(terrainName.UTF8String);
    auto* terrain = dynamic_cast<TerrainObject*>(vo);
    if (!terrain) return NO;
    if (terrain->GetBaseObjectScreenLocation().IsLocked() || terrain->IsFromBase()) return NO;

    rctx->AbortRender(5000);

    // Unproject the touch into world XY. Terrain lives on the
    // ground plane in 2D mode (XZ in 3D, but we treat it the same
    // — the screen-location centerpos / dims define its 2D
    // footprint), so the world XY result maps directly onto its
    // (u,v) grid.
    float worldX = 0, worldY = 0, denom = 0;
    if (_preview->Is3D()) {
        // 3D ray cast onto Y=center plane.
        glm::vec3 ray_origin, ray_direction;
        TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                              ray_origin, ray_direction);
        // Plane through the terrain's centre with normal +Y.
        auto& tloc = terrain->GetObjectScreenLocation();
        glm::vec3 center(tloc.GetHcenterPos(), tloc.GetVcenterPos(), tloc.GetDcenterPos());
        glm::vec3 planeNormal(0, 1, 0);
        float denomR = glm::dot(ray_direction, planeNormal);
        if (std::fabs(denomR) < 1e-6f) return NO;
        float tHit = glm::dot(center - ray_origin, planeNormal) / denomR;
        if (tHit < 0) return NO;
        glm::vec3 hit = ray_origin + ray_direction * tHit;
        worldX = hit.x;
        worldY = hit.z;
    } else {
        if (![self unprojectScreenPoint:point toWorldX:&worldX worldY:&worldY
                              worldDenom:&denom forContext:rctx]) {
            return NO;
        }
    }

    auto& tloc = terrain->GetObjectScreenLocation();
    float cx = tloc.GetHcenterPos();
    float cy = tloc.GetVcenterPos();
    float halfW = std::max(1.0f, tloc.GetMWidth() * 0.5f);
    float halfH = std::max(1.0f, tloc.GetMHeight() * 0.5f);
    // Reject taps outside the terrain footprint.
    if (worldX < cx - halfW || worldX > cx + halfW ||
        worldY < cy - halfH || worldY > cy + halfH) return NO;

    int spacing = std::max(1, terrain->GetSpacing());
    int gridW = std::max(1, terrain->GetWidth());
    int gridD = std::max(1, terrain->GetDepth());
    int numU = gridW / spacing + 1;
    int numV = gridD / spacing + 1;

    // Normalize touch into (u,v) grid coords (0..numU-1, 0..numV-1).
    float u = ((worldX - (cx - halfW)) / (2.0f * halfW)) * (numU - 1);
    float v = ((worldY - (cy - halfH)) / (2.0f * halfH)) * (numV - 1);
    int iu = (int)std::round(u);
    int iv = (int)std::round(v);
    if (iu < 0 || iu >= numU || iv < 0 || iv >= numV) return NO;

    // Read existing point heights. TerrainScreenLocation stores
    // them as a comma-delimited string in its `PointData` attr.
    auto& screenLoc = dynamic_cast<TerrainScreenLocation&>(terrain->GetBaseObjectScreenLocation());
    std::string data = screenLoc.GetDataAsString();
    // Parse into a heights array.
    std::vector<float> heights;
    heights.reserve(numU * numV);
    {
        size_t start = 0;
        while (start <= data.size()) {
            size_t comma = data.find(',', start);
            std::string token = data.substr(start, comma - start);
            heights.push_back(std::strtof(token.c_str(), nullptr));
            if (comma == std::string::npos) break;
            start = comma + 1;
        }
    }
    while ((int)heights.size() < numU * numV) heights.push_back(0.0f);

    auto idxFor = [&](int u_, int v_) { return v_ * numU + u_; };

    // Brush radius in grid steps. Convert points → world via the
    // active 2D zoom (same denom as moveViewObject) when in 2D;
    // 3D mode just uses a small fixed grid radius for now.
    int radiusGrid = 0;
    if (brushRadiusPoints > 0 && !_preview->Is3D()) {
        double scale = _canvas->getScaleFactor();
        if (scale <= 0) scale = 1.0;
        int canvasW = _canvas->getWidth();
        int canvasH = _canvas->getHeight();
        PreviewCamera& cam = _preview->Get2DCamera();
        int virtualW = 0, virtualH = 0;
        _preview->GetVirtualCanvasSize(virtualW, virtualH);
        float s2d = 1.0f;
        if (virtualW != 0 && virtualH != 0 && canvasW > 0 && canvasH > 0) {
            s2d = std::min((float)canvasH / virtualH, (float)canvasW / virtualW);
        }
        float pxToWorld = 1.0f / (cam.GetZoom() * s2d);
        float radiusWorld = (float)(brushRadiusPoints * scale) * pxToWorld;
        radiusGrid = std::max(0, (int)std::round(radiusWorld / (float)spacing));
    }

    if (radiusGrid <= 0) {
        heights[idxFor(iu, iv)] += delta;
    } else {
        // Cosine falloff: full delta at centre, 0 at radius.
        for (int dv = -radiusGrid; dv <= radiusGrid; ++dv) {
            for (int du = -radiusGrid; du <= radiusGrid; ++du) {
                int nu = iu + du;
                int nv = iv + dv;
                if (nu < 0 || nu >= numU || nv < 0 || nv >= numV) continue;
                float dist = std::sqrt((float)(du*du + dv*dv));
                if (dist > (float)radiusGrid) continue;
                float falloff = 0.5f * (1.0f + std::cos((float)M_PI * dist / (float)radiusGrid));
                heights[idxFor(nu, nv)] += delta * falloff;
            }
        }
    }

    // Re-serialize the heights.
    std::string out;
    out.reserve(heights.size() * 6);
    for (size_t i = 0; i < heights.size(); ++i) {
        if (i > 0) out += ',';
        out += std::to_string(heights[i]);
    }
    screenLoc.SetDataFromString(out);
    terrain->IncrementChangeCount();
    terrain->ReloadModel();
    rctx->MarkLayoutViewObjectDirty(terrainName.UTF8String);
    return YES;
}

- (BOOL)moveModel:(NSString*)name
       byDeltaDX:(CGFloat)dx
              dY:(CGFloat)dy
        viewSize:(CGSize)viewSize
     forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    if (_preview->Is3D()) return NO;

    Model* m = rctx->GetModelManager()[std::string([name UTF8String])];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();
    if (loc.IsLocked()) return NO;

    rctx->AbortRender(5000);
    int canvasW = _canvas->getWidth();
    int canvasH = _canvas->getHeight();
    if (canvasW <= 0 || canvasH <= 0) return NO;

    // Screen Δ (points) → window-pixel Δ → world Δ via the inverse of
    // iPadModelPreview::StartDrawing's 2D View matrix. Translation
    // components don't enter Δ-only conversion (they're constant);
    // we just need the scale factor zoom*scale2d. The Y delta picks
    // up an extra sign flip because window-pixel Y is up (Metal
    // convention) while UI-point Y is down.
    double scale = _canvas->getScaleFactor();
    if (scale <= 0) scale = 1.0;
    double dxPx = (double)dx * scale;
    double dyPx = -(double)dy * scale;

    PreviewCamera& cam = _preview->Get2DCamera();
    int virtualW = 0, virtualH = 0;
    _preview->GetVirtualCanvasSize(virtualW, virtualH);
    float scale2d = 1.0f;
    if (virtualW != 0 && virtualH != 0) {
        float scale2dh = (float)canvasH / (float)virtualH;
        float scale2dw = (float)canvasW / (float)virtualW;
        scale2d = std::min(scale2dh, scale2dw);
    }
    float denom = cam.GetZoom() * scale2d;
    if (denom == 0.0f) return NO;
    float worldDX = (float)dxPx / denom;
    float worldDY = (float)dyPx / denom;

    float newH = loc.GetHcenterPos() + worldDX;
    float newV = loc.GetVcenterPos() + worldDY;

    // J-2 — snap the post-delta centre to the rgbeffects grid
    // spacing if the user has snap-to-grid on. When the layout uses
    // a centred origin (Display2DCenter0), snapping is naturally
    // around 0; otherwise it snaps to the show's chosen origin
    // corner.
    if (_snapToGrid) {
        float spacing = (float)std::max((long)1, rctx->GetDisplay2DGridSpacing());
        newH = std::round(newH / spacing) * spacing;
        newV = std::round(newV / spacing) * spacing;
    }

    m->SetHcenterPos(newH);
    m->SetVcenterPos(newV);
    rctx->MarkLayoutModelDirty(std::string([name UTF8String]));
    return YES;
}

- (BOOL)beginBodyDrag3DForModel:(NSString*)name
                   atScreenPoint:(CGPoint)point
                        viewSize:(CGSize)viewSize
                     forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !name || name.length == 0) return NO;
    if (!_preview->Is3D()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[name.UTF8String];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();
    if (loc.IsLocked()) return NO;

    // Choose the drag plane (XY / XZ / YZ) from the current camera
    // angles so a top-down view drags along the floor, a side view
    // drags vertically, etc. SetActivePlane keeps it on the screen
    // location for any downstream code (e.g. handle gizmos).
    loc.RefreshActivePlaneFromCamera(_preview.get());
    const auto plane = loc.GetActivePlane();
    const glm::vec3 center(loc.GetHcenterPos(),
                            loc.GetVcenterPos(),
                            loc.GetDcenterPos());
    glm::vec3 planePt{0.0f}, planeN{0.0f};
    switch (plane) {
        case ModelScreenLocation::MSLPLANE::XZ_PLANE:
            planeN = glm::vec3(0.0f, 1.0f, 0.0f);
            planePt = glm::vec3(0.0f, center.y, 0.0f);
            break;
        case ModelScreenLocation::MSLPLANE::YZ_PLANE:
            planeN = glm::vec3(1.0f, 0.0f, 0.0f);
            planePt = glm::vec3(center.x, 0.0f, 0.0f);
            break;
        case ModelScreenLocation::MSLPLANE::XY_PLANE:
        default:
            planeN = glm::vec3(0.0f, 0.0f, 1.0f);
            planePt = glm::vec3(0.0f, 0.0f, center.z);
            break;
    }

    glm::vec3 origin, dir;
    TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                          origin, dir);
    glm::vec3 hit;
    if (!VectorMath::GetPlaneIntersect(origin, dir, planePt, planeN, hit)) {
        // Camera nearly parallel to plane — body-drag can't define
        // a delta. Fall back to camera orbit by reporting failure.
        return NO;
    }
    _bodyDrag3DActive       = YES;
    _bodyDrag3DSavedCenter  = center;
    _bodyDrag3DAnchor       = hit;
    _bodyDrag3DPlane        = plane;
    _bodyDrag3DPlanePoint   = planePt;
    _bodyDrag3DPlaneNormal  = planeN;
    _bodyDrag3DModelName    = name.UTF8String;
    return YES;
}

- (BOOL)dragBody3DToScreenPoint:(CGPoint)point
                        viewSize:(CGSize)viewSize
                     forDocument:(XLSequenceDocument*)doc {
    if (!_bodyDrag3DActive || !_preview || !doc) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    rctx->AbortRender(5000);
    // J-15 — branch by target. ModelScreenLocation is the base;
    // we just need a reference to the right one.
    ModelScreenLocation* locPtr = nullptr;
    if (_bodyDrag3DTargetIsVO) {
        if (!rctx->HasViewObjectManager()) return NO;
        ViewObject* vo = rctx->GetAllObjects().GetViewObject(_bodyDrag3DModelName);
        if (!vo) return NO;
        locPtr = &vo->GetObjectScreenLocation();
    } else {
        Model* m = rctx->GetModelManager()[_bodyDrag3DModelName];
        if (!m) return NO;
        locPtr = &m->GetModelScreenLocation();
    }
    auto& loc = *locPtr;
    if (loc.IsLocked()) return NO;

    glm::vec3 origin, dir;
    TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                          origin, dir);
    glm::vec3 hit;
    if (!VectorMath::GetPlaneIntersect(origin, dir, _bodyDrag3DPlanePoint,
                                         _bodyDrag3DPlaneNormal, hit)) {
        // Ray rotated parallel mid-drag — just hold position.
        return NO;
    }
    const glm::vec3 delta = hit - _bodyDrag3DAnchor;

    // Only the two in-plane axes move; the perpendicular one
    // keeps its saved value so the model stays on the chosen plane.
    glm::vec3 newCenter = _bodyDrag3DSavedCenter;
    switch (_bodyDrag3DPlane) {
        case ModelScreenLocation::MSLPLANE::XZ_PLANE:
            newCenter.x += delta.x;
            newCenter.z += delta.z;
            break;
        case ModelScreenLocation::MSLPLANE::YZ_PLANE:
            newCenter.y += delta.y;
            newCenter.z += delta.z;
            break;
        case ModelScreenLocation::MSLPLANE::XY_PLANE:
        default:
            newCenter.x += delta.x;
            newCenter.y += delta.y;
            break;
    }

    // Toolbar Lock Axis: clamp the named axis back to saved.
    if (_lockAxis == 1 /*X*/) newCenter.x = _bodyDrag3DSavedCenter.x;
    else if (_lockAxis == 2 /*Y*/) newCenter.y = _bodyDrag3DSavedCenter.y;
    else if (_lockAxis == 3 /*Z*/) newCenter.z = _bodyDrag3DSavedCenter.z;

    if (_snapToGrid) {
        const float spacing = (float)std::max((long)1, rctx->GetDisplay2DGridSpacing());
        newCenter.x = std::round(newCenter.x / spacing) * spacing;
        newCenter.y = std::round(newCenter.y / spacing) * spacing;
        newCenter.z = std::round(newCenter.z / spacing) * spacing;
    }

    loc.SetHcenterPos(newCenter.x);
    loc.SetVcenterPos(newCenter.y);
    loc.SetDcenterPos(newCenter.z);
    if (_bodyDrag3DTargetIsVO) {
        rctx->MarkLayoutViewObjectDirty(_bodyDrag3DModelName);
    } else {
        rctx->MarkLayoutModelDirty(_bodyDrag3DModelName);
    }
    return YES;
}

- (void)endBodyDrag3D {
    _bodyDrag3DActive = NO;
    _bodyDrag3DModelName.clear();
    _bodyDrag3DTargetIsVO = NO;
}

- (BOOL)setHoveredHandleAtScreenPoint:(CGPoint)point
                              viewSize:(CGSize)viewSize
                           forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || _selectedModelName.empty()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[_selectedModelName];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();

    // Build the descriptor set for the current tool (mirrors
    // pickHandle's selection of TwoD vs ThreeD).
    handles::Tool tool = handles::Tool::Translate;
    switch (loc.GetAxisTool()) {
        case ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE: tool = handles::Tool::Translate;   break;
        case ModelScreenLocation::MSLTOOL::TOOL_SCALE:     tool = handles::Tool::Scale;       break;
        case ModelScreenLocation::MSLTOOL::TOOL_ROTATE:    tool = handles::Tool::Rotate;      break;
        case ModelScreenLocation::MSLTOOL::TOOL_XY_TRANS:  tool = handles::Tool::XYTranslate; break;
        case ModelScreenLocation::MSLTOOL::TOOL_ELEVATE:   tool = handles::Tool::Elevate;     break;
        default: break;
    }

    handles::ViewParams view;
    if (_preview->Is3D()) {
        const float zoom = _preview->GetCameraZoomForHandles();
        const int hscale = _preview->GetHandleScale();
        view.axisArrowLength = loc.GetAxisArrowLength(zoom, hscale);
        view.axisHeadLength  = loc.GetAxisHeadLength(zoom, hscale);
        view.axisRadius      = loc.GetAxisRadius(zoom, hscale);
    }
    const auto descs = m->GetHandles(
        _preview->Is3D() ? handles::ViewMode::ThreeD : handles::ViewMode::TwoD,
        tool, view);
    if (descs.empty()) {
        // No descriptors means no possible hover target — clear.
        return [self clearHoveredHandleForDocument:doc];
    }

    handles::ScreenProjection proj;
    proj.projViewMatrix = _preview->GetProjViewMatrix();
    proj.viewportWidth  = _canvas->getWidth();
    proj.viewportHeight = _canvas->getHeight();
    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    glm::vec2 touchPx{
        static_cast<float>(point.x * scaleFactor),
        static_cast<float>(point.y * scaleFactor)
    };
    handles::HitTestOptions opts;
    // Hover tolerance is tighter than touch — a Pencil tip lands
    // within a few points, a trackpad pointer is pixel-precise.
    opts.handleTolerance = 14.0f;
    opts.preferAxisHandles = true;
    auto hit = handles::HitTest(descs, proj, touchPx, opts);

    std::optional<handles::Id> newHover = hit ? std::optional<handles::Id>(hit->id) : std::nullopt;
    if (newHover == loc.GetHighlightedHandleId()) return NO;
    loc.MouseOverHandle(newHover);
    return YES;
}

// World-space (x, y, z) → UIKit screen point (top-left origin,
// in points not pixels). Returns NO when the point is behind the
// camera, the canvas dimensions are invalid, or the point falls
// outside the viewport plus `marginPt`.
- (BOOL)projectWorldPoint:(glm::vec3)world
              toViewPoint:(CGPoint*)outPt
                marginPts:(CGFloat)marginPt {
    if (!_preview || !_canvas || !outPt) return NO;
    const glm::vec4 clip = _preview->GetProjViewMatrix() * glm::vec4(world, 1.0f);
    if (clip.w <= 0.0f) return NO;
    const glm::vec3 ndc = glm::vec3(clip) / clip.w;
    const int w = _canvas->getWidth();
    const int h = _canvas->getHeight();
    if (w <= 0 || h <= 0) return NO;
    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    const CGFloat px = (ndc.x * 0.5 + 0.5) * static_cast<double>(w) / scaleFactor;
    const CGFloat py = (1.0 - (ndc.y * 0.5 + 0.5)) * static_cast<double>(h) / scaleFactor;
    const CGFloat widthPt  = static_cast<double>(w) / scaleFactor;
    const CGFloat heightPt = static_cast<double>(h) / scaleFactor;
    if (px < -marginPt || px > widthPt + marginPt ||
        py < -marginPt || py > heightPt + marginPt) return NO;
    *outPt = CGPointMake(px, py);
    return YES;
}

- (nullable NSValue*)screenAnchorPointForModel:(NSString*)modelName
                                    forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !modelName || modelName.length == 0) return nil;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return nil;
    Model* m = rctx->GetModelManager()[modelName.UTF8String];
    if (!m) return nil;
    auto& loc = m->GetModelScreenLocation();
    // Bottom-centre in world coords. The action bar used to anchor
    // to the top, but every gizmo handle (Y axis arrow, rotate
    // ring, shear puck) lives at or above the model's top edge —
    // bottom is the only side that's consistently clear.
    const glm::vec3 anchor(loc.GetHcenterPos(),
                            loc.GetVcenterPos() - loc.GetMHeight() * 0.5f,
                            loc.GetDcenterPos());
    CGPoint pt;
    if (![self projectWorldPoint:anchor toViewPoint:&pt marginPts:80.0]) {
        return nil;
    }
    return [NSValue valueWithCGPoint:pt];
}

- (nullable NSString*)createModelOfType:(NSString*)type
                           atScreenPoint:(CGPoint)point
                                viewSize:(CGSize)viewSize
                             forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !type || type.length == 0) return nil;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return nil;

    // J-18.5 — abort any in-flight render before adding a new
    // Model to the manager; render workers hold raw Model*
    // references and racing them produces hard-to-reproduce
    // crashes. Mirrors the desktop's AbortRender() guard in
    // LayoutPanel before model creation.
    rctx->AbortRender(5000);
    Model* m = rctx->GetModelManager().CreateDefaultModel(type.UTF8String, "1");
    if (!m) return nil;

    auto& loc = m->GetModelScreenLocation();

    // Mirror desktop's create flow exactly:
    //   `InitializeLocation` calls `FindPlaneIntersection`, which
    //   picks the best world plane (XZ floor / XY wall / YZ side)
    //   based on the current camera angles, then unprojects the
    //   touch onto that plane and sets `worldPos_x/y/z`. This is
    //   why desktop placement of, say, icicles along a roof line
    //   naturally follows the roof's screen-space slope — the
    //   click projects onto the XY wall at Z=0, so a drag along
    //   the roof in screen coords produces a sloped line in
    //   world coords. Hard-coding the XZ floor would always land
    //   on Y=0 regardless of where the user clicked.
    //
    // Desktop's mouseX/mouseY are window-pixel coords; convert
    // the UIKit point through the canvas's scale factor.
    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    const int px = static_cast<int>(point.x * scaleFactor);
    const int py = static_cast<int>(point.y * scaleFactor);
    int initHandle = 0;
    std::vector<NodeBaseClassPtr> emptyNodes;
    loc.InitializeLocation(initHandle, px, py, emptyNodes, _preview.get());

    std::string layoutGroup = rctx->GetActiveLayoutGroup();
    if (layoutGroup.empty() || layoutGroup == "All Models") {
        layoutGroup = "Default";
    }
    m->SetLayoutGroup(layoutGroup);
    m->SetControllerName("");

    rctx->GetModelManager().AddModel(m);
    rctx->MarkLayoutModelDirty(m->GetName());

    // Start the placement `BeginCreate` session so subsequent
    // `dragHandle` calls size the model as the user drags. If the
    // user lifts without dragging, `endHandleDrag` commits the
    // session and the model stays at whatever `InitializeLocation`
    // left it at — same as desktop's tap-without-drag behaviour.
    handles::WorldRay startRay;
    TouchPointToWorldRay(point, scaleFactor, _preview.get(),
                          startRay.origin, startRay.direction);
    auto session = loc.BeginCreate(
        m->GetName(), startRay,
        _preview->Is3D() ? handles::ViewMode::ThreeD : handles::ViewMode::TwoD);
    if (session) {
        _dragSession = std::move(session);
        _dragSessionStartId = _dragSession->GetHandleId();
    }

    return [NSString stringWithUTF8String:m->GetName().c_str()];
}

- (nullable NSString*)importXmodelFromPath:(NSString*)path
                              atScreenPoint:(CGPoint)point
                                   viewSize:(CGSize)viewSize
                                forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !path || path.length == 0) return nil;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return nil;

    // Parse the .xmodel XML before creating anything — failing
    // late would leak a stub model into the show.
    pugi::xml_document xdoc;
    pugi::xml_parse_result parseRes = xdoc.load_file(path.UTF8String);
    if (!parseRes) return nil;
    pugi::xml_node root = xdoc.document_element();
    if (!root) return nil;

    rctx->AbortRender(5000);
    // CreateDefaultModelFromSavedModelNode wants a baseline Model*
    // to mutate / replace. Use "Custom" — the import path swaps it
    // out for the deserialized type anyway, so the placeholder
    // choice only matters in the very narrow paths where the
    // function preserves baseline state (start channel, layout
    // group, hcenter/vcenter — we override those below).
    Model* baseline = rctx->GetModelManager().CreateDefaultModel("Custom", "1");
    if (!baseline) return nil;
    bool cancelled = false;
    Model* imported = baseline->CreateDefaultModelFromSavedModelNode(
        baseline, root, rctx->GetModelManager(), cancelled);
    if (cancelled || !imported) {
        delete baseline;
        return nil;
    }

    // J-4 (import) — match the show's ruler when the imported
    // xmodel carries real-world dimensions but the deserializer
    // didn't already apply them. Two file shapes to handle:
    //   - New format (recent desktop saves): `<dimensions
    //     units="mm" width=N height=N depth=N>` child element.
    //     `XmlDeserializingModelFactory::DeserializeModel(...,
    //     importing=true)` already calls ApplyDimensions for this
    //     path, which sets `modelMgr.SetUsedRuler()`.
    //   - Legacy format (most catalog-downloaded files +
    //     hand-authored exports): `widthmm` / `heightmm` /
    //     `depthmm` attributes on the root element. The
    //     deserializer doesn't see these. Apply them here so the
    //     imported model lands at its real-world size in the
    //     current show's units instead of whatever world-unit
    //     scale the original author saved.
    if (!rctx->GetModelManager().UsedRuler() && RulerObject::GetRuler() != nullptr) {
        const float widthmm  = static_cast<float>(std::strtod(
            root.attribute("widthmm").as_string("0"), nullptr));
        const float heightmm = static_cast<float>(std::strtod(
            root.attribute("heightmm").as_string("0"), nullptr));
        const float depthmm  = static_cast<float>(std::strtod(
            root.attribute("depthmm").as_string("0"), nullptr));
        if (widthmm > 0 && heightmm > 0) {
            imported->ApplyDimensions("mm", widthmm, heightmm, depthmm);
        }
    }

    // Position via InitializeLocation so the imported model lands
    // under the touch point rather than at whatever world coords
    // the .xmodel saved (which is typically the original author's
    // show coordinate, often off-screen for us).
    auto& loc = imported->GetModelScreenLocation();
    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    const int px = static_cast<int>(point.x * scaleFactor);
    const int py = static_cast<int>(point.y * scaleFactor);
    int initHandle = 0;
    std::vector<NodeBaseClassPtr> emptyNodes;
    loc.InitializeLocation(initHandle, px, py, emptyNodes, _preview.get());

    std::string layoutGroup = rctx->GetActiveLayoutGroup();
    if (layoutGroup.empty() || layoutGroup == "All Models") {
        layoutGroup = "Default";
    }
    imported->SetLayoutGroup(layoutGroup);
    // NO_CONTROLLER triggers ReworkStartChannel auto-assign so the
    // imported model doesn't collide with an existing one's
    // start-channel range. Matches desktop's GetXlightsModel
    // (LayoutPanel.cpp ~4629).
    imported->SetControllerName(NO_CONTROLLER, true);

    rctx->GetModelManager().AddModel(imported);
    rctx->MarkLayoutModelDirty(imported->GetName());

    return [NSString stringWithUTF8String:imported->GetName().c_str()];
}

- (BOOL)modelUsesPolyPointLocation:(NSString*)name
                       forDocument:(XLSequenceDocument*)doc {
    if (!doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[name.UTF8String];
    if (!m) return NO;
    return dynamic_cast<PolyPointScreenLocation*>(&m->GetModelScreenLocation()) != nullptr;
}

- (BOOL)appendVertexToPolyline:(NSString*)name
                  atScreenPoint:(CGPoint)point
                       viewSize:(CGSize)viewSize
                    forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[name.UTF8String];
    if (!m) return NO;
    auto* poly = dynamic_cast<PolyPointScreenLocation*>(&m->GetModelScreenLocation());
    if (!poly) return NO;
    if (poly->IsLocked()) return NO;

    // Commit whatever the previous tap/drag left open (the
    // BeginCreate session from the first vertex, or the
    // BeginExtend session from the previous appended vertex).
    // Inlined instead of calling -endHandleDragForDocument:
    // because that path resets `active_axis` to NO_AXIS when no
    // session is active — but `AddHandle` below relies on the
    // axis InitializeLocation seeded for plane projection, so
    // wiping it asserts inside `DragHandle`.
    if (_dragSession) {
        auto result = _dragSession->Commit();
        if (handles::HasDirty(result.dirty, handles::DirtyField::Position) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Dimensions) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Rotation) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Endpoint) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Vertex) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Curve) ||
            handles::HasDirty(result.dirty, handles::DirtyField::Shear)) {
            if (!result.modelName.empty()) {
                rctx->MarkLayoutModelDirty(result.modelName);
            }
        }
        _dragSession.reset();
        _dragSessionStartId = handles::Id{};
        _dragSessionViewObjectName.clear();
    }

    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    const int px = static_cast<int>(point.x * scaleFactor);
    const int py = static_cast<int>(point.y * scaleFactor);

    // CreateDefaultModel for a polyline initialises num_points=2
    // with mPos[0] and mPos[1] both at (0,0,0). On the FIRST
    // append after create the placeholder vertex 1 still equals
    // vertex 0 — promote it to the tap position rather than
    // appending a third vertex (otherwise the zero-length
    // segment 0→1 burns light nodes at the origin). On
    // subsequent appends the placeholder has moved, so a normal
    // AddHandle is correct.
    bool promotePlaceholder = false;
    if (poly->GetNumPoints() == 2) {
        const glm::vec3 p0 = poly->GetPoint(0);
        const glm::vec3 p1 = poly->GetPoint(1);
        if (p0.x == p1.x && p0.y == p1.y && p0.z == p1.z) {
            promotePlaceholder = true;
        }
    }
    if (promotePlaceholder) {
        // AddHandle appends vertex 2 at the tap, then drop the
        // coincident vertex 1; net effect: vertex 1 moves to the
        // tap projection while preserving AddHandle's plane logic.
        // num_points stays at 2 so no per-segment growth is needed.
        poly->AddHandle(_preview.get(), px, py);
        poly->DeleteHandle(1);
    } else {
        poly->AddHandle(_preview.get(), px, py);
        // Match desktop's polyline-create flow
        // (LayoutPanel.cpp:4117-4120): a PolyLineModel keeps a
        // per-segment vector (`_polyLineSizes`) that must grow
        // when a vertex is added. Without it, `InitModel`
        // re-distributes lights based on a stale segment count
        // and nothing renders along the new path until the user
        // touches the model again. MultiPoint has no per-segment
        // array, so the dynamic_cast skip is fine there.
        if (auto* polyModel = dynamic_cast<PolyLineModel*>(m)) {
            polyModel->AddHandle();
        }
    }
    m->Reinitialize();
    const int newIdx = poly->GetNumPoints() - 1;
    if (newIdx <= 0) return NO;

    // Refresh active_plane from the camera before opening the
    // extension session so the new-vertex drag uses the plane
    // that matches the current view (top-down → XZ, side → YZ,
    // front → XY) instead of whatever InitializeLocation seeded
    // at first-vertex create time.
    poly->RefreshActivePlaneFromCamera(_preview.get());

    // Start an extension session on the new vertex so a follow-on
    // drag through `dragHandle:toScreenPoint:` sizes that segment.
    // Tap-without-drag leaves the vertex where AddHandle put it
    // (the next endHandleDragForDocument call commits cleanly).
    handles::WorldRay startRay;
    TouchPointToWorldRay(point, scaleFactor, _preview.get(),
                          startRay.origin, startRay.direction);
    auto session = poly->BeginExtend(
        m->GetName(), startRay,
        _preview->Is3D() ? handles::ViewMode::ThreeD : handles::ViewMode::TwoD,
        newIdx);
    if (session) {
        _dragSession = std::move(session);
        _dragSessionStartId = _dragSession->GetHandleId();
    }

    rctx->MarkLayoutModelDirty(m->GetName());
    return YES;
}

- (NSArray<NSDictionary*>*)modelLabelAnchorsForDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc) return @[];
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return @[];
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    const auto models = rctx->GetModelsForActivePreview();
    for (Model* m : models) {
        if (!m || m->GetDisplayAs() == DisplayAsType::SubModel) continue;
        auto& loc = m->GetModelScreenLocation();
        // Anchor at the model's centre (not its top) so labels sit
        // inside the model body, which reads better when many
        // labels share screen space.
        const glm::vec3 anchor(loc.GetHcenterPos(),
                                loc.GetVcenterPos(),
                                loc.GetDcenterPos());
        CGPoint pt;
        if (![self projectWorldPoint:anchor toViewPoint:&pt marginPts:0.0]) {
            continue;
        }
        NSString* name = [NSString stringWithUTF8String:m->GetName().c_str()];
        [out addObject:@{ @"name": name, @"anchor": [NSValue valueWithCGPoint:pt] }];
    }
    return out;
}

- (BOOL)clearHoveredHandleForDocument:(XLSequenceDocument*)doc {
    if (!doc || _selectedModelName.empty()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[_selectedModelName];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();
    if (!loc.GetHighlightedHandleId().has_value()) return NO;
    loc.MouseOverHandle(std::nullopt);
    return YES;
}

- (nullable NSDictionary*)inspectHandleAtScreenPoint:(CGPoint)point
                                            viewSize:(CGSize)viewSize
                                         forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || _selectedModelName.empty()) return nil;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return nil;
    Model* m = rctx->GetModelManager()[_selectedModelName];
    if (!m) return nil;
    auto& loc = m->GetModelScreenLocation();

    // Use Translate as the lookup tool — vertex / segment / curve-
    // control descriptors are not tool-gated, only axis-gizmo
    // descriptors are. Tool::Translate is the safe lookup choice.
    handles::ViewParams view;
    if (_preview->Is3D()) {
        const float zoom = _preview->GetCameraZoomForHandles();
        const int hscale = _preview->GetHandleScale();
        view.axisArrowLength = loc.GetAxisArrowLength(zoom, hscale);
        view.axisHeadLength  = loc.GetAxisHeadLength(zoom, hscale);
        view.axisRadius      = loc.GetAxisRadius(zoom, hscale);
    }
    const auto descs = m->GetHandles(
        _preview->Is3D() ? handles::ViewMode::ThreeD : handles::ViewMode::TwoD,
        handles::Tool::Translate, view);
    if (descs.empty()) return nil;

    handles::ScreenProjection proj;
    proj.projViewMatrix = _preview->GetProjViewMatrix();
    proj.viewportWidth  = _canvas->getWidth();
    proj.viewportHeight = _canvas->getHeight();
    double scaleFactor = _canvas->getScaleFactor();
    if (scaleFactor <= 0) scaleFactor = 1.0;
    glm::vec2 touchPx{
        static_cast<float>(point.x * scaleFactor),
        static_cast<float>(point.y * scaleFactor)
    };
    handles::HitTestOptions opts;
    opts.handleTolerance = 28.0f;  // long-press is finger-driven; looser slop
    auto hit = handles::HitTest(descs, proj, touchPx, opts);
    if (!hit) return nil;

    NSString* name = [NSString stringWithUTF8String:m->GetName().c_str()];
    switch (hit->id.role) {
        case handles::Role::Vertex:
            return @{
                @"type"        : @"vertex",
                @"modelName"   : name,
                @"vertexIndex" : @(hit->id.index),
            };
        case handles::Role::Segment: {
            const bool curved = loc.HasCurve(hit->id.index);
            return @{
                @"type"         : @"segment",
                @"modelName"    : name,
                @"segmentIndex" : @(hit->id.index),
                @"hasCurve"     : @(curved),
            };
        }
        case handles::Role::CurveControl:
            return @{
                @"type"         : @"curve_control",
                @"modelName"    : name,
                @"segmentIndex" : @(hit->id.segment),
            };
        default:
            return nil;
    }
}


- (BOOL)beginPinchScaleForModel:(NSString*)name forDocument:(XLSequenceDocument*)doc {
    if (!doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[name.UTF8String];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();
    if (loc.IsLocked()) return NO;
    _pinchScaleActive    = YES;
    _pinchScaleSavedScale = loc.GetScaleMatrix();
    _pinchScaleModelName  = name.UTF8String;
    return YES;
}

- (BOOL)applyPinchScaleFactor:(CGFloat)factor forDocument:(XLSequenceDocument*)doc {
    if (!_pinchScaleActive || !doc || _pinchScaleModelName.empty()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    rctx->AbortRender(5000);
    ModelScreenLocation* locPtr = nullptr;
    if (_pinchScaleTargetIsVO) {
        if (!rctx->HasViewObjectManager()) return NO;
        ViewObject* vo = rctx->GetAllObjects().GetViewObject(_pinchScaleModelName);
        if (!vo) return NO;
        locPtr = &vo->GetObjectScreenLocation();
    } else {
        Model* m = rctx->GetModelManager()[_pinchScaleModelName];
        if (!m) return NO;
        locPtr = &m->GetModelScreenLocation();
    }
    auto& loc = *locPtr;
    if (loc.IsLocked()) return NO;
    const float f = std::clamp(static_cast<float>(factor), 0.05f, 50.0f);
    loc.SetScaleMatrix(_pinchScaleSavedScale * f);
    if (_pinchScaleTargetIsVO) {
        rctx->MarkLayoutViewObjectDirty(_pinchScaleModelName);
    } else {
        rctx->MarkLayoutModelDirty(_pinchScaleModelName);
    }
    return YES;
}

- (void)endPinchScale {
    _pinchScaleActive = NO;
    _pinchScaleModelName.clear();
    _pinchScaleTargetIsVO = NO;
}

- (BOOL)beginTwistRotateForModel:(NSString*)name forDocument:(XLSequenceDocument*)doc {
    if (!doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    Model* m = rctx->GetModelManager()[name.UTF8String];
    if (!m) return NO;
    auto& loc = m->GetModelScreenLocation();
    if (loc.IsLocked()) return NO;
    _twistRotateActive       = YES;
    _twistRotateSavedRotation = loc.GetRotationAngles();
    _twistRotateModelName     = name.UTF8String;
    return YES;
}

- (BOOL)applyTwistRotationRadians:(CGFloat)radians forDocument:(XLSequenceDocument*)doc {
    if (!_twistRotateActive || !doc || _twistRotateModelName.empty()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx) return NO;
    rctx->AbortRender(5000);
    ModelScreenLocation* locPtr = nullptr;
    if (_twistRotateTargetIsVO) {
        if (!rctx->HasViewObjectManager()) return NO;
        ViewObject* vo = rctx->GetAllObjects().GetViewObject(_twistRotateModelName);
        if (!vo) return NO;
        locPtr = &vo->GetObjectScreenLocation();
    } else {
        Model* m = rctx->GetModelManager()[_twistRotateModelName];
        if (!m) return NO;
        locPtr = &m->GetModelScreenLocation();
    }
    auto& loc = *locPtr;
    if (loc.IsLocked()) return NO;
    // UIRotationGestureRecognizer reports clockwise as positive;
    // negate to match the "scene follows the fingers" convention
    // used by the camera-rotate path.
    const float degrees = -static_cast<float>(radians) * 180.0f / static_cast<float>(M_PI);
    const glm::vec3 newRot{_twistRotateSavedRotation.x,
                            _twistRotateSavedRotation.y,
                            _twistRotateSavedRotation.z + degrees};
    loc.SetRotation(newRot);
    if (_twistRotateTargetIsVO) {
        rctx->MarkLayoutViewObjectDirty(_twistRotateModelName);
    } else {
        rctx->MarkLayoutModelDirty(_twistRotateModelName);
    }
    return YES;
}

- (void)endTwistRotate {
    _twistRotateActive = NO;
    _twistRotateModelName.clear();
    _twistRotateTargetIsVO = NO;
}

// J-15 — view-object equivalents of the model 3D-gesture entry
// points. Each grabs the screen location from the active VO,
// latches the right saved state, and flags the target as VO so
// the shared apply/end methods route correctly.
- (BOOL)beginBodyDrag3DForViewObject:(NSString*)name
                        atScreenPoint:(CGPoint)point
                             viewSize:(CGSize)viewSize
                          forDocument:(XLSequenceDocument*)doc {
    if (!_preview || !doc || !name || name.length == 0) return NO;
    if (!_preview->Is3D()) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasViewObjectManager()) return NO;
    ViewObject* vo = rctx->GetAllObjects().GetViewObject(name.UTF8String);
    if (!vo) return NO;
    auto& loc = vo->GetObjectScreenLocation();
    if (loc.IsLocked() || vo->IsFromBase()) return NO;

    loc.RefreshActivePlaneFromCamera(_preview.get());
    const auto plane = loc.GetActivePlane();
    const glm::vec3 center(loc.GetHcenterPos(),
                            loc.GetVcenterPos(),
                            loc.GetDcenterPos());
    glm::vec3 planePt{0.0f}, planeN{0.0f};
    switch (plane) {
        case ModelScreenLocation::MSLPLANE::XZ_PLANE:
            planeN = glm::vec3(0.0f, 1.0f, 0.0f);
            planePt = glm::vec3(0.0f, center.y, 0.0f);
            break;
        case ModelScreenLocation::MSLPLANE::YZ_PLANE:
            planeN = glm::vec3(1.0f, 0.0f, 0.0f);
            planePt = glm::vec3(center.x, 0.0f, 0.0f);
            break;
        case ModelScreenLocation::MSLPLANE::XY_PLANE:
        default:
            planeN = glm::vec3(0.0f, 0.0f, 1.0f);
            planePt = glm::vec3(0.0f, 0.0f, center.z);
            break;
    }
    glm::vec3 origin, dir;
    TouchPointToWorldRay(point, _canvas->getScaleFactor(), _preview.get(),
                          origin, dir);
    glm::vec3 hit;
    if (!VectorMath::GetPlaneIntersect(origin, dir, planePt, planeN, hit)) {
        return NO;
    }
    _bodyDrag3DActive       = YES;
    _bodyDrag3DTargetIsVO   = YES;
    _bodyDrag3DSavedCenter  = center;
    _bodyDrag3DAnchor       = hit;
    _bodyDrag3DPlane        = plane;
    _bodyDrag3DPlanePoint   = planePt;
    _bodyDrag3DPlaneNormal  = planeN;
    _bodyDrag3DModelName    = name.UTF8String;
    return YES;
}

- (BOOL)beginPinchScaleForViewObject:(NSString*)name
                           forDocument:(XLSequenceDocument*)doc {
    if (!doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasViewObjectManager()) return NO;
    ViewObject* vo = rctx->GetAllObjects().GetViewObject(name.UTF8String);
    if (!vo) return NO;
    auto& loc = vo->GetObjectScreenLocation();
    if (loc.IsLocked() || vo->IsFromBase()) return NO;
    _pinchScaleActive      = YES;
    _pinchScaleTargetIsVO  = YES;
    _pinchScaleSavedScale  = loc.GetScaleMatrix();
    _pinchScaleModelName   = name.UTF8String;
    return YES;
}

- (BOOL)beginTwistRotateForViewObject:(NSString*)name
                           forDocument:(XLSequenceDocument*)doc {
    if (!doc || !name || name.length == 0) return NO;
    iPadRenderContext* rctx = ContextFromDoc(doc);
    if (!rctx || !rctx->HasViewObjectManager()) return NO;
    ViewObject* vo = rctx->GetAllObjects().GetViewObject(name.UTF8String);
    if (!vo) return NO;
    auto& loc = vo->GetObjectScreenLocation();
    if (loc.IsLocked() || vo->IsFromBase()) return NO;
    _twistRotateActive        = YES;
    _twistRotateTargetIsVO    = YES;
    _twistRotateSavedRotation = loc.GetRotationAngles();
    _twistRotateModelName     = name.UTF8String;
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
            // SubModels share their parent's screenLocation
            // (SubModel.h:29) — they're effects-buffer carve-outs
            // of a parent model, not standalone layout entities.
            // Desktop's LayoutPanel doesn't expose them as
            // selectable / movable; matches that behaviour here.
            // House Preview still renders them via the parent's
            // node loop, so excluding from the LayoutEditor pane's
            // iteration doesn't lose any pixels.
            if (_isLayoutEditor &&
                model->GetDisplayAs() == DisplayAsType::SubModel) {
                continue;
            }
            glm::vec3 c = model->GetModelScreenLocation().GetCenterPosition();
            float z = (viewMatrix * glm::vec4(c, 1.0f)).z;
            keyed.emplace_back(model, z);
        }
        std::stable_sort(keyed.begin(), keyed.end(),
                         [](const std::pair<Model*, float>& a, const std::pair<Model*, float>& b) {
                             return a.second < b.second;
                         });
        const bool is3d = _preview->Is3D();
        // Mirror desktop's `ModelPreview::RenderModels` for the
        // LayoutEditor pane: `allowSelected=true` so `PrepareToDraw`
        // updates `ModelMatrix` (required by `HitTest3D`), a non-null
        // `color` so models render with the layout-edit override
        // colour rather than effect output, and `Selected(true)` on
        // the chosen model so `DisplayModelOnWindow`'s built-in
        // DrawHandles fires. House Preview keeps the playback /
        // effect-color path (c=nullptr, allowSelected=false).
        const bool allowSel = _isLayoutEditor ? true : false;
        // Default = xlLIGHT_GREY, Selected = xlYELLOW. Hardcoded
        // because ColorManager lives in src-ui-wx; defaults match
        // ColorManager.h:141-142.
        static const xlColor sLayoutDefaultColor = xlLIGHT_GREY;
        static const xlColor sLayoutSelectedColor = xlYELLOW;
        // J-10 — group-member tint when a ModelGroup is picked in
        // the sidebar. Yellow for both primary selection and group
        // members — cyan didn't differentiate well against grey
        // at pixel sizes 1–2 (the common case). The sidebar
        // selection still owns the active gizmo so the user can
        // tell them apart by handles, not by colour.
        static const xlColor& sLayoutGroupMemberColor = sLayoutSelectedColor;
        // Resolve the selected group's flat member set once per
        // frame. Two buckets so we can highlight differently:
        //   - selectedGroupMembers: top-level Models that should
        //     render fully tinted yellow.
        //   - selectedGroupSubmodelsByParent: parent Model* →
        //     submodels that belong to the group. After the main
        //     loop, those submodels are rendered as a selection
        //     overlay so only their node ranges light up (matches
        //     desktop's behaviour for submodel-only groups).
        std::set<const Model*> selectedGroupMembers;
        std::map<const Model*, std::vector<Model*>> selectedGroupSubmodelsByParent;
        if (_isLayoutEditor && !_selectedGroupName.empty()) {
            Model* grpModel = ctx->GetModelManager()[_selectedGroupName];
            if (grpModel && grpModel->GetDisplayAs() == DisplayAsType::ModelGroup) {
                auto* grp = static_cast<ModelGroup*>(grpModel);
                for (Model* m : grp->GetFlatModels(true, false)) {
                    if (!m) continue;
                    if (auto* sm = dynamic_cast<SubModel*>(m)) {
                        if (Model* parent = sm->GetParent()) {
                            selectedGroupSubmodelsByParent[parent].push_back(m);
                        }
                    } else {
                        selectedGroupMembers.insert(m);
                    }
                }
            }
        }
        for (const auto& [model, z] : keyed) {
            const xlColor* useColor = nullptr;
            if (_isLayoutEditor) {
                const bool isPrimary = (!_selectedModelName.empty() &&
                                          model->GetName() == _selectedModelName);
                const bool isExtra = (!isPrimary &&
                                       _extraSelectedModels.count(model->GetName()) > 0);
                const bool isGroupMember = (!isPrimary && !isExtra &&
                                             selectedGroupMembers.count(model) > 0);
                const bool isSel = isPrimary || isExtra;
                model->Selected(isSel);
                // 3D gizmo: subclass `DrawHandles` only paints the
                // centre sphere + axis gizmo when `active_handle`
                // has a value. Latch CentreCycle on the PRIMARY
                // only (so multi-select doesn't draw N gizmos),
                // and clear on deselect.
                auto& sloc = model->GetModelScreenLocation();
                if (isPrimary) {
                    if (!sloc.GetActiveHandleId().has_value()) {
                        sloc.SetActiveHandleToCentre();
                    }
                } else if (sloc.GetActiveHandleId().has_value()) {
                    sloc.SetActiveHandle(std::nullopt);
                    sloc.SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
                }
                if (isSel) {
                    useColor = &sLayoutSelectedColor;
                } else if (isGroupMember) {
                    useColor = &sLayoutGroupMemberColor;
                } else {
                    useColor = &sLayoutDefaultColor;
                }
            }
            // Pass the current 2D/3D state rather than a hardcoded true —
            // it drives PrepareToDraw's draw_3d flag (which controls
            // worldPos_z usage and the 2D perspective rotation) and the
            // uiCaches key. Telling a 2D render "we're 3D" made
            // BoxedScreenLocation place models at their saved Z, and with
            // ortho left-handed near=1/far=0 any non-zero Z put geometry
            // outside the frustum.
            model->DisplayModelOnWindow(_preview.get(), graphicsCtx, solidProg, transparentProg,
                                         is3d,
                                         /* color */ useColor,
                                         /* allowSelected */ allowSel,
                                         /* wiring */ false,
                                         /* highlightFirst */ _showFirstPixel,
                                         /* highlightpixel */ 0,
                                         /* boundingBox */ nullptr);
        }

        // J-10 — submodel-only group overlay. After every parent
        // has rendered in default grey, paint each group-member
        // submodel on top of its parent so only the submodel's
        // node range lights up yellow. SubModels share their
        // parent's screen location and have their own `Nodes`
        // vector — `DisplayModelOnWindow` iterates the SubModel's
        // nodes only, producing the desired isolated highlight.
        if (_isLayoutEditor && !selectedGroupSubmodelsByParent.empty()) {
            for (const auto& [parent, submodels] : selectedGroupSubmodelsByParent) {
                for (Model* sm : submodels) {
                    if (!sm) continue;
                    sm->Selected(false);
                    sm->DisplayModelOnWindow(_preview.get(), graphicsCtx, solidProg, transparentProg,
                                              is3d,
                                              /* color */ &sLayoutSelectedColor,
                                              /* allowSelected */ false,
                                              /* wiring */ false,
                                              /* highlightFirst */ false,
                                              /* highlightpixel */ 0,
                                              /* boundingBox */ nullptr);
                }
            }
        }

        // View objects (house meshes, ground images, gridlines, terrain).
        // Only the Default layout group owns view objects (desktop hard-
        // codes their layout_group to "Default"); named groups skip the
        // loop entirely. Also gated on the "View Objects" toggle.
        if (_showViewObjects && ctx->ActivePreviewShowsViewObjects()) {
            auto& allObjects = ctx->GetAllObjects();
            for (auto it = allObjects.begin(); it != allObjects.end(); ++it) {
                ViewObject* vo = it->second;
                if (!vo) continue;
                // J-6 (sidebar canvas sync) — when the Objects tab
                // has a pick, render that object with
                // `allowSelected=true` so its ScreenLocation
                // handles draw. `Selected(true)` lights the
                // selection ring inside the object's `Draw`.
                const bool isSel = (_isLayoutEditor &&
                                     !_selectedViewObjectName.empty() &&
                                     vo->GetName() == _selectedViewObjectName);
                vo->Selected(isSel);
                vo->Draw(_preview.get(), graphicsCtx, solidVOProg, transparentVOProg,
                         isSel);
            }
        }

        // J-2 — Layout Editor in-canvas overlays. 2D-only. The
        // grid + canvas-bounding-box draw before the selection ring
        // so the ring stays on top. Initial overlay state is seeded
        // from rgbeffects.xml flags on the first draw; subsequent
        // draws use whatever the user has toggled in the LayoutEditor
        // controls overlay.
        if (!_preview->Is3D()) {
            if (!_layoutOverlaysSeeded) {
                _showLayoutGrid = ctx->GetDisplay2DGrid() ? YES : NO;
                _showLayoutBoundingBox = ctx->GetDisplay2DBoundingBox() ? YES : NO;
                _layoutOverlaysSeeded = YES;
            }
            const int virtualW = ctx->GetPreviewWidth();
            const int virtualH = ctx->GetPreviewHeight();
            const long spacing = std::max((long)1, ctx->GetDisplay2DGridSpacing());
            const bool center0 = ctx->GetDisplay2DCenter0();
            const float xMin = center0 ? -(float)virtualW * 0.5f : 0.0f;
            const float xMax = xMin + (float)virtualW;
            const float yMin = 0.0f;
            const float yMax = (float)virtualH;

            if (_showLayoutGrid && virtualW > 0 && virtualH > 0) {
                solidProg->addStep([xMin, xMax, yMin, yMax, spacing](xlGraphicsContext* c) {
                    std::unique_ptr<xlVertexAccumulator> va(c->createVertexAccumulator());
                    // 2 vertices per line; estimate count to avoid
                    // realloc churn on dense grids.
                    int nx = (int)((xMax - xMin) / (float)spacing) + 1;
                    int ny = (int)((yMax - yMin) / (float)spacing) + 1;
                    va->PreAlloc((nx + ny) * 2);
                    // Vertical lines, snapped to spacing multiples so
                    // a center0 layout has lines at -spacing, 0,
                    // +spacing rather than at the irregular xMin.
                    float xStart = std::floor(xMin / (float)spacing) * (float)spacing;
                    for (float x = xStart; x <= xMax; x += (float)spacing) {
                        if (x < xMin) continue;
                        va->AddVertex(x, yMin);
                        va->AddVertex(x, yMax);
                    }
                    for (float y = 0.0f; y <= yMax; y += (float)spacing) {
                        va->AddVertex(xMin, y);
                        va->AddVertex(xMax, y);
                    }
                    // Muted grey so the grid recedes against models.
                    xlColor gridColor((uint8_t)100, (uint8_t)100, (uint8_t)100, (uint8_t)180);
                    c->drawLines(va.get(), gridColor);
                });
            }

            if (_showLayoutBoundingBox && virtualW > 0 && virtualH > 0) {
                transparentProg->addStep([xMin, xMax, yMin, yMax](xlGraphicsContext* c) {
                    std::unique_ptr<xlVertexAccumulator> va(c->createVertexAccumulator());
                    va->PreAlloc(8);
                    va->AddRectAsLines(xMin, yMin, xMax, yMax);
                    // Translucent green — matches desktop's
                    // `xlGREENTRANSLUCENT` for `_display2DBox`
                    // (ModelPreview.cpp:1622).
                    xlColor boxColor((uint8_t)0, (uint8_t)200, (uint8_t)0, (uint8_t)160);
                    c->drawLines(va.get(), boxColor);
                });
            }
        }

        // Selection ring + per-type handles draw automatically inside
        // Model::DisplayModelOnWindow when `Selected() && c != nullptr
        // && allowSelected` (Model.cpp:3254). Both conditions are met
        // for the LayoutEditor pane via the loop above, so no extra
        // bridge-side rendering is needed here.
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
