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

#include <memory>

#define PIXEL_SIZE_ON_DIALOGS 2.0

@implementation XLMetalBridge {
    std::unique_ptr<xlStandaloneMetalCanvas> _canvas;
    std::unique_ptr<iPadModelPreview> _preview;
}

- (instancetype)initWithName:(NSString*)name {
    self = [super init];
    if (self) {
        // is3d=true so the canvas allocates a depth buffer + MSAA target.
        // MeshObject (and other view-object renderers) enable depth testing
        // in drawMeshSolids, which crashes validation if no depth attachment
        // is bound. Depth is harmless for existing 2D model rendering.
        _canvas = std::make_unique<xlStandaloneMetalCanvas>(
            std::string([name UTF8String]), true);
        _preview = std::make_unique<iPadModelPreview>(_canvas.get());
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

- (void)setCameraZoom:(float)zoom {
    if (_preview) {
        _preview->SetCameraZoom(zoom);
    }
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

    // Let each model draw itself at its world position
    auto* graphicsCtx = _preview->getCurrentGraphicsContext();
    auto* solidProg = _preview->getCurrentSolidProgram();
    auto* transparentProg = _preview->getCurrentTransparentProgram();
    auto* solidVOProg = _preview->getCurrentSolidViewObjectProgram();
    auto* transparentVOProg = _preview->getCurrentTransparentViewObjectProgram();

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

    // Draw view objects (house meshes, ground images, gridlines, terrain).
    // Matches ModelPreview::Render in src-ui-wx/ui/layout/ModelPreview.cpp —
    // desktop gates this behind is3d but iPad is single-pane so we always draw.
    auto& allObjects = ctx->GetAllObjects();
    for (auto it = allObjects.begin(); it != allObjects.end(); ++it) {
        ViewObject* vo = it->second;
        if (vo) {
            vo->Draw(_preview.get(), graphicsCtx, solidVOProg, transparentVOProg, false);
        }
    }

    // Finish and present
    _preview->EndDrawing(true);
}

@end
