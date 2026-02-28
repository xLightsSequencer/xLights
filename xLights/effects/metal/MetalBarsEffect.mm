#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"
#include "../BarsEffect.h"

class MetalBarsEffectData {
public:
    MetalBarsEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("BarsEffect");
    }
    ~MetalBarsEffectData() {
        if (fn) { [fn release]; }
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalBarsData &bdata, RenderBuffer &buffer) {
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
            [computeEncoder setLabel:@"BarsEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(bdata);
            [computeEncoder setBytes:&bdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = bdata.width * bdata.height;
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


MetalBarsEffect::MetalBarsEffect(int i) : BarsEffect(i) {
    data = new MetalBarsEffectData();
}
MetalBarsEffect::~MetalBarsEffect() {
    if (data) { delete data; }
}

void MetalBarsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < 2048) {
        BarsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float offset = buffer.GetEffectTimeIntervalPosition();
    int paletteRepeat = GetValueCurveInt("Bars_BarCount", 1, SettingsMap, offset, BARCOUNT_MIN, BARCOUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    double cycles = GetValueCurveDouble("Bars_Cycles", 1.0, SettingsMap, offset, BARCYCLES_MIN, BARCYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 10);
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    double center = GetValueCurveDouble("Bars_Center", 0, SettingsMap, position, BARCENTER_MIN, BARCENTER_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    const std::string &dirStr = SettingsMap["CHOICE_Bars_Direction"];
    int direction = 0;
    if      (dirStr == "up")             direction = 0;
    else if (dirStr == "down")           direction = 1;
    else if (dirStr == "expand")         direction = 2;
    else if (dirStr == "compress")       direction = 3;
    else if (dirStr == "Left")           direction = 4;
    else if (dirStr == "Right")          direction = 5;
    else if (dirStr == "H-expand")       direction = 6;
    else if (dirStr == "H-compress")     direction = 7;
    else if (dirStr == "Alternate Up")   direction = 8;
    else if (dirStr == "Alternate Down") direction = 9;
    else if (dirStr == "Alternate Left") direction = 10;
    else if (dirStr == "Alternate Right")direction = 11;
    else {
        // Custom Horz / Custom Vert — always spatial, fall back to CPU
        BarsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    bool highlight               = SettingsMap.GetBool("CHECKBOX_Bars_Highlight", false);
    bool useFirstColorForHighlight = highlight && SettingsMap.GetBool("CHECKBOX_Bars_UseFirstColorForHighlight", false);
    bool show3D                  = SettingsMap.GetBool("CHECKBOX_Bars_3D", false);
    bool gradient                = SettingsMap.GetBool("CHECKBOX_Bars_Gradient", false);

    size_t colorcnt = buffer.GetColorCount();
    if (colorcnt == 0) colorcnt = 1;

    if (highlight && useFirstColorForHighlight) {
        if (colorcnt == 1) {
            useFirstColorForHighlight = false;
        } else {
            colorcnt -= 1;
        }
    }

    // Fall back for spatial palette entries
    for (size_t i = 0; i < colorcnt; i++) {
        if (buffer.palette.IsSpatial(i)) {
            BarsEffect::Render(effect, SettingsMap, buffer);
            return;
        }
    }

    if ((int)colorcnt > MAX_METAL_BARS_COLORS) {
        BarsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int barCount = paletteRepeat * (int)colorcnt;
    if (barCount < 1) barCount = 1;

    MetalBarsData bdata;
    bdata.width     = buffer.BufferWi;
    bdata.height    = buffer.BufferHt;
    bdata.colorCount= (int)colorcnt;
    bdata.highlight = highlight ? 1 : 0;
    bdata.show3D    = show3D    ? 1 : 0;
    bdata.gradient  = gradient  ? 1 : 0;
    bdata.allowAlpha= buffer.allowAlpha ? 1 : 0;
    bdata.useFirstColorForHighlight = useFirstColorForHighlight ? 1 : 0;

    // Populate highlight color
    if (useFirstColorForHighlight) {
        xlColor hc;
        buffer.palette.GetColor(0, hc);
        bdata.highlightColor = hc.asChar4();
    } else {
        bdata.highlightColor = xlWHITE.asChar4();
    }

    // Populate palette colors (shifted by 1 if useFirstColorForHighlight)
    int colorOffset = useFirstColorForHighlight ? 1 : 0;
    for (int i = 0; i < (int)colorcnt; i++) {
        xlColor c;
        buffer.palette.GetColor(i + colorOffset, c);
        bdata.colorsAsRGBA[i] = c.asChar4();
        HSVValue hsv = c.asHSV();
        bdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    // Vertical directions (0-3, 8-9)
    if (direction < 4 || direction == 8 || direction == 9) {
        int barHt = (int)std::ceil((float)buffer.BufferHt / (float)barCount);
        if (barHt < 1) barHt = 1;
        int blockHt = (int)colorcnt * barHt;
        if (blockHt < 1) blockHt = 1;

        int f_offset = (int)(position * blockHt);
        if (direction == 8 || direction == 9) {
            f_offset = (int)(floor(position * barCount) * barHt);
        }
        // Remap alternates to base directions
        int mappedDir = (direction > 4) ? direction - 8 : direction;

        bdata.direction  = mappedDir;
        bdata.barSize    = barHt;
        bdata.blockSize  = blockHt;
        bdata.f_offset   = f_offset;
        bdata.newCenter  = buffer.BufferHt * (100 + (int)center) / 200;
    } else {
        // Horizontal directions (4-7, 10-11)
        int barWi = (int)std::ceil((float)buffer.BufferWi / (float)barCount);
        if (barWi < 1) barWi = 1;
        int blockWi = (int)colorcnt * barWi;
        if (blockWi < 1) blockWi = 1;

        int f_offset = (int)(position * blockWi);
        if (direction > 9) {
            f_offset = (int)(floor(position * barCount) * barWi);
        }
        // Remap alternates to base directions
        int mappedDir = (direction > 9) ? direction - 6 : direction;

        bdata.direction  = mappedDir;
        bdata.barSize    = barWi;
        bdata.blockSize  = blockWi;
        bdata.f_offset   = f_offset;
        bdata.newCenter  = buffer.BufferWi * (100 + (int)center) / 200;
    }

    if (data->Render(bdata, buffer)) {
        return;
    }
    BarsEffect::Render(effect, SettingsMap, buffer);
}
