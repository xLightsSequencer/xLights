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

static bool functionsLoaded = false;

#ifndef __WXMAC__
#ifdef _MSC_VER
    #include "GL\glext.h"
#else
    #include <GL/glext.h>
#endif
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

#ifndef __WXOSX__

static const char * getStringForSource(GLenum source) {

    switch(source) {
        case GL_DEBUG_SOURCE_API_ARB:
            return("API");
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            return("Window System");
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            return("Shader Compiler");
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            return("Third Party");
        case GL_DEBUG_SOURCE_APPLICATION_ARB:
            return("Application");
        case GL_DEBUG_SOURCE_OTHER_ARB:
            return("Other");
        default:
            return("");
    }
}

// aux function to translate severity to string
static const char *getStringForSeverity(GLenum severity) {

    switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH_ARB:
            return("High");
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            return("Medium");
        case GL_DEBUG_SEVERITY_LOW_ARB:
            return("Low");
        default:
            return("");
    }
}

// aux function to translate type to string
static const char * getStringForType(GLenum type) {

    switch(type) {
        case GL_DEBUG_TYPE_ERROR_ARB:
            return("Error");
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            return("Deprecated Behaviour");
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            return("Undefined Behaviour");
        case GL_DEBUG_TYPE_PORTABILITY_ARB:
            return("Portability Issue");
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            return("Performance Issue");
        case GL_DEBUG_TYPE_OTHER_ARB:
            return("Other");
        default:
            return("");
    }
}

void CALLBACK DebugLog(GLenum source , GLenum type , GLuint id , GLenum severity ,
                       GLsizei length , const GLchar * message , const GLvoid * userParam)
{
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));

    logger_opengl.info("Type : %s; Source : %s; ID : %d; Severity : % s\n Message: %s",
                        getStringForType( type ),
                        getStringForSource( source ),
                        id,
                        getStringForSeverity( severity ),
                        message);
}
void CALLBACK DebugLogAMD(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,void *userParam) {
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    
    logger_opengl.info("%s; ID : %d; Severity : % s\n Message: %s",
                        getStringForType( category ),
                        id,
                        getStringForSeverity( severity ),
                        message);
}


void AddDebugLog(xlGLCanvas *c) {
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
    PFNGLDEBUGMESSAGECONTROLARBPROC glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)wglGetProcAddress("glDebugMessageControlARB");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    if (glDebugMessageCallbackARB != nullptr) {
        logger_opengl.info("Did not find debug callback ARB, attempting 4.3");
        glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallback");
        glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)wglGetProcAddress("glDebugMessageControl");
    }
    if (glDebugMessageCallbackARB != nullptr) {
        logger_opengl.info("Adding debug callback.  %X", glDebugMessageControlARB);
        LOG_GL_ERRORV(glDebugMessageCallbackARB(DebugLog, c));
        if (glDebugMessageControlARB != nullptr) {
            GLuint unusedIds = 0;
            glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
        }
    } else {
        PFNGLDEBUGMESSAGECALLBACKAMDPROC glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)wglGetProcAddress("glDebugMessageCallbackAMD");
        if (glDebugMessageCallbackARB != nullptr) {
            logger_opengl.info("Adding AMD debug callback");
            LOG_GL_ERRORV(glDebugMessageCallbackAMD(DebugLogAMD, c));
        } else {
            logger_opengl.info("Cannot add debug callback");
        }
    }
}
#else 
void AddDebugLog(xlGLCanvas *c) {
}
#endif


DrawGLUtils::xlGLCacheInfo *Create33Cache(bool, bool, bool, bool);
DrawGLUtils::xlGLCacheInfo *Create21Cache();
DrawGLUtils::xlGLCacheInfo *Create15Cache();

void xlGLCanvas::SetCurrentGLContext() {
    if (m_context == nullptr) {
        CreateGLContext();
    }
    m_context->SetCurrent(*this);
    if (!functionsLoaded) {
        DrawGLUtils::LoadGLFunctions();
        functionsLoaded = true;
    }
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
        if (logger_opengl.isDebugEnabled()) {
            AddDebugLog(this);
        }
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
            log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
            if (logger_opengl.isDebugEnabled()) {
                atts.PlatformDefaults().OGLVersion(3, 3).CoreProfile().ForwardCompatible().DebugCtx().EndList();
            } else {
                atts.PlatformDefaults().OGLVersion(3, 3).CoreProfile().EndList();
            }
            m_context = new wxGLContext(this, nullptr, &atts);
            if (!m_context->IsOK()) {
                delete m_context;
                m_context = nullptr;
            }
        }
        if (m_context == nullptr) {
            m_context = new wxGLContext(this);
        }
        if (!functionsLoaded) {
            functionsLoaded = DrawGLUtils::LoadGLFunctions();
        }
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


