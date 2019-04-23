#ifndef XLGLCANVAS_H
#define XLGLCANVAS_H

#include "wx/glcanvas.h"
#include "DrawGLUtils.h"

class wxImage;


class xlGLCanvas
    : public wxGLCanvas
{
    public:
        xlGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                   const wxSize &size=wxDefaultSize,
                   long style=0,
                   const wxString &name=wxPanelNameStr,
                   bool only2d = true);
        virtual ~xlGLCanvas();

        void SetCurrentGLContext();

        int getWidth() const { return mWindowWidth; }
        int getHeight() const { return mWindowHeight; }

        double translateToBacking(double x);

        void DisplayWarning(const wxString &msg);

		  // Grab a copy of the front buffer (at window dimensions by default); it's the
		  // caller's responsibility to delete the image when done with it
		  wxImage *GrabImage( wxSize size = wxSize(0,0) );

		  virtual void render( const wxSize& = wxSize(0,0) ) {};

		  class CaptureHelper
		  {
		  public:
			  // note: width & height without content-scale factor
			  CaptureHelper(int i_width, int i_height, double i_contentScaleFactor) : width(i_width), height(i_height), contentScaleFactor(i_contentScaleFactor), tmpBuf(nullptr) {};
			  virtual ~CaptureHelper();

			  bool ToRGB(unsigned char *buf, unsigned int bufSize, bool padToEvenDims=false);
		  protected:
			  const int width;
			  const int height;
			  const double contentScaleFactor;
			  unsigned char *tmpBuf;
		  };
    
        static wxGLContext *GetSharedContext() { return m_sharedContext; }
    protected:
      	DECLARE_EVENT_TABLE()

        size_t mWindowWidth;
        size_t mWindowHeight;
        int mWindowResized;
        bool mIsInitialized;

        virtual void InitializeGLCanvas() = 0;  // pure virtual method to initialize canvas
        void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
        void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
        void Resized(wxSizeEvent& evt);
        void OnEraseBackGround(wxEraseEvent& event) {};

        void CreateGLContext();


        virtual bool UsesVertexTextureAccumulator() {return true;}
        virtual bool UsesVertexColorAccumulator() {return true;}
        virtual bool UsesVertexAccumulator() {return true;}
        virtual bool UsesVertex3Accumulator() {return false;}
        virtual bool UsesVertex3ColorAccumulator() { return false; }
        virtual bool UsesVertex3TextureAccumulator() { return false; }
        virtual bool UsesAddVertex() {return true;}

        DrawGLUtils::xlGLCacheInfo *cache;

    private:
        wxString _name;
        wxGLContext* m_context;
        bool m_coreProfile;
    
        static wxGLContext *m_sharedContext;
};

#endif // XLGLCANVAS_H
