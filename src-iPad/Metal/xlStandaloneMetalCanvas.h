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

// xlStandaloneMetalCanvas — wx-free IMetalCanvas for iPad (and potentially
// any standalone Metal rendering surface).  Wraps a CAMetalLayer provided
// by SwiftUI via MTKView or a plain UIView.

#include "graphics/metal/IMetalCanvas.h"

#include <string>

class xlStandaloneMetalCanvas : public IMetalCanvas {
public:
    // `useLogicalSize` flips the canvas into CG-style logical-point
    // coordinates: `setSize` receives bounds in points (not pixels),
    // `xlMetalGraphicsContext::SetViewport` builds its ortho matrix
    // in logical-point space, and the Metal viewport is scaled up
    // to backing pixels via `translateToBacking`. Model / House
    // previews use the default (pixel coords) because they own
    // their own projection matrices; the grid canvases use logical
    // so Swift-side draw code can push point-space values unchanged.
    xlStandaloneMetalCanvas(const std::string& name, bool is3d = false,
                             bool useLogicalSize = false);
    ~xlStandaloneMetalCanvas() override;

    // IMetalCanvas
    std::string getName() const override { return _name; }
    xlColor ClearBackgroundColor() const override { return xlBLACK; }
    bool usesMSAA() override { return _is3d; }
    bool RequiresDepthBuffer() const override { return _is3d; }
    bool drawingUsingLogicalSize() const override { return _useLogicalSize; }
    double translateToBacking(double x) const override;
    void* getMetalDelegate() override;

    // iPad-specific: attach/detach the CAMetalLayer from SwiftUI
#ifdef __OBJC__
    void setMetalLayer(__strong CAMetalLayer* layer);
    CAMetalLayer* getMetalLayer() const { return _layer; }
#endif

    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    void setSize(int w, int h) { _width = w; _height = h; }
    void setScaleFactor(double s) { _scaleFactor = s; }
    double getScaleFactor() const { return _scaleFactor; }

private:
    std::string _name;
    bool _is3d;
    bool _useLogicalSize = false;
    int _width = 0;
    int _height = 0;
    double _scaleFactor = 2.0;
    void* _delegate = nullptr;  // IMetalCanvasDelegate*, allocated in .mm

#ifdef __OBJC__
    CAMetalLayer* _layer = nil;
#endif
};
