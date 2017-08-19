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
    Mix_Unmask1, /**<  Effect 2 color shows where Effect 1 is not black */
    Mix_Unmask2, /**<  Effect 1 color shows where Effect 2 is black */
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
			blur = 0;
            rotation = 0.0f;
            rotations = 0;
            zoom = 1.0f;
            zoomquality = 1;
            pivotpointx = 50;
            pivotpointy = 50;
            sparkle_count = 0;
            music_sparkle_count = false;
            brightness = 100;
            hueadjust = 0;
            saturationadjust = 0;
            valueadjust = 0;
            contrast = 0;
            fadeFactor = 0.0;
            mixType = Mix_Normal;
            effectMixThreshold = 0.0;
            effectMixVaries = false;
            BufferHt = BufferWi = 0;
            persistent = false;
            usingModelBuffers = false;

            fadeInSteps = fadeOutSteps = 0;
            inTransitionAdjust = outTransitionAdjust = 0;
            inTransitionReverse = outTransitionReverse = false;
        }
        RenderBuffer buffer;
        std::string bufferType;
        std::string bufferTransform;
        std::string subBuffer;
        std::string blurValueCurve;
        std::string sparklesValueCurve;
        std::string brightnessValueCurve;
        std::string hueAdjustValueCurve;
        std::string saturationAdjustValueCurve;
        std::string valueAdjustValueCurve;
        std::string rotationValueCurve;
        std::string zoomValueCurve;
        std::string rotationsValueCurve;
        std::string pivotpointxValueCurve;
        std::string pivotpointyValueCurve;
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
        ValueCurve ZoomValueCurve;
        ValueCurve RotationsValueCurve;
        ValueCurve PivotPointXValueCurve;
        ValueCurve PivotPointYValueCurve;
        int sparkle_count;
        bool music_sparkle_count;
		int blur;
        int rotation;
        float rotations;
        float zoom;
        int zoomquality;
        int pivotpointx;
        int pivotpointy;
        int brightness;
        int hueadjust;
        int saturationadjust;
        int valueadjust;
        int contrast;
        double fadeFactor;
        MixTypes mixType;
        float effectMixThreshold;
        bool effectMixVaries;
        bool persistent;
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

        std::vector<uint8_t> mask;
        void calculateMask(bool isFirstFrame);
        void calculateMask(const std::string &type, bool mode, bool isFirstFrame);
        bool isMasked(int x, int y);
        
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

    int CurrentLayer;

    void GetMixedColor(int node, xlColor& c, const std::vector<bool> & validLayers, int EffectPeriod);
    xlColor mixColors(const wxCoord &x, const wxCoord &y, const xlColor &c0, const xlColor &c1, int layer);
    void reset(int layers, int timing);
	void Blur(LayerInfo* layer, float offset);
    void RotoZoom(LayerInfo* layer, float offset);

    std::string modelName;
    std::string lastBufferType;
    std::string lastBufferTransform;
    const Model *model;
    Model *zbModel;
    SingleLineModel *ssModel;
    xLightsFrame *frame;
public:
    void GetNodeChannelValues(size_t nodenum, unsigned char *buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char *buf);
    xlColor GetNodeColor(size_t nodenum) const;
    xlColor GetNodeMaskColor(size_t nodenum) const;
    int NodeStartChannel(size_t nodenum) const;
    int GetNodeCount() const;
    int GetChanCountPerNode() const;

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
    
    
    void InitBuffer(const Model &pbc, int layers, int timing, bool zeroBased=false);
    void InitStrandBuffer(const Model &pbc, int strand, int timing, int layers);
    void InitNodeBuffer(const Model &pbc, int strand, int node, int timing);
    void InitPerModelBuffers(const ModelGroup& model, int layer, int timing);

    void Clear(int which);
    
    void SetLayerSettings(int layer, const SettingsMap &settings);
    bool IsPersistent(int layer);
    
    void SetMixType(int layer, const std::string& MixName);
    void SetPalette(int layer, xlColorVector& newcolors, xlColorCurveVector& newcc);
    void SetLayer(int newlayer, int period, bool ResetState);
    void SetTimes(int layer, int startTime, int endTime);

    void CalcOutput(int EffectPeriod, const std::vector<bool> &validLayers);
    void SetColors(int layer, const unsigned char *fdata);    
    void GetColors(unsigned char *fdata, const std::list<NodeRange> &restrictRange);
};
typedef std::unique_ptr<PixelBufferClass> PixelBufferClassPtr;

#endif // PIXELBUFFER_H
