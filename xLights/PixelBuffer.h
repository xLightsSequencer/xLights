/***************************************************************
 * Name:      PixelBuffer.h
 * Purpose:   Implements pixel buffer and effects
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-10-21
 * Copyright: 2012 by Matt Brown
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

#include <wx/xml/xml.h>

#include "models/Model.h"
#include "models/SingleLineModel.h"
#include "RenderBuffer.h"
#include "ValueCurve.h"
#include "RenderUtils.h"
#include "Color.h"

#define BLUR_MIN 1
#define BLUR_MAX 15
#define RZ_ROTATION_MIN 0
#define RZ_ROTATION_MAX 100
#define RZ_ZOOM_MIN 0
#define RZ_ZOOM_MAX 30
#define RZ_ZOOM_DIVISOR 10
#define RZ_ROTATIONS_MIN 0
#define RZ_ROTATIONS_MAX 200
#define RZ_ROTATIONS_DIVISOR 10
#define RZ_PIVOTX_MIN 0
#define RZ_PIVOTX_MAX 100
#define RZ_PIVOTY_MIN 0
#define RZ_PIVOTY_MAX 100
#define RZ_XROTATION_MIN 0
#define RZ_XROTATION_MAX 360
#define RZ_YROTATION_MIN 0
#define RZ_YROTATION_MAX 360
#define RZ_XPIVOT_MIN 0
#define RZ_XPIVOT_MAX 100
#define RZ_YPIVOT_MIN 0
#define RZ_YPIVOT_MAX 100

/**
 * \brief enumeration of the different techniques used in layering effects
 */

enum MixTypes
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
    Mix_Max,
    Mix_Min
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
            ModelBufferHt = 0;
            ModelBufferWi = 0;
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
            mixType = Mix_Normal;
            effectMixThreshold = 0.0;
            effectMixVaries = false;
            canvas = false;
            BufferHt = BufferWi = 0;
            persistent = false;
            usingModelBuffers = false;
            freezeAfterFrame = 10000;
            suppressUntil = 0;
            fadeInSteps = fadeOutSteps = 0;
            inTransitionAdjust = outTransitionAdjust = 0;
            inTransitionReverse = outTransitionReverse = false;
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
        int ModelBufferHt;
        int ModelBufferWi;
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
        bool usingModelBuffers;
        std::vector<std::unique_ptr<RenderBuffer>> modelBuffers;
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
    int numLayers;
    std::vector<LayerInfo*> layers;
    int frameTimeInMs;

    //both fg and bg may be modified, bg will contain the new, mixed color to be the bg for the next mix
    void mixColors(const wxCoord &x, const wxCoord &y, xlColor &fg, xlColor &bg, int layer);
    void reset(int layers, int timing, bool isNode = false);
	void Blur(LayerInfo* layer, float offset);
    void RotoZoom(LayerInfo* layer, float offset);
    void RotateX(LayerInfo* layer, float offset);
    void RotateY(LayerInfo* layer, float offset);
    void RotateZAndZoom(LayerInfo* layer, float offset);
    void GetMixedColor(int node, xlColor& c, const std::vector<bool> & validLayers, int EffectPeriod);

    std::string modelName;
    std::string lastBufferType;
    std::string lastCamera;
    std::string lastBufferTransform;
    const Model *model;
    Model *zbModel;
    SingleLineModel *ssModel;
    xLightsFrame *frame;
public:
    void GetMixedColor(int x, int y, xlColor& c, const std::vector<bool> & validLayers, int EffectPeriod);
    void GetNodeChannelValues(size_t nodenum, unsigned char *buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char *buf);
    xlColor GetNodeColor(size_t nodenum) const;
    xlColor GetNodeMaskColor(size_t nodenum) const;
    int NodeStartChannel(size_t nodenum) const;
    int GetNodeCount() const;
    int GetChanCountPerNode() const;
    MixTypes GetMixType(int layer) const;
    bool IsCanvasMix(int layer) const;
    int GetFrameTimeInMS() const { return frameTimeInMs; }

    bool IsVariableSubBuffer(int layer) const;
    void PrepareVariableSubBuffer(int EffectPeriod, int layer);

    PixelBufferClass(xLightsFrame *f);
    virtual ~PixelBufferClass();

    const std::string &GetModelName() const
    { return modelName;};
    const Model* GetModel() const { return model; }

    RenderBuffer &BufferForLayer(int i, int idx);
    int BufferCountForLayer(int i);
    void MergeBuffersForLayer(int i);

    int GetLayerCount() const;
    void InitBuffer(const Model &pbc, int layers, int timing, bool zeroBased=false);
    void InitStrandBuffer(const Model &pbc, int strand, int timing, int layers);
    void InitNodeBuffer(const Model &pbc, int strand, int node, int timing);
    void InitPerModelBuffers(const ModelGroup& model, int layer, int timing);

    void Clear(int which);

    void SetLayerSettings(int layer, const SettingsMap &settings);
    bool IsPersistent(int layer);
    int GetFreezeFrame(int layer);
    int GetSuppressUntil(int layer);

    void SetMixType(int layer, const std::string& MixName);
    void SetPalette(int layer, xlColorVector& newcolors, xlColorCurveVector& newcc);
    void SetLayer(int newlayer, int period, bool ResetState);
    void SetTimes(int layer, int startTime, int endTime);

    void CalcOutput(int EffectPeriod, const std::vector<bool> &validLayers, int saveLayer = 0);
    void SetColors(int layer, const unsigned char *fdata);
    void GetColors(unsigned char *fdata, const std::vector<bool> &restrictRange);
};
typedef std::unique_ptr<PixelBufferClass> PixelBufferClassPtr;

#endif // PIXELBUFFER_H
