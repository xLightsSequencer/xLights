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


        //wxFile file("c:\\temp\\out.txt", wxFile::OpenMode::write);

        int max = DescribePixelFormat(m_hDC, iPixelFormat,
         sizeof(PIXELFORMATDESCRIPTOR), &pfd);
        int match = -1;
        for (int x = 0; x < max; x++) {
            DescribePixelFormat(m_hDC, x,
                                sizeof(PIXELFORMATDESCRIPTOR), &pfd);

            if (pfd.cRedBits == 8
                && pfd.cGreenBits == 8
                && pfd.cBlueBits == 8
                && pfd.cAlphaBits == 0   //don't need alpha things, has issues on Windows8 and Aero
                && (pfd.dwFlags & PFD_SUPPORT_OPENGL)
                && (pfd.dwFlags & PFD_DRAW_TO_WINDOW)
                && (pfd.dwFlags & PFD_DOUBLEBUFFER)
                && pfd.iPixelType == PFD_TYPE_RGBA) {
                match = x;
            }
/*
            file.Write(wxString::Format("%d   cd: %d   ab: %d  r:%d g:%d b:%d  %X      %d\n",
                                         x,
                                         pfd.cColorBits,
                                         pfd.cAlphaBits,
                                        pfd.cRedBits,
                                        pfd.cGreenBits,
                                        pfd.cBlueBits,
                                         pfd.dwFlags,
                                         match));
                                         */
        }


        //file.Close();
        if (match != -1) {
            iPixelFormat = match;
        }
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


static const int GLARGS[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

xlGLCanvas::xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos,
                const wxSize &size, long style, const wxString &name)
#ifndef __WXMSW__
    :wxGLCanvas(parent, id, GLARGS, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS)
#else
    :wxWindow(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS)
#endif
{
    m_context = new GL_CONTEXT_CLASS(this);
}

xlGLCanvas::~xlGLCanvas()
{
    delete m_context;
}

void xlGLCanvas::SetCurrentGLContext() {
     m_context->SetCurrent(*this);
}

#ifdef __WXMSW__
bool xlGLCanvas::SwapBuffers() {
    m_context->SwapBuffers();
}
#endif


