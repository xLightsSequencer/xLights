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

    // 3D perspective camera mirroring desktop ModelPreview 3D setup
    // (see ModelPreview::Render in src-ui-wx/ui/layout/ModelPreview.cpp).
    // Uses PreviewCamera defaults: angleX=20°, angleY=5°, distance=-2000.
    _ctx->SetViewport(0, h, w, 0, true);

    glm::mat4 ViewTranslatePan = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    // Zoom by moving the camera closer/farther along Z. zoom=1 is default,
    // zoom=2 halves the distance (appears 2× larger), zoom=0.5 doubles it.
    const float zoomedZ = -2000.0f / static_cast<float>(_cameraZoom);
    glm::mat4 ViewTranslateDistance = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, zoomedZ));
    glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 viewMatrix = ViewTranslateDistance * ViewRotateX * ViewRotateY * ViewTranslatePan;

    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    _projMatrix = glm::perspective(glm::radians(45.0f), aspect, 1.0f, 200000.0f);
    _viewMatrix = viewMatrix;
    _projViewMatrix = _projMatrix * viewMatrix;

    _ctx->SetCamera(viewMatrix);
    _ctx->PushMatrix();

    // Set this preview as contextual value so models can retrieve it
    _ctx->setContextualValue("modelPreview", static_cast<IModelPreview*>(this));
    _ctx->enableBlending();

    // Create programs for model and view-object rendering.
    // Mirrors desktop ModelPreview which keeps separate solid/transparent
    // programs for models and view objects so draws can be ordered
    // solid-VO → solid-models → transparent-VO → transparent-models
    // (see ModelPreview::EndDrawing in src-ui-wx/ui/layout/ModelPreview.cpp).
    _solidProgram = _ctx->createGraphicsProgram();
    _transparentProgram = _ctx->createGraphicsProgram();
    _solidViewObjectProgram = _ctx->createGraphicsProgram();
    _transparentViewObjectProgram = _ctx->createGraphicsProgram();

    return true;
}

void iPadModelPreview::EndDrawing(bool swapBuffers) {
    if (!_isDrawing || !_ctx) return;

    // Run accumulated drawing steps in the same order desktop uses in 3D mode:
    // solid view objects → solid models → transparent view objects → transparent models.
    // This ensures opaque geometry writes depth first and transparent pieces
    // (semi-transparent meshes, image alpha) blend correctly on top.
    if (_solidViewObjectProgram) {
        _solidViewObjectProgram->runSteps(_ctx);
    }
    if (_solidProgram) {
        _solidProgram->runSteps(_ctx);
    }
    if (_transparentViewObjectProgram) {
        _transparentViewObjectProgram->runSteps(_ctx);
    }
    if (_transparentProgram) {
        _transparentProgram->runSteps(_ctx);
    }

    _ctx->PopMatrix();

    delete _solidProgram;
    delete _transparentProgram;
    delete _solidViewObjectProgram;
    delete _transparentViewObjectProgram;
    _solidProgram = nullptr;
    _transparentProgram = nullptr;
    _solidViewObjectProgram = nullptr;
    _transparentViewObjectProgram = nullptr;

    _ctx->Commit(swapBuffers, nil);
    delete _ctx;
    _ctx = nullptr;
    _isDrawing = false;
}
