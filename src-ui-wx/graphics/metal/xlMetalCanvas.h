#pragma once


#include "osxUtilsUI/wxMetalCanvas.hpp"
#include "graphics/xlGraphicsContext.h"
#include "graphics/metal/IMetalCanvas.h"

class MSAATextureInfo;
class DepthTextureInfo;
class CaptureBufferInfo;
extern "C" {
   struct AVFrame;
}
class xlMetalCanvas : public wxMetalCanvas, public IMetalCanvas {
public:
    xlMetalCanvas(wxWindow *parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = 0,
                  const wxString& name = "",
                  bool only2d = true);
    xlMetalCanvas(wxWindow *parent, const wxString& name) : xlMetalCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, name) {}

    virtual ~xlMetalCanvas();


    int getWidth() const { return mWindowWidth; }
    int getHeight() const { return mWindowHeight; }

    void Resized(wxSizeEvent& evt);
    void OnEraseBackGround(wxEraseEvent& event) {};

    xlColor ClearBackgroundColor() const override;

    double translateToBacking(double x) const override;
    double mapLogicalToAbsolute(double x) const;

    bool drawingUsingLogicalSize() const override;

    bool Is3D() { return is3d; }

    // IMetalCanvas interface
    std::string getName() const override { return wxMetalCanvas::getName(); }
    bool usesMSAA() override { return wxMetalCanvas::usesMSAA(); }
    bool RequiresDepthBuffer() const override { return wxMetalCanvas::RequiresDepthBuffer(); }
    void* getMetalDelegate() override;

#ifdef __OBJC__
    // Non-virtual ObjC accessors used by the delegate implementation
    id<MTLTexture> getMSAATexture(int w, int h);
    id<MTLTexture> getDepthTexture(int w, int h);
#endif

    void captureNextFrame(int w, int h);
    bool getFrameForExport(int w, int h, AVFrame *, uint8_t *buffer, int bufferSize);
    wxImage *GrabImage(wxSize size = wxSize(0,0));

    virtual void render() {};
protected:
    DECLARE_EVENT_TABLE()

    virtual void PrepareCanvas();
    virtual xlGraphicsContext* PrepareContextForDrawing();
    virtual void FinishDrawing(xlGraphicsContext *ctx, bool display = true);
    wxImage *GrabCapturedImage();



    size_t mWindowWidth;
    size_t mWindowHeight;
    int mWindowResized;
    bool mIsInitialized = false;
    bool firstDraw = true;
    wxString mName;

    MSAATextureInfo *msaaTextures[3] = {nullptr, nullptr, nullptr};
    int curMSAATexture = 0;
    DepthTextureInfo *depthTextures[3] = {nullptr, nullptr, nullptr};
    int curDepthTexture = 0;

    CaptureBufferInfo *captureBuffer = nullptr;
    void *metalDelegate_ = nullptr;
};
