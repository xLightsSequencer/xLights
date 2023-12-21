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
#include "xlOGL3GraphicsContext.h"

BEGIN_EVENT_TABLE(xlGLCanvas, wxGLCanvas)
    EVT_SIZE(xlGLCanvas::Resized)
    EVT_ERASE_BACKGROUND(xlGLCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#include <wx/log.h>
#include <wx/config.h>
#include <wx/msgdlg.h>
#include <log4cpp/Category.hh>
#include "../xlMesh.h"
#include "DrawGLUtils.h"


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
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

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
#include "GL/glext.h"
static bool hasOpenGL3FramebufferObjects()
{
    return true;
}
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
        for (auto &ver : vertexArrayIds) {
            LOG_GL_ERRORV(glDeleteVertexArrays(1, &ver.second));
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
    glEnable(GL_DEBUG_OUTPUT);
}
#else
void AddDebugLog(xlGLCanvas *c) {
    glEnable(GL_DEBUG_OUTPUT);
}
#endif


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
    bool canScale = hasOpenGL3FramebufferObjects() && IsCoreProfile();
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
        
        wxGLContextAttrs atts;
        atts.PlatformDefaults().OGLVersion(3, 3).CoreProfile();
        if (logger_opengl_trace.isDebugEnabled()) {
            atts.ForwardCompatible().DebugCtx();
        }
        atts.EndList();
        glGetError();
        LOG_GL_ERRORV(m_context = new wxGLContext(this, base, &atts));
        if (!m_context || !m_context->IsOK()) {
            logger_opengl.debug("Could not create a valid CoreProfile context");
            if (m_context) {
                LOG_GL_ERRORV(delete m_context);
            }
            m_context = nullptr;
            
            wxGLContextAttrs atts2;
            atts2.PlatformDefaults().OGLVersion(2, 1);
            if (logger_opengl_trace.isDebugEnabled()) {
                atts2.ForwardCompatible().DebugCtx();
            }
            atts2.EndList();
            glGetError();
            LOG_GL_ERRORV(m_context = new wxGLContext(this, base, &atts2));
        }
        if (m_context && !m_context->IsOK()) {
            LOG_GL_ERRORV(delete m_context);
            m_context = nullptr;
        }
        if (m_context == nullptr) {
            glGetError();
            LOG_GL_ERRORV(m_context = new wxGLContext(this, base));
        }
        if (m_context && !m_context->IsOK()) {
            LOG_GL_ERRORV(delete m_context);
            m_context = nullptr;
        }
        wxLog::SetLogLevel(cur);
        wxLog::Resume();

        
        if (m_context == nullptr) {
            logger_opengl.error("Error creating GL context.");
            m_context = m_sharedContext;
        } else if (m_sharedContext == nullptr) {
            //use this as the shared context, then create a new one.
            m_sharedContext = m_context;
            m_context->SetCurrent(*this);
            
            const GLubyte* str = glGetString(GL_VERSION);
            if (str[0] <= '1') {
                static bool hasWarned = false;
                if (!hasWarned) {
                    hasWarned = true;
                    CallAfter([]() {
                        wxMessageBox("OpenGL 1.x/2.x are no longer supported.  Please update your video drivers.",
                                     "OpenGL Version",
                                      wxICON_INFORMATION | wxCENTER | wxOK);
                    });
                }
            }
            
            if (!xlOGL3GraphicsContext::InitializeSharedContext())
            {
                logger_opengl.error("Failed to initialise shared OpenGL context.");
            }

            m_context = nullptr;
            CreateGLContext();
        } else {
            m_context->SetCurrent(*this);
            const GLubyte* str = glGetString(GL_VERSION);
            const GLubyte* rend = glGetString(GL_RENDERER);
            const GLubyte* vend = glGetString(GL_VENDOR);
            wxString configs = wxString::Format("%s - glVer:  %s  (%s)(%s)",
                (const char*)GetName().c_str(),
                (const char*)str,
                (const char*)rend,
                (const char*)vend);

            if (str[0] > '3') {
                isCoreProfile = true;
            } else if (str[0] == '3') {
                isCoreProfile = str[2] >= '3';
            } else {
                isCoreProfile = false;
            }
            logger_opengl.info(std::string(configs.c_str()));
            printf("%s\n", (const char*)configs.c_str());
            
            if (logger_opengl.isDebugEnabled()) {
                AddDebugLog(this);
            }
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
    return GetContentScaleFactor() * x;
}
double xlGLCanvas::mapLogicalToAbsolute(double x) const {
    if (drawingUsingLogicalSize()) {
        return x;
    }
    return translateToBacking(x);
}

void xlGLCanvas::PrepareCanvas() {
    InitializeGLCanvas();
}

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

    return new xlOGL3GraphicsContext(this);
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

bool xlGLCanvas::bindVertexArrayID(GLuint pid) {
    if (!IsCoreProfile()) {
        return false;
    }
    GLuint vid = vertexArrayIds[pid];
    if (vid == 0) {
        LOG_GL_ERRORV(glGenVertexArrays(1, &vid));
        vertexArrayIds[pid] = vid;
    }
    LOG_GL_ERRORV(glBindVertexArray(vid));
    return true;
}
