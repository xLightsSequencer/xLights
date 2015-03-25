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

#ifdef __WXMSW__
        bool SwapBuffers();
#endif
    protected:
    private:
        GL_CONTEXT_CLASS* m_context;
};

#endif // XLGLCANVAS_H
