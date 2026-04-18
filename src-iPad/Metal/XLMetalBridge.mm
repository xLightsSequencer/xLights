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

#include <memory>
#include <string>

#define PIXEL_SIZE_ON_DIALOGS 2.0

@implementation XLMetalBridge {
    std::unique_ptr<xlStandaloneMetalCanvas> _canvas;
    std::unique_ptr<iPadModelPreview> _preview;
    std::string _previewModel;   // set via setPreviewModel:
    BOOL _isModelPreview;        // YES = single-model pane; NO = full house
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
        // Model preview defaults to 2D (fit-to-window single-model view); the
        // House preview keeps the 3D default.
        if (_isModelPreview) {
            _preview->SetIs3D(false);
        }
    }
    return self;
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

- (void)drawModelsForDocument:(XLSequenceDocument*)doc atMS:(int)frameMS pointSize:(float)pointSize {
    if (_canvas->getMetalLayer() == nil) return;
    if (_canvas->getWidth() == 0 || _canvas->getHeight() == 0) return;

    iPadRenderContext* ctx = static_cast<iPadRenderContext*>([doc renderContext]);
    if (!ctx) return;

    // Set channel data on all models for this frame
    ctx->SetModelColors(frameMS);

    // Set current frame time so models can query it
    _preview->SetCurrentFrameTime(frameMS);

    // Start a single drawing pass — acquires one drawable
    if (!_preview->StartDrawing(pointSize)) return;

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
        if (!_previewModel.empty()) {
            auto& models = ctx->GetModelManager();
            Model* m = models[_previewModel];
            if (m) {
                m->DisplayEffectOnWindow(_preview.get(), pointSize);
            }
        }
    } else {
        // House Preview: every model at its world position, view objects on top.
        // Sort models back-to-front by camera-space Z of their world centre so
        // alpha-blended pixels from one model composite over models behind them.
        // Matches ModelPreview::RenderModels on desktop.
        auto models = ctx->GetModelManager().GetModels();
        const glm::mat4& viewMatrix = _preview->GetViewMatrix();
        std::vector<std::pair<Model*, float>> keyed;
        keyed.reserve(models.size());
        for (auto& [name, model] : models) {
            if (model->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
            glm::vec3 c = model->GetModelScreenLocation().GetCenterPosition();
            float z = (viewMatrix * glm::vec4(c, 1.0f)).z;
            keyed.emplace_back(model, z);
        }
        std::stable_sort(keyed.begin(), keyed.end(),
                         [](const std::pair<Model*, float>& a, const std::pair<Model*, float>& b) {
                             return a.second < b.second;
                         });
        for (const auto& [model, z] : keyed) {
            model->DisplayModelOnWindow(_preview.get(), graphicsCtx, solidProg, transparentProg,
                                         true, nullptr, false, false, false, 0, nullptr);
        }

        // View objects (house meshes, ground images, gridlines, terrain)
        auto& allObjects = ctx->GetAllObjects();
        for (auto it = allObjects.begin(); it != allObjects.end(); ++it) {
            ViewObject* vo = it->second;
            if (vo) {
                vo->Draw(_preview.get(), graphicsCtx, solidVOProg, transparentVOProg, false);
            }
        }
    }

    // Finish and present
    _preview->EndDrawing(true);
}

@end
