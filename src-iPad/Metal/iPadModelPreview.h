#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// iPadModelPreview — IModelPreview implementation for iPad house / model previews.
// Wraps xlStandaloneMetalCanvas and manages the xlMetalGraphicsContext lifecycle
// so that Model::DisplayEffectOnWindow() can draw directly via Metal.
//
// Camera state is held in a pair of PreviewCamera instances (2D and 3D) that
// mirror the desktop ModelPreview setup so zoom/pan/rotate/reset map onto the
// same API desktop uses.

#include "graphics/IModelPreview.h"
#include "render/ViewpointMgr.h"
#include "xlStandaloneMetalCanvas.h"

#include <glm/glm.hpp>
#include <string>

#ifdef __OBJC__
#import <Metal/Metal.h>
#endif

class xlMetalGraphicsContext;

class iPadModelPreview : public IModelPreview {
public:
    iPadModelPreview(xlStandaloneMetalCanvas* canvas);
    ~iPadModelPreview() override;

    // IModelPreview
    int getWidth() const override;
    int getHeight() const override;
    std::string getName() const override { return _name; }
    void SetName(const std::string& name) { _name = name; }

    int GetVirtualCanvasWidth() const override;
    int GetVirtualCanvasHeight() const override;
    void GetVirtualCanvasSize(int& w, int& h) const override;

    // Mirrors desktop ModelPreview::GetCameraZoomForHandles
    // (ModelPreview.cpp:1304): 1.0 in 2D so the handle width is a
    // fixed world-unit size and the View matrix's scale handles
    // visual sizing. Returning the actual 2D zoom here would scale
    // the handle math AND the View matrix, making handles
    // quadratically tiny / huge as the user zooms.
    float GetCameraZoomForHandles() const override {
        return _is3d ? _camera3d.GetZoom() : 1.0f;
    }
    int GetHandleScale() const override { return 1; }
    float GetCameraRotationX() const override { return ActiveCamera().GetAngleX(); }
    float GetCameraRotationY() const override { return ActiveCamera().GetAngleY(); }
    glm::mat4& GetProjViewMatrix() override { return _projViewMatrix; }
    glm::mat4& GetProjMatrix() override { return _projMatrix; }
    glm::mat4& GetViewMatrix() override { return _viewMatrix; }

    xlGraphicsContext* getCurrentGraphicsContext() override;
    xlGraphicsProgram* getCurrentSolidProgram() override { return _solidProgram; }
    xlGraphicsProgram* getCurrentTransparentProgram() override { return _transparentProgram; }
    xlGraphicsProgram* getCurrentSolidViewObjectProgram() { return _solidViewObjectProgram; }
    xlGraphicsProgram* getCurrentTransparentViewObjectProgram() { return _transparentViewObjectProgram; }

    bool StartDrawing(double pointSize, bool fromPaint = false) override;
    void EndDrawing(bool swapBuffers = true) override;

    /// Reason the most-recent `StartDrawing` returned false. Empty
    /// when nothing has gone wrong yet (or the last call succeeded).
    /// XLMetalBridge surfaces this to SwiftUI when it can't draw.
    const std::string& GetLastStartDrawingFailure() const {
        return _lastStartDrawingFailure;
    }
    /// Public access to the cached preview name — used in log
    /// prefixes ("XLMetalBridge[ModelPreview]: …").
    const std::string& GetName() const { return _name; }

    double calcPixelSize(double i) override { return i; }

    // Camera access — callers drive zoom/pan/rotate via the active camera's
    // PreviewCamera setters (see ViewpointMgr.h).
    PreviewCamera& Get2DCamera() { return _camera2d; }
    PreviewCamera& Get3DCamera() { return _camera3d; }
    const PreviewCamera& ActiveCamera() const { return _is3d ? _camera3d : _camera2d; }
    PreviewCamera& ActiveCamera() { return _is3d ? _camera3d : _camera2d; }

    bool Is3D() const override { return _is3d; }
    void SetIs3D(bool v) { _is3d = v; }

    void ResetCamera() { ActiveCamera().Reset(); }

    bool IsNoCurrentModel() override { return _currentModel.empty(); }
    const std::string& GetCurrentModel() const { return _currentModel; }
    void SetCurrentModel(const std::string& name) { _currentModel = name; }

    uint32_t getCurrentFrameTime() const override { return _currentFrameTime; }
    void SetCurrentFrameTime(uint32_t ms) { _currentFrameTime = ms; }

    // Set virtual canvas size (model coordinate space)
    void SetVirtualCanvasSize(int w, int h) { _virtualW = w; _virtualH = h; }

    // Mirror desktop's `Display2DCenter0` flag: when true, world X=0 is
    // placed at the horizontal centre of the preview in 2D mode. Shows
    // laid out around a centred origin (e.g. models at X = -600..+600)
    // need this to be on, or they render off-screen.
    void SetCenter2D0(bool v) { _center2D0 = v; }

private:
    std::string _name = "iPadPreview";
    xlStandaloneMetalCanvas* _canvas;
    xlMetalGraphicsContext* _ctx = nullptr;
    xlGraphicsProgram* _solidProgram = nullptr;
    xlGraphicsProgram* _transparentProgram = nullptr;
    xlGraphicsProgram* _solidViewObjectProgram = nullptr;
    xlGraphicsProgram* _transparentViewObjectProgram = nullptr;
    glm::mat4 _projViewMatrix{1.0f};
    glm::mat4 _projMatrix{1.0f};
    glm::mat4 _viewMatrix{1.0f};
    uint32_t _currentFrameTime = 0;
    // 0 = no virtual canvas scaling (Model Preview single-model fit-to-window
    // mode). Set explicitly (via SetVirtualCanvasSize) for panes that render
    // models at their world positions in a specific virtual space.
    int _virtualW = 0;
    int _virtualH = 0;
    bool _isDrawing = false;
    bool _is3d = true;
    bool _center2D0 = false;
    PreviewCamera _camera2d{false};
    PreviewCamera _camera3d{true};
    std::string _currentModel;  // empty = "render everything" (House Preview mode)
    std::string _lastStartDrawingFailure;
};
