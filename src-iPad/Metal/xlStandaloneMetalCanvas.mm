/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "xlStandaloneMetalCanvas.h"
#include "osxUtils/MetalDeviceManager.h"

// --- Triple-buffered MSAA texture ---
class StandaloneMSAATextureInfo {
public:
    StandaloneMSAATextureInfo(int w, int h, int sampleCount)
        : width(w), height(h), sampleCount(sampleCount) {}

    ~StandaloneMSAATextureInfo() {
        if (texture != nil) {
            [texture release];
        }
    }

    id<MTLTexture> getTexture() {
        if (texture == nil) {
            auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                          width:width
                                                                         height:height
                                                                      mipmapped:NO];
            desc.usage = MTLTextureUsageRenderTarget;
            desc.storageMode = MTLStorageModePrivate;
            desc.textureType = MTLTextureType2DMultisample;
            desc.sampleCount = sampleCount;
            texture = [MetalDeviceManager::instance().getMTLDevice() newTextureWithDescriptor:desc];
        }
        return texture;
    }

    int width, height, sampleCount;
    id<MTLTexture> texture = nil;
};

// --- Triple-buffered depth texture ---
class StandaloneDepthTextureInfo {
public:
    StandaloneDepthTextureInfo(int w, int h, int sampleCount)
        : width(w), height(h), sampleCount(sampleCount) {}

    ~StandaloneDepthTextureInfo() {
        if (texture != nil) {
            [texture release];
        }
    }

    id<MTLTexture> getTexture() {
        if (texture == nil) {
            auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                          width:width
                                                                         height:height
                                                                      mipmapped:NO];
            desc.usage = MTLTextureUsageRenderTarget;
            // iPad GPUs support memoryless — ideal for transient depth
            desc.storageMode = MTLStorageModeMemoryless;
            desc.textureType = MTLTextureType2DMultisample;
            desc.sampleCount = sampleCount;
            texture = [MetalDeviceManager::instance().getMTLDevice() newTextureWithDescriptor:desc];
        }
        return texture;
    }

    int width, height, sampleCount;
    id<MTLTexture> texture = nil;
};

// --- IMetalCanvasDelegate implementation ---
class StandaloneMetalCanvasDelegate : public IMetalCanvasDelegate {
public:
    explicit StandaloneMetalCanvasDelegate(xlStandaloneMetalCanvas* c) : canvas(c) {
        for (int i = 0; i < 3; i++) {
            msaaTextures[i] = nullptr;
            depthTextures[i] = nullptr;
        }
    }

    ~StandaloneMetalCanvasDelegate() override {
        for (int i = 0; i < 3; i++) {
            delete msaaTextures[i];
            delete depthTextures[i];
        }
    }

    // Shared Metal resources — delegate to MetalDeviceManager
    id<MTLDevice> getMTLDevice() override {
        return MetalDeviceManager::instance().getMTLDevice();
    }
    id<MTLCommandQueue> getMTLCommandQueue() override {
        return MetalDeviceManager::instance().getMTLCommandQueue();
    }
    id<MTLCommandQueue> getBltCommandQueue() override {
        return MetalDeviceManager::instance().getBltCommandQueue();
    }
    id<MTLLibrary> getMTLLibrary() override {
        return MetalDeviceManager::instance().getMTLLibrary();
    }
    int getMSAASampleCount() override {
        return MetalDeviceManager::instance().getMSAASampleCount();
    }
    id<MTLDepthStencilState> getDepthStencilStateLE() override {
        return MetalDeviceManager::instance().getDepthStencilStateLE();
    }
    id<MTLDepthStencilState> getDepthStencilStateL() override {
        return MetalDeviceManager::instance().getDepthStencilStateL();
    }

    id<MTLRenderPipelineState> getPipelineState(const std::string& name,
                                                const char* vShader,
                                                const char* fShader,
                                                bool blending) override {
        CAMetalLayer* layer = canvas->getMetalLayer();
        MTLPixelFormat fmt = layer ? layer.pixelFormat : MTLPixelFormatBGRA8Unorm;
        return MetalDeviceManager::instance().getPipelineState(
            name, vShader, fShader, blending,
            canvas->RequiresDepthBuffer(), canvas->usesMSAA(), fmt);
    }

    void addToSyncPoint(id<MTLCommandBuffer>& buffer, id<CAMetalDrawable>& drawable) override {
        if (drawable != nil) {
            [buffer presentDrawable:drawable];
        }
        [buffer commit];
    }

    id<CAMetalDrawable> getNextDrawable() override {
        CAMetalLayer* layer = canvas->getMetalLayer();
        if (layer == nil || layer.drawableSize.width == 0 || layer.drawableSize.height == 0) {
            return nil;
        }
        return [layer nextDrawable];
    }

    id<MTLTexture> getMSAATexture(int w, int h) override {
        curMSAA = (curMSAA + 1) % 3;
        if (msaaTextures[curMSAA] != nullptr &&
            (msaaTextures[curMSAA]->width != w || msaaTextures[curMSAA]->height != h)) {
            delete msaaTextures[curMSAA];
            msaaTextures[curMSAA] = nullptr;
        }
        if (msaaTextures[curMSAA] == nullptr) {
            msaaTextures[curMSAA] = new StandaloneMSAATextureInfo(w, h, getMSAASampleCount());
        }
        return msaaTextures[curMSAA]->getTexture();
    }

    id<MTLTexture> getDepthTexture(int w, int h) override {
        curDepth = (curDepth + 1) % 3;
        if (depthTextures[curDepth] != nullptr &&
            (depthTextures[curDepth]->width != w || depthTextures[curDepth]->height != h)) {
            delete depthTextures[curDepth];
            depthTextures[curDepth] = nullptr;
        }
        if (depthTextures[curDepth] == nullptr) {
            depthTextures[curDepth] = new StandaloneDepthTextureInfo(w, h, getMSAASampleCount());
        }
        return depthTextures[curDepth]->getTexture();
    }

private:
    xlStandaloneMetalCanvas* canvas;
    StandaloneMSAATextureInfo* msaaTextures[3];
    StandaloneDepthTextureInfo* depthTextures[3];
    int curMSAA = 0;
    int curDepth = 0;
};

// --- xlStandaloneMetalCanvas implementation ---

xlStandaloneMetalCanvas::xlStandaloneMetalCanvas(const std::string& name, bool is3d)
    : _name(name), _is3d(is3d) {
    MetalDeviceManager::instance().retain();
}

xlStandaloneMetalCanvas::~xlStandaloneMetalCanvas() {
    delete static_cast<StandaloneMetalCanvasDelegate*>(_delegate);
    _delegate = nullptr;
    MetalDeviceManager::instance().release();
}

double xlStandaloneMetalCanvas::translateToBacking(double x) const {
    return x * _scaleFactor;
}

void* xlStandaloneMetalCanvas::getMetalDelegate() {
    if (_delegate == nullptr) {
        _delegate = new StandaloneMetalCanvasDelegate(this);
    }
    return _delegate;
}

void xlStandaloneMetalCanvas::setMetalLayer(CAMetalLayer* layer) {
    _layer = layer;
    if (layer != nil) {
        layer.device = MetalDeviceManager::instance().getMTLDevice();
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        layer.framebufferOnly = YES;
        // Prevent nextDrawable from blocking forever
        layer.allowsNextDrawableTimeout = YES;
        // Use 3 drawables for triple buffering
        layer.maximumDrawableCount = 3;
    }
}
