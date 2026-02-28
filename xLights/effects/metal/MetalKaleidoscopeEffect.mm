#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"
#include "../KaleidoscopeEffect.h"
#include <cmath>

static inline double toRadiansLocal(double deg) {
    return deg * M_PI / 180.0;
}

class MetalKaleidoscopeEffectData {
public:
    MetalKaleidoscopeEffectData() {
        fnSquare2   = MetalComputeUtilities::INSTANCE.FindComputeFunction("KaleidoscopeEffectSquare2");
        fnRadial    = MetalComputeUtilities::INSTANCE.FindComputeFunction("KaleidoscopeEffectRadial");
        fnTriangle  = MetalComputeUtilities::INSTANCE.FindComputeFunction("KaleidoscopeEffectTriangle");
    }
    ~MetalKaleidoscopeEffectData() {
        if (fnSquare2)  { [fnSquare2 release];  }
        if (fnRadial)   { [fnRadial release];   }
        if (fnTriangle) { [fnTriangle release]; }
    }

    bool canRenderType(const std::string& type) {
        if (type == "Square 2") return fnSquare2 != nil;
        if (type == "Radial")   return fnRadial != nil;
        if (type == "6-Fold" || type == "8-Fold" || type == "12-Fold") return fnTriangle != nil;
        return false;
    }

    bool Render(const std::string& type, KaleidoscopeData &kdata, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
            if (!rbcd) return false;

            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) return false;

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) return false;

            // All kaleidoscope types read from canvas and write back — need a source copy
            id<MTLBuffer> bufferCopy = rbcd->getPixelBufferCopy();
            {
                id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
                [blitEncoder setLabel:@"KaleidoscopeCopySource"];
                [blitEncoder copyFromBuffer:bufferResult
                               sourceOffset:0
                                   toBuffer:bufferCopy
                          destinationOffset:0
                                       size:(kdata.width * kdata.height * 4)];
                [blitEncoder endEncoding];
            }

            id<MTLComputePipelineState> fn = nil;
            if (type == "Square 2")  fn = fnSquare2;
            else if (type == "Radial") fn = fnRadial;
            else fn = fnTriangle;   // 6-Fold, 8-Fold, 12-Fold

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"KaleidoscopeEffect"];
            [computeEncoder setComputePipelineState:fn];

            [computeEncoder setBytes:&kdata length:sizeof(kdata) atIndex:0];
            [computeEncoder setBuffer:bufferCopy   offset:0 atIndex:1];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:2];

            int w = fn.threadExecutionWidth;
            int h = fn.maxTotalThreadsPerThreadgroup / w;
            MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
            MTLSize threadsPerGrid = MTLSizeMake(kdata.width, kdata.height, 1);

            [computeEncoder dispatchThreads:threadsPerGrid
                      threadsPerThreadgroup:threadsPerThreadgroup];
            [computeEncoder endEncoding];
        }
        return true;
    }

private:
    id<MTLComputePipelineState> fnSquare2   = nil;
    id<MTLComputePipelineState> fnRadial    = nil;
    id<MTLComputePipelineState> fnTriangle  = nil;
};


MetalKaleidoscopeEffect::MetalKaleidoscopeEffect(int i) : KaleidoscopeEffect(i) {
    data = new MetalKaleidoscopeEffectData();
}

MetalKaleidoscopeEffect::~MetalKaleidoscopeEffect() {
    if (data) {
        delete data;
    }
}

void MetalKaleidoscopeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    std::string type = SettingsMap.Get("CHOICE_Kaleidoscope_Type", "Triangle");

    // Only GPU-accelerate the "new" types — Square 2, 6-Fold, 8-Fold, 12-Fold, Radial
    if (type != "Square 2" && type != "6-Fold" && type != "8-Fold" && type != "12-Fold" && type != "Radial") {
        KaleidoscopeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRenderType(type) || (buffer.BufferWi * buffer.BufferHt) < 2048) {
        KaleidoscopeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float progress = buffer.GetEffectTimeIntervalPosition(1.f);

    int xCentreP = GetValueCurveInt("Kaleidoscope_X", 50, SettingsMap, progress, KALEIDOSCOPE_X_MIN, KALEIDOSCOPE_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yCentreP = GetValueCurveInt("Kaleidoscope_Y", 50, SettingsMap, progress, KALEIDOSCOPE_Y_MIN, KALEIDOSCOPE_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xCentre  = xCentreP * buffer.BufferWi / 100;
    int yCentre  = yCentreP * buffer.BufferHt / 100;
    int size     = GetValueCurveInt("Kaleidoscope_Size", 5, SettingsMap, progress, KALEIDOSCOPE_SIZE_MIN, KALEIDOSCOPE_SIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int rotation = GetValueCurveInt("Kaleidoscope_Rotation", 0, SettingsMap, progress, KALEIDOSCOPE_ROTATION_MIN, KALEIDOSCOPE_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    double rotRad = toRadiansLocal((double)rotation);

    KaleidoscopeData kdata;
    kdata.width  = buffer.BufferWi;
    kdata.height = buffer.BufferHt;
    kdata.cx     = (float)xCentre;
    kdata.cy     = (float)yCentre;
    kdata.rotRad = (float)rotRad;

    if (type == "Square 2") {
        kdata.style   = KALEIDOSCOPE_STYLE_SQUARE2;
        kdata.size    = (float)size / 2.0f;   // halfSize
        kdata.maxIter = 0;
    } else if (type == "Radial") {
        kdata.style   = KALEIDOSCOPE_STYLE_RADIAL;
        kdata.size    = (float)std::max(2, size);  // number of segments
        kdata.maxIter = 0;
    } else {
        // Triangle types: compute vertices on CPU and send to GPU
        int styleEnum;
        if (type == "6-Fold")       styleEnum = KALEIDOSCOPE_STYLE_6FOLD;
        else if (type == "8-Fold")  styleEnum = KALEIDOSCOPE_STYLE_8FOLD;
        else                        styleEnum = KALEIDOSCOPE_STYLE_12FOLD;

        kdata.style = styleEnum;

        KaleidoscopeTriangle tri = KaleidoscopeEffect::ComputeTriangle(type, (double)xCentre, (double)yCentre, (double)size, rotRad);
        kdata.v[0].x = (float)tri.v[0].x;
        kdata.v[0].y = (float)tri.v[0].y;
        kdata.v[1].x = (float)tri.v[1].x;
        kdata.v[1].y = (float)tri.v[1].y;
        kdata.v[2].x = (float)tri.v[2].x;
        kdata.v[2].y = (float)tri.v[2].y;

        int maxDim  = std::max(buffer.BufferWi, buffer.BufferHt);
        int maxIter = std::max(50, (maxDim * 3) / std::max(size, 1));
        if (maxIter > 500) maxIter = 500;
        kdata.maxIter = maxIter;
        kdata.size    = (float)size;
    }

    if (data->Render(type, kdata, buffer)) {
        return;
    }

    // GPU render failed — fall back to CPU
    KaleidoscopeEffect::Render(effect, SettingsMap, buffer);
}
