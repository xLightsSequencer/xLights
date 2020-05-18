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

BEGIN_EVENT_TABLE(xlGLCanvas, wxGLCanvas)
    EVT_SIZE(xlGLCanvas::Resized)
    EVT_ERASE_BACKGROUND(xlGLCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#include "osxMacUtils.h"

#include <wx/log.h>
#include <wx/config.h>
#include <wx/msgdlg.h>
#include <log4cpp/Category.hh>

static const int DEPTH_BUFFER_BITS[] = {32, 24, 16, 12, 10, 8};

wxGLContext *xlGLCanvas::m_sharedContext = nullptr;

static wxGLAttributes GetAttributes(int &zdepth) {
    DrawGLUtils::SetupDebugLogging();
    
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    
    wxGLAttributes atts;
    for (int x = 0; x < 6; x++) {
        atts.Reset();
        atts.PlatformDefaults()
            .RGBA()
            .MinRGBA(8, 8, 8, 8)
            .DoubleBuffer()
            .Depth(DEPTH_BUFFER_BITS[x])
            .EndList();
        if (wxGLCanvas::IsDisplaySupported(atts)) {
            logger_opengl.debug("Depth of %d supported, using it", DEPTH_BUFFER_BITS[x]);
            zdepth = DEPTH_BUFFER_BITS[x];
            return atts;
        }
        logger_opengl.debug("Depth of %d not supported", DEPTH_BUFFER_BITS[x]);
    }
    logger_opengl.debug("Could not find an attribs thats working with MnRGBA\n");
    // didn't find a display, try without MinRGBA
    for (int x = 0; x < 6; x++) {
        atts.Reset();
        atts.PlatformDefaults()
            .RGBA()
            .DoubleBuffer()
            .Depth(DEPTH_BUFFER_BITS[x])
            .EndList();
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

xlGLCanvas::CaptureHelper::~CaptureHelper()
{
    if (tmpBuf != nullptr)
    {
        delete[] tmpBuf;
        tmpBuf = nullptr;
    }
}

bool xlGLCanvas::CaptureHelper::ToRGB(unsigned char *buf, unsigned int bufSize, bool padToEvenDims/*=false*/)
{
    int w = width * contentScaleFactor;
    int h = height * contentScaleFactor;

    bool padWidth = padToEvenDims && (w % 2);
    bool padHeight = padToEvenDims && (h % 2);
    int widthWithPadding = padWidth ? (w + 1) : w;
    int heightWithPadding = padHeight ? (h + 1) : h;
    unsigned int reqSize = widthWithPadding * 3 * heightWithPadding;
    if (bufSize < reqSize)
        return false;

    if (tmpBuf == nullptr)
    {
        typedef unsigned char uchar;
        tmpBuf = new uchar[w * 4 * h];
    }

    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tmpBuf);

    unsigned char *dst = buf;
    if (padHeight)
    {
        memset(dst, 0, widthWithPadding * 3);
        dst += widthWithPadding * 3;
    }
    for (int y = h - 1; y >= 0; --y)
    {
        const unsigned char *src = tmpBuf + 4 * w * y;
        for (int x = 0; x < w; ++x, src += 4, dst += 3)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
        }
        if (padWidth)
        {
            dst[0] = dst[1] = dst[2] = 0x00;
            dst += 3;
        }
    }

    return true;
}

static int tempZDepth = 0;
xlGLCanvas::xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos,
                       const wxSize &size, long style, const wxString &name,
                       bool only2d)
    :   wxGLCanvas(parent, GetAttributes(tempZDepth), id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS | style, name),
        mWindowWidth(0),
        mWindowHeight(0),
        mWindowResized(false),
        mIsInitialized(false),
        m_context(nullptr),
        m_coreProfile(true),
        cache(nullptr),
        m_zDepth(tempZDepth),
        _name(name)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("                    Creating GL Canvas for %s", (const char *)name.c_str());

    xlSetOpenGLRetina(*this);
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
        int ret = SetPixelFormat(m_hDC, iPixelFormat, &pfd);
    }
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
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl_trace"));

    logger_opengl.info("Type : %s; Source : %s; ID : %d; Severity : % s\n Message: %s",
                       getStringForType( type ),
                       getStringForSource( source ),
                       id,
                       getStringForSeverity( severity ),
                       message);
}
void CALLBACK DebugLogAMD(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,void *userParam) {
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl_trace"));

    logger_opengl.info("%s; ID : %d; Severity : % s\n Message: %s",
                       getStringForType( category ),
                       id,
                       getStringForSeverity( severity ),
                       message);
}


void AddDebugLog(xlGLCanvas *c) {
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl_trace"));
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


DrawGLUtils::xlGLCacheInfo *Create33Cache(bool, bool, bool, bool, bool, bool, bool);
DrawGLUtils::xlGLCacheInfo *Create11Cache();

void xlGLCanvas::DisplayWarning(const wxString &msg) {
    ::DisplayWarning("Graphics Driver Problem: " + msg, this);
}

wxImage * xlGLCanvas::GrabImage(wxSize size /*=wxSize(0,0)*/)
{
    if (m_context == nullptr)
        return nullptr;

    if (!m_context->SetCurrent(*this))
        return nullptr;

    int width = mWindowWidth * GetContentScaleFactor();
    int height = mWindowHeight * GetContentScaleFactor();
    bool canScale = hasOpenGL3FramebufferObjects() && DrawGLUtils::IsCoreProfile();
    if (canScale && size != wxSize(0, 0))
    {
        width = size.GetWidth();
        height = size.GetHeight();
    }

    // We'll grab the image as 4-byte-aligned RGBA and then convert to the
    // RGB format that wxImage uses; also doing a vertical flip along the way.
    width += width % 4;

    GLubyte *tmpBuf = new GLubyte[width * 4 * height];

    GLint currentUnpackAlignment = 1;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &currentUnpackAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    if (canScale)
    {
        GLuint fbID = 0, rbID = 0;

        glGenRenderbuffers(1, &rbID);
        glBindRenderbuffer(GL_RENDERBUFFER, rbID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

        glGenFramebuffers(1, &fbID);
        glBindFramebuffer(GL_FRAMEBUFFER, fbID);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbID);

        render(wxSize(width,height));

        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmpBuf);

        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbID);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &rbID);
    }
    else
    {
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
    unsigned char *buf = (unsigned char *)malloc(dstSize.GetWidth() * 3 * dstSize.GetHeight());
    unsigned char *dst = buf;
    for (int y = dstSize.GetHeight() - 1; y >= 0; --y)
    {
        const unsigned char *src = tmpBuf + 4 * width * y;
        for (int x = 0; x < dstSize.GetWidth(); ++x, src += 4, dst += 3)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
        }
    }

    delete[] tmpBuf;

    return new wxImage(dstSize.GetWidth(), dstSize.GetHeight(), buf, false);
}


void xlGLCanvas::SetCurrentGLContext() {
    glGetError();
    if (m_context == nullptr) {
        LOG_GL_ERRORV(CreateGLContext());
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

        static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        const GLubyte* str = glGetString(GL_VERSION);
        const GLubyte* rend = glGetString(GL_RENDERER);
        const GLubyte* vend = glGetString(GL_VENDOR);
        wxString configs = wxString::Format("%s - glVer:  %s  (%s)(%s)",
                                            (const char *)GetName().c_str(),
                                            (const char *)str,
                                            (const char *)rend,
                                            (const char *)vend);

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
        printf("%s\n", (const char *)configs.c_str());
        if (ver >= 3 && (str[0] > '3' || (str[0] == '3' && str[2] >= '3'))) {
            if (logger_opengl.isDebugEnabled()) {
                AddDebugLog(this);
            }
            logger_opengl.info("Try creating 3.3 Cache for %s", (const char *)_name.c_str());
            LOG_GL_ERRORV(cache = Create33Cache(UsesVertexTextureAccumulator(),
                UsesVertexColorAccumulator(),
                UsesVertexAccumulator(),
                UsesAddVertex(),
                UsesVertex3Accumulator(),
                UsesVertex3TextureAccumulator(),
                UsesVertex3ColorAccumulator()));
            if (cache != nullptr) _ver = 3;
        }
        if (cache == nullptr) {
            logger_opengl.info("Try creating 1.1 Cache for %s", (const char *)_name.c_str());
            LOG_GL_ERRORV(cache = Create11Cache());
            if (cache != nullptr) _ver = 1;
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
        //trying to detect OGL verions and stuff can result in unwanted logs
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

double xlGLCanvas::translateToBacking(double x) {
    return xlTranslateToRetina(*this, x);
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
