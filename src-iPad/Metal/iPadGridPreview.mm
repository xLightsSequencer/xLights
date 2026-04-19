/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "iPadGridPreview.h"
#include "graphics/metal/xlMetalGraphicsContext.h"

iPadGridPreview::iPadGridPreview(const std::string& name)
    : _name(name),
      // `useLogicalSize=true` so the grid canvas's coordinate system
      // matches Core Graphics' point-space convention: Swift draws at
      // logical-point values and `SetViewport` does the
      // logical-to-backing scale for us when building the Metal
      // viewport.
      _canvas(name, /* is3d */ false, /* useLogicalSize */ true) {
}

iPadGridPreview::~iPadGridPreview() {
    if (_ctx) {
        // EndFrame was never called — drop the drawable without
        // presenting rather than leaking the Metal resources.
        delete _ctx;
        _ctx = nullptr;
    }
}

void iPadGridPreview::SetMetalLayer(void* layer) {
    _canvas.setMetalLayer((__bridge CAMetalLayer*)layer);
}

void iPadGridPreview::SetDrawableSize(int w, int h, double scale) {
    // `w`/`h` arrive as the MTKView's drawable size (pixels). For the
    // logical-size canvas we want `getWidth/getHeight` to return
    // points — `xlMetalGraphicsContext::SetViewport` multiplies back
    // to pixels via `translateToBacking` when setting the Metal
    // viewport.
    double s = scale <= 0 ? 1.0 : scale;
    _canvas.setSize((int)(w / s), (int)(h / s));
    _canvas.setScaleFactor(s);
}

xlGraphicsContext* iPadGridPreview::BeginFrame() {
    if (_isDrawing || !_canvas.getMetalLayer()) return nullptr;
    int w = _canvas.getWidth();
    int h = _canvas.getHeight();
    if (w == 0 || h == 0) return nullptr;

    auto* mctx = new xlMetalGraphicsContext(&_canvas, nil, /* enqueImmediate */ false);
    if (!mctx->isValid()) {
        delete mctx;
        return nullptr;
    }
    _ctx = mctx;

    // Y-down ortho in logical-point coordinates: (0,0) is top-left,
    // (w,h) is bottom-right. Passing (topleft_x=0, topleft_y=0,
    // bottomright_x=w, bottomright_y=h) yields
    // `matrix_ortho_left_hand(0, w, h, 0, 1, 0)` which flips Y so
    // positive Y points down (matching CG). With the canvas in
    // logical-size mode (see constructor), the projection is in
    // points and the Metal viewport is scaled up via
    // `translateToBacking` for the backing pixels.
    _ctx->SetViewport(0, 0, w, h, /* is3D */ false);
    _ctx->enableBlending(true);
    _isDrawing = true;
    return _ctx;
}

void iPadGridPreview::EndFrame(bool present) {
    if (!_isDrawing || !_ctx) return;
    // Downcast to Metal to call Commit — this is always valid because
    // BeginFrame only stores xlMetalGraphicsContext instances in _ctx.
    auto* mctx = static_cast<xlMetalGraphicsContext*>(_ctx);
    mctx->Commit(present, nil);
    delete mctx;
    _ctx = nullptr;
    _isDrawing = false;
}
