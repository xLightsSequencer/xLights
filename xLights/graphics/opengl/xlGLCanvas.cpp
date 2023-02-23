/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/file.h>
#include <wx/image.h>
#include "xlGLCanvas.h"
#include "UtilFunctions.h"
#include "../../ExternalHooks.h"

BEGIN_EVENT_TABLE(xlGLCanvas, wxGLCanvas)
    EVT_SIZE(xlGLCanvas::Resized)
    EVT_ERASE_BACKGROUND(xlGLCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#include <wx/log.h>
#include <wx/config.h>
#include <wx/msgdlg.h>
#include <log4cpp/Category.hh>
#include "Image.h"
#include "../xlMesh.h"
#include "DrawGLUtils.h"

#ifndef GL_POINT_SMOOTH
#define GL_POINT_SMOOTH                0x0B10
#endif

static const int DEPTH_BUFFER_BITS[] = {32, 24, 16, 12, 10, 8};

wxGLContext *xlGLCanvas::m_sharedContext = nullptr;

static wxGLAttributes GetAttributes(int &zdepth, bool only2d) {
    DrawGLUtils::SetupDebugLogging();
    
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    
    wxGLAttributes atts;
    for (size_t x = only2d ? 5 : 0; x < 6; ++x) {
        atts.Reset();
        atts.PlatformDefaults()
            .RGBA()
            .MinRGBA(8, 8, 8, 8)
            .DoubleBuffer();
        if (!only2d) {
            atts.Depth(DEPTH_BUFFER_BITS[x]);
        }
        atts.EndList();
        if (wxGLCanvas::IsDisplaySupported(atts)) {
            logger_opengl.debug("Depth of %d supported, using it", DEPTH_BUFFER_BITS[x]);
            zdepth = DEPTH_BUFFER_BITS[x];
            return atts;
        }
        logger_opengl.debug("Depth of %d not supported", DEPTH_BUFFER_BITS[x]);
    }
    logger_opengl.debug("Could not find an attribs thats working with MnRGBA\n");
    // didn't find a display, try without MinRGBA
    for (size_t x = only2d ? 5 : 0; x < 6; ++x) {
        atts.Reset();
        atts.PlatformDefaults()
            .RGBA()
            .DoubleBuffer();
        if (!only2d) {
            atts.Depth(DEPTH_BUFFER_BITS[x]);
        }
        atts.EndList();
        if (wxGLCanvas::IsDisplaySupported(atts)) {
            logger_opengl.debug("Depth of %d supported without MinRGBA, using it", DEPTH_BUFFER_BITS[x]);
            zdepth = DEPTH_BUFFER_BITS[x];
            return atts;
        }
        logger_opengl.debug("Depth of %d not supported without MinRGBA", DEPTH_BUFFER_BITS[x]);
    }
    logger_opengl.debug("Could not find an attribs thats working");
    zdepth = 0;
    atts.Reset();
    atts.PlatformDefaults()
        .RGBA()
        .DoubleBuffer()
        .EndList();
    if (wxGLCanvas::IsDisplaySupported(atts)) {
        return atts;
    }
               
    atts.PlatformDefaults()
        .RGBA()
        .MinRGBA(8, 8, 8, 8)
        .DoubleBuffer()
        .EndList();
    if (!wxGLCanvas::IsDisplaySupported(atts)) {
        atts.Reset();
        atts.PlatformDefaults()
            .RGBA()
            .DoubleBuffer()
            .EndList();
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

extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

static bool hasOpenGL3FramebufferObjects()
{
    return glGenFramebuffers != nullptr
        && glBindFramebuffer != nullptr
        && glDeleteFramebuffers != nullptr
        && glGenRenderbuffers != nullptr
        && glDeleteRenderbuffers != nullptr
        && glBindRenderbuffer != nullptr
        && glRenderbufferStorage != nullptr
        && glFramebufferRenderbuffer != nullptr;
}
#else
static bool hasOpenGL3FramebufferObjects()
{
    return true;
}
#endif


static int tempZDepth = 0;
xlGLCanvas::xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name,
    bool only2d)
    : wxGLCanvas(parent, GetAttributes(tempZDepth, only2d), id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS | style, name),
    mWindowWidth(0),
    mWindowHeight(0),
    mWindowResized(false),
    mIsInitialized(false),
    m_context(nullptr),
    cache(nullptr),
    m_coreProfile(true),
    _name(name.ToStdString()),
    m_zDepth(only2d ? 0 : tempZDepth),
    is3d(!only2d)
{
    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("                    Creating GL Canvas for %s", (const char*)name.c_str());

    this->GetGLCTXAttrs().PlatformDefaults();

#ifdef __WXMSW__
    int origPixelFormat = GetPixelFormat(m_hDC);
    PIXELFORMATDESCRIPTOR pfdOrig;
    DescribePixelFormat(m_hDC,
        origPixelFormat,
        sizeof(PIXELFORMATDESCRIPTOR),
        &pfdOrig
    );
    if ((pfdOrig.dwFlags & PFD_DOUBLEBUFFER) == 0) {
        //For some reason, it didn't honor the DOUBLEBUFFER flag, we'll try and recreate the
        //context and try again using raw Windows OpenGL code.
        //(this tends to happen with the generic GDI driver in Windows 10)
        parent->RemoveChild(this);
        ::ReleaseDC(((HWND)GetHWND()), m_hDC);
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
        m_hDC = nullptr;

        int r = CreateWindow(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS | style, name);

        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
            1,                     // version number
            PFD_DRAW_TO_WINDOW     // support window
            | PFD_SUPPORT_OPENGL   // support OpenGL
            | PFD_DOUBLEBUFFER     // double buffered
            ,
            PFD_TYPE_RGBA,         // RGBA type
            24,                    // 24-bit color depth
            0, 0, 0, 0, 0, 0,      // color bits ignored
            8,                     // alpha buffer
            0,                     // shift bit ignored
            0,                     // no accumulation buffer
            0, 0, 0, 0,            // accum bits ignored
            only2d ? (uint8_t)0 : (uint8_t)16,       // 16-bit z-buffer
            0,                     // no stencil buffer
            0,                     // no auxiliary buffer
            PFD_MAIN_PLANE,        // main layer
            0,                     // reserved
            0, 0, 0                // layer masks ignored
        };
        m_hDC = ::GetDC(((HWND)GetHWND()));
        int iPixelFormat = ChoosePixelFormat(m_hDC, &pfd);

        DescribePixelFormat(m_hDC,
            iPixelFormat,
            sizeof(PIXELFORMATDESCRIPTOR),
            &pfd
        );
        m_zDepth = 0;
        if (!only2d) {
            m_zDepth = 16;
        }

        SetPixelFormat(m_hDC, iPixelFormat, &pfd);
    }
#endif
}

xlGLCanvas::xlGLCanvas(wxWindow* parent,
    const wxGLAttributes& dispAttrs,
    const wxString& name)
    : wxGLCanvas(parent, dispAttrs, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS, name),
    mWindowWidth(0),
    mWindowHeight(0),
    mWindowResized(false),
    mIsInitialized(false),
    m_context(nullptr),
    cache(nullptr),
    m_coreProfile(true),
    _name(name.ToStdString()),
    m_zDepth(0)
{
    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("                    Creating GL Canvas for %s", (const char*)name.c_str());

    this->GetGLCTXAttrs().PlatformDefaults();

#ifdef __WXMSW__
    int origPixelFormat = GetPixelFormat(m_hDC);
    PIXELFORMATDESCRIPTOR pfdOrig;
    DescribePixelFormat(m_hDC,
        origPixelFormat,
        sizeof(PIXELFORMATDESCRIPTOR),
        &pfdOrig
    );
    printf("PixelFormatFlags:  %08X\n", pfdOrig.dwFlags);
#endif
}

xlGLCanvas::~xlGLCanvas()
{
    if (m_context && m_context != m_sharedContext) {
        m_context->SetCurrent(*this);
        if (cache != nullptr) {
            DrawGLUtils::DestroyCache(cache);
        }
        delete m_context;
    }
}

#ifdef __WXMSW__
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
static const char* getStringForType(GLenum type)
{
    switch (type) {
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

void CALLBACK DebugLog(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const GLvoid* userParam)
{
    static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl_trace"));

    logger_opengl.info("Type : %s; Source : %s; ID : %d; Severity : % s\n Message: %s",
        getStringForType(type),
        getStringForSource(source),
        id,
        getStringForSeverity(severity),
        message);
}

void CALLBACK DebugLogAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, void* userParam)
{
    static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl_trace"));

    logger_opengl.info("%s; ID : %d; Severity : % s\n Message: %s",
        getStringForType(category),
        id,
        getStringForSeverity(severity),
        message);
}


void AddDebugLog(xlGLCanvas* c)
{
    static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl_trace"));
    PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
    PFNGLDEBUGMESSAGECONTROLARBPROC glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)wglGetProcAddress("glDebugMessageControlARB");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    if (glDebugMessageCallbackARB == nullptr) {
        logger_opengl.debug("Did not find debug callback ARB, attempting 4.3");
        glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallback");
        glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)wglGetProcAddress("glDebugMessageControl");
    }

    if (glDebugMessageCallbackARB != nullptr) {
        logger_opengl.debug("Adding debug callback.  %X", glDebugMessageControlARB);
        LOG_GL_ERRORV(glDebugMessageCallbackARB(DebugLog, c));
        if (glDebugMessageControlARB != nullptr) {
            GLuint unusedIds = 0;
            glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
        }
    }
    PFNGLDEBUGMESSAGECALLBACKAMDPROC glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)wglGetProcAddress("glDebugMessageCallbackAMD");
    if (glDebugMessageCallbackAMD != nullptr) {
        logger_opengl.debug("Adding AMD debug callback");
        LOG_GL_ERRORV(glDebugMessageCallbackAMD(DebugLogAMD, c));
    }
}
#else
void AddDebugLog(xlGLCanvas *c) {
}
#endif


DrawGLUtils::xlGLCacheInfo *Create33Cache();
DrawGLUtils::xlGLCacheInfo *Create11Cache();

void xlGLCanvas::DisplayWarning(const wxString& msg)
{
    ::DisplayWarning("Graphics Driver Problem: " + msg, this);
}

wxImage* xlGLCanvas::GrabImage(wxSize size /*=wxSize(0,0)*/)
{
    if (m_context == nullptr)
        return nullptr;

    if (!m_context->SetCurrent(*this))
        return nullptr;

    int width = mWindowWidth * GetContentScaleFactor();
    int height = mWindowHeight * GetContentScaleFactor();
    bool canScale = hasOpenGL3FramebufferObjects() && DrawGLUtils::IsCoreProfile();
    if (canScale && size != wxSize(0, 0)) {
        width = size.GetWidth();
        height = size.GetHeight();
    }

    // We'll grab the image as 4-byte-aligned RGBA and then convert to the
    // RGB format that wxImage uses; also doing a vertical flip along the way.
    width += width % 4;

    GLubyte* tmpBuf = new GLubyte[width * 4 * height];

    GLint currentUnpackAlignment = 1;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &currentUnpackAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    if (canScale) {
        GLuint fbID = 0, rbID = 0;

        glGenRenderbuffers(1, &rbID);
        glBindRenderbuffer(GL_RENDERBUFFER, rbID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

        glGenFramebuffers(1, &fbID);
        glBindFramebuffer(GL_FRAMEBUFFER, fbID);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbID);

        render();

        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmpBuf);

        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbID);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &rbID);
    }
    else {
        GLint currentReadBuffer = GL_NONE;
        glGetIntegerv(GL_READ_BUFFER, &currentReadBuffer);

        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmpBuf);

        glReadBuffer(currentReadBuffer);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, currentUnpackAlignment);

    // copying to wxImage
    wxSize dstSize = (canScale && size != wxSize(0, 0))
        ? wxSize(width, height)
        : wxSize(mWindowWidth * GetContentScaleFactor(),
            mWindowHeight * GetContentScaleFactor());
    unsigned char* buf = (unsigned char*)malloc(dstSize.GetWidth() * 3 * dstSize.GetHeight());
    if (buf != nullptr) {
        unsigned char* dst = buf;
        for (int y = dstSize.GetHeight() - 1; y >= 0; --y) {
            const unsigned char* src = tmpBuf + 4 * width * y;
            for (size_t x = 0; x < dstSize.GetWidth(); ++x, src += 4, dst += 3) {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
            }
        }
    }

    delete[] tmpBuf;

    return new wxImage(dstSize.GetWidth(), dstSize.GetHeight(), buf, false);
}

void xlGLCanvas::SetCurrentGLContext()
{
    static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    static bool errorDisplayed = false;
    glGetError();
    if (m_context == nullptr) {
        LOG_GL_ERRORV(CreateGLContext());
        if (m_context == nullptr) {
            if (!errorDisplayed) {
                errorDisplayed = true;
                logger_opengl.error("Could not create GL context ... aborting.");
                wxMessageBox("Critical error preparing context to draw on. Likely you need to update your video drivers.");
            }
            return;
        }
    }
    LOG_GL_ERRORV(m_context->SetCurrent(*this));
    if (!functionsLoaded) {
        LOG_GL_ERRORV(DrawGLUtils::LoadGLFunctions());
        functionsLoaded = true;
    }
    if (cache == nullptr) {
        wxConfigBase* config = wxConfigBase::Get();
        int ver = 99;
        config->Read("ForceOpenGLVer", &ver, 99);

        const GLubyte* str = glGetString(GL_VERSION);
        const GLubyte* rend = glGetString(GL_RENDERER);
        const GLubyte* vend = glGetString(GL_VENDOR);
        wxString configs = wxString::Format("%s - glVer:  %s  (%s)(%s)",
            (const char*)GetName().c_str(),
            (const char*)str,
            (const char*)rend,
            (const char*)vend);

        if (wxString(rend) == "GDI Generic"
            || wxString(vend).Contains("Microsoft")) {

            bool warned;
            config->Read("GDI-Warned", &warned, false);
            if (!warned) {
                config->Write("GDI-Warned", true);
                wxString msg = wxString::Format("Generic non-accelerated graphics driver detected (%s - %s). Performance will be poor.  "
                    "Please install updated video drivers for your video card.",
                    vend, rend);
                CallAfter(&xlGLCanvas::DisplayWarning, msg);
            }
            //need to use 1.x
            ver = 1;
        }

        logger_opengl.info(std::string(configs.c_str()));
        printf("%s\n", (const char*)configs.c_str());
        if (ver >= 3 && (str[0] > '3' || (str[0] == '3' && str[2] >= '3'))) {
            if (logger_opengl.isDebugEnabled()) {
                AddDebugLog(this);
            }
            logger_opengl.info("Try creating 3.3 Cache for %s", (const char*)_name.c_str());
            LOG_GL_ERRORV(cache = Create33Cache());
            if (cache != nullptr) _ver = 3;
        }
        if (cache == nullptr) {
            logger_opengl.info("Try creating 1.1 Cache for %s", (const char*)_name.c_str());
            LOG_GL_ERRORV(cache = Create11Cache());
            if (cache != nullptr) _ver = 1;
        }
        static bool hasWarned = false;
        if (_ver == 1 && !hasWarned) {
            hasWarned = true;
            wxMessageBox("OpenGL 1.x/2.x are known to have issues with xLights and will soon be removed.  Please update your video drivers.",
                         "OpenGL Version",
                          wxICON_INFORMATION | wxCENTER | wxOK);
        }
        if (cache == nullptr) {
            _ver = 0;
            logger_opengl.error("All attempts at cache creation have failed.");
        }
    }
    LOG_GL_ERRORV(DrawGLUtils::SetCurrentCache(cache));
}

void xlGLCanvas::CreateGLContext() {
    static log4cpp::Category &logger_opengl_trace = log4cpp::Category::getInstance(std::string("log_opengl_trace"));
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    if (m_context == nullptr) {
        wxGLContext *base = m_sharedContext;
        //trying to detect OGL versions and stuff can result in unwanted logs
        wxLogLevel cur = wxLog::GetLogLevel();
        wxLog::SetLogLevel(wxLOG_Error);
        wxLog::Suspend();

        wxConfigBase* config = wxConfigBase::Get();
        int ver = 99;
        config->Read("ForceOpenGLVer", &ver, 99);

        static bool supportsCoreProfile = true;

        if (supportsCoreProfile && m_coreProfile && ver >= 3) {
            wxGLContextAttrs atts;
            atts.PlatformDefaults().OGLVersion(3, 3).CoreProfile();
            if (logger_opengl_trace.isDebugEnabled()) {
                atts.ForwardCompatible().DebugCtx().EndList();
            }
            atts.EndList();
            glGetError();
            LOG_GL_ERRORV(m_context = new wxGLContext(this, base, &atts));
            if (!m_context->IsOK()) {
                logger_opengl.debug("Could not create a valid CoreProfile context");
                LOG_GL_ERRORV(delete m_context);
                m_context = nullptr;
                //supportsCoreProfile = false;
            } else {
                _ver = 3;
                LOG_GL_ERROR();
                const GLubyte* rend = glGetString(GL_RENDERER);
                if (wxString(rend) == "GDI Generic") {
                    //no way 3.x is going to work, software rendered, flip to 1.x
                    _ver = 1;
                    LOG_GL_ERRORV(delete m_context);
                    m_context = nullptr;
                    supportsCoreProfile = false;
                }
            }
        } else {
            _ver = 1;
        }
        if (m_context == nullptr) {
            glGetError();
            LOG_GL_ERRORV(m_context = new wxGLContext(this, base));
        }
        if (!functionsLoaded) {
            LOG_GL_ERROR();
            functionsLoaded = DrawGLUtils::LoadGLFunctions();
            glGetError(); // likely a function not there
        }
        if (m_context && !m_context->IsOK()) {
            LOG_GL_ERRORV(delete m_context);
            _ver = 0;
            m_context = nullptr;
        }
        wxLog::SetLogLevel(cur);
        wxLog::Resume();

        if (m_context == nullptr) {
            _ver = 0;
            logger_opengl.error("Error creating GL context.");
            m_context = m_sharedContext;
            InitializeGLContext();
        } else if (m_sharedContext == nullptr) {
            //use this as the shared context, then create a new one.
            m_sharedContext = m_context;
            m_context = nullptr;
            CreateGLContext();
        } else {
            InitializeGLContext();
        }
    }
}

void xlGLCanvas::Resized(wxSizeEvent& evt)
{
    mWindowWidth = evt.GetSize().GetWidth();
    mWindowHeight = evt.GetSize().GetHeight();
    mWindowResized = true;
#ifdef __WXOSX__
    Refresh();
#endif
}

double xlGLCanvas::translateToBacking(double x) const {
    return xlTranslateToRetina(*this, x);
}
double xlGLCanvas::mapLogicalToAbsolute(double x) const {
    if (drawingUsingLogicalSize()) {
        return x;
    }
    return translateToBacking(x);
}

// Inits the OpenGL viewport for drawing in 2D.
void xlGLCanvas::prepare2DViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y)
{
    DrawGLUtils::SetViewport(*this, topleft_x, topleft_y, bottomright_x, bottomright_y);
    mWindowResized = false;
}

void xlGLCanvas::prepare3DViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y)
{
    DrawGLUtils::SetViewport3D(*this, topleft_x, topleft_y, bottomright_x, bottomright_y);
    mWindowResized = false;
}


void xlGLCanvas::PrepareCanvas() {
    InitializeGLCanvas();
}


//#define GL_CLAMP_TO_EDGE 0x812F
class GLGraphicsContext : public xlGraphicsContext {
public:
    class xlGLTexture : public xlTexture {
    public:
        xlGLTexture() : xlTexture() {}
        xlGLTexture(const wxImage &i) {
            wxImage img = i.Mirror(false);
            image.load(img, false, false);
        }

        xlGLTexture(int w, int h, bool bgr, bool alpha) {
            this->alpha = alpha;
            GLuint _texId;
            LOG_GL_ERRORV( glGenTextures( 1, &_texId ) );
            LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, _texId ) );

            GLuint tp = bgr ? GL_BGRA : GL_RGBA;
            LOG_GL_ERRORV( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, tp, GL_UNSIGNED_BYTE, nullptr ) );
            LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
            LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
            LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
            LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );

            LOG_GL_ERRORV( ::glBindTexture( GL_TEXTURE_2D, 0 ) );
            image.width = w;
            image.height = h;
            image.setID(_texId);
        }
        virtual ~xlGLTexture() {
        }

        virtual void UpdatePixel(int x, int y, const xlColor &c, bool copyAlpha) override {
            DrawGLUtils::UpdateTexturePixel(image.getID(), (double)x, (double)y, c, copyAlpha);
        }
        virtual void UpdateData(uint8_t *data, bool bgr, bool alpha) override {
            LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, image.getID() ) );
            if (bgr && alpha) {
                LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_BGRA, GL_UNSIGNED_BYTE, data ) );
            } else if (bgr && !alpha) {
                LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_BGR, GL_UNSIGNED_BYTE, data ) );
            } else if (!bgr && alpha) {
                LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, data ) );
            } else if (!bgr && !alpha) {
                LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, data ) );
            }
            LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, 0 ) );
        }

        Image image;
        bool alpha = true;
    };

    GLGraphicsContext(xlGLCanvas *c) : xlGraphicsContext(c), canvas(c) {}
    virtual ~GLGraphicsContext() {}


    virtual xlVertexAccumulator *createVertexAccumulator() override {
        DrawGLUtils::xlVertexAccumulator *r = new DrawGLUtils::xlVertexAccumulator();
        return r;
    }
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() override {
        DrawGLUtils::xlVertexColorAccumulator *r = new DrawGLUtils::xlVertexColorAccumulator();
        return r;
    }

    virtual xlVertexTextureAccumulator *createVertexTextureAccumulator() override {
        DrawGLUtils::xlVertexTextureAccumulator *r = new DrawGLUtils::xlVertexTextureAccumulator();
        return r;
    }
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) override {
        xlGLTexture *t = new xlGLTexture();
        GLuint tid = 0;
        DrawGLUtils::CreateOrUpdateTexture(bitmaps[0], bitmaps[1], bitmaps[2], &tid);
        t->image.setID(tid);
        return t;
    }
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images) override {
        xlGLTexture *t = new xlGLTexture();
        GLuint tid = 0;
        DrawGLUtils::CreateOrUpdateTexture(wxBitmap(images[0]), wxBitmap(images[1]), wxBitmap(images[2]), &tid);
        t->image.setID(tid);
        t->image.width = images[0].GetWidth();
        t->image.height = images[0].GetHeight();
        t->image.textureWidth = images[0].GetWidth();
        t->image.textureHeight = images[0].GetHeight();
        return t;
    }
    virtual xlTexture *createTexture(const wxImage &image) override {
        return new xlGLTexture(image);
    }
    virtual xlTexture *createTexture(int w, int h, bool bgr, bool alpha) override {
        return new xlGLTexture(w, h, bgr, alpha);
    }
    virtual xlTexture *createTextureForFont(const xlFontInfo &font) override {
        return createTexture(font.getImage());
    }
    virtual xlGraphicsProgram *createGraphicsProgram() override {
        return new xlGraphicsProgram(createVertexColorAccumulator());
    }


    //drawing methods
    virtual xlGraphicsContext* drawLines(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexAccumulator*>(vac);
        DrawGLUtils::Draw(*v, c, GL_LINES, isBlending ? GL_LINE_SMOOTH : enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawLineStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexAccumulator*>(vac);
        DrawGLUtils::Draw(*v, c, GL_LINE_STRIP, isBlending ? GL_LINE_SMOOTH : enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawTriangles(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexAccumulator*>(vac);
        DrawGLUtils::Draw(*v, c, GL_TRIANGLES, enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexAccumulator*>(vac);
        DrawGLUtils::Draw(*v, c, GL_TRIANGLE_STRIP, enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawPoints(xlVertexAccumulator *vac, const xlColor &c, float pointSize, bool smoothPoints, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexAccumulator*>(vac);
        LOG_GL_ERRORV(glPointSize(pointSize));
        DrawGLUtils::Draw(*v, c, GL_POINTS, smoothPoints ? GL_POINT_SMOOTH : enableCapabilities, start, count);
        return this;
    }

    virtual xlGraphicsContext* drawLines(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexColorAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexColorAccumulator*>(vac);
        DrawGLUtils::Draw(*v, GL_LINES, isBlending ? GL_LINE_SMOOTH : enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawLineStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexColorAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexColorAccumulator*>(vac);
        DrawGLUtils::Draw(*v, GL_LINE_STRIP, isBlending ? GL_LINE_SMOOTH : enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawTriangles(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexColorAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexColorAccumulator*>(vac);
        DrawGLUtils::Draw(*v, GL_TRIANGLES, enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexColorAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexColorAccumulator*>(vac);
        DrawGLUtils::Draw(*v, GL_TRIANGLE_STRIP, enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawPoints(xlVertexColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexColorAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexColorAccumulator*>(vac);
        LOG_GL_ERRORV(glPointSize(pointSize));
        DrawGLUtils::Draw(*v, GL_POINTS, smoothPoints ? GL_POINT_SMOOTH : enableCapabilities, start, count);
        return this;
    }

    class glVertexIndexedColorAccumulator : public xlVertexIndexedColorAccumulator {
    public:
        virtual void Reset() override {
            vac.Reset();
        }
        virtual void PreAlloc(unsigned int i) override {
            vac.PreAlloc(i);
            colorIndexes.reserve(i);
        };
        virtual void AddVertex(float x, float y, float z, uint32_t cIdx) override {
            vac.AddVertex(x, y, z, xlBLACK);
            colorIndexes.push_back(cIdx);
        }
        virtual uint32_t getCount() override { return vac.getCount(); }

        virtual void SetColorCount(int c) override {
            colors.resize(c);
        }
        virtual uint32_t GetColorCount() override { return colors.size(); }
        virtual void SetColor(uint32_t idx, const xlColor &c) override {
            colors[idx] = c;
        }
        
        // mark this as ready to be copied to graphics card, after finalize,
        // vertices cannot be added, but if mayChange is set, the vertex/color
        // data can change via SetVertex and then flushed to push the
        // new data to the graphics card
        virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) override {
            for (size_t x = 0; x < colorIndexes.size(); ++x) {
                vac.SetVertex(x, colors[colorIndexes[x]]);
            }
            vac.Finalize(mayChangeVertices, mayChangeColors);
        }
        virtual void SetVertex(uint32_t vertex, float x, float y, float z, uint32_t cIdx) override  {
            vac.SetVertex(vertex, x, y, z);
            colorIndexes[vertex] = cIdx;
        }
        virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
            vac.SetVertex(vertex, x, y, z);
        }
        virtual void SetVertex(uint32_t vertex, uint32_t cIdx) override {
            colorIndexes[vertex] = cIdx;
        }
        virtual void FlushRange(uint32_t start, uint32_t len) override {
            vac.FlushRange(start, len);
        }
        virtual void FlushColors(uint32_t start, uint32_t len) override {
            for (size_t x = 0; x < getCount(); ++x) {
                int idx = colorIndexes[x];
                if (idx >= start && (idx < (start + len))) {
                    vac.SetVertex(x, colors[idx]);
                }
            }
            vac.FlushRange(0, getCount());
        }
        
        DrawGLUtils::xlVertexColorAccumulator vac;
        std::vector<uint32_t> colorIndexes;
        std::vector<xlColor> colors;
    };
    virtual xlVertexIndexedColorAccumulator *createVertexIndexedColorAccumulator() override {
        return new glVertexIndexedColorAccumulator();
    }
    virtual xlGraphicsContext* drawLines(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override {
        glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
        drawLines(&va->vac, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawLineStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override {
        glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
        drawLineStrip(&va->vac, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawTriangles(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override {
        glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
        drawTriangles(&va->vac, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override {
        glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
        drawTriangleStrip(&va->vac, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawPoints(xlVertexIndexedColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override {
        glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
        drawPoints(&va->vac, pointSize, smoothPoints, start, count);
        return this;
    }

    
    virtual xlGraphicsContext* drawTexture(xlTexture *texture,
                             float x, float y, float x2, float y2,
                             float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                             bool nearest = true,
                             int brightness = 100, int alpha = 255) override {
        xlGLTexture *t = (xlGLTexture*)texture;
        tx *= t->image.tex_coord_x;
        ty *= t->image.tex_coord_y;
        tx2 *= t->image.tex_coord_x;
        ty2 *= t->image.tex_coord_y;
        DrawGLUtils::xlVertexTextureAccumulator vta(t->image.getID(), (uint8_t)alpha);
        vta.brightness = brightness;
        vta.AddVertex(x, y, tx, ty);
        vta.AddVertex(x, y2, tx, ty2);
        vta.AddVertex(x2, y2, tx2, ty2);
        vta.AddVertex(x, y, tx, ty);
        vta.AddVertex(x2, y2, tx2, ty2);
        vta.AddVertex(x2, y, tx2, ty);
        DrawGLUtils::Draw(vta, GL_TRIANGLES, enableCapabilities);
        return this;
    }
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) override {
        DrawGLUtils::xlVertexTextureAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexTextureAccumulator*>(vac);
        v->brightness = brightness;
        v->alpha = alpha;
        
        xlGLTexture *t = (xlGLTexture*)texture;
        v->id = t->image.getID();
        v->forceColor = false;
        DrawGLUtils::Draw(*v, GL_TRIANGLES, enableCapabilities, start, count);
        return this;
    }
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c, int start = 0, int count = -1) override {
        DrawGLUtils::xlVertexTextureAccumulator *v = dynamic_cast<DrawGLUtils::xlVertexTextureAccumulator*>(vac);
        xlGLTexture *t = (xlGLTexture*)texture;
        v->id = t->image.getID();
        v->forceColor = true;
        v->color = c;
        DrawGLUtils::Draw(*v, GL_TRIANGLES, enableCapabilities, start, count);
        return this;
    }

    class xlGLMesh : public xlMesh {
    public:
        xlGLMesh(const std::string &file, GLGraphicsContext *ctx) : xlMesh(ctx, file) {
            create3DMesh(ctx);
        }
        virtual ~xlGLMesh() {
            if (mesh) {
                delete mesh;
            }
        }
        void create3DMesh(GLGraphicsContext *ctx) {
            if (mesh != nullptr && !materialsNeedResyncing) {
                return;
            }
            if (mesh) {
                delete mesh;
            }
            materialsNeedResyncing = false;
            mesh = DrawGLUtils::createMesh();
            for (auto &s : objects.GetShapes()) {
                if (!s.mesh.indices.empty()) {
                    for (int idx = 0; idx < s.mesh.material_ids.size(); ++idx) {
                        float vert[3][3];
                        float norms[3][3];
                        float tc[3][2];
                        GLint texture = -1;
                        uint8_t color[3][4];
                        xlColor c(xlWHITE);
                        if (s.mesh.material_ids[idx] >= 0) {
                            if (this->materials[s.mesh.material_ids[idx]].texture && !this->materials[s.mesh.material_ids[idx]].forceColor) {
                                xlGLTexture *t = (xlGLTexture*)this->materials[s.mesh.material_ids[idx]].texture;
                                texture = t->image.getID();
                            }
                            c = this->materials[s.mesh.material_ids[idx]].color;
                        }
                        for (size_t x = 0; x < 3; ++x) {
                            tinyobj::index_t vi = s.mesh.indices[idx*3 + x];

                            auto& objVerts = objects.GetAttrib().vertices;
                            if ((vi.vertex_index < 0) || ((vi.vertex_index * 3 + 2) >= objVerts.size())) {
                                vert[x][0] = 0;
                                vert[x][1] = 0;
                                vert[x][2] = 0;
                            } else {
                                vert[x][0] = objVerts[vi.vertex_index * 3];
                                vert[x][1] = objVerts[vi.vertex_index * 3 + 1];
                                vert[x][2] = objVerts[vi.vertex_index * 3 + 2];
                            }

                            auto& objNorms = objects.GetAttrib().normals;
                            if ((vi.vertex_index < 0) || ((vi.normal_index * 3 + 2) >= objNorms.size())) {
                                vert[x][0] = 0;
                                vert[x][1] = 0;
                                vert[x][2] = 0;
                            } else {
                                norms[x][0] = objNorms[vi.normal_index * 3];
                                norms[x][1] = objNorms[vi.normal_index * 3 + 1];
                                norms[x][2] = objNorms[vi.normal_index * 3 + 2];
                            }

                            auto& objTexCoords = objects.GetAttrib().texcoords;
                            if ((vi.vertex_index < 0) || ((vi.texcoord_index * 2 + 1) >= objTexCoords.size())) {
                                tc[x][0] = 0;
                                tc[x][1] = 0;
                            } else {
                                tc[x][0] = objTexCoords[vi.texcoord_index * 2];
                                tc[x][1] = objTexCoords[vi.texcoord_index * 2 + 1];
                            }
                            
                            color[x][0] = c.red;
                            color[x][1] = c.green;
                            color[x][2] = c.blue;
                            color[x][3] = c.alpha;
                        }
                        mesh->addSurface(vert, tc, norms, color, texture);
                    }
                }
                if (!s.lines.indices.empty()) {
                    for (size_t idx = 0 ; idx + 1 < s.lines.indices.size(); idx += 2) {
                        float vert[2][3];
                        for (size_t x = 0; x < 2; ++x) {
                            tinyobj::index_t vi = s.lines.indices[idx + x];
                            vert[x][0] = objects.GetAttrib().vertices[vi.vertex_index * 3];
                            vert[x][1] = objects.GetAttrib().vertices[vi.vertex_index * 3 + 1];
                            vert[x][2] = objects.GetAttrib().vertices[vi.vertex_index * 3 + 2];
                        }
                        mesh->addLine(vert);
                    }
                }
                /*
                if (!s.points.indices.empty()) {
                    xlMetalSubMesh *sm = new xlMetalSubMesh();
                    sm->name = s.name;
                    sm->type = MTLPrimitiveTypePoint;
                    sm->startIndex = indexes.size();
                    for (auto &idx : s.points.indices) {
                        indexes.push_back(getOrAddIndex(indexMap, input, idx));
                    }
                    sm->count = indexes.size() - sm->startIndex;
                    subMeshes.push_back(sm);
                }
                 */
            }
        }
        DrawGLUtils::xl3DMesh* mesh = nullptr;
    };
    virtual xlMesh *loadMeshFromObjFile(const std::string &file) override {
        return new xlGLMesh(file, this);
    }
    virtual xlGraphicsContext* drawMeshSolids(xlMesh *mesh, int brightness, bool useViewMatrix) override {
        xlGLMesh *glm = (xlGLMesh*)mesh;
        glm->create3DMesh(this);
        if (glm->mesh) {
            glDepthFunc(GL_LESS);
            DrawGLUtils::xlAccumulator vac;
            vac.AddMesh(glm->mesh, false, brightness, false);
            DrawGLUtils::Draw(vac);
            glDepthFunc(GL_LEQUAL);
        }
        return this;
    }
    virtual xlGraphicsContext* drawMeshTransparents(xlMesh *mesh, int brightness) override {
        xlGLMesh *glm = (xlGLMesh*)mesh;
        glm->create3DMesh(this);
        if (glm->mesh) {
            glDepthFunc(GL_LESS);
            DrawGLUtils::xlAccumulator vac;
            vac.AddMesh(glm->mesh, false, brightness, true);
            DrawGLUtils::Draw(vac);
            glDepthFunc(GL_LEQUAL);
        }
        return this;
    }
    virtual xlGraphicsContext* drawMeshWireframe(xlMesh *mesh, int brightness) override {
        xlGLMesh *glm = (xlGLMesh*)mesh;
        glm->create3DMesh(this);
        if (glm->mesh) {
            glDepthFunc(GL_LESS);
            DrawGLUtils::xlAccumulator vac;
            vac.AddMesh(glm->mesh, true, brightness, false);
            DrawGLUtils::Draw(vac);
            glDepthFunc(GL_LEQUAL);
        }
        return this;
    }

    virtual xlGraphicsContext* enableBlending(bool e = true) override {
        if (e) {
            LOG_GL_ERRORV(glEnable(GL_BLEND));
            LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
        } else {
            LOG_GL_ERRORV(glDisable(GL_BLEND));
        }
        isBlending = e;
        return this;
    }

    // Setup the Viewport
    xlGraphicsContext* SetViewport(int x1, int y1, int x2, int y2, bool is3D) override {
        if (is3D) {
            DrawGLUtils::SetViewport3D(*canvas, x1, y1, x2, y2);
        } else {
            DrawGLUtils::SetViewport(*canvas, x1, y1, x2, y2);
        }
        return this;
    }


    //manipulating the matrices
    virtual xlGraphicsContext* PushMatrix() override {
        DrawGLUtils::PushMatrix();
        return this;
    }
    virtual xlGraphicsContext* PopMatrix() override {
        DrawGLUtils::PopMatrix();
        return this;
    }
    virtual xlGraphicsContext* Translate(float x, float y, float z) override {
        DrawGLUtils::Translate(x, y, z);
        return this;
    }
    virtual xlGraphicsContext* Rotate(float angle, float x, float y, float z) override {
        DrawGLUtils::Rotate(angle, x, y, z);
        return this;
    }
    virtual xlGraphicsContext* Scale(float w, float h, float z) override {
        DrawGLUtils::Scale(w, h, z);
        return this;
    }
    virtual xlGraphicsContext* ScaleViewMatrix(float w, float h, float z) override {
        DrawGLUtils::Scale(w, h, z);
        return this;
    }
    virtual xlGraphicsContext* TranslateViewMatrix(float x, float y, float z) override {
        DrawGLUtils::Translate(x, y, z);
        return this;
    }

    virtual xlGraphicsContext* SetCamera(const glm::mat4 &m) override {
        DrawGLUtils::SetCamera(m);
        return this;
    }
    virtual xlGraphicsContext* SetModelMatrix(const glm::mat4 &m) override {
        DrawGLUtils::SetModelMatrix(m);
        return this;
    }
    virtual xlGraphicsContext* ApplyMatrix(const glm::mat4 &m) override {
        DrawGLUtils::ApplyMatrix(m);
        return this;
    }

    int enableCapabilities = 0;
    bool isBlending = false;
    xlGLCanvas *canvas;
};

xlGraphicsContext *xlGLCanvas::PrepareContextForDrawing() {
    return PrepareContextForDrawing(ClearBackgroundColor());
}
xlGraphicsContext* xlGLCanvas::PrepareContextForDrawing(const xlColor &bg) {
    InitializeGLContext();
    SetCurrentGLContext();

    float r = bg.red;
    float g = bg.green;
    float b = bg.blue;
    float a = bg.alpha;
    r /= 255.0f;
    g /= 255.0f;
    b /= 255.0f;
    a /= 255.0f;
    LOG_GL_ERRORV(glClearColor(r, g, b, a));
    LOG_GL_ERRORV(glDisable(GL_BLEND));
    if (!is3d) {
        LOG_GL_ERRORV(glDisable(GL_DEPTH_TEST));
    } else {
        LOG_GL_ERRORV(glEnable(GL_DEPTH_TEST));
    }
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | (is3d ? GL_DEPTH_BUFFER_BIT : 0)));

    return new GLGraphicsContext(this);
}
void xlGLCanvas::FinishDrawing(xlGraphicsContext* ctx, bool display) {
    if (display) {
        SwapBuffers();
    }
    delete ctx;
}

bool xlGLCanvas::getFrameForExport(int w, int h, AVFrame *, uint8_t *buffer, int bufferSize) {
    bool padWidth = (w % 2);
    bool padHeight = (h % 2);
    int widthWithPadding = padWidth ? (w + 1) : w;
    int heightWithPadding = padHeight ? (h + 1) : h;
    unsigned int reqSize = widthWithPadding * 3 * heightWithPadding;
    if (bufferSize < reqSize) {
        return false;
    }
    uint8_t *tmpBuf = new uint8_t[w * 4 * h];
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tmpBuf);

    unsigned char *dst = buffer;
    if (padHeight) {
        memset(dst, 0, widthWithPadding * 3);
        dst += widthWithPadding * 3;
    }
    for (int y = h - 1; y >= 0; --y) {
        const unsigned char *src = tmpBuf + 4 * w * y;
        for (size_t x = 0; x < w; ++x, src += 4, dst += 3) {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
        }
        if (padWidth) {
            dst[0] = dst[1] = dst[2] = 0x00;
            dst += 3;
        }
    }
    delete [] tmpBuf;
    
    return true;
}
