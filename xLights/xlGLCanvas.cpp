#include <wx/file.h>
#include "xlGLCanvas.h"

BEGIN_EVENT_TABLE(xlGLCanvas, wxGLCanvas)
EVT_SIZE(xlGLCanvas::Resized)
EVT_ERASE_BACKGROUND(xlGLCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#include "osxMacUtils.h"

#include <wx/log.h>
#include <log4cpp/Category.hh>

static wxGLAttributes GetAttributes() {
    wxGLAttributes atts;
    atts.PlatformDefaults().RGBA().MinRGBA(8, 8, 8, 8).DoubleBuffer().Depth(16).EndList();
    if (!wxGLCanvas::IsDisplaySupported(atts)) {
        atts.Reset();
        atts.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();
    }
    return atts;
}


#ifndef __WXMAC__
static bool functionsLoaded = false;
extern void LoadGLFunctions();

#ifdef _MSC_VER
    #include "GL\glext.h"
#else
    #include <GL/glext.h>
#endif
extern PFNGLUSEPROGRAMPROC glUseProgram;
#endif

xlGLCanvas::xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos,
                       const wxSize &size, long style, const wxString &name,
                       bool coreProfile)
    :   wxGLCanvas(parent, GetAttributes(), id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS | style, name),
        mWindowWidth(0),
        mWindowHeight(0),
        mWindowResized(false),
        mIsInitialized(false),
        m_context(nullptr),
        m_coreProfile(coreProfile),
        cache(nullptr)
{
    xlSetOpenGLRetina(*this);
    //CreateGLContext();
    this->GetGLCTXAttrs().PlatformDefaults();
}

xlGLCanvas::~xlGLCanvas()
{
    if (cache != nullptr) {
        DrawGLUtils::DestroyCache(cache);
    }
    delete m_context;
}

DrawGLUtils::xlGLCacheInfo *Create33Cache();
DrawGLUtils::xlGLCacheInfo *Create21Cache();
DrawGLUtils::xlGLCacheInfo *Create15Cache();

void xlGLCanvas::SetCurrentGLContext() {
    if (m_context == nullptr) {
        CreateGLContext();
    }
    m_context->SetCurrent(*this);
#ifndef __WXMAC__
    if (!functionsLoaded) {
        LoadGLFunctions();
        functionsLoaded = true;
    }
#endif
    if (cache == nullptr) {
        log4cpp::Category::getRoot().info(wxString::Format("%s - glVer:  %s  (%s)(%s)\n", (const char *)GetName().c_str(),
                                          glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR)).c_str());

        printf("%s - glVer:  %s  (%s)(%s)\n", (const char *)GetName().c_str(),
               glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));
        const GLubyte* str = glGetString(GL_VERSION);
        const GLubyte* vend = glGetString(GL_RENDERER);
        if (str[0] > '3' || (str[0] == '3' && str[2] >= '3')) {
            cache = Create33Cache();
        }
        if (cache == nullptr && str[0] >= '2') {
            if (!wxString(vend).Contains("AMD")) {
                cache = Create21Cache();
            }
        }
        if (cache == nullptr) {
            cache = Create15Cache();
        }
    }
    DrawGLUtils::SetCurrentCache(cache);
}
void xlGLCanvas::CreateGLContext() {
    if (m_context == nullptr) {
        //trying to detect OGL verions and stuff can result in unwanted logs
        wxLogLevel cur = wxLog::GetLogLevel();
        wxLog::SetLogLevel(wxLOG_Error);
        wxLog::Suspend();

        if (m_coreProfile) {

            wxGLContextAttrs atts;
            atts.PlatformDefaults().CoreProfile().OGLVersion(3, 3).EndList();
            m_context = new wxGLContext(this, nullptr, &atts);
            if (!m_context->IsOK()) {
                delete m_context;
                m_context = nullptr;
            }
        }
        if (m_context == nullptr) {
            m_context = new wxGLContext(this);
        }
#ifndef __WXMAC__
        if (!functionsLoaded) {
            LoadGLFunctions();
            functionsLoaded = glUseProgram != nullptr;
        }
#endif
        if (!m_context->IsOK()) {
            delete m_context;
            m_context = nullptr;
        }
        wxLog::SetLogLevel(cur);
        wxLog::Resume();
    }
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


