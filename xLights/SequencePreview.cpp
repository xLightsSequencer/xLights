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

BEGIN_EVENT_TABLE(SequencePreview, wxGLCanvas)
//EVT_MOTION(SequencePreview::mouseMoved)
//EVT_LEFT_DOWN(SequencePreview::mouseDown)
//EVT_LEFT_UP(SequencePreview::mouseReleased)
//EVT_RIGHT_DOWN(SequencePreview::rightClick)
//EVT_LEAVE_WINDOW(SequencePreview::mouseLeftWindow)
//EVT_SIZE(SequencePreview::resized)
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
void SequencePreview::rightClick(wxMouseEvent& event) {
    int i=0;
}
void SequencePreview::mouseLeftWindow(wxMouseEvent& event) {}
void SequencePreview::keyPressed(wxKeyEvent& event) {}
void SequencePreview::keyReleased(wxKeyEvent& event) {}

SequencePreview::SequencePreview(wxPanel* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
	m_context = new wxGLContext(this);
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

SequencePreview::~SequencePreview()
{
	delete m_context;
}

void SequencePreview::ClearBackground()
{
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SwapBuffers();
    return;
}

void SequencePreview::resized(wxSizeEvent& evt)
{
}


/** Inits the OpenGL viewport for drawing in 2D. */
void SequencePreview::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int SequencePreview::getWidth()
{
    return GetSize().x;
}

int SequencePreview::getHeight()
{
    return GetSize().y;
}

void SequencePreview::InitializePreview()
{
    mIsInitialized = true;
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
}

void SequencePreview::StartDrawing(wxDouble pointSize)
{
    mIsDrawing = true;
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    glPointSize( pointSize );
    glBegin(GL_POINTS);
}

void SequencePreview::DrawPoint(const wxColour &color, wxDouble x, wxDouble y)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    glVertex2f(x, y);
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
    if(mIsDrawing || !mIsInitialized) return;
    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFlush();
    SwapBuffers();
}
