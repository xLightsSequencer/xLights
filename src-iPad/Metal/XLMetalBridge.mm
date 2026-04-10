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

#include <memory>

#define PIXEL_SIZE_ON_DIALOGS 2.0

@implementation XLMetalBridge {
    std::unique_ptr<xlStandaloneMetalCanvas> _canvas;
    std::unique_ptr<iPadModelPreview> _preview;
}

- (instancetype)initWithName:(NSString*)name {
    self = [super init];
    if (self) {
        _canvas = std::make_unique<xlStandaloneMetalCanvas>(
            std::string([name UTF8String]), false);
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

    auto models = ctx->GetModelManager().GetModels();
    for (auto& [name, model] : models) {
        // Skip model groups — their member models are drawn individually
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        model->DisplayModelOnWindow(_preview.get(), graphicsCtx, solidProg, transparentProg,
                                     false, nullptr, false, false, false, 0, nullptr);
    }

    // Finish and present
    _preview->EndDrawing(true);
}

@end
