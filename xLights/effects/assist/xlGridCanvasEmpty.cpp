#include "xlGridCanvasEmpty.h"
#include "../../DrawGLUtils.h"

BEGIN_EVENT_TABLE(xlGridCanvasEmpty, xlGridCanvas)
EVT_PAINT(xlGridCanvasEmpty::render)
END_EVENT_TABLE()

xlGridCanvasEmpty::xlGridCanvasEmpty(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : xlGridCanvas(parent, id, pos, size, style, name)
{
    mRows = 10;
    mColumns = 10;
    mCellSize = 20;
}

xlGridCanvasEmpty::~xlGridCanvasEmpty()
{
}

void xlGridCanvasEmpty::ForceRefresh()
{
}

void xlGridCanvasEmpty::SetEffect(Effect* effect_)
{
    mEffect = effect_;
}

void xlGridCanvasEmpty::InitializeGLCanvas()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Black Background
    glDisable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    mIsInitialized = true;
}

void xlGridCanvasEmpty::render( wxPaintEvent& event )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
    if(!IsShownOnScreen()) return;

    SetCurrentGLContext();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    wxPaintDC(this);
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }

    if( mEffect != nullptr )
    {
        DrawBaseGrid();
        DrawEmptyEffect();
    }

    SwapBuffers();
}

void xlGridCanvasEmpty::DrawEmptyEffect()
{
    xlColor redLine = xlRED;
    glEnable(GL_BLEND);
    DrawGLUtils::DrawLine(redLine,255,0,0,mWindowWidth,mWindowHeight,5);
    DrawGLUtils::DrawLine(redLine,255,mWindowWidth,0,0,mWindowHeight,5);
    glDisable(GL_BLEND);
}

