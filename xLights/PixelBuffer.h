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

#include <wx/xml/xml.h>

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
    Mix_Highlight_Vibrant
};

class Effect;
class SequenceElements;
class SettingsMap;
class DimmingCurve;
class ModelGroup;
class MetalPixelBufferComputeData;

class PixelBufferClass {
private:
    class LayerInfo {
    public:
        LayerInfo(xLightsFrame* frame, PixelBufferClass *p, const Model *m) :
            buffer(frame, p, m) {
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
        int BufferHt = 0;
        int BufferWi = 0;
        int BufferDp = 0;
        int BufferOffsetX = 0;
        int BufferOffsetY = 0;
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
        int sparkle_count = 0;
        bool use_music_sparkle_count = false;
        float music_sparkle_count_factor = 1.0;
        int blur = 0;
        int rotation = 0;
        int xrotation = 0;
        int yrotation = 0;
        float rotations = 0.0f;
        float zoom = 1.0f;
        int zoomquality = 1;
        std::string rotationorder = "X, Y, Z";
        int pivotpointx = 50;
        int pivotpointy = 50;
        int xpivot = 50;
        int ypivot = 0;
        int brightness = 100;
        int hueadjust = 0;
        int saturationadjust = 0;
        int valueadjust = 0;
        int contrast = 0;
        double fadeFactor = 1.0;
        MixTypes mixType = MixTypes::Mix_Normal;
        float effectMixThreshold = 0.0f;
        bool effectMixVaries = false;
        bool canvas = false;
        bool persistent = false;
        bool renderingDisabled = false;
        int fadeInSteps = 0;
        int fadeOutSteps = 0;
        std::string inTransitionType;
        std::string outTransitionType;
        std::string type;
        std::string transform;
        int inTransitionAdjust = 0;
        int outTransitionAdjust = 0;
        bool inTransitionReverse = false;
        bool outTransitionReverse = false;
        float inMaskFactor = 1.0f;
        float outMaskFactor = 1.0f;
        int stagger = 0;

        std::vector<std::unique_ptr<RenderBuffer>>* modelBuffers = nullptr;
        std::vector<std::unique_ptr<RenderBuffer>> shallowModelBuffers;
        std::vector<std::unique_ptr<RenderBuffer>> deepModelBuffers;
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

        void renderTransitions(bool isFirstFrame, RenderBuffer* prevRB);
        void calculateMask(const std::string& type, bool mode, bool isFirstFrame);
        bool isMasked(int x, int y, int z);

        void clear();

        float outputHueAdjust = 0.0f;
        float outputSaturationAdjust = 0.0f;
        float outputValueAdjust = 0.0f;
        bool needsHSVAdjust = false;
        int outputSparkleCount = 0;
        int outputBrightnessAdjust = 100;
        float outputEffectMixThreshold = 0.0f;

        void calculateNodeOutputParams(int effectPeriod);

    private:
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
    };

    PixelBufferClass(const PixelBufferClass& cls);
    PixelBufferClass& operator=(const PixelBufferClass&);
    int numLayers = 0;
    std::vector<LayerInfo*> layers;
    std::vector<uint16_t> sparklesVector;
    uint16_t *sparkles = nullptr;
    int frameTimeInMs = 50;

    // both fg and bg may be modified, bg will contain the new, mixed color to be the bg for the next mix
    void mixColors(const wxCoord& x, const wxCoord& y, const wxCoord& z, xlColor& fg, xlColor& bg, int layer);
    void reset(int layers, int timing, bool isNode = false);
    void Blur(LayerInfo* layer, float offset);
    void RotoZoom(LayerInfo* layer, float offset);
    void RotateX(RenderBuffer& buffer, GPURenderUtils::RotoZoomSettings& settings);
    void RotateY(RenderBuffer& buffer, GPURenderUtils::RotoZoomSettings& settings);
    void RotateZAndZoom(RenderBuffer& buffer, GPURenderUtils::RotoZoomSettings& settings);

    void GetMixedColor(int node, const std::vector<bool>& validLayers, int EffectPeriod, int saveLayer, bool saveToPixels);

    std::string modelName;
    std::string lastBufferType;
    std::string lastCamera;
    std::string lastBufferTransform;
    const Model* model = nullptr;
    Model* zbModel = nullptr;
    SingleLineModel* ssModel = nullptr;
    xLightsFrame* frame = nullptr;

public:
    static std::vector<std::string> GetMixTypes();
    void GetMixedColor(int x, int y, int z, xlColor& c, const std::vector<bool>& validLayers, int EffectPeriod);
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

    PixelBufferClass(xLightsFrame* f);
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
    void InitBuffer(const Model& pbc, int layers, int timing, bool zeroBased = false);
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
    void SetColors(int layer, const unsigned char* fdata);
    void GetColors(unsigned char* fdata, const std::vector<bool>& restrictRange);

    // place for GPU Renderers to attach extra data/objects it needs
    void* gpuRenderData = nullptr;
    friend class MetalPixelBufferComputeData;
};

typedef std::unique_ptr<PixelBufferClass> PixelBufferClassPtr;
