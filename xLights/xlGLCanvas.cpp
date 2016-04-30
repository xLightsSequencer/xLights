#include <wx/file.h>
#include "xlGLCanvas.h"

BEGIN_EVENT_TABLE(xlGLCanvas, wxGLCanvas)
EVT_SIZE(xlGLCanvas::Resized)
EVT_ERASE_BACKGROUND(xlGLCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#include "osxMacUtils.h"

static wxGLAttributes GetAttributes() {
    wxGLAttributes atts;
    atts.PlatformDefaults().MinRGBA(8, 8, 8, 8).DoubleBuffer().Depth(16).EndList();
    return atts;
}

xlGLCanvas::xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos,
                       const wxSize &size, long style, const wxString &name,
                       bool coreProfile)
    :   wxGLCanvas(parent, GetAttributes(), id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS | style),
        mWindowWidth(0),
        mWindowHeight(0),
        mWindowResized(false),
        mIsInitialized(false),
        cache(nullptr),
        m_context(nullptr)
{
#ifdef __WXOSX__
    if (coreProfile) {
        wxGLContextAttrs atts;
        atts.PlatformDefaults().CoreProfile().OGLVersion(3, 3).EndList();
        
        //Set NeedsARB to false cause if we fail to find a 3.3 context, we're ok
        //dropping down to 2.x/1.x and we'll detect that later
        atts.SetNeedsARB(false);
        m_context = new wxGLContext(this, nullptr, &atts);
        if (!m_context->IsOK()) {
            delete m_context;
            m_context = nullptr;
        }
    }
#endif
    if (m_context == nullptr) {
        wxGLContextAttrs atts;
        atts.PlatformDefaults().EndList();
        m_context = new wxGLContext(this, nullptr, &atts);
    }
    xlSetOpenGLRetina(*this);
}

xlGLCanvas::~xlGLCanvas()
{
    if (cache != nullptr) {
        DrawGLUtils::DestroyCache(cache);
    }
    delete m_context;
}

void xlGLCanvas::SetCurrentGLContext() {
    m_context->SetCurrent(*this);
    if (cache == nullptr) {
        cache = DrawGLUtils::CreateCache();
    }
    DrawGLUtils::SetCurrentCache(cache);
}

void xlGLCanvas::Resized(wxSizeEvent& evt)
{
    mWindowWidth = evt.GetSize().GetWidth();
    mWindowHeight = evt.GetSize().GetHeight();
    mWindowResized = true;
}

double xlGLCanvas::translateToBacking(double x) {
    return xlTranslateToRetina(*this, x);
}


// Inits the OpenGL viewport for drawing in 2D.
void xlGLCanvas::prepare2DViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y)
{
    DrawGLUtils::SetViewport(*this, topleft_x, topleft_y, bottomright_x, bottomright_y);
    mWindowResized = false;
}


