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
#ifdef __LINUX__
    if(!IsShownOnScreen()) return;
#endif
    SetCurrentGLContext();

    LOG_GL_ERRORV(glClearColor(0.0f, 0.0f, 0.0f, 0.0f)); // Black Background
    LOG_GL_ERRORV(glEnable(GL_BLEND));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    mIsInitialized = true;
}

void xlGridCanvasEmpty::render( wxPaintEvent& event )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
#ifdef __LINUX__
    if(!IsShownOnScreen()) return;
#endif


    SetCurrentGLContext();

    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);

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
    DrawGLUtils::xlVertexAccumulator va;
    va.PreAlloc(8);
    va.AddVertex(0,0);
    va.AddVertex(mWindowWidth,mWindowHeight);
    va.AddVertex(0,mWindowHeight);
    va.AddVertex(mWindowWidth,0);
    DrawGLUtils::Draw(va, redLine, GL_LINES, GL_BLEND);
}

