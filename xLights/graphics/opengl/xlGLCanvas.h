#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/glcanvas.h"
#include "../xlGraphicsContext.h"


class wxImage;

extern "C" {
   struct AVFrame;
}

class xlGLCanvas
    : public wxGLCanvas
{
    public:
        xlGLCanvas(wxWindow* parent,
                   wxWindowID id,
                   const wxPoint &pos=wxDefaultPosition,
                   const wxSize &size=wxDefaultSize,
                   long style=0,
                   const wxString &name=wxPanelNameStr,
                   bool only2d = true);
        xlGLCanvas(wxWindow* parent,
                   const wxGLAttributes& dispAttrs,
                   const wxString &name = wxPanelNameStr);
        virtual ~xlGLCanvas();

        void SetCurrentGLContext();

        const std::string &getName() const { return _name; }
    
        int getWidth() const { return mWindowWidth; }
        int getHeight() const { return mWindowHeight; }

        double translateToBacking(double x) const;
        double mapLogicalToAbsolute(double x) const;


        void DisplayWarning(const wxString &msg);

		// Grab a copy of the front buffer (at window dimensions by default); it's the
		// caller's responsibility to delete the image when done with it
		wxImage *GrabImage( wxSize size = wxSize(0,0) );
        void captureNextFrame(int w, int h) {}
        bool getFrameForExport(int w, int h, AVFrame *, uint8_t *buffer, int bufferSize);

        virtual void render() {};
    
        int GetZDepth() const { return m_zDepth;}
        bool IsCoreProfile() const { return isCoreProfile;}
        static wxGLContext *GetSharedContext() { return m_sharedContext; }

        virtual xlColor ClearBackgroundColor() const { return xlBLACK; }
        virtual bool drawingUsingLogicalSize() const { return true; }


        virtual void PrepareCanvas();
        virtual xlGraphicsContext* PrepareContextForDrawing();
        virtual xlGraphicsContext* PrepareContextForDrawing(const xlColor &bg);
        virtual void FinishDrawing(xlGraphicsContext* ctx, bool display = true);
        void Resized(wxSizeEvent& evt);

        virtual bool RequiresDepthBuffer() const { return false; }

    
        bool bindVertexArrayID(GLuint pid);
    protected:
      	DECLARE_EVENT_TABLE()

        size_t mWindowWidth;
        size_t mWindowHeight;
        int mWindowResized;
        bool mIsInitialized;

        virtual void InitializeGLCanvas() { mIsInitialized = true; };
        virtual void InitializeGLContext() {}
        void OnEraseBackGround(wxEraseEvent& event) {};

        void CreateGLContext();

        bool is3d = false;
    private:
        std::string _name;
        wxGLContext* m_context = nullptr;
        int  m_zDepth = 0;
        bool isCoreProfile = false;
        std::map<GLuint, GLuint> vertexArrayIds;
    
        static wxGLContext *m_sharedContext;
};
