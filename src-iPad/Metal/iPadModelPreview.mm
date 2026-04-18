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

    // SetViewport builds the context's projection matrix internally: perspective
    // for is3D, ortho(0, w, 0, h, 1, 0) for 2D. Passing the wrong flag makes
    // vertices clip out of NDC — Model Preview's 2D geometry disappears under
    // a perspective projection.
    _ctx->SetViewport(0, h, w, 0, _is3d);

    // Mirror desktop ModelPreview::StartDrawing matrix construction
    // (src-ui-wx/ui/layout/ModelPreview.cpp). 2D uses an orthographic
    // projection driven by zoom + panx/pany/zoom_corr; 3D uses perspective
    // with the camera's view matrix from PreviewCamera::GetViewMatrix().
    PreviewCamera& cam = ActiveCamera();
    if (_is3d) {
        float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
        _projMatrix = glm::perspective(glm::radians(45.0f), aspect, 1.0f, 200000.0f);
        _viewMatrix = cam.GetViewMatrix();
    } else {
        // 2D ortho mirroring desktop ModelPreview::StartDrawing (ModelPreview.cpp:1241).
        // Projection is in WINDOW pixel space; ViewMatrix carries the
        // virtual→window scale and camera pan/zoom. When _virtualW/_virtualH
        // are 0 (default), scale2d stays 1 and the model is drawn directly in
        // pixel space — that's the Model Preview single-model fit-to-window
        // case, where Model::DisplayEffectOnWindow already produces geometry
        // in the (0..w, 0..h) range via its ctx->Translate/Scale steps.
        float scale2d = 1.0f;
        float scale_corrx = 0.0f;
        float scale_corry = 0.0f;
        if (_virtualW != 0 && _virtualH != 0) {
            float scale2dh = (float)h / (float)_virtualH;
            float scale2dw = (float)w / (float)_virtualW;
            if (scale2dh < scale2dw) {
                scale2d = scale2dh;
                scale_corrx = ((scale2dw*(float)_virtualW - (scale2d*(float)_virtualW)) * cam.GetZoom()) / 2.0f;
            } else {
                scale2d = scale2dw;
                scale_corry = ((scale2dh*(float)_virtualH - (scale2d*(float)_virtualH)) * cam.GetZoom()) / 2.0f;
            }
        }
        glm::mat4 ViewScale = glm::scale(glm::mat4(1.0f),
            glm::vec3(cam.GetZoom() * scale2d, cam.GetZoom() * scale2d, 1.0f));
        glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f),
            glm::vec3(cam.GetPanX()*cam.GetZoom() - cam.GetZoomCorrX() + scale_corrx,
                      cam.GetPanY()*cam.GetZoom() - cam.GetZoomCorrY() + scale_corry,
                      0.0f));
        _viewMatrix = ViewTranslate * ViewScale;
        _projMatrix = glm::ortho(0.0f, (float)w, 0.0f, (float)h);
    }
    _projViewMatrix = _projMatrix * _viewMatrix;

    _ctx->SetCamera(_viewMatrix);
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
