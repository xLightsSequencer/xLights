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

// iPadModelPreview — IModelPreview implementation for the iPad house preview.
// Wraps xlStandaloneMetalCanvas and manages the xlMetalGraphicsContext lifecycle
// so that Model::DisplayEffectOnWindow() can draw directly via Metal.

#include "graphics/IModelPreview.h"
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
    std::string getName() const override { return "iPadHousePreview"; }

    int GetVirtualCanvasWidth() const override;
    int GetVirtualCanvasHeight() const override;
    void GetVirtualCanvasSize(int& w, int& h) const override;

    float GetCameraZoomForHandles() const override { return 1.0f; }
    int GetHandleScale() const override { return 1; }
    float GetCameraRotationX() const override { return 0.0f; }
    float GetCameraRotationY() const override { return 0.0f; }
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

    double calcPixelSize(double i) override { return i * 2.0; }
    uint32_t getCurrentFrameTime() const override { return _currentFrameTime; }

    bool Is3D() const override { return true; }
    bool IsNoCurrentModel() override { return true; }

    void SetCurrentFrameTime(uint32_t ms) { _currentFrameTime = ms; }

    // Set virtual canvas size (model coordinate space)
    void SetVirtualCanvasSize(int w, int h) { _virtualW = w; _virtualH = h; }

private:
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
    int _virtualW = 1920;
    int _virtualH = 1080;
    bool _isDrawing = false;
};
