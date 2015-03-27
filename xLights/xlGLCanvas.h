#ifndef XLGLCANVAS_H
#define XLGLCANVAS_H

#include "wx/glcanvas.h"

#ifndef __WXMSW__
#define GL_CONTEXT_CLASS wxGLContext
#else
#define GL_CONTEXT_CLASS xlightsGLContext
class xlightsGLContext;
#endif


class xlGLCanvas
#ifdef __WXMSW__
    : public wxWindow
#else
    : public wxGLCanvas
#endif // __WXMSW__
{
    public:
        xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGLCanvas();

        void SetCurrentGLContext();

        int getWidth() { return mWindowWidth; }
        int getHeight() { return mWindowHeight; }

#ifdef __WXMSW__
        bool SwapBuffers();
#endif
    protected:
      	DECLARE_EVENT_TABLE()

        int mWindowWidth;
        int mWindowHeight;
        int mWindowResized;
        bool mIsInitialized;

        virtual void InitializeGLCanvas() = 0;  // pure virtual method to initialize canvas
        void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
        void Resized(wxSizeEvent& evt);

    private:
        GL_CONTEXT_CLASS* m_context;
};

#endif // XLGLCANVAS_H
