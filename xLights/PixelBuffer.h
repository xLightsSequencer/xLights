#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>

#include "models/Model.h"
#include "models/SingleLineModel.h"
#include "RenderBuffer.h"
#include "ValueCurve.h"
#include "RenderUtils.h"
#include "GPURenderUtils.h"
#include "Color.h"

 /**
 * \brief enumeration of the different techniques used in layering effects
 */

enum class MixTypes
{
    Mix_Normal,  /** Layered with Alpha channel considered **/
    Mix_Effect1, /**<  Effect 1 only */
    Mix_Effect2, /**<  Effect 2 only */
    Mix_Mask1,   /**<  Effect 2 color shows where Effect 1 is black */
    Mix_Mask2,   /**<  Effect 1 color shows where Effect 2 is black */
    Mix_Unmask1, /**<  Effect 2 color shows where Effect 1 is not black but with no fade ... black becomes white*/
    Mix_Unmask2, /**<  Effect 1 color shows where Effect 2 is not black but with no fade ... black becomes white*/
    Mix_TrueUnmask1, /**<  Effect 2 color shows where Effect 1 is not black */
    Mix_TrueUnmask2, /**<  Effect 1 color shows where Effect 2 is black */
    Mix_1_reveals_2,  /**<  Effect 2 color only shows if Effect 1 is black  1 reveals 2 */
    Mix_2_reveals_1,  /**<  Effect 1 color only shows if Effect 2 is black */
    Mix_Layered, /**<  Effect 1 is back ground and shows only when effect 2 is black */
    Mix_Average, /**<  Average color value between effects per pixel */
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

class PixelBufferClass
{
private:
    class LayerInfo {
    public:
        LayerInfo(xLightsFrame *frame) : buffer(frame) {
            inMaskFactor = 0;
            outMaskFactor = 0;
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
            fadeFactor = 0.0;
            mixType = MixTypes::Mix_Normal;
            effectMixThreshold = 0.0;
            effectMixVaries = false;
            canvas = false;
            BufferHt = BufferWi = BufferOffsetX = BufferOffsetY = 0;
            persistent = false;
            modelBuffers = nullptr;
            freezeAfterFrame = 10000;
            suppressUntil = 0;
            fadeInSteps = fadeOutSteps = 0;
            inTransitionAdjust = outTransitionAdjust = 0;
            inTransitionReverse = outTransitionReverse = false;
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
        bool use_music_sparkle_count;
        float music_sparkle_count_factor;
		int blur;
        int rotation;
        int xrotation;
        int yrotation;
        float rotations;
        float zoom;
        int zoomquality;
        std::string rotationorder;
        int pivotpointx;
        int pivotpointy;
        int xpivot;
        int ypivot;
        int brightness;
        int hueadjust;
        int saturationadjust;
        int valueadjust;
        int contrast;
        double fadeFactor;
        MixTypes mixType;
        float effectMixThreshold;
        bool effectMixVaries;
        bool canvas = false;
        bool persistent = false;
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
        float inMaskFactor;
        float outMaskFactor;
        int stagger;

        std::vector<std::unique_ptr<RenderBuffer>> *modelBuffers = nullptr;
        std::vector<std::unique_ptr<RenderBuffer>> shallowModelBuffers;
        std::vector<std::unique_ptr<RenderBuffer>> deepModelBuffers;
        bool isChromaKey = false;
        xlColor chromaKeyColour = xlBLACK;
        xlColor sparklesColour = xlWHITE;
        int chromaSensitivity = 1;
        int freezeAfterFrame = 99999;
        int suppressUntil = 0;

        std::vector<uint8_t> mask;
        void renderTransitions(bool isFirstFrame, const RenderBuffer* prevRB);
        void calculateMask(const std::string &type, bool mode, bool isFirstFrame);
        bool isMasked(int x, int y);

        void clear();
        
        
        float outputHueAdjust;
        float outputSaturationAdjust;
        float outputValueAdjust;
        bool  needsHSVAdjust = false;
        int   outputSparkleCount = 0;
        int   outputBrightnessAdjust = 0;
        float outputEffectMixThreshold;
        
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
    };

    PixelBufferClass(const PixelBufferClass &cls);
    PixelBufferClass &operator=(const PixelBufferClass &);
    int numLayers = 0;
    std::vector<LayerInfo*> layers;
    int frameTimeInMs = 50;

    //both fg and bg may be modified, bg will contain the new, mixed color to be the bg for the next mix
    void mixColors(const wxCoord &x, const wxCoord &y, xlColor &fg, xlColor &bg, int layer);
    void reset(int layers, int timing, bool isNode = false);
	void Blur(LayerInfo* layer, float offset);
    void RotoZoom(LayerInfo* layer, float offset);
    void RotateX(RenderBuffer &buffer, GPURenderUtils::RotoZoomSettings &settings);
    void RotateY(RenderBuffer &buffer, GPURenderUtils::RotoZoomSettings &settings);
    void RotateZAndZoom(RenderBuffer &buffer, GPURenderUtils::RotoZoomSettings &settings);
    
    void GetMixedColor(int node, const std::vector<bool> & validLayers, int EffectPeriod, int saveLayer);

    std::string modelName;
    std::string lastBufferType;
    std::string lastCamera;
    std::string lastBufferTransform;
    const Model *model = nullptr;
    Model *zbModel = nullptr;
    SingleLineModel *ssModel = nullptr;
    xLightsFrame *frame = nullptr;

public:
    static std::vector<std::string> GetMixTypes();
    void GetMixedColor(int x, int y, xlColor& c, const std::vector<bool>& validLayers, int EffectPeriod);
    void GetNodeChannelValues(size_t nodenum, unsigned char *buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char *buf);
    xlColor GetNodeColor(size_t nodenum) const;
    xlColor GetNodeMaskColor(size_t nodenum) const;
    uint32_t NodeStartChannel(size_t nodenum) const;
    uint32_t GetNodeCount() const;
    uint32_t GetChanCountPerNode() const;
    MixTypes GetMixType(int layer) const;
    bool IsCanvasMix(int layer) const;
    int GetFrameTimeInMS() const { return frameTimeInMs; }

    bool IsVariableSubBuffer(int layer) const;
    void PrepareVariableSubBuffer(int EffectPeriod, int layer);

    PixelBufferClass(xLightsFrame *f);
    virtual ~PixelBufferClass();

    const std::string &GetModelName() const { return modelName; }
    const Model* GetModel() const { return model; }

    RenderBuffer &BufferForLayer(int i, int idx);
    uint32_t BufferCountForLayer(int i);
    void UnMergeBuffersForLayer(int i);
    void MergeBuffersForLayer(int i);

    int GetLayerCount() const;
    void InitBuffer(const Model &pbc, int layers, int timing, bool zeroBased=false);
    void InitStrandBuffer(const Model &pbc, int strand, int timing, int layers);
    void InitNodeBuffer(const Model &pbc, int strand, int node, int timing);
    void InitPerModelBuffers(const ModelGroup& model, int layer, int timing);
    void InitPerModelBuffersDeep(const ModelGroup& model, int layer, int timing);

    void Clear(int which);

    void SetLayerSettings(int layer, const SettingsMap &settings);
    bool IsPersistent(int layer);
    int GetFreezeFrame(int layer);
    int GetSuppressUntil(int layer);

    void SetMixType(int layer, const std::string& MixName);
    void SetPalette(int layer, xlColorVector& newcolors, xlColorCurveVector& newcc);
    void SetLayer(int newlayer, int period, bool ResetState);
    void SetTimes(int layer, int startTime, int endTime);

    
    void HandleLayerBlurZoom(int EffectPeriod, int layer);
    void CalcOutput(int EffectPeriod, const std::vector<bool> &validLayers, int saveLayer = 0);
    void SetColors(int layer, const unsigned char *fdata);
    void GetColors(unsigned char *fdata, const std::vector<bool> &restrictRange);

    //place for GPU Renderers to attach extra data/objects it needs
    void *gpuRenderData = nullptr;
};

typedef std::unique_ptr<PixelBufferClass> PixelBufferClassPtr;
