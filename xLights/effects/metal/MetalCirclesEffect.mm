#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"
#include "../CirclesEffect.h"

class MetalCirclesEffectData {
public:
    MetalCirclesEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("CirclesEffect");
    }
    ~MetalCirclesEffectData() {
        if (fn) { [fn release]; }
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalCirclesData &cdata, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
            if (!rbcd) return false;

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) return false;

            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"CirclesEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(cdata);
            [computeEncoder setBytes:&cdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = cdata.width * cdata.height;
            NSInteger threads = std::min(pixelCount, maxThreads);
            MTLSize gridSize = MTLSizeMake(pixelCount, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);

            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];
            [computeEncoder endEncoding];
        }
        return true;
    }

private:
    id<MTLComputePipelineState> fn = nil;
};


MetalCirclesEffect::MetalCirclesEffect(int i) : CirclesEffect(i) {
    data = new MetalCirclesEffectData();
}
MetalCirclesEffect::~MetalCirclesEffect() {
    if (data) { delete data; }
}

void MetalCirclesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < 2048) {
        CirclesEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    int number      = GetValueCurveInt("Circles_Count", 3, SettingsMap, oset, CIRCLES_COUNT_MIN, CIRCLES_COUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int circleSpeed = GetValueCurveInt("Circles_Speed", 10, SettingsMap, oset, CIRCLES_SPEED_MIN, CIRCLES_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int radius      = GetValueCurveInt("Circles_Size",  5, SettingsMap, oset, CIRCLES_SIZE_MIN,  CIRCLES_SIZE_MAX,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    bool plasma   = SettingsMap.GetBool("CHECKBOX_Circles_Plasma",    false);
    bool radial   = SettingsMap.GetBool("CHECKBOX_Circles_Radial",    false);
    bool radial3D = SettingsMap.GetBool("CHECKBOX_Circles_Radial_3D", false);
    bool fade     = SettingsMap.GetBool("CHECKBOX_Circles_Linear_Fade", false);
    bool bubbles  = SettingsMap.GetBool("CHECKBOX_Circles_Bubbles",   false);
    bool bounce   = SettingsMap.GetBool("CHECKBOX_Circles_Bounce",    false);
    bool collide  = SettingsMap.GetBool("CHECKBOX_Circles_Collide",   false);

    // Bubbles uses DrawCircle(filled=false) — outline only, not easily GPU-invertible
    if (bubbles) {
        CirclesEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    size_t colorCnt = buffer.GetColorCount();
    if (colorCnt == 0) colorCnt = 1;
    if ((int)colorCnt > MAX_METAL_CIRCLES_COLORS) {
        CirclesEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    for (size_t i = 0; i < colorCnt; i++) {
        if (buffer.palette.IsSpatial(i)) {
            CirclesEffect::Render(effect, SettingsMap, buffer);
            return;
        }
    }
    if (number > MAX_METAL_CIRCLES_BALLS) number = MAX_METAL_CIRCLES_BALLS;

    MetalCirclesData cdata;
    cdata.width      = buffer.BufferWi;
    cdata.height     = buffer.BufferHt;
    cdata.colorCount = (int)colorCnt;
    cdata.allowAlpha = buffer.allowAlpha ? 1 : 0;
    cdata.numBalls   = number;
    cdata.wrap       = (!bounce && !collide) ? 1 : 0;

    // Populate palette colors
    for (int i = 0; i < (int)colorCnt; i++) {
        xlColor c;
        buffer.palette.GetColor(i, c);
        cdata.colorsAsRGBA[i] = c.asChar4();
        HSVValue hsv = c.asHSV();
        cdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (radial || radial3D) {
        // Pure math — no cache needed
        int start_x = buffer.BufferWi / 2;
        int start_y = buffer.BufferHt / 2;
        start_x += (int)(GetValueCurveInt("Circles_XC", 0, SettingsMap, oset, CIRCLES_POS_MIN, CIRCLES_POS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / float(CIRCLES_POS_MAX) * start_x);
        start_y += (int)(GetValueCurveInt("Circles_YC", 0, SettingsMap, oset, CIRCLES_POS_MIN, CIRCLES_POS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / float(CIRCLES_POS_MAX) * start_y);

        int effectState = (buffer.curPeriod - buffer.curEffStartPer) * circleSpeed * buffer.frameTimeInMs / 50;
        int barht   = buffer.BufferHt / (radius + 1);
        if (barht < 1) barht = 1;
        int maxRadius = effectState > buffer.BufferHt ? buffer.BufferHt : effectState / 2 + radius;
        int blockHt = (int)colorCnt * barht;
        int f_offset = effectState / 4 % (blockHt + 1);

        cdata.mode        = radial3D ? CIRCLES_MODE_RADIAL_3D : CIRCLES_MODE_RADIAL;
        cdata.cx          = start_x;
        cdata.cy          = start_y;
        cdata.barSize     = barht;
        cdata.blockSize   = blockHt > 0 ? blockHt : 1;
        cdata.f_offset    = f_offset;
        cdata.maxRadius   = maxRadius;
        cdata.number      = number;
        cdata.effectState = effectState;
    } else {
        // Update ball positions via the base class — no pixel drawing
        CirclesRenderCache *cache = UpdateCacheState(effect, SettingsMap, buffer);

        int actualBalls = std::min(cache->numBalls, MAX_METAL_CIRCLES_BALLS);
        cdata.numBalls = actualBalls;

        RgbBalls *src = plasma ? (RgbBalls*)cache->metaballs : cache->balls;
        for (int i = 0; i < actualBalls; i++) {
            cdata.balls[i].x        = src[i]._x;
            cdata.balls[i].y        = src[i]._y;
            cdata.balls[i].radius   = src[i]._radius;
            cdata.balls[i].colorIdx = src[i]._colorindex;
        }

        cdata.mode = plasma ? CIRCLES_MODE_METABALLS :
                     (fade  ? CIRCLES_MODE_FADING    : CIRCLES_MODE_REGULAR);
    }

    if (!data->Render(cdata, buffer)) {
        // GPU render failed — fall back to CPU
        CirclesEffect::Render(effect, SettingsMap, buffer);
    }
}
