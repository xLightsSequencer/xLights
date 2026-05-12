//
//  xlMetalCanvas.cpp
//  xLights-macOSLib
//
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <Accelerate/Accelerate.h>

#include "xlMetalCanvas.h"
#include "graphics/metal/xlMetalGraphicsContext.h"

#include "utils/ExternalHooks.h"

#include "wx/osx/private.h"
#include "../xlGraphicsBase.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

BEGIN_EVENT_TABLE(xlMetalCanvas, wxMetalCanvas)
    EVT_SIZE(xlMetalCanvas::Resized)
    EVT_ERASE_BACKGROUND(xlMetalCanvas::OnEraseBackGround)  // Override to do nothing on this event
END_EVENT_TABLE()


class MSAATextureInfo {
public:
    MSAATextureInfo(xlMetalCanvas *c) : canvas(c) {
        CAMetalLayer *layer = (CAMetalLayer *)[c->getMTKView() layer];
        width = [layer drawableSize].width;
        height = [layer drawableSize].height;
    }
    ~MSAATextureInfo() {
        if (msaaTexture != nil) {
            [msaaTexture release];
        }
    }
    
    bool supportsMemoryless() {
        if (@available(macOS 11.0, *)) {
            return [canvas->getMTLDevice() respondsToSelector: @selector(supportsFamily:)]
                && [canvas->getMTLDevice() supportsFamily: MTLGPUFamilyApple5];
        } else {
            return false;
        }
    }

    id<MTLTexture> getMSAATexture() {
        if (msaaTexture == nil) {
            MTLTextureDescriptor * msaaDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                                              width:width
                                                                                                             height:height
                                                                                                          mipmapped:NO];
            msaaDesc.usage = MTLTextureUsageRenderTarget;
            // TBDR GPUs (Apple Silicon — M1+ on Mac, every iOS device)
            // can keep the MSAA samples in tile memory only and resolve
            // them straight into the drawable at end-of-pass — no
            // system-memory backing required. The render pass already
            // uses `loadAction = Clear` + `storeAction = MultisampleResolve`,
            // which are the prerequisites for memoryless. Saves the
            // full MSAA surface size × triple buffering on every M-series
            // Mac. Intel Macs aren't TBDR, so fall back to Private there.
            if (@available(macOS 11.0, *)) {
                msaaDesc.storageMode = supportsMemoryless()
                    ? MTLStorageModeMemoryless
                    : MTLStorageModePrivate;
            } else {
                msaaDesc.storageMode = MTLStorageModePrivate;
            }
            msaaDesc.textureType = MTLTextureType2DMultisample;
            msaaDesc.sampleCount = canvas->getMSAASampleCount();
            msaaTexture = [canvas->getMTLDevice() newTextureWithDescriptor:msaaDesc];
            std::string n = canvas->getName() + "-MSAATexture";
            NSString *ns = [NSString stringWithUTF8String:n.c_str()];
            [msaaTexture setLabel:ns];
        }
        return msaaTexture;
    }

    int width;
    int height;
    id<MTLTexture> msaaTexture = nil;
    xlMetalCanvas *canvas;
};

class DepthTextureInfo {
public:
    DepthTextureInfo(xlMetalCanvas *c) : canvas(c) {
        CAMetalLayer *layer = (CAMetalLayer *)[c->getMTKView() layer];
        width = [layer drawableSize].width;
        height = [layer drawableSize].height;
    }
    ~DepthTextureInfo() {
        if (depthTexture != nil) {
            [depthTexture release];
        }
    }

    bool supportsMemoryless() {
        if (@available(macOS 11.0, *)) {
            return [canvas->getMTLDevice() respondsToSelector: @selector(supportsFamily:)]
                && [canvas->getMTLDevice() supportsFamily: MTLGPUFamilyApple5];
        } else {
            return false;
        }
    }
    
    id<MTLTexture> getDepthTexture() {
        if (depthTexture == nil) {
            MTLTextureDescriptor * depthBufferDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                              width:width
                                                                                                             height:height
                                                                                                          mipmapped:NO];
            depthBufferDescriptor.usage = MTLTextureUsageRenderTarget;
            depthBufferDescriptor.storageMode = MTLStorageModePrivate;

            if (supportsMemoryless()) {
                if (@available(macOS 11.0, *)) {
                    depthBufferDescriptor.storageMode = MTLStorageModeMemoryless;
                }
            }
            depthBufferDescriptor.textureType = MTLTextureType2DMultisample;
            depthBufferDescriptor.sampleCount = canvas->getMSAASampleCount();
            depthTexture = [canvas->getMTLDevice() newTextureWithDescriptor:depthBufferDescriptor];
            std::string n = canvas->getName() + "-DepthBuffer";
            NSString *ns = [NSString stringWithUTF8String:n.c_str()];
            [depthTexture setLabel:ns];
        }
        return depthTexture;
    }
    
    int width;
    int height;
    id<MTLTexture> depthTexture = nil;
    xlMetalCanvas *canvas;
};

// Concrete IMetalCanvasDelegate that forwards to xlMetalCanvas/wxMetalCanvas.
// Only compiled in ObjC++ (.mm), so ObjC types are fine here.
class xlMetalCanvasDelegate : public IMetalCanvasDelegate {
public:
    explicit xlMetalCanvasDelegate(xlMetalCanvas *c) : canvas(c) {}

    id<MTLDevice> getMTLDevice() override { return canvas->wxMetalCanvas::getMTLDevice(); }
    id<MTLCommandQueue> getMTLCommandQueue() override { return canvas->wxMetalCanvas::getMTLCommandQueue(); }
    id<MTLCommandQueue> getBltCommandQueue() override { return canvas->wxMetalCanvas::getBltCommandQueue(); }
    id<MTLLibrary> getMTLLibrary() override { return canvas->wxMetalCanvas::getMTLLibrary(); }
    int getMSAASampleCount() override { return canvas->wxMetalCanvas::getMSAASampleCount(); }
    id<MTLDepthStencilState> getDepthStencilStateLE() override { return canvas->wxMetalCanvas::getDepthStencilStateLE(); }
    id<MTLDepthStencilState> getDepthStencilStateL() override { return canvas->wxMetalCanvas::getDepthStencilStateL(); }
    id<MTLRenderPipelineState> getPipelineState(const std::string& name,
                                                const char* vShader,
                                                const char* fShader,
                                                bool blending) override {
        return canvas->wxMetalCanvas::getPipelineState(name, vShader, fShader, blending);
    }
    void addToSyncPoint(id<MTLCommandBuffer>& buffer, id<CAMetalDrawable>& drawable) override {
        canvas->wxMetalCanvas::addToSyncPoint(buffer, drawable);
    }
    id<CAMetalDrawable> getNextDrawable() override {
        MTKView *view = canvas->getMTKView();
        if (view == nil || [view window] == nil) {
            return nil;
        }
        CAMetalLayer *layer = (CAMetalLayer *)[view layer];
        if (layer == nil || layer.drawableSize.width == 0 || layer.drawableSize.height == 0) {
            return nil;
        }
        return [layer nextDrawable];
    }
    id<MTLTexture> getMSAATexture(int w, int h) override { return canvas->getMSAATexture(w, h); }
    id<MTLTexture> getDepthTexture(int w, int h) override { return canvas->getDepthTexture(w, h); }
private:
    xlMetalCanvas *canvas;
};

void* xlMetalCanvas::getMetalDelegate() {
    if (metalDelegate_ == nullptr) {
        metalDelegate_ = new xlMetalCanvasDelegate(this);
    }
    return metalDelegate_;
}

xlMetalCanvas::xlMetalCanvas(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name,
                             bool only2d)
    : wxMetalCanvas(parent, id, pos, size, style, name, only2d),
mWindowWidth(0),
mWindowHeight(0),
mWindowResized(false),
mIsInitialized(false),
mName(name)
{
    //triple buffer the extra textures to avoid contention
    for (int x = 0; x < 3; x++) {
        msaaTextures[x] = nullptr;
        depthTextures[x] = nullptr;
    }
}
class CaptureBufferInfo {
public:
    CaptureBufferInfo(int w, int h) : width(w), height(h) {
        int bytesPerRow = w * 4;
        int bufferSize = bytesPerRow * h;
        auto dev = MetalDeviceManager::instance().getMTLDevice();
        buffer = [[dev newBufferWithLength:bufferSize options:MTLResourceStorageModeShared] retain];

        MTLTextureDescriptor *description = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                               width:w
                                                                                              height:h
                                                                                           mipmapped:false];
        description.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
        target = [dev newTextureWithDescriptor:description];
    }
    ~CaptureBufferInfo() {
        if (buffer != nil) {
            [buffer release];
        }
        if (target != nil) {
            [target release];
        }
    }
    
    int width = 0;
    int height = 0;
    id<MTLBuffer> buffer = nil;
    id<MTLTexture> target = nil;
    bool captureNext = false;
};

xlMetalCanvas::~xlMetalCanvas() {
    delete static_cast<xlMetalCanvasDelegate*>(metalDelegate_);
    if (captureBuffer) {
        delete captureBuffer;
    }
    for (int x = 0; x < 3; x++) {
        if (msaaTextures[x]) {
            delete msaaTextures[x];
        }
        if (depthTextures[x]) {
            delete depthTextures[x];
        }
    }
}
xlColor xlMetalCanvas::ClearBackgroundColor() const {
    return xlBLACK;
}
bool xlMetalCanvas::drawingUsingLogicalSize() const {
    return false;
}

double xlMetalCanvas::translateToBacking(double x) const {
    return GetDPIScaleFactor() * x;
}
double xlMetalCanvas::mapLogicalToAbsolute(double x) const {
    if (drawingUsingLogicalSize()) {
        return x;
    }
    return translateToBacking(x);
}


void xlMetalCanvas::Resized(wxSizeEvent& evt)
{
    if (drawingUsingLogicalSize()) {
        mWindowWidth = evt.GetSize().GetWidth();
        mWindowHeight = evt.GetSize().GetHeight();
    } else {
        mWindowWidth = evt.GetSize().GetWidth() * GetContentScaleFactor();
        mWindowHeight = evt.GetSize().GetHeight() * GetContentScaleFactor();
    }
    mWindowResized = true;
    for (int x = 0; x < 3; x++) {
        if (msaaTextures[x]) {
            delete msaaTextures[x];
            msaaTextures[x] = nullptr;
        }
        if (depthTextures[x]) {
            delete depthTextures[x];
            depthTextures[x] = nullptr;
        }
    }
    Refresh();
}

void xlMetalCanvas::PrepareCanvas() {
    if (!mIsInitialized) {
        MTKView *view = getMTKView();
        if (view == nil || [view window] == nil) {
            return;
        }
        //just make sure this will load
        getPipelineState("singleColorProgram", "singleColorVertexShader", "colorFragmentShader", false);
        mIsInitialized = true;
    }
}
xlGraphicsContext * xlMetalCanvas::PrepareContextForDrawing() {
    MTKView *view = getMTKView();
    if (view == nil || [view window] == nil) {
        return nullptr;
    }
    CAMetalLayer *layer = (CAMetalLayer *)[view layer];
    if (layer != nil) {
        int drawW = (int)round(layer.drawableSize.width);
        int drawH = (int)round(layer.drawableSize.height);
        if (drawW > 0 && drawH > 0 && (drawW != mWindowWidth || drawH != mWindowHeight)) {
            mWindowWidth = drawW;
            mWindowHeight = drawH;
            mWindowResized = true;
        }
    }
    xlMetalGraphicsContext *ret = new xlMetalGraphicsContext(this, captureBuffer == nullptr || !captureBuffer->captureNext ? nil : captureBuffer->target, !firstDraw);
    if (!ret->isValid()) {
        delete ret;
        return nullptr;
    }
    firstDraw = false;
    return ret;
}
void xlMetalCanvas::FinishDrawing(xlGraphicsContext *ctx, bool display) {
    @autoreleasepool {
        xlMetalGraphicsContext *mgx = (xlMetalGraphicsContext*)ctx;
        if (captureBuffer && captureBuffer->captureNext) {
            mgx->Commit(false, captureBuffer->buffer);
        } else {
            mgx->Commit(display, nil);
        }
        if (captureBuffer) {
            captureBuffer->captureNext = false;
        }
        delete mgx;
    }
}

id<MTLTexture> xlMetalCanvas::getMSAATexture(int w, int h) {
    curMSAATexture++;
    if (curMSAATexture == 3) {
        curMSAATexture = 0;
    }
    if (msaaTextures[curMSAATexture] != nullptr) {
        if (msaaTextures[curMSAATexture]->width != w || msaaTextures[curMSAATexture]->height != h) {
            delete msaaTextures[curMSAATexture];
            msaaTextures[curMSAATexture] = nullptr;
        }
    }
    if (msaaTextures[curMSAATexture] == nullptr) {
        msaaTextures[curMSAATexture] = new MSAATextureInfo(this);
    }
    return msaaTextures[curMSAATexture]->getMSAATexture();
}
id<MTLTexture> xlMetalCanvas::getDepthTexture(int w, int h) {
    curDepthTexture++;
    if (curDepthTexture == 3) {
        curDepthTexture = 0;
    }
    if (depthTextures[curDepthTexture] != nullptr) {
        if (depthTextures[curDepthTexture]->width != w || depthTextures[curDepthTexture]->height != h) {
            delete depthTextures[curDepthTexture];
            depthTextures[curDepthTexture] = nullptr;
        }
    }
    if (depthTextures[curDepthTexture] == nullptr) {
        depthTextures[curDepthTexture] = new DepthTextureInfo(this);
    }
    return depthTextures[curDepthTexture]->getDepthTexture();
}


void xlMetalCanvas::captureNextFrame(int w, int h) {
    if (captureBuffer && (captureBuffer->width != w || captureBuffer->height != h)) {
        delete captureBuffer;
        captureBuffer = nullptr;
    }
    if (captureBuffer == nullptr) {
        captureBuffer = new CaptureBufferInfo(w, h);
    }
    captureBuffer->captureNext = true;
}


extern void VideoToolboxCreateFrame(CIImage *image, AVFrame *f, id<MTLDevice> d);

bool xlMetalCanvas::getFrameForExport(int w, int h, AVFrame *f, uint8_t *buffer, int bufferSize) {
    if (captureBuffer == nullptr || captureBuffer->buffer == nil) {
        return true;
    }
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    if (f->format == AV_PIX_FMT_VIDEOTOOLBOX) {
        @autoreleasepool {
            NSMutableDictionary *dict = [[NSMutableDictionary alloc]init];
            [dict setObject:(__bridge id)colorSpace  forKey:kCIImageColorSpace];
            
            CIImage *image = [CIImage imageWithMTLTexture:captureBuffer->target options:dict];
            CIImage *i2 = [image imageByApplyingCGOrientation:kCGImagePropertyOrientationDownMirrored];
                        
            VideoToolboxCreateFrame(i2, f, getMTLDevice());
            
            [dict release];
        }
        return false;
    }
    // Fail fast if the caller-provided RGB buffer is too small.
    size_t requiredSize = (size_t)w * (size_t)h * 3;
    if (buffer == nullptr || bufferSize < 0 || (size_t)bufferSize < requiredSize) {
        return false;
    }

    // SIMD-accelerated BGRA -> RGB conversion via Accelerate.framework.
    vImage_Buffer srcBuf = { captureBuffer->buffer.contents, (vImagePixelCount)h, (vImagePixelCount)w, (size_t)w * 4 };
    vImage_Buffer dstBuf = { buffer, (vImagePixelCount)h, (vImagePixelCount)w, (size_t)w * 3 };
    vImage_Error err = vImageConvert_BGRA8888toRGB888(&srcBuf, &dstBuf, kvImageNoFlags);
    if (err == kvImageNoError) {
        return true;
    }

    // Fall back to a scalar BGRA -> RGB conversion if Accelerate rejects the buffers.
    uint8_t *src = (uint8_t *)captureBuffer->buffer.contents;
    uint8_t *dst = buffer;
    for (int x = 0; x < w * h; x++, src += 4, dst += 3) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
    }
    return true;
}



wxImage *xlMetalCanvas::GrabImage(wxSize size) {
    int w = GetContentScaleFactor() * getWidth();
    int h = GetContentScaleFactor() * getHeight();
    captureNextFrame(w, h);
    render();
    return GrabCapturedImage();
}

wxImage *xlMetalCanvas::GrabCapturedImage() {
    if (captureBuffer == nullptr || captureBuffer->buffer == nil) {
        return nullptr;
    }
    uint8_t *src = (uint8_t*)captureBuffer->buffer.contents;  ///BGRA
    uint8_t *dest = (uint8_t*)malloc(captureBuffer->width * captureBuffer->height * 3);
    uint8_t *dst = dest;
    for (int x = 0; x < captureBuffer->width * captureBuffer->height; x++, src += 4, dst += 3) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
    }
    wxImage *img = new wxImage(captureBuffer->width, captureBuffer->height);
    img->SetData(dest);
    return img;
}
