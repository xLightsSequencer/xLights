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

#include <cstdint>
#include <string>

#include <glm/mat4x4.hpp>

// Forward declarations — no wx headers required
class xlGraphicsContext;
class xlGraphicsProgram;

// Abstract interface for a model preview canvas.
// Implemented by ModelPreview (desktop wx) and any non-wx preview (CLI, iPad).
// Allows models/, render/, and effects/ to call back into the preview without
// depending on wxWidgets or the concrete ModelPreview class.
class IModelPreview {
public:
    virtual ~IModelPreview() = default;

    // --- Dimensions ---
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    // Identity — used as a cache key for per-preview textures.
    // Returns std::string (not wxString) so this header stays wx-free.
    virtual std::string getName() const = 0;

    // --- Virtual canvas ---
    virtual int GetVirtualCanvasWidth() const = 0;
    virtual int GetVirtualCanvasHeight() const = 0;
    virtual void GetVirtualCanvasSize(int& w, int& h) const = 0;

    // --- Camera / projection ---
    virtual float GetCameraZoomForHandles() const = 0;
    virtual int GetHandleScale() const = 0;
    virtual float GetCameraRotationX() const = 0;
    virtual float GetCameraRotationY() const = 0;
    virtual glm::mat4& GetProjViewMatrix() = 0;
    virtual glm::mat4& GetProjMatrix() = 0;

    // --- Graphics context and programs ---
    virtual xlGraphicsContext* getCurrentGraphicsContext() = 0;
    virtual xlGraphicsProgram* getCurrentSolidProgram() = 0;
    virtual xlGraphicsProgram* getCurrentTransparentProgram() = 0;

    // --- Drawing lifecycle ---
    // pointSize uses double (wxDouble is just typedef double)
    virtual bool StartDrawing(double pointSize, bool fromPaint = false) = 0;
    virtual void EndDrawing(bool swapBuffers = true) = 0;

    // --- Utilities ---
    virtual double calcPixelSize(double i) = 0;
    // Returns the 2D viewport scale factor (zoom × world-to-screen) for sizing GL_POINTS correctly
    // in the 2D layout view. Returns 1.0 for 3D views (perspective projection makes per-node
    // depth compensation impractical) and for non-layout previews.
    virtual double getViewScale() const { return 1.0; }
    // Returns the HiDPI backing scale factor only (translateToBacking(1.0)), without any zoom or
    // world-to-screen scale. Used to size circle triangles (Solid/Blended Circle pixel styles) so
    // that the view-matrix scale is not applied twice.
    virtual double getBackingScaleFactor() const { return 1.0; }
    virtual uint32_t getCurrentFrameTime() const = 0;

    // --- State queries ---
    virtual bool Is3D() const = 0;
    virtual bool IsNoCurrentModel() = 0;
};
