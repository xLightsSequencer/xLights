#include "wx/wx.h"
#include "wx/sizer.h"
#ifdef __WXMAC__
 #include "OpenGL/glu.h"
 #include "OpenGL/gl.h"
#else
 #include <GL/glu.h>
 #include <GL/gl.h>
#endif

#include "ModelPreview.h"

BEGIN_EVENT_TABLE(ModelPreview, xlGLCanvas)
EVT_MOTION(ModelPreview::mouseMoved)
EVT_LEFT_DOWN(ModelPreview::mouseLeftDown)
EVT_LEFT_UP(ModelPreview::mouseLeftUp)
EVT_LEAVE_WINDOW(ModelPreview::mouseLeftWindow)
EVT_RIGHT_DOWN(ModelPreview::rightClick)
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

void ModelPreview::mouseWheelMoved(wxMouseEvent& event) {}
//void ModelPreview::rightClick(wxMouseEvent& event) {}
void ModelPreview::keyPressed(wxKeyEvent& event) {}
void ModelPreview::keyReleased(wxKeyEvent& event) {}

ModelPreview::ModelPreview(wxPanel* parent) :
    xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

ModelPreview::~ModelPreview()
{
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
}

void ModelPreview::InitializeGLCanvas()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // Rotate Axis and tranlate
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(180,0,0,1);
    glRotatef(180,0,1,0);
    glTranslatef(0,-getHeight(),0);
    mIsInitialized = true;
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

bool ModelPreview::StartDrawing(wxDouble pointSize)
{
    if( !IsShownOnScreen() ) return false;
    if(!mIsInitialized) { InitializeGLCanvas(); }
    mIsInitialized = true;
    mPointSize = pointSize;
    mIsDrawing = true;
    SetCurrentGLContext();
    wxClientDC dc(this);
    glPointSize( mPointSize );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }
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
    return true;
}

void ModelPreview::EndDrawing()
{
    SwapBuffers();
    mIsDrawing = false;
}

