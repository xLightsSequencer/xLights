#ifndef XLGLCANVAS_H
#define XLGLCANVAS_H

#include "wx/glcanvas.h"
#include "DrawGLUtils.h"


class xlGLCanvas
    : public wxGLCanvas
{
    public:
        xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                   const wxSize &size=wxDefaultSize,
                   long style=0,
                   const wxString &name=wxPanelNameStr,
                   bool coreProfile = false);
        virtual ~xlGLCanvas();

        void SetCurrentGLContext();

        int getWidth() { return mWindowWidth; }
        int getHeight() { return mWindowHeight; }

        double translateToBacking(double x);

    protected:
      	DECLARE_EVENT_TABLE()

        int mWindowWidth;
        int mWindowHeight;
        int mWindowResized;
        bool mIsInitialized;

        virtual void InitializeGLCanvas() = 0;  // pure virtual method to initialize canvas
        void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
        void Resized(wxSizeEvent& evt);
        void OnEraseBackGround(wxEraseEvent& event) {};

    private:
        wxGLContext* m_context;
        DrawGLUtils::xlGLCacheInfo *cache;
};

#endif // XLGLCANVAS_H
