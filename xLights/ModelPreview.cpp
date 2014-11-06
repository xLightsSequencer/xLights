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

#include "ModelPreview.h"

BEGIN_EVENT_TABLE(ModelPreview, wxGLCanvas)
EVT_MOTION(ModelPreview::mouseMoved)
EVT_LEFT_DOWN(ModelPreview::mouseLeftDown)
EVT_LEFT_UP(ModelPreview::mouseLeftUp)
EVT_LEAVE_WINDOW(ModelPreview::mouseLeftWindow)
EVT_RIGHT_DOWN(ModelPreview::rightClick)
//EVT_SIZE(ModelPreview::resized)
//EVT_KEY_DOWN(ModelPreview::keyPressed)
//EVT_KEY_UP(ModelPreview::keyReleased)
//EVT_MOUSEWHEEL(ModelPreview::mouseWheelMoved)
EVT_PAINT(ModelPreview::render)
END_EVENT_TABLE()

void ModelPreview::mouseMoved(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftDown(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::rightClick(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftUp(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftWindow(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::render( wxPaintEvent& event )
{
    if (mIsInitialized)
    {
        event.ResumePropagation(1);
        event.Skip (); // continue the event
    }
}

void ModelPreview::resized(wxSizeEvent& event)
{
    if (mIsInitialized)
    {
        event.ResumePropagation(1);
        event.Skip (); // continue the event
    }
}


void ModelPreview::mouseWheelMoved(wxMouseEvent& event) {}
//void ModelPreview::rightClick(wxMouseEvent& event) {}
void ModelPreview::keyPressed(wxKeyEvent& event) {}
void ModelPreview::keyReleased(wxKeyEvent& event) {}

ModelPreview::ModelPreview(wxPanel* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
	m_context = new wxGLContext(this);
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

ModelPreview::~ModelPreview()
{
	delete m_context;
}

void ModelPreview::ClearBackground()
{
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SwapBuffers();
    return;
}

int ModelPreview::getWidth()
{
    return GetSize().x;
}

int ModelPreview::getHeight()
{
    return GetSize().y;
}

void ModelPreview::SetCanvasSize(int width,int height)
{
    SetSize(width,height);
    wxSize s;
    s.SetWidth(width);
    s.SetHeight(height);
    SetMaxSize(s);
    SetMinSize(s);
}

void ModelPreview::InitializePreview(wxString img,int brightness)
{
    image = NULL;
    mBackgroundImage = img;
    mBackgroundImageExists = wxFileExists(mBackgroundImage)?true:false;
    mBackgroundBrightness = brightness;
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
}

/** Inits the OpenGL viewport for drawing in 2D. */
void ModelPreview::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // Rotate Axis and tranlate
    glRotatef(180,0,0,1);
    glRotatef(180,0,1,0);
    glTranslatef(0,-getHeight(),0);
    // Set view port
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
}


void ModelPreview::SetOrigin()
{
}

void ModelPreview::SetbackgroundImage(wxString img)
{
    image = NULL;
    mBackgroundImage = img;
    mBackgroundImageExists = wxFileExists(mBackgroundImage)?true:false;
}

void ModelPreview::SetBackgroundBrightness(int brightness)
{
   mBackgroundBrightness = brightness;
   if(mBackgroundBrightness < 0 || mBackgroundBrightness > 100)
   {
        mBackgroundBrightness = 100;
   }
}

void ModelPreview::SetPointSize(wxDouble pointSize)
{
    mPointSize = pointSize;
    glPointSize( mPointSize );
}

void ModelPreview::StartDrawing(wxDouble pointSize)
{
    mIsInitialized = true;
    mPointSize = pointSize;
    mIsDrawing = true;
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this);
    glPointSize( mPointSize );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    if(mBackgroundImageExists)
    {
        if (image == NULL)
        {
           image = new Image(mBackgroundImage);
           sprite = new xLightsDrawable(image);
        }
        float intensity = mBackgroundBrightness*.01;
        glColor3f(intensity, intensity, intensity);
        glEnable(GL_TEXTURE_2D);   // textures
        sprite->render();
    }
    glDisable(GL_TEXTURE_2D);   // textures
}

void ModelPreview::DrawPoint(const wxColour &color, wxDouble x, wxDouble y)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    glBegin(GL_POINTS);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

void ModelPreview::DrawRectangle(const wxColour &color, wxDouble x, wxDouble y,int width, int height)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x+width, y);
    glVertex2f(x+width, y+height);
    glVertex2f(x, y+height);
    glEnd();
}

void ModelPreview::DrawLine(const wxColour &color, wxDouble x1, wxDouble y1,wxDouble x2, wxDouble y2)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();

}



void ModelPreview::EndDrawing()
{
    SwapBuffers();
    mIsDrawing = false;
}

