/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#include <map>
#include <functional>
#include "ISPCComputeUtilities.h"

#include "../../Parallel.h"


ISPCComputeUtilities ISPCComputeUtilities::INSTANCE;

#ifndef __WXMSW__
#define HASISPC
#endif


#ifdef HASISPC
#include "LayerBlendingFunctions.ispc.h"


class ISPCBlendFunctionInfo {
public:
    ISPCBlendFunctionInfo(const char *fn, std::function<void(const struct ispc::LayerBlendingData &, uint32_t * result, const uint32_t *, const uint32_t *)>  &&f, int mtd = 0) : name(fn), mixTypeData(mtd), function(f) {
        
    }
    ~ISPCBlendFunctionInfo() {
        
    }
    std::string name;
    int mixTypeData;
    std::function<void(const struct ispc::LayerBlendingData &, uint32_t * result, const uint32_t *, const uint32_t *)> function;
};

class ISPCComputeUtilitiesData {
public:
    ISPCComputeUtilitiesData() {
        blendFunctions[MixTypes::Mix_Normal] = new ISPCBlendFunctionInfo("NormalBlendFunction", ispc::NormalBlendFunction);
        blendFunctions[MixTypes::Mix_Effect1] = new ISPCBlendFunctionInfo("Effect1_2_Function", ispc::Effect1_2_Function);
        blendFunctions[MixTypes::Mix_Effect2] = new ISPCBlendFunctionInfo("Effect1_2_Function", ispc::Effect1_2_Function, 1);
        blendFunctions[MixTypes::Mix_Mask1] = new ISPCBlendFunctionInfo("Mask1Function", ispc::Mask1Function);
        blendFunctions[MixTypes::Mix_Mask2] = new ISPCBlendFunctionInfo("Mask2Function", ispc::Mask2Function);
        blendFunctions[MixTypes::Mix_Unmask1] = new ISPCBlendFunctionInfo("Unmask1Function", ispc::Unmask1Function);
        blendFunctions[MixTypes::Mix_Unmask2] = new ISPCBlendFunctionInfo("Unmask2Function", ispc::Unmask2Function);
        blendFunctions[MixTypes::Mix_TrueUnmask1] = new ISPCBlendFunctionInfo("TrueUnmask1Function", ispc::TrueUnmask1Function);
        blendFunctions[MixTypes::Mix_TrueUnmask2] = new ISPCBlendFunctionInfo("TrueUnmask2Function", ispc::TrueUnmask2Function);
        blendFunctions[MixTypes::Mix_Shadow_1on2] = new ISPCBlendFunctionInfo("Shadow_1on2Function", ispc::Shadow_1on2Function);
        blendFunctions[MixTypes::Mix_Shadow_2on1] = new ISPCBlendFunctionInfo("Shadow_2on1Function", ispc::Shadow_2on1Function);
        blendFunctions[MixTypes::Mix_Layered] = new ISPCBlendFunctionInfo("LayeredFunction", ispc::LayeredFunction);
        blendFunctions[MixTypes::Mix_Average] = new ISPCBlendFunctionInfo("AveragedFunction", ispc::AveragedFunction);
        blendFunctions[MixTypes::Mix_1_reveals_2] = new ISPCBlendFunctionInfo("Reveal12Function", ispc::Reveal12Function);
        blendFunctions[MixTypes::Mix_2_reveals_1] = new ISPCBlendFunctionInfo("Reveal21Function", ispc::Reveal21Function);
        blendFunctions[MixTypes::Mix_Additive] = new ISPCBlendFunctionInfo("AdditiveFunction", ispc::AdditiveFunction);
        blendFunctions[MixTypes::Mix_Subtractive] = new ISPCBlendFunctionInfo("SubtractiveFunction", ispc::SubtractiveFunction);
        blendFunctions[MixTypes::Mix_Max] = new ISPCBlendFunctionInfo("MaxFunction", ispc::MaxFunction);
        blendFunctions[MixTypes::Mix_Min] = new ISPCBlendFunctionInfo("MinFunction", ispc::MinFunction);
        blendFunctions[MixTypes::Mix_AsBrightness] = new ISPCBlendFunctionInfo("AsBrightnessFunction", ispc::AsBrightnessFunction);
        blendFunctions[MixTypes::Mix_Highlight] = new ISPCBlendFunctionInfo("HighlightFunction", ispc::HighlightFunction);
        blendFunctions[MixTypes::Mix_Highlight_Vibrant] = new ISPCBlendFunctionInfo("HighlightVibrantFunction", ispc::HighlightVibrantFunction);
        blendFunctions[MixTypes::Mix_BottomTop] = new ISPCBlendFunctionInfo("BottomTopFunction", ispc::BottomTopFunction);
        blendFunctions[MixTypes::Mix_LeftRight] = new ISPCBlendFunctionInfo("LeftRightFunction", ispc::LeftRightFunction);
    }
    ~ISPCComputeUtilitiesData() {
        for (auto &bf : blendFunctions) {
            delete bf.second;
        }
    }
    
    
    std::map<MixTypes, ISPCBlendFunctionInfo*> blendFunctions;
    
};

ISPCComputeUtilities::ISPCComputeUtilities() : data(nullptr) {
    data = new ISPCComputeUtilitiesData();
}
ISPCComputeUtilities::~ISPCComputeUtilities() {
    if (data) {
        delete data;
    }
}

bool ISPCComputeUtilities::blendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) {
    if (pixelBuffer->blendDataBuffer.size() < pixelBuffer->layers[saveLayer]->buffer.GetNodeCount()) {
        pixelBuffer->blendDataBuffer.resize(pixelBuffer->layers[saveLayer]->buffer.GetNodeCount());
    }
    uint32_t * tmpBufferBlend = (uint32_t *)&pixelBuffer->blendDataBuffer[0];

    
    // first load the pixel data into the buffers for blending on each layer
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (validLayers[l]) {
            auto layer = pixelBuffer->layers[l];
            std::vector<uint32_t> &tmpBufferLayer = layer->buffer.blendBuffer;
            uint32_t * result = (uint32_t *)&tmpBufferLayer[0];
            
            ispc::LayerBlendingData data;
            data.nodeCount = layer->buffer.GetNodeCount();
            data.startNode = 0;
            data.endNode = layer->buffer.GetNodeCount();
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
            data.chromaColor = layer->chromaKeyColour.GetRGBA();
            data.effectMixThreshold = layer->outputEffectMixThreshold;
            data.effectMixVaries = layer->effectMixVaries;
            data.brightnessLevel = layer->brightnessLevel;
            data.fadeFactor = layer->fadeFactor;
            
            // first, we grab the color for the node from the buffer for the layer and into a single [] of colors matching the nodes
            const uint32_t *src = (const uint32_t *)layer->buffer.pixels;
            ispc::GetColorsISPCKernel(data, (uint32_t*)result, src, layer->mask, &layer->buffer.indexVector[0]);
        }
    }
    constexpr int maxPerBlock = 4096;
    int count = pixelBuffer->layers[saveLayer]->buffer.GetNodeCount();
    if (count < maxPerBlock) {
        blendLayers(pixelBuffer, effectPeriod, validLayers, saveLayer, 0, count);
    } else {
        int blocks = count / maxPerBlock;
        if ((count % maxPerBlock) != 0) {
            ++blocks;
        }
        parallel_for(
            0, blocks, [this, pixelBuffer, effectPeriod, &validLayers, saveLayer, count](int i) {
                int start = i * maxPerBlock;
                int end = std::min(count, start + maxPerBlock);
                blendLayers(pixelBuffer, effectPeriod, validLayers, saveLayer, start, end);
            },
            1);
    }
    
    if (saveToPixels) {
        auto layer = pixelBuffer->layers[saveLayer];
        
        uint32_t*target = (uint32_t *)layer->buffer.pixels;

        ispc::LayerBlendingData data;
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
        data.chromaColor = layer->chromaKeyColour.GetRGBA();
        data.effectMixThreshold = layer->outputEffectMixThreshold;
        data.effectMixVaries = layer->effectMixVaries;
        data.fadeFactor = layer->fadeFactor;
        ispc::PutColorsForNodes(data, target, tmpBufferBlend, nullptr, &layer->buffer.indexVector[0]);
    }
    
    xlColor *colors = (xlColor*)(&pixelBuffer->blendDataBuffer[0]);
    int nc = pixelBuffer->layers[saveLayer]->buffer.GetNodeCount();
    for (int x = 0; x < nc; x++) {
        pixelBuffer->layers[saveLayer]->buffer.Nodes[x]->SetColor(colors[x]);
    }
    return true;
}

void ISPCComputeUtilities::blendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, int start, int end) {
    uint32_t * tmpBufferBlend = (uint32_t *)&pixelBuffer->blendDataBuffer[0];
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (validLayers[l]) {
            auto layer = pixelBuffer->layers[l];
            std::vector<uint32_t> &tmpBufferLayer = layer->buffer.blendBuffer;
            uint32_t * result = (uint32_t *)&tmpBufferLayer[0];

            ispc::LayerBlendingData data;
            data.nodeCount = start;
            data.startNode = end;
            data.endNode = layer->buffer.GetNodeCount();
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
            data.chromaColor = layer->chromaKeyColour.GetRGBA();
            data.effectMixThreshold = layer->outputEffectMixThreshold;
            data.effectMixVaries = layer->effectMixVaries;
            data.brightnessLevel = layer->brightnessLevel;
            data.fadeFactor = layer->fadeFactor;
            
            if (layer->needsHSVAdjust) {
                ispc::AdjustHSV(data, result);
            }
            if (layer->use_music_sparkle_count ||
                layer->sparkle_count > 0 ||
                layer->outputSparkleCount > 0) {
                
                data.sparkleColor = layer->sparklesColour.GetRGBA();
                ispc::ApplySparkles(data, result, &pixelBuffer->sparklesVector[0]);
            }
            if (layer->contrast != 0 || layer->outputBrightnessAdjust != 100) {
                ispc::AdjustBrightnessContrast(data, result);
            }
            if (layer->brightnessLevel) {
                ispc::AdjustBrightnessLevel(data, result);
            }
        }
    }
    
    // now all the pixels are loaded and adjusted, now start the blending
    bool first = true;
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (validLayers[l]) {
            auto layer = pixelBuffer->layers[l];
            std::vector<uint32_t> &tmpBufferLayer = layer->buffer.blendBuffer;
            uint32_t * src = (uint32_t *)&tmpBufferLayer[0];

            ispc::LayerBlendingData data;
            data.startNode = start;
            data.endNode = end;
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
            data.chromaColor = layer->chromaKeyColour.GetRGBA();
            data.effectMixThreshold = layer->outputEffectMixThreshold;
            data.effectMixVaries = layer->effectMixVaries;
            data.brightnessLevel = layer->brightnessLevel;
            data.fadeFactor = layer->fadeFactor;
            
            if (first) {
                first = false;
                ispc::FirstLayerFade(data, tmpBufferBlend, src);
            } else {
                if (!layer->buffer.allowAlpha && layer->fadeFactor != 1.0) {
                    // need to fade the first here as we're not mixing anything
                    ispc::NonAlphaFade(data, src);
                }
                
                auto &f = this->data->blendFunctions[layer->mixType];
                data.mixTypeData = f->mixTypeData;
                f->function(data, tmpBufferBlend, src, &layer->buffer.indexVector[0]);
            }
        }
    }
}


#else


ISPCComputeUtilities::ISPCComputeUtilities() : data(nullptr) {
}
ISPCComputeUtilities::~ISPCComputeUtilities() {
}
bool ISPCComputeUtilities::blendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) {
    return false;
}
void ISPCComputeUtilities::blendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, int start, int end) {
}

#endif
