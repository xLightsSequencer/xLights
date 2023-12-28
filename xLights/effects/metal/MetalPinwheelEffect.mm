
#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"

#include <array>

class MetalPinwheelEffectData {
public:
    MetalPinwheelEffectData() {
        for (auto &f : functions) {
            f = nil;
        }
        functions[0] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PinwheelEffectStyle0");
        //functions[1] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PinwheelEffectStyle0");
        //functions[2] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PinwheelEffectStyle0");
        //functions[3] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PinwheelEffectStyle0");
        //functions[4] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PinwheelEffectStyle0");
        //functions[5] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PinwheelEffectStyle0");
    }
    ~MetalPinwheelEffectData() {
        for (auto &f : functions) {
            if (f != nil) {
                [f release];
            }
        }
    }
    bool canRenderStyle(int style) {
        return style < functions.size() && functions[style] != nil;
    }

    bool Render(int idx, MetalPinwheelData &data, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) {
                return false;
            }
            
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
            [computeEncoder setLabel:@"PinwheelEffect"];
            [computeEncoder setComputePipelineState:functions[idx]];

            NSInteger dataSize = sizeof(data);
            [computeEncoder setBytes:&data length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = functions[idx].maxTotalThreadsPerThreadgroup;
            dataSize = data.width * data.height;
            NSInteger threads = std::min(dataSize, maxThreads);
            MTLSize gridSize = MTLSizeMake(dataSize, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);

            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];

            [computeEncoder endEncoding];
        }
        return true;
    }
    std::array<id<MTLComputePipelineState>, 11> functions;
};

MetalPinwheelEffect::MetalPinwheelEffect(int i) : PinwheelEffect(i) {
    data = new MetalPinwheelEffectData();
}
MetalPinwheelEffect::~MetalPinwheelEffect() {
    if (data) {
        delete data;
    }
}

void MetalPinwheelEffect::RenderNewArms(RenderBuffer& buffer, PinwheelEffect::PinwheelData &data) {
        
    MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || ((buffer.BufferWi * buffer.BufferHt) < 1024) || data.hasSpacial) {
        PinwheelEffect::RenderNewArms(buffer, data);
        return;
    }
    
    MetalPinwheelData rdata;
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
    rdata.pinwheel_arms = data.pinwheel_arms;
    rdata.xc_adj = data.xc_adj;
    rdata.yc_adj = data.yc_adj;
    rdata.degrees_per_arm = data.degrees_per_arm;
    rdata.pinwheel_twist = data.pinwheel_twist;
    rdata.max_radius= data.max_radius;
    rdata.poffset = data.poffset;
    rdata.pw3dType = data.pw3dType;
    rdata.pinwheel_rotation = data.pinwheel_rotation;
    rdata.tmax = data.tmax;
    rdata.pos = data.pos;
    rdata.allowAlpha = buffer.allowAlpha;

    rdata.numColors = data.colorsAsColor.size();
    for (int x = 0; x < rdata.numColors; x++) {
        rdata.colorsAsColor[x] = data.colorsAsColor[x].asChar4();
        rdata.colorsAsHSV[x] = {(float)data.colorsAsHSV[x].hue, (float)data.colorsAsHSV[x].saturation, (float)data.colorsAsHSV[x].value};
    }
    if (this->data->Render(0, rdata, buffer)) {
        return;
    }
    PinwheelEffect::RenderNewArms(buffer, data);
}
