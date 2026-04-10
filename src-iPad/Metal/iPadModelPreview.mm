/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "iPadModelPreview.h"
#include "graphics/metal/xlMetalGraphicsContext.h"
#include "graphics/xlGraphicsAccumulators.h"

#include <glm/gtc/matrix_transform.hpp>

iPadModelPreview::iPadModelPreview(xlStandaloneMetalCanvas* canvas)
    : _canvas(canvas) {
}

iPadModelPreview::~iPadModelPreview() {
    // _ctx is stack-managed in StartDrawing/EndDrawing, never owned here
}

int iPadModelPreview::getWidth() const {
    return _canvas->getWidth();
}

int iPadModelPreview::getHeight() const {
    return _canvas->getHeight();
}

int iPadModelPreview::GetVirtualCanvasWidth() const {
    return _virtualW;
}

int iPadModelPreview::GetVirtualCanvasHeight() const {
    return _virtualH;
}

void iPadModelPreview::GetVirtualCanvasSize(int& w, int& h) const {
    w = _virtualW;
    h = _virtualH;
}

xlGraphicsContext* iPadModelPreview::getCurrentGraphicsContext() {
    return _ctx;
}

bool iPadModelPreview::StartDrawing(double pointSize, bool fromPaint) {
    if (_isDrawing) return false;
    if (_canvas->getMetalLayer() == nil) return false;

    int w = _canvas->getWidth();
    int h = _canvas->getHeight();
    if (w == 0 || h == 0) return false;

    // Create the Metal graphics context — acquires a drawable
    _ctx = new xlMetalGraphicsContext(_canvas, nil, false);
    if (!_ctx->isValid()) {
        delete _ctx;
        _ctx = nullptr;
        return false;
    }

    _isDrawing = true;

    _ctx->SetViewport(0, h, w, 0, false);

    // Match desktop ModelPreview 2D setup:
    // Models live in virtual canvas space (0..virtualW, 0..virtualH)
    // Scale to fit window while maintaining aspect ratio
    float scale2d = 1.0f;
    float scaleCorrX = 0.0f;
    float scaleCorrY = 0.0f;
    if (_virtualW > 0 && _virtualH > 0) {
        float scaleH = (float)h / (float)_virtualH;
        float scaleW = (float)w / (float)_virtualW;
        if (scaleH < scaleW) {
            scale2d = scaleH;
            scaleCorrX = (scaleW * _virtualW - scale2d * _virtualW) / 2.0f;
        } else {
            scale2d = scaleW;
            scaleCorrY = (scaleH * _virtualH - scale2d * _virtualH) / 2.0f;
        }
    }

    glm::mat4 viewScale = glm::scale(glm::mat4(1.0f), glm::vec3(scale2d, scale2d, 1.0f));
    glm::mat4 viewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(scaleCorrX, scaleCorrY, 0.0f));
    glm::mat4 viewMatrix = viewTranslate * viewScale;

    _projMatrix = glm::ortho(0.0f, (float)w, 0.0f, (float)h);
    _projViewMatrix = _projMatrix * viewMatrix;

    _ctx->SetCamera(viewMatrix);
    _ctx->PushMatrix();

    // Set this preview as contextual value so models can retrieve it
    _ctx->setContextualValue("modelPreview", static_cast<IModelPreview*>(this));
    _ctx->enableBlending();

    // Create programs for model rendering
    _solidProgram = _ctx->createGraphicsProgram();
    _transparentProgram = _ctx->createGraphicsProgram();

    return true;
}

void iPadModelPreview::EndDrawing(bool swapBuffers) {
    if (!_isDrawing || !_ctx) return;

    // Run accumulated drawing steps from models
    if (_solidProgram) {
        _solidProgram->runSteps(_ctx);
    }
    if (_transparentProgram) {
        _transparentProgram->runSteps(_ctx);
    }

    _ctx->PopMatrix();

    delete _solidProgram;
    delete _transparentProgram;
    _solidProgram = nullptr;
    _transparentProgram = nullptr;

    _ctx->Commit(swapBuffers, nil);
    delete _ctx;
    _ctx = nullptr;
    _isDrawing = false;
}
