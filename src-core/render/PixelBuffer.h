#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Color.h"
#include "GPURenderUtils.h"
#include "RenderBuffer.h"
#include "RenderUtils.h"
#include "ValueCurve.h"
#include "models/Model.h"
#include "models/SingleLineModel.h"

/**
 * \brief enumeration of the different techniques used in layering effects
 */

enum class MixTypes {
    Mix_Normal,      /** Layered with Alpha channel considered **/
    Mix_Effect1,     /**<  Effect 1 only */
    Mix_Effect2,     /**<  Effect 2 only */
    Mix_Mask1,       /**<  Effect 2 color shows where Effect 1 is black */
    Mix_Mask2,       /**<  Effect 1 color shows where Effect 2 is black */
    Mix_Unmask1,     /**<  Effect 2 color shows where Effect 1 is not black but with no fade ... black becomes white*/
    Mix_Unmask2,     /**<  Effect 1 color shows where Effect 2 is not black but with no fade ... black becomes white*/
    Mix_TrueUnmask1, /**<  Effect 2 color shows where Effect 1 is not black */
    Mix_TrueUnmask2, /**<  Effect 1 color shows where Effect 2 is black */
    Mix_1_reveals_2, /**<  Effect 2 color only shows if Effect 1 is black  1 reveals 2 */
    Mix_2_reveals_1, /**<  Effect 1 color only shows if Effect 2 is black */
    Mix_Layered,     /**<  Effect 1 is back ground and shows only when effect 2 is black */
    Mix_Average,     /**<  Average color value between effects per pixel */
    Mix_BottomTop,
    Mix_LeftRight,
    Mix_Shadow_1on2, /**< Take value and saturation from Effect 1 and put them onto effect 2, leave hue alone on effect 2 */
    Mix_Shadow_2on1, /**< Take value and saturation from Effect 3 and put them onto effect 2, leave hue alone on effect 1 */
    Mix_Additive,
    Mix_Subtractive,
    Mix_AsBrightness,
    Mix_Max,
    Mix_Min,
    Mix_Highlight,
    Mix_Highlight_Vibrant,
    Mix_LAST /** not a real mix type -- sentinel/count terminator, must stay last */
};

class Effect;
class SequenceElements;
class SettingsMap;
class DimmingCurve;
class ModelGroup;
class MetalPixelBufferComputeData;

// The sub-buffer resolved for one frame, quantized to buffer cells. Node
// mapping depends on the sub-buffer definition only through these integers,
// so an unchanged rect means the node mapping does not need rebuilding.
struct SubBufferRect {
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;

    bool operator==(const SubBufferRect& o) const {
        return x1 == o.x1 && y1 == o.y1 && x2 == o.x2 && y2 == o.y2;
    }
    bool operator!=(const SubBufferRect& o) const {
        return !(*this == o);
    }
};

// A parsed sub-buffer definition string: the six fields (x1, y1, x2, y2 and
// the x/y centre offsets) as either a static value or a value curve. Parsing
// is hoisted out of the per-frame path.
class SubBufferSpec {
public:
    void Parse(const std::string& subBuffer);
    void Invalidate() {
        parsed = false;
    }
    bool IsParsed() const {
        return parsed;
    }
    SubBufferRect ComputeRect(float progress, long startMS, long endMS, int bufferWi, int bufferHi);

private:
    bool parsed = false;
    bool hasCurve[6] = { false, false, false, false, false, false };
    float statics[6] = { 0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 0.0f };
    ValueCurve curves[6];
};

class PixelBufferClass {
private:
    class LayerInfo {
    public:
        LayerInfo(RenderContext* ctx, PixelBufferClass *p, const Model *m) :
            buffer(ctx, p, m) {
            inMaskFactor = 1.0;
            outMaskFactor = 1.0;
            blur = 0;
            rotation = 0.0f;
            rotations = 0;
            zoom = 1.0f;
            zoomquality = 1;
            rotationorder = "X, Y, Z";
            pivotpointx = 50;
            pivotpointy = 50;
            xrotation = 0;
            yrotation = 0;
            xpivot = 50;
            ypivot = 0;
            sparkle_count = 0;
            use_music_sparkle_count = false;
            music_sparkle_count_factor = 1.0;
            brightness = 100;
            hueadjust = 0;
            saturationadjust = 0;
            valueadjust = 0;
            contrast = 0;
            fadeFactor = 1.0;
            mixType = MixTypes::Mix_Normal;
            effectMixThreshold = 0.0;
            effectMixVaries = false;
            brightnessLevel = false;
            canvas = false;
            BufferHt = BufferWi = BufferOffsetX = BufferOffsetY = 0;
            persistent = false;
            modelBuffers = nullptr;
            freezeAfterFrame = 10000;
            suppressUntil = 0;
            fadeInSteps = fadeOutSteps = 0;
            inTransitionAdjust = outTransitionAdjust = 0;
            inTransitionReverse = outTransitionReverse = false;
            inTransitionBlur = outTransitionBlur = 0;
            stagger = 0;
        }
        RenderBuffer buffer;
        std::string bufferType;
        std::string camera;
        std::string bufferTransform;
        std::string subBuffer;
        std::string blurValueCurve;
        std::string sparklesValueCurve;
        std::string brightnessValueCurve;
        std::string hueAdjustValueCurve;
        std::string saturationAdjustValueCurve;
        std::string valueAdjustValueCurve;
        std::string rotationValueCurve;
        std::string xrotationValueCurve;
        std::string yrotationValueCurve;
        std::string zoomValueCurve;
        std::string rotationsValueCurve;
        std::string pivotpointxValueCurve;
        std::string pivotpointyValueCurve;
        std::string xpivotValueCurve;
        std::string ypivotValueCurve;
        std::string rotoZoom;
        int BufferHt;
        int BufferWi;
        int BufferOffsetX;
        int BufferOffsetY;
        ValueCurve BlurValueCurve;
        ValueCurve SparklesValueCurve;
        ValueCurve BrightnessValueCurve;
        ValueCurve HueAdjustValueCurve;
        ValueCurve SaturationAdjustValueCurve;
        ValueCurve ValueAdjustValueCurve;
        ValueCurve RotationValueCurve;
        ValueCurve XRotationValueCurve;
        ValueCurve YRotationValueCurve;
        ValueCurve ZoomValueCurve;
        ValueCurve RotationsValueCurve;
        ValueCurve PivotPointXValueCurve;
        ValueCurve PivotPointYValueCurve;
        ValueCurve XPivotValueCurve;
        ValueCurve YPivotValueCurve;
        ValueCurve InTransitionAdjustValueCurve;
        ValueCurve OutTransitionAdjustValueCurve;
        int sparkle_count;
        bool use_music_sparkle_count = false;
        float music_sparkle_count_factor;
        int blur;
        int rotation;
        int xrotation;
        int yrotation;
        float rotations;
        float zoom;
        int zoomquality;
        std::string rotationorder;
        bool brightnessLevel = false;
        int pivotpointx;
        int pivotpointy;
        int xpivot;
        int ypivot;
        int brightness;
        int hueadjust;
        int saturationadjust;
        int valueadjust;
        int contrast;
        double fadeFactor = 1.0f;
        MixTypes mixType;
        float effectMixThreshold;
        bool effectMixVaries;
        bool canvas = false;
        bool persistent = false;
        bool renderingDisabled = false;
        int fadeInSteps;
        int fadeOutSteps;
        std::string inTransitionType;
        std::string outTransitionType;
        std::string type;
        std::string transform;
        int inTransitionAdjust;
        int outTransitionAdjust;
        bool inTransitionReverse;
        bool outTransitionReverse;
        int inTransitionBlur;
        int outTransitionBlur;
        float inMaskFactor = 1.0f;
        float outMaskFactor = 1.0f;
        int stagger;

        std::vector<std::unique_ptr<RenderBuffer>>* modelBuffers = nullptr;
        std::vector<std::unique_ptr<RenderBuffer>> shallowModelBuffers;
        std::vector<std::unique_ptr<RenderBuffer>> deepModelBuffers;

        // Flattened node/coord walks for the "Per Model" group <-> member-model
        // pixel copies, built once per layer settings change.
        struct PerModelCopyMap {
            struct Op {
                int32_t dst;
                int32_t src; // < 0 when the source coord falls outside its buffer
            };
            struct ModelOps {
                std::vector<Op> unmerge; // group pixel -> model pixel
                std::vector<Op> merge;   // model pixel -> group pixel
                size_t nodeStart = 0;
                size_t nodeCount = 0;
                int wi = 0;
                int ht = 0;
                bool dmx = false; // model SetPixel() does DMX channel translation, not a plain store
            };
            std::vector<ModelOps> models;
            const std::vector<std::unique_ptr<RenderBuffer>>* builtFor = nullptr;
            size_t groupNodeCount = 0;
            int groupWi = 0;
            int groupHt = 0;
            // Two member models write the same group-buffer cell, so the merge
            // result depends on model order (last model wins). Merging across
            // models in parallel would make that racy, so it stays serial.
            bool groupOverlap = false;
            bool valid = false;

            void invalidate() {
                valid = false;
            }
            void clear() {
                models.clear();
                builtFor = nullptr;
                groupNodeCount = 0;
                groupWi = groupHt = 0;
                groupOverlap = false;
                valid = false;
            }
        };
        PerModelCopyMap perModelMap;
        bool isChromaKey = false;
        xlColor chromaKeyColour = xlBLACK;
        xlColor sparklesColour = xlWHITE;
        int chromaSensitivity = 1;
        int freezeAfterFrame = 99999;
        int suppressUntil = 0;

        std::vector<uint8_t> maskVector;
        uint8_t* mask = nullptr;
        size_t maskSize = 0;
        size_t maskMaxSize = 0;

        SubBufferSpec subBufferSpec;
        SubBufferRect subBufferRect;
        bool subBufferRectValid = false;
        int fullBufferWi = 0;
        int fullBufferHt = 0;

        void renderTransitions(bool isFirstFrame, RenderBuffer* prevRB);
        void calculateMask(const std::string& type, bool mode, bool isFirstFrame);
        bool isMasked(int x, int y);

        void clear();

        float outputHueAdjust = 0.0f;
        float outputSaturationAdjust = 0.0f;
        float outputValueAdjust = 0.0f;
        bool needsHSVAdjust = false;
        int outputSparkleCount = 0;
        int outputBrightnessAdjust = 100;
        float outputEffectMixThreshold = 0.0f;

        struct ColorAdjust {
            float hueAdjust = 0.0f;
            float saturationAdjust = 0.0f;
            float valueAdjust = 0.0f;
            int brightnessAdjust = 100;
            bool needsHSVAdjust = false;
        };
        // The output* members above are only refreshed for layers that get
        // rendered this frame - a frozen layer (or the canvas layer itself,
        // which is blended before its own transitions run) keeps stale ones, and
        // the node blend depends on that. So the pixel-grid path keeps its own
        // copy rather than sharing them.
        ColorAdjust mixedColorAdjust;
        int mixedColorAdjustPeriod = -1;

        void calculateColorAdjust(int effectPeriod, ColorAdjust& adjust);
        void calculateNodeOutputParams(int effectPeriod);

    private:
        void blurMaskToAlpha(int blurAmount);
        void createSquareExplodeMask(bool end);
        void createCircleExplodeMask(bool end);
        void createWipeMask(bool end);
        void createFromMiddleMask(bool end);
        void createClockMask(bool end);
        void createBlindsMask(bool end);
        void createBlendMask(bool end);
        void createSlideChecksMask(bool end);
        void createSlideBarsMask(bool end);

        friend class MetalPixelBufferComputeData;
        friend class VulkanPixelBufferComputeData;
    };

    PixelBufferClass(const PixelBufferClass& cls);
    PixelBufferClass& operator=(const PixelBufferClass&);
    int numLayers = 0;
    std::vector<LayerInfo*> layers;
    std::vector<uint16_t> sparklesVector;
    std::vector<uint32_t> blendDataBuffer;
    uint16_t *sparkles = nullptr;
    int frameTimeInMs = 50;
    // Set once in reset() from layers[0]'s node/model set -- valid for the
    // buffer's whole render-job lifetime (models are fixed once assigned;
    // a model settings edit bumps the model generation and rebuilds the
    // render tree, which reallocates this buffer via a fresh reset()).
    bool anyDimmingCurve = false;

    // both fg and bg may be modified, bg will contain the new, mixed color to be the bg for the next mix
    void mixColors(int x, int y, xlColor& fg, xlColor& bg, int layer);
    void reset(int layers, int timing, bool isNode = false);
    void Blur(LayerInfo* layer, float offset);
    void RotoZoom(LayerInfo* layer, float offset);
    void RotateX(RenderBuffer& buffer, GPURenderUtils::RotoZoomSettings& settings);
    void RotateY(RenderBuffer& buffer, GPURenderUtils::RotoZoomSettings& settings);
    void RotateZAndZoom(RenderBuffer& buffer, GPURenderUtils::RotoZoomSettings& settings);

    void GetMixedColor(int node, const std::vector<bool>& validLayers, int EffectPeriod, int saveLayer, bool saveToPixels);

    void BuildPerModelCopyMap(LayerInfo* inf);
    bool PerModelCopyMapUsable(const LayerInfo* inf) const;
    void UnMergeBuffersForLayerScalar(int layer);
    void MergeBuffersForLayerScalar(int layer);

    std::string modelName;
    std::string lastBufferType;
    std::string lastCamera;
    std::string lastBufferTransform;
    const Model* model = nullptr;
    Model* zbModel = nullptr;
    SingleLineModel* ssModel = nullptr;
    RenderContext* renderContext = nullptr;

public:
    static std::vector<std::string> GetMixTypes();
    // Hoists the per-layer curve/HSV params GetMixedColor(x, y, ...) needs. Call
    // once (single threaded) for an EffectPeriod before running the pixel loop;
    // GetMixedColor recomputes them itself for any layer that was missed.
    void PrepareMixedColorParams(const std::vector<bool>& validLayers, int EffectPeriod);
    void GetMixedColor(int x, int y, xlColor& c, const std::vector<bool>& validLayers, int EffectPeriod);
    void GetNodeChannelValues(size_t nodenum, unsigned char* buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char* buf);
    xlColor GetNodeColor(size_t nodenum) const;
    const xlColor &GetNodeMaskColor(size_t nodenum) const;
    uint32_t NodeStartChannel(size_t nodenum) const;
    uint32_t GetNodeCount() const;
    uint32_t GetChanCountPerNode() const;
    MixTypes GetMixType(int layer) const;
    bool IsCanvasMix(int layer) const;
    int GetFrameTimeInMS() const {
        return frameTimeInMs;
    }

    bool IsVariableSubBuffer(int layer) const;
    void PrepareVariableSubBuffer(int EffectPeriod, int layer);

    PixelBufferClass(RenderContext* ctx);
    virtual ~PixelBufferClass();

    const std::string& GetModelName() const {
        return modelName;
    }
    const Model* GetModel() const {
        return model;
    }

    RenderBuffer& BufferForLayer(int i, int idx);
    uint32_t BufferCountForLayer(int i);
    void UnMergeBuffersForLayer(int i);
    void MergeBuffersForLayer(int i);

    int GetLayerCount() const;
    void InitBuffer(const Model& pbc, int layers, int timing);
    void InitStrandBuffer(const Model& pbc, int strand, int timing, int layers);
    void InitNodeBuffer(const Model& pbc, int strand, int node, int timing);
    void InitPerModelBuffers(const ModelGroup& model, int layer, int timing);
    void InitPerModelBuffersDeep(const ModelGroup& model, int layer, int timing);

    void Clear(int which);

    void SetLayerSettings(int layer, const SettingsMap& settings, bool layerEnabled);
    bool IsRenderingDisabled(int layer) const;
    bool IsPersistent(int layer);
    int GetFreezeFrame(int layer);
    int GetSuppressUntil(int layer);

    void SetMixType(int layer, const std::string& MixName);
    void SetPalette(int layer, xlColorVector& newcolors, xlColorCurveVector& newcc);
    void SetLayer(int newlayer, int period, bool ResetState);
    void SetTimes(int layer, int startTime, int endTime);

    void HandleLayerBlurZoom(int EffectPeriod, int layer);
    void HandleLayerTransitions(int EffectPeriod, int layer);
    void CalcOutput(int EffectPeriod, const std::vector<bool>& validLayers, int saveLayer = 0, bool saveToPixels = false);
    void SetColors(int layer, const unsigned char* fdata, unsigned int numChannels);
    void GetColors(unsigned char* fdata, const std::vector<bool>& restrictRange, unsigned int numChannels);

    // place for GPU Renderers to attach extra data/objects it needs
    void* gpuRenderData = nullptr;
    friend class MetalPixelBufferComputeData;
    friend class VulkanPixelBufferComputeData;
    friend class ISPCComputeUtilities;
};

typedef std::unique_ptr<PixelBufferClass> PixelBufferClassPtr;
