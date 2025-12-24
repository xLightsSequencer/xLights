
#include "MetalComputeUtilities.hpp"
#include "../../PixelBuffer.h"
#include "../../RenderBuffer.h"

#include <MetalPerformanceShaders/MetalPerformanceShaders.h>

#include "MetalEffectDataTypes.h"
#include "DissolveTransitionPattern.h"

MetalComputeUtilities MetalComputeUtilities::INSTANCE;


inline void setLabel(id<MTLComputeCommandEncoder> enc, const std::string &s, int layer = -1) {
#ifdef DEBUG
    std::string s2 = s;
    if (layer != -1) {
        s2 += "-";
        s2 += std::to_string(layer);
    }
    NSString* mn = [NSString stringWithUTF8String:s2.c_str()];
    [enc setLabel:mn];
#endif
}
inline void setLabel(id<MTLBuffer> buf, const std::string &s) {
#ifdef DEBUG
    NSString* mn = [NSString stringWithUTF8String:s.c_str()];
    [buf setLabel:mn];
#endif
}

MetalPixelBufferComputeData::MetalPixelBufferComputeData() {
    sparkleBuffer = nil;
    tmpBufferBlend = nil;
}
MetalPixelBufferComputeData::~MetalPixelBufferComputeData() {
    if (sparkleBuffer != nil) {
        [sparkleBuffer release];
    }
    if (tmpBufferBlend != nil) {
        [tmpBufferBlend release];
    }
}

bool MetalPixelBufferComputeData::doBlendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) {
    if (pixelBuffer->layers[saveLayer]->buffer.GetNodeCount() < 2048) {
        return false;
    }
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (validLayers[l]) {
            if (MetalComputeUtilities::INSTANCE.blendFunctions.find(pixelBuffer->layers[l]->mixType) == MetalComputeUtilities::INSTANCE.blendFunctions.end()) {
                return false;
            }
        }
    }
        
    if (!pixelBuffer->sparklesVector.empty()) {
        if (pixelBuffer->sparkles == &pixelBuffer->sparklesVector[0] && sparkleBuffer != nil) {
            [sparkleBuffer release];
        }
        if (sparkleBuffer == nil) {
            sparkleBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithBytes:&pixelBuffer->sparklesVector[0]
                                             length:pixelBuffer->sparklesVector.size() * sizeof(uint16_t)
                                            options:MTLResourceStorageModeShared] retain];
            std::string name = pixelBuffer->GetModelName() + "SparkleBuffer";
            setLabel(sparkleBuffer, name);
            pixelBuffer->sparkles = static_cast<uint16_t*>(sparkleBuffer.contents);
        }
    }
    if (tmpBufferBlend == nil) {
        int len = pixelBuffer->layers[saveLayer]->buffer.GetNodeCount() * sizeof(uint32_t);
        tmpBufferBlend = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:len options:MTLResourceStorageModeShared] retain];
        setLabel(tmpBufferBlend, pixelBuffer->GetModelName() + "-BlendBuffer");
    }
    MetalRenderBufferComputeData *slRMRB = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&pixelBuffer->layers[saveLayer]->buffer);
    if (!slRMRB) {
        return false;
    }
    if (slRMRB->isCommitted()) {
        slRMRB->waitForCompletion();
    }
    id<MTLCommandBuffer> commandBuffer = slRMRB->getCommandBuffer("-Blend");
    if (commandBuffer == nil) {
        return false;
    }
    
    // first load the pixel data into the buffers for blending on each layer
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (validLayers[l]) {
            auto layer = pixelBuffer->layers[l];
            MetalRenderBufferComputeData *layerCD = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&layer->buffer);
            id<MTLBuffer> tmpBufferLayer = layerCD->getBlendBuffer();
            
            LayerBlendingData data;
            data.nodeCount = layer->buffer.GetNodeCount();
            data.bufferHi = layer->buffer.BufferHt;
            data.bufferWi = layer->buffer.BufferWi;
            data.useMask = layer->maskSize > 0;
            data.hueAdjust = layer->outputHueAdjust;
            data.valueAdjust = layer->outputValueAdjust;
            data.saturationAdjust = layer->outputSaturationAdjust;
            data.outputSparkleCount = layer->outputSparkleCount;
            data.contrast = layer->contrast;
            data.brightness = layer->outputBrightnessAdjust;
            data.isChromaKey = layer->isChromaKey;
            data.chromaSensitivity = layer->chromaSensitivity;
            data.chromaColor = layer->chromaKeyColour.asChar4();
            data.effectMixThreshold = layer->outputEffectMixThreshold;
            data.effectMixVaries = layer->effectMixVaries;
            data.brightnessLevel = layer->brightnessLevel;
            data.fadeFactor = layer->fadeFactor;
            
            // first, we grab the color for the node from the buffer for the layer
            id<MTLBuffer> lcdPixelBuffer = layerCD->getPixelBuffer();
            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.getColorsFunction];
            setLabel(computeEncoder, "GetColors", l);
            int dataSize = sizeof(data);
            [computeEncoder setBytes:&data length:dataSize atIndex:0];
            [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:1];
            [computeEncoder setBuffer:lcdPixelBuffer offset:0 atIndex:2];
            if (layerCD->maskBuffer == nil) {
                uint8_t tmp[4] = {0, 0, 0, 0};
                [computeEncoder setBytes:tmp length:sizeof(tmp) atIndex:3];
            } else {
                [computeEncoder setBuffer:layerCD->maskBuffer offset:0 atIndex:3];
            }
            [computeEncoder setBuffer:layerCD->getIndexBuffer() offset:0 atIndex:4];
            NSInteger maxThreads = MetalComputeUtilities::INSTANCE.getColorsFunction.maxTotalThreadsPerThreadgroup;
            NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
            MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];
            [computeEncoder endEncoding];
            
            if (layer->needsHSVAdjust) {
                id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
                [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.adjustHSVFunction];
                setLabel(computeEncoder, "AdjustHSV", l);
                int dataSize = sizeof(data);
                [computeEncoder setBytes:&data length:dataSize atIndex:0];
                [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:1];
                NSInteger maxThreads = MetalComputeUtilities::INSTANCE.adjustHSVFunction.maxTotalThreadsPerThreadgroup;
                NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
                MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
                MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
                [computeEncoder dispatchThreads:gridSize
                          threadsPerThreadgroup:threadsPerThreadgroup];
                [computeEncoder endEncoding];
            }
            if (layer->use_music_sparkle_count ||
                layer->sparkle_count > 0 ||
                layer->outputSparkleCount > 0) {
                
                data.sparkleColor = layer->sparklesColour.asChar4();
                
                id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
                [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.applySparklesFunction];
                setLabel(computeEncoder, "ApplySparkles", l);
                int dataSize = sizeof(data);
                [computeEncoder setBytes:&data length:dataSize atIndex:0];
                [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:1];
                [computeEncoder setBuffer:sparkleBuffer offset:0 atIndex:2];
                NSInteger maxThreads = MetalComputeUtilities::INSTANCE.applySparklesFunction.maxTotalThreadsPerThreadgroup;
                NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
                MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
                MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
                [computeEncoder dispatchThreads:gridSize
                          threadsPerThreadgroup:threadsPerThreadgroup];
                [computeEncoder endEncoding];
            }
            if (layer->contrast != 0 || layer->outputBrightnessAdjust != 100) {
                id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
                [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.brightnessContrastFunction];
                setLabel(computeEncoder, "ApplyBrightnessContrast", l);
                int dataSize = sizeof(data);
                [computeEncoder setBytes:&data length:dataSize atIndex:0];
                [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:1];
                NSInteger maxThreads = MetalComputeUtilities::INSTANCE.brightnessContrastFunction.maxTotalThreadsPerThreadgroup;
                NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
                MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
                MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
                [computeEncoder dispatchThreads:gridSize
                          threadsPerThreadgroup:threadsPerThreadgroup];
                [computeEncoder endEncoding];
            }
            if (layer->brightnessLevel) {
                id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
                [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.brightnessLevelFunction];
                setLabel(computeEncoder, "ApplyBrightnessLevel", l);
                int dataSize = sizeof(data);
                [computeEncoder setBytes:&data length:dataSize atIndex:0];
                [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:1];
                NSInteger maxThreads = MetalComputeUtilities::INSTANCE.brightnessLevelFunction.maxTotalThreadsPerThreadgroup;
                NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
                MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
                MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
                [computeEncoder dispatchThreads:gridSize
                          threadsPerThreadgroup:threadsPerThreadgroup];
                [computeEncoder endEncoding];
            }
        }
    }
    
    // now all the pixels are loaded and adjusted, now start the blending
    bool first = true;
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (validLayers[l]) {
            auto layer = pixelBuffer->layers[l];
            MetalRenderBufferComputeData *layerCD = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&layer->buffer);
            id<MTLBuffer> tmpBufferLayer = layerCD->getBlendBuffer();

            LayerBlendingData data;
            data.nodeCount = layer->buffer.GetNodeCount();
            data.bufferHi = layer->buffer.BufferHt;
            data.bufferWi = layer->buffer.BufferWi;
            data.useMask = layer->maskSize > 0;
            data.hueAdjust = layer->outputHueAdjust;
            data.valueAdjust = layer->outputValueAdjust;
            data.saturationAdjust = layer->outputSaturationAdjust;
            data.outputSparkleCount = layer->outputSparkleCount;
            data.contrast = layer->contrast;
            data.brightness = layer->outputBrightnessAdjust;
            data.isChromaKey = layer->isChromaKey;
            data.chromaSensitivity = layer->chromaSensitivity;
            data.chromaColor = layer->chromaKeyColour.asChar4();
            data.effectMixThreshold = layer->outputEffectMixThreshold;
            data.effectMixVaries = layer->effectMixVaries;
            data.fadeFactor = layer->fadeFactor;

            if (first) {
                first = false;
                id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
                [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.firstLayerFadeFunction];
                setLabel(computeEncoder, "ApplyFadeBottomLayer", l);
                int dataSize = sizeof(data);
                [computeEncoder setBytes:&data length:dataSize atIndex:0];
                [computeEncoder setBuffer:tmpBufferBlend offset:0 atIndex:1];
                [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:2];
                NSInteger maxThreads = MetalComputeUtilities::INSTANCE.firstLayerFadeFunction.maxTotalThreadsPerThreadgroup;
                NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
                MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
                MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
                [computeEncoder dispatchThreads:gridSize
                          threadsPerThreadgroup:threadsPerThreadgroup];
                [computeEncoder endEncoding];
            } else {
                if (!layer->buffer.allowAlpha && layer->fadeFactor != 1.0) {
                    // need to fade the first here as we're not mixing anything
                    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
                    [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.nonAlphaFadeFunction];
                    setLabel(computeEncoder, "ApplyNonAlphaFade", l);
                    int dataSize = sizeof(data);
                    [computeEncoder setBytes:&data length:dataSize atIndex:0];
                    [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:1];
                    NSInteger maxThreads = MetalComputeUtilities::INSTANCE.nonAlphaFadeFunction.maxTotalThreadsPerThreadgroup;
                    NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
                    MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
                    MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
                    [computeEncoder dispatchThreads:gridSize
                              threadsPerThreadgroup:threadsPerThreadgroup];
                    [computeEncoder endEncoding];
                }
                
                id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
                auto &f = MetalComputeUtilities::INSTANCE.blendFunctions[layer->mixType];
                data.mixTypeData = f->mixTypeData;

                [computeEncoder setComputePipelineState:f->function];
                setLabel(computeEncoder, f->name, l);
                int dataSize = sizeof(data);
                [computeEncoder setBytes:&data length:dataSize atIndex:0];
                [computeEncoder setBuffer:tmpBufferBlend offset:0 atIndex:1];
                [computeEncoder setBuffer:tmpBufferLayer offset:0 atIndex:2];
                if (f->needIndexes) {
                    [computeEncoder setBuffer:layerCD->getIndexBuffer() offset:0 atIndex:3];
                }
                NSInteger maxThreads = f->function.maxTotalThreadsPerThreadgroup;
                NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
                MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
                MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
                [computeEncoder dispatchThreads:gridSize
                          threadsPerThreadgroup:threadsPerThreadgroup];
                [computeEncoder endEncoding];
            }
        }
    }
    if (saveToPixels) {
        auto layer = pixelBuffer->layers[saveLayer];
        MetalRenderBufferComputeData *layerCD = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&layer->buffer);
        LayerBlendingData data;
        data.nodeCount = layer->buffer.GetNodeCount();
        data.bufferHi = layer->buffer.BufferHt;
        data.bufferWi = layer->buffer.BufferWi;
        data.useMask = false;
        data.hueAdjust = layer->outputHueAdjust;
        data.valueAdjust = layer->outputValueAdjust;
        data.saturationAdjust = layer->outputSaturationAdjust;
        data.outputSparkleCount = layer->outputSparkleCount;
        data.contrast = layer->contrast;
        data.brightness = layer->outputBrightnessAdjust;
        data.isChromaKey = layer->isChromaKey;
        data.chromaSensitivity = layer->chromaSensitivity;
        data.chromaColor = layer->chromaKeyColour.asChar4();
        data.effectMixThreshold = layer->outputEffectMixThreshold;
        data.effectMixVaries = layer->effectMixVaries;
        data.fadeFactor = layer->fadeFactor;
        
        id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
        [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.putColorsFunction];
        setLabel(computeEncoder, "PutColors", saveLayer);
        int dataSize = sizeof(data);
        [computeEncoder setBytes:&data length:dataSize atIndex:0];
        [computeEncoder setBuffer:layerCD->getPixelBuffer() offset:0 atIndex:1];
        [computeEncoder setBuffer:tmpBufferBlend offset:0 atIndex:2];
        uint8_t tmp[4] = {0, 0, 0, 0};
        [computeEncoder setBytes:tmp length:sizeof(tmp) atIndex:3];
        [computeEncoder setBuffer:layerCD->getIndexBuffer() offset:0 atIndex:4];
        
        NSInteger maxThreads = MetalComputeUtilities::INSTANCE.putColorsFunction.maxTotalThreadsPerThreadgroup;
        NSInteger threads = std::min((NSInteger)data.nodeCount, maxThreads);
        MTLSize gridSize = MTLSizeMake(data.nodeCount, 1, 1);
        MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);
        [computeEncoder dispatchThreads:gridSize
                  threadsPerThreadgroup:threadsPerThreadgroup];
        [computeEncoder endEncoding];
    }
    slRMRB->commit();
    
    for (int ii = (pixelBuffer->numLayers - 1); ii >= 0; --ii) {
        if (!validLayers[ii]) {
            continue;
        }
        if (ii != saveLayer) {
            GPURenderUtils::waitForRenderCompletion(&pixelBuffer->layers[ii]->buffer);
        }
    }
    slRMRB->waitForCompletion();
    
    xlColor *colors = (xlColor*)(tmpBufferBlend.contents);
    int nc = pixelBuffer->layers[saveLayer]->buffer.GetNodeCount();
    for (int x = 0;  x < nc; x++) {
        pixelBuffer->layers[saveLayer]->buffer.Nodes[x]->SetColor(colors[x]);
    }
    return true;
}

bool MetalPixelBufferComputeData::doTransitions(PixelBufferClass *pixelBuffer, int layer, RenderBuffer *prevRB) {
    PixelBufferClass::LayerInfo *li = pixelBuffer->layers[layer];
    int ms = li->BufferHt * li->BufferWi;
    if (ms < 1024) {
        li->maskSize = 0; // start with empty mask
        return false;
    }
    if (li->inMaskFactor < 1.0 || li->outMaskFactor < 1.0) {
        if (ms > li->maskMaxSize) {
            MetalRenderBufferComputeData *bd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&li->buffer);
            if (bd->maskBuffer) {
                [bd->maskBuffer release];
            }
            li->maskMaxSize = ms;
            bd->maskBuffer= [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:li->maskMaxSize options:MTLResourceStorageModeShared] retain];
            std::string name = li->buffer.GetModelName() + "MaskBuffer-" + std::to_string(layer);
            setLabel(bd->maskBuffer, name);
            li->mask = static_cast<uint8_t*>(bd->maskBuffer.contents);
        }
    }
    li->maskSize = 0; // start with empty mask

    const auto &tiIn = MetalComputeUtilities::INSTANCE.transitions.find(li->inTransitionType);
    if (tiIn == MetalComputeUtilities::INSTANCE.transitions.end()) {
        return false;
    }
    const auto &tiOut = MetalComputeUtilities::INSTANCE.transitions.find(li->outTransitionType);
    if (tiOut == MetalComputeUtilities::INSTANCE.transitions.end()) {
        return false;
    }

    TransitionData data;
    data.width = li->BufferWi;
    data.height = li->BufferHt;
    data.hasPrev = prevRB != nullptr;
    if (prevRB) {
        data.pWidth = prevRB->BufferWi;
        data.pHeight = prevRB->BufferHt;
    }
    if (li->inMaskFactor < 1.0) {
        data.progress = li->inMaskFactor;
        data.adjust = li->inTransitionAdjust;
        data.reverse = li->inTransitionReverse;
        data.out = false;
        if (li->InTransitionAdjustValueCurve.IsActive()) {
            data.adjust = li->InTransitionAdjustValueCurve.GetOutputValueAt(li->inMaskFactor, li->buffer.GetStartTimeMS(), li->buffer.GetEndTimeMS());
        }
        if (tiIn->second->reversed) {
            data.progress =  1.0f - li->inMaskFactor;
        }
        if (tiIn->second->type == 1) {
            li->maskSize = li->BufferHt * li->BufferWi;
            if (!doMap(tiIn->second->function, data, &li->buffer)) {
                return false;
            }
        } else if (tiIn->second->type == 2) {
            if (!doTransition(tiIn->second->function, data, &li->buffer, prevRB)) {
                return false;
            }
        } else if (tiIn->second->type == 3) {
            if (!doTransition(tiIn->second->function, data, &li->buffer, MetalComputeUtilities::INSTANCE.dissolveBuffer)) {
                return false;
            }
        }
    }
    if (li->outMaskFactor < 1.0) {
        data.progress = li->outMaskFactor;
        data.adjust = li->outTransitionAdjust;
        data.reverse = li->outTransitionReverse;
        data.out = true;
        if (li->OutTransitionAdjustValueCurve.IsActive()) {
            data.adjust = li->OutTransitionAdjustValueCurve.GetOutputValueAt(li->outMaskFactor, li->buffer.GetStartTimeMS(), li->buffer.GetEndTimeMS());
        }
        if (tiOut->second->reversed) {
            data.progress =  1.0f - li->outMaskFactor;
        }
        if (tiOut->second->type == 1) {
            li->maskSize = li->BufferHt * li->BufferWi;
            if (!doMap(tiOut->second->function, data, &li->buffer)) {
                return false;
            }
        } else if (tiOut->second->type == 2) {
            if (!doTransition(tiOut->second->function, data, &li->buffer, prevRB)) {
                return false;
            }
        } else if (tiOut->second->type == 3) {
            if (!doTransition(tiOut->second->function, data, &li->buffer, MetalComputeUtilities::INSTANCE.dissolveBuffer)) {
                return false;
            }
        }
    }
    return true;
}
bool MetalPixelBufferComputeData::doMap(id<MTLComputePipelineState> &f, TransitionData &data, RenderBuffer *buffer) {
    MetalRenderBufferComputeData *bd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(buffer);
    id<MTLCommandBuffer> commandBuffer = bd->getCommandBuffer("-Map");
    if (commandBuffer == nil) {
        return false;
    }

    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
    [computeEncoder setLabel:f.label];
    [computeEncoder setComputePipelineState:f];

    int dataSize = sizeof(data);
    [computeEncoder setBytes:&data length:dataSize atIndex:0];
    [computeEncoder setBuffer:bd->maskBuffer offset:0 atIndex:1];
    int w = f.threadExecutionWidth;
    int h = f.maxTotalThreadsPerThreadgroup / w;
    MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
    MTLSize threadsPerGrid = MTLSizeMake(data.width, data.height, 1);
    [computeEncoder dispatchThreads:threadsPerGrid
              threadsPerThreadgroup:threadsPerThreadgroup];

    [computeEncoder endEncoding];
    return true;
}

bool MetalPixelBufferComputeData::doTransition(id<MTLComputePipelineState> &f, TransitionData &data, RenderBuffer *buffer, RenderBuffer *prevRB) {
    id<MTLBuffer> bufferPrev = nil;
    if (prevRB) bufferPrev = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(prevRB)->getPixelBuffer();
    return doTransition(f, data, buffer, bufferPrev);
}
bool MetalPixelBufferComputeData::doTransition(id<MTLComputePipelineState> &f, TransitionData &data, RenderBuffer *buffer, id<MTLBuffer> &prev) {
    MetalRenderBufferComputeData *bd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(buffer);
    id<MTLCommandBuffer> commandBuffer = bd->getCommandBuffer("-Transition");
    if (commandBuffer == nil) {
        return false;
    }
    id<MTLBuffer> bufferResult = bd->getPixelBuffer();
    id<MTLBuffer> bufferCopy = bd->getPixelBufferCopy();
    if (prev == nil) {
        prev = bufferCopy;
    }
    id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
    [blitCommandEncoder setLabel:@"CopyDataToCopyBuffer"];
    [blitCommandEncoder copyFromBuffer:bufferResult
                          sourceOffset:0
                              toBuffer:bufferCopy
                     destinationOffset:0
                                  size:(data.width*data.height*4)];
    [blitCommandEncoder endEncoding];
    
    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
    [computeEncoder setLabel:f.label];
    [computeEncoder setComputePipelineState:f];
    
    int dataSize = sizeof(data);
    [computeEncoder setBytes:&data length:dataSize atIndex:0];
    [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
    [computeEncoder setBuffer:bufferCopy offset:0 atIndex:2];
    [computeEncoder setBuffer:prev offset:0 atIndex:3];
    int w = f.threadExecutionWidth;
    int h = f.maxTotalThreadsPerThreadgroup / w;
    MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
    MTLSize threadsPerGrid = MTLSizeMake(data.width, data.height, 1);
    [computeEncoder dispatchThreads:threadsPerGrid
              threadsPerThreadgroup:threadsPerThreadgroup];

    [computeEncoder endEncoding];
    return true;
}


static std::mutex commandBufferMutex;
std::atomic<uint32_t> MetalRenderBufferComputeData::commandBufferCount(0);
#define MAX_COMMANDBUFFER_COUNT 256

MetalRenderBufferComputeData::MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pbd, int l) : renderBuffer(rb), pixelBufferData(pbd), layer(l) {
    commandBuffer = nil;
    pixelBuffer = nil;
    pixelBufferCopy = nil;
    pixelTexture = nil;
    maskBuffer = nil;
    blendBuffer = nil;
    pixelBufferSize = 0;
    pixelTextureSize = {0, 0};
    indexBuffer = nil;
    indexes = nullptr;
    indexesSize = 0;
}
MetalRenderBufferComputeData::~MetalRenderBufferComputeData() {
    pixelBufferData = nullptr;
    @autoreleasepool {
        if (commandBuffer != nil) {
            [commandBuffer release];
            --commandBufferCount;
        }
        if (pixelBuffer != nil) {
            [pixelBuffer release];
        }
        if (pixelBufferCopy != nil) {
            [pixelBufferCopy release];
        }
        if (pixelTexture != nil) {
            [pixelTexture release];
        }
        if (maskBuffer != nil) {
            [maskBuffer release];
        }
        if (indexBuffer != nil)  {
            [indexBuffer release];
        }
        if (blendBuffer != nil) {
            [blendBuffer release];
        }
    }
}

id<MTLCommandBuffer> MetalRenderBufferComputeData::getCommandBuffer(const std::string &postfix) {
    if (commandBuffer != nil && committed) {
        // This should not happen.  If we get here, some work was sent
        // to the GPU, but then nothing asked for the result so the
        // work was irrelevant.   That would need to be tracked down.
        waitForCompletion();
    }
    if (commandBuffer == nil) {
        int max = MAX_COMMANDBUFFER_COUNT - 4;
        if (MetalComputeUtilities::INSTANCE.prioritizeGraphics()) {
            // use a lower command buffer count if the GPU is needed for frontend
            // 64 is the "default" in macOS, we'll try it
            max = 64;
        }
        
        if (commandBufferCount.fetch_add(1) > max) {
            --commandBufferCount;
            return nil;
        }
        std::unique_lock<std::mutex> lock(commandBufferMutex);
#ifdef DEBUG
        if (@available(macOS 11.0, *)) {
            MTLCommandBufferDescriptor *descriptor = [MTLCommandBufferDescriptor new];
            descriptor.errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
            commandBuffer = [[MetalComputeUtilities::INSTANCE.commandQueue commandBufferWithDescriptor:descriptor] retain];
            [descriptor release];
        } else {
            commandBuffer = [[MetalComputeUtilities::INSTANCE.commandQueue commandBuffer] retain];
        }
#else
        commandBuffer = [[MetalComputeUtilities::INSTANCE.commandQueue commandBuffer] retain];
#endif
        std::string modelName = renderBuffer->GetModelName() + "-" + std::to_string(layer);
        if (!postfix.empty()) {
            modelName += postfix;
        }
        NSString* mn = [NSString stringWithUTF8String:modelName.c_str()];
        [commandBuffer setLabel:mn];
    }
    return commandBuffer;
}
void MetalRenderBufferComputeData::abortCommandBuffer() {
    @autoreleasepool {
        [commandBuffer release];
        commandBuffer = nil;
        --commandBufferCount;
    }
}
id<MTLBuffer> MetalRenderBufferComputeData::getBlendBuffer() {
    if (blendBuffer == nil) {
        int len = renderBuffer->GetNodeCount() * sizeof(uint32_t);
        blendBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:len options:MTLResourceStorageModeShared] retain];
        setLabel(blendBuffer, renderBuffer->GetModelName() + "-WorkBuffer" + std::to_string(layer));
    }
    return blendBuffer;
}

id<MTLBuffer> MetalRenderBufferComputeData::getIndexBuffer() {
    return indexBuffer;
}

id<MTLBuffer> MetalRenderBufferComputeData::getPixelBufferCopy() {
    if (pixelBufferCopy == nil) {
        int bufferSize = std::max((int)renderBuffer->GetPixelCount(), (int)pixelBufferSize) * 4;
        id<MTLBuffer> newBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:bufferSize options:MTLResourceStorageModePrivate] retain];
        std::string name = renderBuffer->GetModelName() + "PixelBufferCopy";
        setLabel(newBuffer, name);
        pixelBufferCopy = newBuffer;
    }
    return pixelBufferCopy;
}



void MetalRenderBufferComputeData::bufferResized() {
    if (pixelBuffer && pixelBufferSize < renderBuffer->GetPixelCount()) {
        //buffer needs to get bigger
        getPixelBuffer(false);
    }
    int indexCount = renderBuffer->Nodes.size();
    for (auto &n : renderBuffer->Nodes) {
        if (n->Coords.size() > 1) {
            indexCount += n->Coords.size() + 1;
        }
    }
    if (indexesSize < indexCount) {
        if (indexBuffer != nil) {
            [indexBuffer release];
        }
        indexBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:(indexCount * sizeof(int32_t)) options:MTLResourceStorageModeShared] retain];
        std::string name = renderBuffer->GetModelName() + "IndexBuffer";
        setLabel(indexBuffer, name);
        indexes = static_cast<int32_t*>(indexBuffer.contents);
        indexesSize = indexCount;
    }
    int idx = 0;
    int extraIdx = renderBuffer->Nodes.size();
    for (auto &n : renderBuffer->Nodes) {
        if (n->Coords.size() > 1) {
            indexes[idx] = extraIdx | 0x80000000;
            int countIdx = extraIdx++;
            indexes[countIdx] = n->Coords.size();
            for (auto &c : n->Coords) {
                if (c.bufY < 0 || c.bufY >= renderBuffer->BufferHt ||
                    c.bufX < 0 || c.bufX >= renderBuffer->BufferWi ) {
                    indexes[countIdx] -= 1;
                } else {
                    int32_t pidx = c.bufY * renderBuffer->BufferWi + c.bufX;
                    indexes[extraIdx++] = pidx;
                }
            }
        } else if (n->Coords[0].bufY < 0 || n->Coords[0].bufY >= renderBuffer->BufferHt ||
                   n->Coords[0].bufX < 0 || n->Coords[0].bufX >= renderBuffer->BufferWi ) {
            indexes[idx] = -1;
        } else {
            int32_t pidx = n->Coords[0].bufY * renderBuffer->BufferWi + n->Coords[0].bufX;
            indexes[idx] = pidx;
        }
        ++idx;
    }
}

id<MTLBuffer> MetalRenderBufferComputeData::getPixelBuffer(bool sendToGPU) {
    if (pixelBufferSize < renderBuffer->GetPixelCount()) {
        if (pixelBuffer) {
            [pixelBuffer release];
        }
        if (pixelBufferCopy) {
            [pixelBufferCopy release];
            pixelBufferCopy = nil;
        }
        int bufferSize = renderBuffer->GetPixelCount() * 4;
        id<MTLBuffer> newBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:bufferSize options:MTLResourceStorageModeShared] retain];
        std::string name = renderBuffer->GetModelName() + "PixelBuffer";
        setLabel(newBuffer, name);
        memcpy(newBuffer.contents, renderBuffer->pixels, pixelBufferSize == 0 ? bufferSize : pixelBufferSize * 4);
        pixelBufferSize = renderBuffer->pixelVector.size();
        pixelBuffer = newBuffer;
        renderBuffer->pixels = static_cast<xlColor*>(pixelBuffer.contents);
        currentDataLocation = BUFFER;
    }
    if (currentDataLocation == TEXTURE) {
        //use GPU to copy over to Buffer
        NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
        NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
        MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
        id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
        [blitCommandEncoder setLabel:@"CopyTextureToBuffer"];
        [blitCommandEncoder copyFromTexture:pixelTexture
                                sourceSlice:0
                                sourceLevel:0
                               sourceOrigin:{0,0,0}
                                 sourceSize:size
                                   toBuffer:pixelBuffer
                          destinationOffset:0
                     destinationBytesPerRow:bytesPerRow
                   destinationBytesPerImage:bytesPerImage];
        [blitCommandEncoder endEncoding];
        currentDataLocation = BUFFER;
    }
    return pixelBuffer;
}
id<MTLTexture> MetalRenderBufferComputeData::getPixelTexture() {
    getPixelBuffer(true);
    
    if (pixelTexture != nil &&
        (renderBuffer->BufferWi != pixelTextureSize.first
        || renderBuffer->BufferHt != pixelTextureSize.second)) {
     
        @autoreleasepool {
            [pixelTexture release];
            pixelTexture = nil;
        }
        pixelTextureSize = { 0, 0 };
    }
    
    if (pixelTexture == nil) {
        @autoreleasepool {
            MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: MTLPixelFormatRGBA8Unorm
                                                                                         width: renderBuffer->BufferWi
                                                                                        height: renderBuffer->BufferHt
                                                                                     mipmapped: NO];
            d.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            d.storageMode = MTLStorageModePrivate;
            // Create the texture from the device by using the descriptor
            pixelTexture = [[MetalComputeUtilities::INSTANCE.device newTextureWithDescriptor:d] retain];
            
            std::string name = renderBuffer->GetModelName() + "PixelTexture";
            NSString* mn = [NSString stringWithUTF8String:name.c_str()];
            [pixelTexture setLabel:mn];
            pixelTextureSize = { renderBuffer->BufferWi, renderBuffer->BufferHt };
        }
    }
    if (currentDataLocation == BUFFER) {
        NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
        NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
        MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
        id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
        [blitCommandEncoder setLabel:@"CopyBufferToTexture"];
        [blitCommandEncoder copyFromBuffer:pixelBuffer
                              sourceOffset:0
                         sourceBytesPerRow:bytesPerRow
                       sourceBytesPerImage:bytesPerImage
                                sourceSize:size
                                 toTexture:pixelTexture
                          destinationSlice:0
                          destinationLevel:0
                         destinationOrigin:{0,0,0}];
        [blitCommandEncoder endEncoding];
        currentDataLocation = TEXTURE;
    }
    return pixelTexture;
}

void MetalRenderBufferComputeData::commit() {
    if (commandBuffer != nil && !committed) {
        @autoreleasepool {
            if (currentDataLocation == TEXTURE) {
                //use GPU to copy over to Buffer
                NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
                NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
                MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
                id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
                [blitCommandEncoder setLabel:@"CopyTextureToBufferForCommit"];

                [blitCommandEncoder copyFromTexture:pixelTexture
                                        sourceSlice:0
                                        sourceLevel:0
                                       sourceOrigin:{0,0,0}
                                         sourceSize:size
                                           toBuffer:pixelBuffer
                                  destinationOffset:0
                             destinationBytesPerRow:bytesPerRow
                           destinationBytesPerImage:bytesPerImage];
                [blitCommandEncoder endEncoding];
                currentDataLocation = BUFFER;
            }
        }
        std::unique_lock<std::mutex> lock(commandBufferMutex);
        [commandBuffer commit];
        committed = true;
    }
}


void MetalRenderBufferComputeData::waitForCompletion() {
    if (commandBuffer != nil) {
        @autoreleasepool {
            commit();
#ifdef DEBUG
            [commandBuffer waitUntilScheduled];
            MTLCommandBufferStatus status = [commandBuffer status];
            int cnt = 0;
            while (status < MTLCommandBufferStatusCompleted && cnt++ < 250) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                status = [commandBuffer status];
            }
            if (@available(macOS 11.0, *)) {
                NSError *error = [commandBuffer error];
                if (error != nil) {
                    int ec = error.code;
                    //id info = error.userInfo[MTLCommandBufferEncoderInfoErrorKey];
                    printf("ERROR!!  ec: %d  \n", ec);
                }
            }
            [commandBuffer waitUntilCompleted];
#else
            [commandBuffer waitUntilCompleted];
#endif
            [commandBuffer release];
            commandBuffer = nil;
            committed = false;
            --commandBufferCount;
        }
    }
}
bool MetalRenderBufferComputeData::blur(int radius) {
    if ((renderBuffer->BufferHt < (radius * 2)) || (renderBuffer->BufferWi < (radius * 2)) || ((renderBuffer->BufferWi * renderBuffer->BufferHt) < 1024)) {
        // Smallish buffer, overhead of sending to GPU will be more than the gain
        return false;
    }
    if (renderBuffer->BufferHt > 16384 || renderBuffer->BufferWi > 16384) {
        // max size of Textures on macOS
        return false;
    }
    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = getCommandBuffer("-Blur");
        if (commandBuffer == nil) {
            return false;
        }
        getPixelTexture();
        
        /*
        float sigma = radius - 1;
        sigma = std::sqrt(sigma);
        MPSImageGaussianBlur* gblur = [[MPSImageGaussianBlur alloc] initWithDevice:MetalComputeUtilities::INSTANCE.device sigma:sigma];
        */
        //tent blur is closest to what is implemented on the C++/CPU side
        float r = (radius - 1) * 2 - 1;
        MPSImageTent *gblur = [[MPSImageTent alloc] initWithDevice:MetalComputeUtilities::INSTANCE.device
                                                     kernelWidth:r
                                                    kernelHeight:r];
        [gblur setEdgeMode:MPSImageEdgeModeClamp];
        [gblur setLabel:@"Blur"];

        MPSCopyAllocator myAllocator = ^id <MTLTexture>( MPSKernel * __nonnull filter,
                                                        __nonnull id <MTLCommandBuffer> cmdBuf,
                                                        __nonnull id <MTLTexture> sourceTexture)
        {
            MTLPixelFormat format = sourceTexture.pixelFormat;
            MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: format
                                                                                         width: sourceTexture.width
                                                                                        height: sourceTexture.height
                                                                                     mipmapped: NO];
            [sourceTexture release];
            d.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            return [cmdBuf.device newTextureWithDescriptor: d];
        };
        [commandBuffer pushDebugGroup:@"Blur"];
        BOOL ok = [gblur encodeToCommandBuffer:commandBuffer
                                inPlaceTexture:&pixelTexture
                         fallbackCopyAllocator:myAllocator];
        [commandBuffer popDebugGroup];
        [gblur release];
        return ok;
    }
}

bool MetalRenderBufferComputeData::rotoZoom(GPURenderUtils::RotoZoomSettings &settings) {
    if ((renderBuffer->BufferWi * renderBuffer->BufferHt) < 256) {
        // Smallish buffer, overhead of sending to GPU will be more than the gain
        return false;
    }
    
    RotoZoomData data;
    data.width = renderBuffer->BufferWi;
    data.height = renderBuffer->BufferHt;
    
    data.offset = settings.offset;
    data.xrotation = settings.xrotation;
    data.xpivot = settings.xpivot;
    data.yrotation = settings.yrotation;
    data.ypivot = settings.ypivot;
    data.zrotation = settings.zrotation;
    data.zoom = settings.zoom;
    data.zoomquality = settings.zoomquality;
    data.pivotpointx = settings.pivotpointx;
    data.pivotpointy = settings.pivotpointy;
    
    id<MTLBuffer> bufferResult = getPixelBuffer();
    if (bufferResult == nil) {
        return false;
    }
    id<MTLBuffer> bufferCopy = getPixelBufferCopy();
    if (bufferCopy == nil) {
        return false;
    }
    bool dbg = false;
    if (commandBuffer != nil) {
        dbg = true;
        [commandBuffer pushDebugGroup:@"RotoZoom"];
    }
    for (auto &c : settings.rotationorder) {
        switch (c) {
            case 'X':
                if (data.xrotation != 0 && data.xrotation != 360) {
                    callRotoZoomFunction(MetalComputeUtilities::INSTANCE.xrotateFunction, data);
                }
                break;
            case 'Y':
                if (data.yrotation != 0 && data.yrotation != 360) {
                    callRotoZoomFunction(MetalComputeUtilities::INSTANCE.yrotateFunction, data);
                }
                break;
            case 'Z':
                if (data.zrotation != 0.0 || data.zoom != 1.0) {
                    callRotoZoomFunction(MetalComputeUtilities::INSTANCE.zrotateFunction, data);
                }
                break;
        }
    }
    if (dbg) {
        [commandBuffer popDebugGroup];
    }
    return true;
}

bool MetalRenderBufferComputeData::callRotoZoomFunction(id<MTLComputePipelineState> &function, RotoZoomData &data) {
    id<MTLCommandBuffer> commandBuffer = getCommandBuffer("-RotoZoom");
    if (commandBuffer == nil) {
        return false;
    }
    id<MTLBuffer> bufferResult = getPixelBuffer();
    id<MTLBuffer> bufferCopy = getPixelBufferCopy();
    id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
    [blitCommandEncoder setLabel:@"CopyDataToCopyBuffer"];
    [blitCommandEncoder copyFromBuffer:bufferResult
                          sourceOffset:0
                              toBuffer:bufferCopy
                     destinationOffset:0
                                  size:(data.width*data.height*4)];
    [blitCommandEncoder endEncoding];
    
    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
    [computeEncoder setLabel:MetalComputeUtilities::INSTANCE.rotateBlankFunction.label];
    [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.rotateBlankFunction];
    
    NSInteger dataSize = sizeof(data);
    [computeEncoder setBytes:&data length:dataSize atIndex:0];
    [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
    int w = MetalComputeUtilities::INSTANCE.rotateBlankFunction.threadExecutionWidth;
    int h = MetalComputeUtilities::INSTANCE.rotateBlankFunction.maxTotalThreadsPerThreadgroup / w;
    MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
    MTLSize threadsPerGrid = MTLSizeMake(data.width, data.height, 1);
    [computeEncoder dispatchThreads:threadsPerGrid
              threadsPerThreadgroup:threadsPerThreadgroup];
    [computeEncoder endEncoding];
    
    computeEncoder = [commandBuffer computeCommandEncoder];
    [computeEncoder setLabel:function.label];
    [computeEncoder setComputePipelineState:function];
    
    dataSize = sizeof(data);
    [computeEncoder setBytes:&data length:dataSize atIndex:0];
    [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
    [computeEncoder setBuffer:bufferCopy offset:0 atIndex:2];
    w = function.threadExecutionWidth;
    h = function.maxTotalThreadsPerThreadgroup / w;
    threadsPerThreadgroup = MTLSizeMake(w, h, 1);
    threadsPerGrid = MTLSizeMake(data.width, data.height, 1);
    [computeEncoder dispatchThreads:threadsPerGrid
              threadsPerThreadgroup:threadsPerThreadgroup];

    [computeEncoder endEncoding];
    return true;
}

MetalRenderBufferComputeData *MetalRenderBufferComputeData::getMetalRenderBufferComputeData(RenderBuffer *b) {
    return static_cast<MetalRenderBufferComputeData*>(b->gpuRenderData);
}



MetalComputeUtilities::MetalComputeUtilities() {
    enabled = false;
    device = nil;
    dissolveBuffer = nil;
    
    NSArray *devices = MTLCopyAllDevices();
    for (id d in devices) {
        if ([d isRemovable]) {
            device = [d retain];
        }
    }
    [devices release];
    if (device == nil) {
        device = MTLCreateSystemDefaultDevice();
    }
    if (device.argumentBuffersSupport == MTLArgumentBuffersTier1) {
        device = nil;
        return;
    }
    
    NSError *libraryError = NULL;
    NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"EffectComputeFunctions" ofType:@"metallib"];
    if (!libraryFile) {
        NSLog(@"Library file error");
        return;
    }
    
    NSURL *libraryURL = [NSURL fileURLWithPath:libraryFile];
    library = [device newLibraryWithURL:libraryURL error:&libraryError];
    if (!library) {
        NSLog(@"Library error: %@", libraryError);
        return;
    }
    [library setLabel:@"EffectComputeFunctionsLibrary"];
    
    commandQueue = [device newCommandQueueWithMaxCommandBufferCount:MAX_COMMANDBUFFER_COUNT];
    if (!commandQueue) {
        return;
    }
#ifdef DEBUG
    printf("CommandQueue: %p\n", (void*)commandQueue);
#endif
    [commandQueue setLabel:@"MetalEffectCommandQueue"];
    enabled = true;
    
    xrotateFunction = FindComputeFunction("RotoZoomRotateX");
    yrotateFunction = FindComputeFunction("RotoZoomRotateY");
    zrotateFunction = FindComputeFunction("RotoZoomRotateZ");
    rotateBlankFunction = FindComputeFunction("RotoZoomBlank");
    
    getColorsFunction = FindComputeFunction("GetColorsForNodes");
    putColorsFunction = FindComputeFunction("PutColorsForNodes");
    adjustHSVFunction = FindComputeFunction("AdjustHSV");
    applySparklesFunction = FindComputeFunction("ApplySparkles");
    brightnessContrastFunction = FindComputeFunction("AdjustBrightnessContrast");
    brightnessLevelFunction = FindComputeFunction("AdjustBrightnessLevel");
    firstLayerFadeFunction = FindComputeFunction("FirstLayerFade");
    nonAlphaFadeFunction = FindComputeFunction("NonAlphaFade");

    transitions[""] = new TransitionInfo(0);
    transitions["None"] = new TransitionInfo(0);
    transitions["Fade"] = new TransitionInfo(0);
    
    transitions["Wipe"] = new TransitionInfo("wipeTransition", 1);
    transitions["Clock"] = new TransitionInfo("clockTransition", 1);
    transitions["From Middle"] = new TransitionInfo("fromMiddleTransition", 1);
    transitions["Circle Explode"] = new TransitionInfo("circleExplodeTransition", 1);
    transitions["Square Explode"] = new TransitionInfo("squareExplodeTransition", 1);
    transitions["Blend"] = new TransitionInfo("blendTransition", 1);
    transitions["Slide Checks"] = new TransitionInfo("slideChecksTransition", 1);
    transitions["Slide Bars"] = new TransitionInfo("slideBarsTransition", 1);
    transitions["Blinds"] = new TransitionInfo("blindsTransition", 1);
    
    transitions["Shatter"] = new TransitionInfo("shatterTransition", 2, true);
    transitions["Star"] = new TransitionInfo("starTransition", 2);
    transitions["Pinwheel"] = new TransitionInfo("pinwheelTransition", 2, true);
    transitions["Bow Tie"] = new TransitionInfo("bowTieTransition", 2);
    transitions["Blobs"] = new TransitionInfo("blobsTransition", 2);
    transitions["Fold"] = new TransitionInfo("foldTransition", 2);
    transitions["Zoom"] = new TransitionInfo("zoomTransition", 2);
    transitions["Circular Swirl"] = new TransitionInfo("circularSwirlTransition", 2, true);
    transitions["Doorway"] = new TransitionInfo("doorwayTransition", 2);
    transitions["Swap"] = new TransitionInfo("swapTransition", 2);
    transitions["Circles"] = new TransitionInfo("circlesTransition", 2, true);
    transitions["Dissolve"] = new TransitionInfo("dissolveTransition", 3, true);
    
    blendFunctions[MixTypes::Mix_Normal] = new BlendFunctionInfo("NormalBlendFunction");
    blendFunctions[MixTypes::Mix_Effect1] = new BlendFunctionInfo("Effect1_2_Function");
    blendFunctions[MixTypes::Mix_Effect2] = new BlendFunctionInfo("Effect1_2_Function", 1);
    blendFunctions[MixTypes::Mix_Mask1] = new BlendFunctionInfo("Mask1Function");
    blendFunctions[MixTypes::Mix_Mask2] = new BlendFunctionInfo("Mask2Function");
    blendFunctions[MixTypes::Mix_Unmask1] = new BlendFunctionInfo("Unmask1Function");
    blendFunctions[MixTypes::Mix_Unmask2] = new BlendFunctionInfo("Unmask2Function");
    blendFunctions[MixTypes::Mix_TrueUnmask1] = new BlendFunctionInfo("TrueUnmask1Function");
    blendFunctions[MixTypes::Mix_TrueUnmask2] = new BlendFunctionInfo("TrueUnmask2Function");
    blendFunctions[MixTypes::Mix_Shadow_1on2] = new BlendFunctionInfo("Shadow_1on2Function");
    blendFunctions[MixTypes::Mix_Shadow_2on1] = new BlendFunctionInfo("Shadow_2on1Function");
    blendFunctions[MixTypes::Mix_Layered] = new BlendFunctionInfo("LayeredFunction");
    blendFunctions[MixTypes::Mix_Average] = new BlendFunctionInfo("AveragedFunction");
    blendFunctions[MixTypes::Mix_1_reveals_2] = new BlendFunctionInfo("Reveal12Function");
    blendFunctions[MixTypes::Mix_2_reveals_1] = new BlendFunctionInfo("Reveal21Function");
    blendFunctions[MixTypes::Mix_Additive] = new BlendFunctionInfo("AdditiveFunction");
    blendFunctions[MixTypes::Mix_Subtractive] = new BlendFunctionInfo("SubtractiveFunction");
    blendFunctions[MixTypes::Mix_Max] = new BlendFunctionInfo("MaxFunction");
    blendFunctions[MixTypes::Mix_Min] = new BlendFunctionInfo("MinFunction");
    blendFunctions[MixTypes::Mix_AsBrightness] = new BlendFunctionInfo("AsBrightnessFunction");
    blendFunctions[MixTypes::Mix_Highlight] = new BlendFunctionInfo("HighlightFunction");
    blendFunctions[MixTypes::Mix_Highlight_Vibrant] = new BlendFunctionInfo("HighlightVibrantFunction");
    blendFunctions[MixTypes::Mix_BottomTop] = new BlendFunctionInfo("BottomTopFunction",0, true);
    blendFunctions[MixTypes::Mix_LeftRight] = new BlendFunctionInfo("LeftRightFunction",0, true);
    
    int bufferSize = DissolvePatternWidth * DissolvePatternHeight;
    dissolveBuffer = [[device newBufferWithBytes:DissolveTransitonPattern
                                          length:bufferSize
                                         options:MTLResourceStorageModeShared] retain];

    [dissolveBuffer setLabel:@"DissolveTransitonPattern"];
}
MetalComputeUtilities::~MetalComputeUtilities() {
    @autoreleasepool {
        commandQueue = nil;
        library = nil;
        device = nil;
        
        xrotateFunction = nil;
        yrotateFunction = nil;
        zrotateFunction = nil;
        rotateBlankFunction = nil;
        
        getColorsFunction = nil;
        putColorsFunction = nil;
        adjustHSVFunction = nil;
        applySparklesFunction = nil;
        brightnessContrastFunction = nil;
        brightnessLevelFunction = nil;
        firstLayerFadeFunction = nil;
        nonAlphaFadeFunction = nil;
        
        for (auto &a : transitions) {
            delete a.second;
            a.second = nullptr;
        }
        transitions.clear();
        
        for (auto &a : blendFunctions) {
            delete a.second;
            a.second = nullptr;
        }
        blendFunctions.clear();
        
        if (dissolveBuffer != nil) {
            [dissolveBuffer release];
            dissolveBuffer = nil;
        }
    }
}
MetalComputeUtilities::TransitionInfo::TransitionInfo(int t) : type(t), reversed(false), function(nil) {
}
MetalComputeUtilities::TransitionInfo::TransitionInfo(const char *fn, int t, bool r) : type(t), reversed(r) {
    function = MetalComputeUtilities::INSTANCE.FindComputeFunction(fn);
}
MetalComputeUtilities::TransitionInfo::~TransitionInfo() {
    function = nil;
}

MetalComputeUtilities::BlendFunctionInfo::BlendFunctionInfo(const char *fn, int mtd, bool ni) : name(fn), mixTypeData(mtd), needIndexes(ni) {
    function = MetalComputeUtilities::INSTANCE.FindComputeFunction(fn);
}
MetalComputeUtilities::BlendFunctionInfo::~BlendFunctionInfo() {
    function = nil;
}

id<MTLComputePipelineState> MetalComputeUtilities::FindComputeFunction(const char *name) {
    NSString *fname = @(name);
    id<MTLFunction> function = [library newFunctionWithName:fname];
    NSError *error = NULL;
    
    MTLComputePipelineDescriptor *desc = [MTLComputePipelineDescriptor new];
    desc.computeFunction = function;
    desc.label = fname;
    
    //id<MTLComputePipelineState> ps = [device newComputePipelineStateWithFunction:function error:&error];
    id<MTLComputePipelineState> ps = [device newComputePipelineStateWithDescriptor:desc
                                                                           options:MTLPipelineOptionNone
                                                                        reflection:nil
                                                                             error:&error];
    [function release];
    [desc release];
    if (!ps) {
        NSLog(@"Library error: %@", error);
    }
    return ps;
}




extern "C" {
bool isMetalComputeSupported() {
    return MetalComputeUtilities::INSTANCE.enabled;
}
}

