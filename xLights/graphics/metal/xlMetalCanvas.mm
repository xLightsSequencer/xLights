//
//  xlMetalCanvas.cpp
//  xLights-macOSLib
//
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

#include "xlMetalCanvas.h"
#include "xlMetalGraphicsContext.h"

#include "ExternalHooks.h"

BEGIN_EVENT_TABLE(xlMetalCanvas, wxMetalCanvas)
    EVT_SIZE(xlMetalCanvas::Resized)
    EVT_ERASE_BACKGROUND(xlMetalCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()


xlMetalCanvas::xlMetalCanvas(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name,
                             bool only2d)
    : wxMetalCanvas(parent, id, pos, size, style, name),
mWindowWidth(0),
mWindowHeight(0),
mWindowResized(false),
mIsInitialized(false),
mName(name),
is3d(!only2d)
{
}


xlMetalCanvas::~xlMetalCanvas() {
}

double xlMetalCanvas::translateToBacking(double x) const {
    return xlTranslateToRetina(*this, x);
}
double xlMetalCanvas::mapLogicalToAbsolute(double x) const {
    if (drawingUsingLogicalSize()) {
        return x;
    }
    return translateToBacking(x);
}


void xlMetalCanvas::Resized(wxSizeEvent& evt)
{
    mWindowWidth = evt.GetSize().GetWidth() * GetContentScaleFactor();
    mWindowHeight = evt.GetSize().GetHeight() * GetContentScaleFactor();
    mWindowResized = true;
    Refresh();
}


void xlMetalCanvas::PrepareCanvas() {
    if (!mIsInitialized) {
        //just make sure this will load
        getPipelineState("singleColorProgram", "singleColorVertexShader", "singleColorFragmentShader");
        mIsInitialized = true;
    }
}
xlGraphicsContext * xlMetalCanvas::PrepareContextForDrawing() {
    xlMetalGraphicsContext *ret = new xlMetalGraphicsContext(this);
    if (!ret->hasDrawable()) {
        delete ret;
        return nullptr;
    }
    return ret;
}
void xlMetalCanvas::FinishDrawing(xlGraphicsContext *ctx) {
    delete ctx;
}
