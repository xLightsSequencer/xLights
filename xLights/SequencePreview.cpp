#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/glcanvas.h"
#ifdef __WXMAC__
 #include "OpenGL/glu.h"
 #include "OpenGL/gl.h"
#else
 #include <GL/glu.h>
 #include <GL/gl.h>
#endif

#include "SequencePreview.h"
#include "RgbEffects.h"

BEGIN_EVENT_TABLE(SequencePreview, xlGLCanvas)
//EVT_MOTION(SequencePreview::mouseMoved)
//EVT_LEFT_DOWN(SequencePreview::mouseDown)
//EVT_LEFT_UP(SequencePreview::mouseReleased)
//EVT_RIGHT_DOWN(SequencePreview::rightClick)
//EVT_LEAVE_WINDOW(SequencePreview::mouseLeftWindow)
//EVT_KEY_DOWN(SequencePreview::keyPressed)
//EVT_KEY_UP(SequencePreview::keyReleased)
//EVT_MOUSEWHEEL(SequencePreview::mouseWheelMoved)
EVT_PAINT(SequencePreview::render)
END_EVENT_TABLE()
// some useful events to use
void SequencePreview::mouseMoved(wxMouseEvent& event) {}
void SequencePreview::mouseDown(wxMouseEvent& event) {}
void SequencePreview::mouseWheelMoved(wxMouseEvent& event) {}
void SequencePreview::mouseReleased(wxMouseEvent& event) {}
void SequencePreview::rightClick(wxMouseEvent& event) {}
void SequencePreview::mouseLeftWindow(wxMouseEvent& event) {}
void SequencePreview::keyPressed(wxKeyEvent& event) {}
void SequencePreview::keyReleased(wxKeyEvent& event) {}


SequencePreview::SequencePreview(wxPanel* parent) :
    xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

SequencePreview::~SequencePreview()
{
}

void SequencePreview::InitializeGLCanvas()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    glLoadIdentity();
    mIsInitialized = true;
}

bool SequencePreview::StartDrawing(wxDouble pointSize)
{
    if(mIsDrawing){return false;}
    mIsDrawing = true;
    if(!mIsInitialized) { InitializeGLCanvas(); }
    SetCurrentGLContext();
    wxClientDC dc(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }
    glPointSize( pointSize );
    glBegin(GL_POINTS);
    return(true);
}

void SequencePreview::EndDrawing()
{
    glEnd();
    glFlush();
    SwapBuffers();
    mIsDrawing = false;
}

void SequencePreview::render( wxPaintEvent& evt )
{
    if(mIsDrawing) return;
    if(!mIsInitialized) { InitializeGLCanvas(); }
    SetCurrentGLContext();
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFlush();
    SwapBuffers();
}
