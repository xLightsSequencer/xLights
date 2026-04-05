#pragma once


#include "osxUtils/wxMetalCanvas.hpp"
#include "../../../graphics/xlGraphicsContext.h"
#include "../../../graphics/metal/IMetalCanvas.h"

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

    // IMetalCanvas interface — non-ObjC methods
    std::string getName() const override { return wxMetalCanvas::getName(); }
    bool usesMSAA() override { return wxMetalCanvas::usesMSAA(); }
    bool RequiresDepthBuffer() const override { return wxMetalCanvas::RequiresDepthBuffer(); }

#ifdef __OBJC__
    // IMetalCanvas interface — ObjC methods (delegate to MetalDeviceManager via wxMetalCanvas)
    id<MTLDevice> getMTLDevice() override { return wxMetalCanvas::getMTLDevice(); }
    id<MTLCommandQueue> getMTLCommandQueue() override { return wxMetalCanvas::getMTLCommandQueue(); }
    id<MTLCommandQueue> getBltCommandQueue() override { return wxMetalCanvas::getBltCommandQueue(); }
    id<MTLLibrary> getMTLLibrary() override { return wxMetalCanvas::getMTLLibrary(); }
    int getMSAASampleCount() override { return wxMetalCanvas::getMSAASampleCount(); }
    id<MTLDepthStencilState> getDepthStencilStateLE() override { return wxMetalCanvas::getDepthStencilStateLE(); }
    id<MTLDepthStencilState> getDepthStencilStateL() override { return wxMetalCanvas::getDepthStencilStateL(); }
    id<MTLRenderPipelineState> getPipelineState(const std::string& name,
                                                const char* vShader,
                                                const char* fShader,
                                                bool blending) override {
        return wxMetalCanvas::getPipelineState(name, vShader, fShader, blending);
    }
    void addToSyncPoint(id<MTLCommandBuffer>& buffer, id<CAMetalDrawable>& drawable) override {
        wxMetalCanvas::addToSyncPoint(buffer, drawable);
    }
    id<CAMetalDrawable> getNextDrawable() override;
    id<MTLTexture> getMSAATexture(int w, int h) override;
    id<MTLTexture> getDepthTexture(int w, int h) override;
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
};

