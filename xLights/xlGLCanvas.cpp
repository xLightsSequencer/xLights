#include <wx/file.h>
#include "xlGLCanvas.h"

BEGIN_EVENT_TABLE(xlGLCanvas, wxGLCanvas)
EVT_SIZE(xlGLCanvas::Resized)
EVT_ERASE_BACKGROUND(xlGLCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#include "osxMacUtils.h"

#include <wx/log.h>
#include <wx/config.h>
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
    if (m_context) {
        m_context->SetCurrent(*this);
        if (cache != nullptr) {
            DrawGLUtils::DestroyCache(cache);
        }
        delete m_context;
    }
}

DrawGLUtils::xlGLCacheInfo *Create33Cache(bool, bool, bool, bool);
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
    wxConfigBase* config = wxConfigBase::Get();
    int ver = 99;
    config->Read("ForceOpenGLVer", &ver, 99);

    if (cache == nullptr) {
        log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        const GLubyte* str = glGetString(GL_VERSION);
        const GLubyte* rend = glGetString(GL_RENDERER);
        const GLubyte* vend = glGetString(GL_VENDOR);
        wxString config = wxString::Format("%s - glVer:  %s  (%s)(%s)",
            (const char *)GetName().c_str(),
            (const char *)str,
            (const char *)rend,
            (const char *)vend);
        logger_opengl.info(std::string(config.c_str()));
        printf("%s\n", (const char *)config.c_str());
        if (ver >= 3 && (str[0] > '3' || (str[0] == '3' && str[2] >= '3'))) {
            logger_opengl.info("Try creating 33 Cache");
            cache = Create33Cache(UsesVertexTextureAccumulator(),
                                  UsesVertexColorAccumulator(),
                                  UsesVertexAccumulator(),
                                  UsesAddVertex());
        }
        if (cache == nullptr && ver >=2 && str[0] >= '2') {
            logger_opengl.info("Try creating 21 Cache");
            cache = Create21Cache();
        }
        if (cache == nullptr) {
            logger_opengl.info("Try creating 15 Cache");
            cache = Create15Cache();
        }
        if (cache == nullptr) {
            logger_opengl.error("All attempts at cache creation have failed.");
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

        wxConfigBase* config = wxConfigBase::Get();
        int ver = 99;
        config->Read("ForceOpenGLVer", &ver, 99);


        if (m_coreProfile && ver >= 3) {
            wxGLContextAttrs atts;
            atts.PlatformDefaults().OGLVersion(3, 3).CoreProfile().EndList();
            //atts.PlatformDefaults().OGLVersion(2, 1).EndList();
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

        if (m_context == nullptr)
        {
            log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
            logger_opengl.error("Error creating GL context.");
        }
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


