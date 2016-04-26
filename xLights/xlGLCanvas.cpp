#include <wx/file.h>
#include "xlGLCanvas.h"


#ifdef __WXMSW__

class GL_CONTEXT_CLASS {
public:
    GL_CONTEXT_CLASS(xlGLCanvas *win) {
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
            1,                     // version number
            PFD_DRAW_TO_WINDOW |   // support window
            PFD_SUPPORT_OPENGL   // support OpenGL
            | PFD_DOUBLEBUFFER      // double buffered
            ,
            PFD_TYPE_RGBA,         // RGBA type
            24,                    // 24-bit color depth
            0, 0, 0, 0, 0, 0,      // color bits ignored
            0,                     // no alpha buffer
            0,                     // shift bit ignored
            0,                     // no accumulation buffer
            0, 0, 0, 0,            // accum bits ignored
            16,                    // 32-bit z-buffer
            0,                     // no stencil buffer
            0,                     // no auxiliary buffer
            PFD_MAIN_PLANE,        // main layer
            0,                     // reserved
            0, 0, 0                // layer masks ignored
        };
        m_hDC = ::GetDC(win->GetHWND());
        origWin = win;
        int iPixelFormat = ChoosePixelFormat(m_hDC, &pfd);


        /*
        wxFile file("c:\\temp\\out.txt", wxFile::OpenMode::write);

        int max = DescribePixelFormat(m_hDC, iPixelFormat,
         sizeof(PIXELFORMATDESCRIPTOR), &pfd);
        int match = -1;
        for (int x = 0; x < max; x++) {
            DescribePixelFormat(m_hDC, x,
                                sizeof(PIXELFORMATDESCRIPTOR), &pfd);

            if (pfd.cRedBits == 8
                && pfd.cGreenBits == 8
                && pfd.cBlueBits == 8
                //&& pfd.cAlphaBits == 8   //don't need alpha things, has issues on Windows8 and Aero
                && (pfd.dwFlags & PFD_SUPPORT_OPENGL)
                && (pfd.dwFlags & PFD_DRAW_TO_WINDOW)
                && (pfd.dwFlags & PFD_DOUBLEBUFFER)
                && pfd.iPixelType == PFD_TYPE_RGBA) {
                match = x;
            }

            int accel = true;
            if ((pfd.dwFlags & PFD_GENERIC_FORMAT)
                && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED)) {
                accel = false;
            }
            file.Write(wxString::Format("%2d pt:%d  cd:%2d  ab:%d r:%d g:%d b:%d d:%2d  acc:%d  %X      %d\n",
                                         x,
                                         pfd.iPixelType,
                                         pfd.cColorBits,
                                         pfd.cAlphaBits,
                                        pfd.cRedBits,
                                        pfd.cGreenBits,
                                        pfd.cBlueBits,
                                        pfd.cDepthBits,
                                        accel,
                                         pfd.dwFlags,
                                         match));
        }


        file.Close();
        if (match != -1) {
            iPixelFormat = match;
        }
         */
        DescribePixelFormat(m_hDC, iPixelFormat,
                            sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        SetPixelFormat(m_hDC, iPixelFormat, &pfd);
        m_glContext = wglCreateContext(m_hDC);
    }
    ~GL_CONTEXT_CLASS() {
        wglDeleteContext(m_glContext);
        ::ReleaseDC(origWin->GetHWND(), m_hDC);
    }
    bool SetCurrent(xlGLCanvas &win) {
        if ( !wglMakeCurrent(m_hDC, m_glContext) )
        {
            wxLogLastError(wxT("wglMakeCurrent"));
            return false;
        }
        return true;
    }
    bool SwapBuffers() {
        if ( !::SwapBuffers(m_hDC) )
        {
            wxLogLastError(wxT("SwapBuffers"));
            return false;
        }
        return true;
    }
    HDC m_hDC;
    xlGLCanvas *origWin;
    HGLRC m_glContext;
};
#endif

#ifdef __WXMSW__
BEGIN_EVENT_TABLE(xlGLCanvas, wxWindow)
#else
BEGIN_EVENT_TABLE(xlGLCanvas, wxGLCanvas)
#endif // __WXMSW__
EVT_SIZE(xlGLCanvas::Resized)
EVT_ERASE_BACKGROUND(xlGLCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()

#include "osxMacUtils.h"

static const int GLARGS[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};
static const int CORE_GLARGS[] = {WX_GL_CORE_PROFILE, WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

xlGLCanvas::xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos,
                       const wxSize &size, long style, const wxString &name,
                       bool coreProfile)
#ifndef __WXMSW__
    :wxGLCanvas(parent, id, coreProfile ? CORE_GLARGS : GLARGS, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS | style),
#else
    :   wxWindow(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS | style),
#endif
        mWindowWidth(0),
        mWindowHeight(0),
        mWindowResized(false),
        mIsInitialized(false),
        m_context(new GL_CONTEXT_CLASS(this)),
        cache(nullptr)
{
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

#ifdef __WXMSW__
bool xlGLCanvas::SwapBuffers() {
    return m_context->SwapBuffers();
}
#endif

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


