//
//  xlMetalCanvas.cpp
//  xLights-macOSLib
//
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

#include "xlMetalCanvas.h"
#include "xlMetalGraphicsContext.h"

#include "ExternalHooks.h"

#include "../opengl/xlGLCanvas.h"
#include "wx/osx/private.h"
#include "../xlGraphicsBase.h"

BEGIN_EVENT_TABLE(xlMetalCanvas, wxMetalCanvas)
    EVT_SIZE(xlMetalCanvas::Resized)
    EVT_ERASE_BACKGROUND(xlMetalCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#if !defined(FORCE_OPENGL_BASE)

@interface wxNSCustomOpenGLView : NSOpenGLView
{
}
@end
@implementation wxNSCustomOpenGLView

+ (void)initialize
{
    static BOOL initialized = NO;
    if (!initialized)
    {
        initialized = YES;
        wxOSXCocoaClassAddWXMethods( self );
    }
}

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL) acceptsFirstResponder
{
    return YES;
}

// for special keys

- (void)doCommandBySelector:(SEL)aSelector
{
    wxWidgetCocoaImpl* impl = (wxWidgetCocoaImpl* ) wxWidgetImpl::FindFromWXWidget( self );
    if (impl)
        impl->doCommandBySelector(aSelector, self, _cmd);
}

- (NSOpenGLContext *) openGLContext
{
    // Prevent the NSOpenGLView from making it's own context
    // We want to force using wxGLContexts
    return NULL;
}

@end

#endif

xlMetalCanvas::xlMetalCanvas(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name,
                             bool only2d)
    : wxMetalCanvas(parent, id, pos, size, style, name, only2d),
mWindowWidth(0),
mWindowHeight(0),
mWindowResized(false),
mIsInitialized(false),
mName(name),
fallback(nullptr)
{
#if !defined(FORCE_OPENGL_BASE)
    if (!wxPlatformInfo::Get().CheckOSVersion(10, 14)) {
        //We need a Metal supported graphics card.  Only 10.14 guarentees that so
        //If we aren't on 10.14 or better, we'll fall back to OpenGL
        fallback = new xlGLCanvas(parent, id, pos, size, style, name, only2d);

        NSRect r = wxOSXGetFrameForControl( this, pos , size ) ;
        wxNSCustomOpenGLView* v = [[[wxNSCustomOpenGLView alloc] initWithFrame:r] retain];
        [v setWantsBestResolutionOpenGLSurface:YES];

        wxWidgetCocoaImpl* c = new wxWidgetCocoaImpl( this, v, wxWidgetImpl::Widget_UserKeyEvents | wxWidgetImpl::Widget_UserMouseEvents );
        SetPeer(c);
        MacPostControlCreate(pos, size);
        fallback->SetPeer(new wxWidgetCocoaImpl( this, v, wxWidgetImpl::Widget_UserKeyEvents | wxWidgetImpl::Widget_UserMouseEvents ));
    }
#endif
}


xlMetalCanvas::~xlMetalCanvas() {
    if (fallback) {
        delete fallback;
    }
}
xlColor xlMetalCanvas::ClearBackgroundColor() const {
    return xlBLACK;
}
bool xlMetalCanvas::drawingUsingLogicalSize() const {
    if (fallback) {
        return fallback->drawingUsingLogicalSize();
    }
    return false;
}

double xlMetalCanvas::translateToBacking(double x) const {
    if (fallback) {
        return fallback->translateToBacking(x);
    }
    return xlTranslateToRetina(*this, x);
}
double xlMetalCanvas::mapLogicalToAbsolute(double x) const {
    if (fallback) {
        return fallback->mapLogicalToAbsolute(x);
    }
    if (drawingUsingLogicalSize()) {
        return x;
    }
    return translateToBacking(x);
}


void xlMetalCanvas::Resized(wxSizeEvent& evt)
{
    if (fallback) {
        fallback->Resized(evt);
        mWindowWidth = fallback->getWidth();
        mWindowHeight = fallback->getHeight();
    } else if (drawingUsingLogicalSize()) {
        mWindowWidth = evt.GetSize().GetWidth();
        mWindowHeight = evt.GetSize().GetHeight();
    } else {
        mWindowWidth = evt.GetSize().GetWidth() * GetContentScaleFactor();
        mWindowHeight = evt.GetSize().GetHeight() * GetContentScaleFactor();
    }
    mWindowResized = true;
    Refresh();
}

void xlMetalCanvas::PrepareCanvas() {
    if (!mIsInitialized) {
        if (fallback) {
            fallback->PrepareCanvas();
        } else {
            //just make sure this will load
            getPipelineState("singleColorProgram", "singleColorVertexShader", "singleColorFragmentShader", false);
        }
        mIsInitialized = true;
    }
}
xlGraphicsContext * xlMetalCanvas::PrepareContextForDrawing() {
    if (fallback) {
        return fallback->PrepareContextForDrawing(this->ClearBackgroundColor());
    }
    xlMetalGraphicsContext *ret = new xlMetalGraphicsContext(this);
    if (!ret->hasDrawable()) {
        delete ret;
        return nullptr;
    }
    return ret;
}
void xlMetalCanvas::FinishDrawing(xlGraphicsContext *ctx) {
    if (fallback) {
        return fallback->FinishDrawing(ctx);
    } else {
        delete ctx;
    }
}
