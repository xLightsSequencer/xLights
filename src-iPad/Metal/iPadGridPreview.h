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

// iPadGridPreview — 2D Metal rendering surface for the effect-grid
// canvases (model effects, timing effects, top chrome). Unlike
// `iPadModelPreview` which inherits from `IModelPreview` and manages
// 3D camera matrices, this class just owns the Metal canvas +
// xlGraphicsContext for the frame and exposes an ortho projection in
// pixel coordinates. Each frame: BeginFrame → caller issues draws via
// `ctx()` → EndFrame commits. No solid/transparent program queuing;
// the grid is simple enough that direct draws are fine.

#include "xlStandaloneMetalCanvas.h"

#include <memory>
#include <string>

class xlMetalGraphicsContext;
class xlGraphicsContext;

class iPadGridPreview {
public:
    explicit iPadGridPreview(const std::string& name);
    ~iPadGridPreview();

    // Swift → CAMetalLayer lifecycle. `setMetalLayer` mirrors the
    // preview bridge; size+scale are set on drawable-size changes.
    void SetMetalLayer(void* layer);  // __strong CAMetalLayer*
    void SetDrawableSize(int w, int h, double scale);
    int  GetWidth() const { return _canvas.getWidth(); }
    int  GetHeight() const { return _canvas.getHeight(); }

    // Frame lifecycle. `BeginFrame` constructs the xlMetalGraphicsContext
    // (which acquires a drawable), sets a 2D ortho viewport in pixel
    // coordinates, and returns the context for the caller to draw into.
    // `EndFrame` commits + presents and releases the context. The ctx
    // is only valid between Begin and End.
    xlGraphicsContext* BeginFrame();
    void EndFrame(bool present = true);

    // Active context — nullptr outside a BeginFrame/EndFrame pair.
    xlGraphicsContext* ctx() { return _ctx; }

private:
    std::string _name;
    xlStandaloneMetalCanvas _canvas;
    // Held as the base type so forward-declarations in this header
    // work without dragging the full Metal context definition in.
    // The .mm downcasts where it needs Metal-specific methods
    // (`Commit`, `isValid`).
    xlGraphicsContext* _ctx = nullptr;
    bool _isDrawing = false;
};
