/***************************************************************
 * Name:      PixelBuffer.cpp
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

#include "PixelBuffer.h"
#include <wx/image.h>
#include <wx/tokenzr.h>
#include "DimmingCurve.h"
#include "models/ModelManager.h"
#include "models/SingleLineModel.h"
#include "UtilClasses.h"
#include "AudioManager.h"
#include <log4cpp/Category.hh>

#include <random>

// This is needed for visual studio
#ifdef _MSC_VER
#define M_PI_2 1.57079632679489661923
#endif

PixelBufferClass::PixelBufferClass(xLightsFrame *f, bool b) : frame(f)
{
    numLayers = 0;
    zbModel = nullptr;
    ssModel = nullptr;
    onlyOnMain = b;
}

PixelBufferClass::~PixelBufferClass()
{
    if (ssModel != nullptr)
    {
        delete ssModel;
    }
    if (zbModel != nullptr)
    {
        delete zbModel;
    }
    for (int x = 0; x < numLayers; x++)
    {
        delete layers[x];
    }
}


void PixelBufferClass::reset(int nlayers, int timing)
{
    for (int x = 0; x < numLayers; x++)
    {
        delete layers[x];
    }
    layers.clear();
    frameTimeInMs = timing;

    numLayers = nlayers;
    layers.resize(nlayers);

    for (int x = 0; x < numLayers; x++)
    {
        layers[x] = new LayerInfo(frame, onlyOnMain);
        layers[x]->buffer.SetFrameTimeInMs(frameTimeInMs);
        model->InitRenderBufferNodes("Default", "None", layers[x]->Nodes, layers[x]->BufferWi, layers[x]->BufferHt);
        layers[x]->bufferType = "Default";
        layers[x]->bufferTransform = "None";
        layers[x]->subBuffer = "";
        layers[x]->brightnessValueCurve = "";
        layers[x]->blurValueCurve = "";
        layers[x]->rotationValueCurve = "";
        layers[x]->zoomValueCurve = "";
        layers[x]->rotationsValueCurve = "";
        layers[x]->pivotpointxValueCurve = "";
        layers[x]->pivotpointyValueCurve = "";
        layers[x]->buffer.InitBuffer(layers[x]->BufferHt, layers[x]->BufferWi);
    }
}


void PixelBufferClass::InitBuffer(const Model &pbc, int layers, int timing, bool zeroBased)
{
    modelName = pbc.name;
    if (zeroBased)
    {
        zbModel = pbc.GetModelManager().CreateModel(pbc.GetModelXml(), zeroBased);
        model = zbModel;
    }
    else
    {
        model = &pbc;
    }
    reset(layers, timing);
}
void PixelBufferClass::InitStrandBuffer(const Model &pbc, int strand, int timing)
{
    if (ssModel == nullptr) {
        ssModel = new SingleLineModel(pbc.GetModelManager());
    }
        
    ssModel->Reset(pbc.GetStrandLength(strand), pbc, strand);
    model = ssModel;
    reset(2, timing);
}
void PixelBufferClass::InitNodeBuffer(const Model &pbc, int strand, int node, int timing)
{
    if (ssModel == nullptr) {
        ssModel = new SingleLineModel(pbc.GetModelManager());
    }
    ssModel->Reset(1, pbc, strand, node);
    model = ssModel;
    reset(2, timing);
}

void PixelBufferClass::Clear(int which)
{
    xlColor bgColor(0, 0, 0, 0);
    if (which != -1)
    {
        layers[which]->buffer.Clear(bgColor); //just clear this one
    }
    else
    {
        //clear them all
        for (size_t i = 0; i < numLayers; i++)
        {
            layers[i]->buffer.Clear(bgColor);
        }
    }
}

void PixelBufferClass::GetNodeChannelValues(size_t nodenum, unsigned char *buf)
{
    layers[0]->Nodes[nodenum]->GetForChannels(buf);
}
void PixelBufferClass::SetNodeChannelValues(size_t nodenum, const unsigned char *buf)
{
    layers[0]->Nodes[nodenum]->SetFromChannels(buf);
}
xlColor PixelBufferClass::GetNodeColor(size_t nodenum) const
{
    xlColor color;
    layers[0]->Nodes[nodenum]->GetColor(color);
    return color;
}
int PixelBufferClass::NodeStartChannel(size_t nodenum) const
{
    return layers[0]->Nodes.size() && nodenum < layers[0]->Nodes.size() ? layers[0]->Nodes[nodenum]->ActChan: 0;
}
int PixelBufferClass::GetNodeCount() const
{
    return layers[0]->Nodes.size();
}
int PixelBufferClass::GetChanCountPerNode() const
{
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0)
    {
        return 0;
    }
    return layers[0]->Nodes[0]->GetChanCount();
}


bool MixTypeHandlesAlpha(MixTypes mt)
{
    switch (mt)
    {
    case Mix_Normal:
        return true;
    default:
        return false;
    }
}

// convert MixName to MixType enum
void PixelBufferClass::SetMixType(int layer, const std::string& MixName)
{
    MixTypes MixType;
    if (MixName == "Effect 1")
    {
        MixType=Mix_Effect1;
    }
    else if (MixName == "Effect 2")
    {
        MixType=Mix_Effect2;
    }
    else if (MixName == "1 is Mask")
    {
        MixType=Mix_Mask1;
    }
    else if (MixName == "2 is Mask")
    {
        MixType=Mix_Mask2;
    }
    else if (MixName == "1 is Unmask")
    {
        MixType=Mix_Unmask1;
    }
    else if (MixName == "2 is Unmask")
    {
        MixType=Mix_Unmask2;
    }
    else if (MixName == "1 reveals 2")
    {
        MixType=Mix_1_reveals_2;
    }
    else if (MixName == "2 reveals 1")
    {
        MixType=Mix_2_reveals_1;
    }
    else if (MixName == "Shadow 1 on 2")
    {
        MixType=Mix_Shadow_1on2;
    }
    else if (MixName == "Shadow 2 on 1")
    {
        MixType=Mix_Shadow_2on1;
    }
    else if (MixName == "Layered")
    {
        MixType=Mix_Layered;
    }
    else if (MixName == "Normal")
    {
        MixType=Mix_Normal;
    }
    else if (MixName == "Average")
    {
        MixType=Mix_Average;
    }
    else if (MixName == "Bottom-Top")
    {
        MixType=Mix_BottomTop;
    }
    else if (MixName == "Left-Right")
    {
        MixType=Mix_LeftRight;
    }
    else
    {
        MixType=Mix_Effect1;
    }
    layers[layer]->mixType = MixType;
    layers[layer]->buffer.SetAllowAlphaChannel(MixTypeHandlesAlpha(MixType));
}


xlColor PixelBufferClass::mixColors(const wxCoord &x, const wxCoord &y, const xlColor &fg, const xlColor &c1, int layer)
{
    static const int n = 0;  //increase to change the curve of the crossfade
    xlColor c0 = fg;

    HSVValue hsv0;
    HSVValue hsv1;
    bool handlesAlpha = MixTypeHandlesAlpha(layers[layer]->mixType);
    if (!handlesAlpha && layers[layer]->fadeFactor != 1.0)
    {
        //need to fade the first here as we're not mixing anything
        hsv0 = c0.asHSV();
        hsv0.value *= layers[layer]->fadeFactor;
        c0 = hsv0;
    }

    float svthresh = layers[layer]->effectMixThreshold;
    if (layers[layer]->effectMixVaries)
    {
        //vary mix threshold gradually during effect interval -DJ
        layers[layer]->effectMixThreshold = layers[layer]->buffer.GetEffectTimeIntervalPosition();
    }
    if (layers[layer]->effectMixThreshold < 0)
    {
        layers[layer]->effectMixThreshold = 0;
    }

    xlColor c;
    double emt, emtNot;
    switch (layers[layer]->mixType)
    {
    case Mix_Normal:
        c0.alpha = c0.alpha * layers[layer]->fadeFactor * (1.0 - layers[layer]->effectMixThreshold);
        c = c0.AlphaBlend(c1);
        break;
    case Mix_Effect1:
    case Mix_Effect2:
    {
        if (!layers[layer]->effectMixVaries)
        {
            emt = layers[layer]->effectMixThreshold;
            if ((emt > 0.000001) && (emt < 0.99999))
            {
                emtNot = 1-layers[layer]->effectMixThreshold;
                //make cross-fade linear
                emt = cos((M_PI/4)*(pow(2*emt-1,2*n+1)+1));
                emtNot = cos((M_PI/4)*(pow(2*emtNot-1,2*n+1)+1));
            }
            else
            {
                emtNot = layers[layer]->effectMixThreshold;
                emt = 1 - layers[layer]->effectMixThreshold;
            }
        }
        else
        {
            emt = layers[layer]->effectMixThreshold;
            emtNot = 1-layers[layer]->effectMixThreshold;
        }

        xlColor c2(c1);
        if (layers[layer]->mixType == Mix_Effect2)
        {
            c0.Set(c0.Red()*(emtNot),c0.Green()*(emtNot), c0.Blue()*(emtNot));
            c2.Set(c1.Red()*(emt),c1.Green()*(emt), c1.Blue()*(emt));
        }
        else
        {
            c0.Set(c0.Red()*(emt),c0.Green()*(emt), c0.Blue()*(emt));
            c2.Set(c1.Red()*(emtNot),c1.Green()*(emtNot), c1.Blue()*(emtNot));
        }
        c.Set(c0.Red()+c2.Red(), c0.Green()+c2.Green(), c0.Blue()+c2.Blue());
        break;
    }
    case Mix_Mask1:
        // first masks second
        hsv0 = c0.asHSV();
        if (hsv0.value <= layers[layer]->effectMixThreshold)
        {
            // only if effect 1 is black
            c=c1;  // then show the color of effect 2
        }
        else
        {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Mask2:
        // second masks first
        hsv1 = c1.asHSV();
        if (hsv1.value <= layers[layer]->effectMixThreshold)
        {
            c=c0;
        }
        else
        {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Unmask1:
        // first unmasks second
        c0.toHSV(hsv0);
        c1.toHSV(hsv1);
        if (hsv0.value > layers[layer]->effectMixThreshold)
        {
            // if effect 1 is non black
            hsv1.value = hsv0.value;
            c = hsv1;
        }
        else
        {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Unmask2:
        // second unmasks first
        c0.toHSV(hsv0);
        c1.toHSV(hsv1);
        if (hsv1.value > layers[layer]->effectMixThreshold)
        {
            // if effect 2 is non black
            hsv0.value = hsv1.value;
            c = hsv0;
        }
        else
        {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Shadow_1on2:
        // Effect 1 shadows onto effect 2
        c0.toHSV(hsv0);
        c1.toHSV(hsv1);
        //   if (hsv0.value > effectMixThreshold[layer]) {
        // if effect 1 is non black
        //  to shadow we will shift the hue on the primary layer using the hue and brightness from the
        //  other layer
        if(hsv0.value>0.0) hsv1.hue = hsv1.hue + (hsv0.value*(hsv1.hue-hsv0.hue))/5.0;
        // hsv1.value = hsv0.value;
        //hsv1.saturation = hsv0.saturation;
        c = hsv1;
        //   }
        break;
    case Mix_Shadow_2on1:
        // Effect 2 shadows onto effect 1
        c0.toHSV(hsv0);
        c1.toHSV(hsv1);
//if (hsv1.value > effectMixThreshold[layer]) {
        // if effect 1 is non black
        if(hsv1.value>0.0) hsv0.hue = hsv0.hue + (hsv1.value*(hsv0.hue-hsv1.hue))/2.0;
        //hsv0.value = hsv1.value;
//hsv0.saturation = hsv1.saturation;
        c = hsv0;
        //    }
        break;
    case Mix_Layered:
        c1.toHSV(hsv1);
        if (hsv1.value <= layers[layer]->effectMixThreshold)
        {
            c=c0;
        }
        else
        {
            c=c1;
        }
        break;
    case Mix_Average:
        // only average when both colors are non-black
        if (c0 == xlBLACK)
        {
            c=c1;
        }
        else if (c1 == xlBLACK)
        {
            c=c0;
        }
        else
        {
            c.Set( (c0.Red()+c1.Red())/2, (c0.Green()+c1.Green())/2, (c0.Blue()+c1.Blue())/2 );
        }
        break;
    case Mix_BottomTop:
        c= y < layers[layer]->BufferHt/2 ? c0 : c1;
        break;
    case Mix_LeftRight:
        c= x < layers[layer]->BufferWi/2 ? c0 : c1;
        break;
    case Mix_1_reveals_2:
        c0.toHSV(hsv0);
        c = hsv0.value > layers[layer]->effectMixThreshold ? c0 : c1; // if effect 1 is non black
        break;
    case Mix_2_reveals_1:
        c1.toHSV(hsv1);
        c = hsv1.value > layers[layer]->effectMixThreshold ? c1 : c0; // if effect 2 is non black
        break;
    }
    if (layers[layer]->effectMixVaries)
    {
        layers[layer]->effectMixThreshold = svthresh; //put it back afterwards in case next row didn't change it
    }
    return c;
}


void PixelBufferClass::GetMixedColor(int node, xlColor& c, const std::vector<bool> & validLayers, int EffectPeriod)
{

    unsigned short &sparkle = layers[0]->Nodes[node]->sparkle;
    HSVValue hsv;
    int cnt = 0;
    xlColor color;
    c = xlBLACK;
    for (int layer = numLayers - 1; layer >= 0; layer--)
    {
        if (validLayers[layer])
        {
            int effStartPer, effEndPer;
            layers[0]->buffer.GetEffectPeriods(effStartPer, effEndPer);
            float offset = ((float)EffectPeriod - (float)effStartPer) / ((float)effEndPer - (float)effStartPer);
            offset = std::min(offset, 1.0f);

            int x = layers[layer]->Nodes[node]->Coords[0].bufX;
            int y = layers[layer]->Nodes[node]->Coords[0].bufY;

            if (x < 0 || y < 0
                || x >= layers[layer]->BufferWi
                || y >= layers[layer]->BufferHt
                || layers[layer]->isMasked(x, y)) {
                color = xlBLACK;
                color.alpha = 0;
            } else {
                layers[layer]->buffer.GetPixel(x, y, color);
            }

            // add sparkles
            if ((layers[layer]->music_sparkle_count || layers[layer]->sparkle_count > 0) && color != xlBLACK)
            {
                int sc = layers[layer]->sparkle_count;
                if (layers[layer]->music_sparkle_count && layers[layer]->buffer.GetMedia() != NULL)
                {
                    float f = 0.0;
                    std::list<float>* pf = layers[layer]->buffer.GetMedia()->GetFrameData(layers[layer]->buffer.curPeriod, FRAMEDATA_HIGH, "");
                    if (pf != NULL)
                    {
                        f = *pf->begin();
                    }
                    sc = (int)((float)sc * f);
                }
                switch (sparkle % (208 - sc))
                {
                case 1:
                case 7:
                    // too dim
                    //color.Set("#444444");
                    break;
                case 2:
                case 6:
                    color.Set(0x88, 0x88, 0x88);
                    break;
                case 3:
                case 5:
                    color.Set(0xbb, 0xbb, 0xbb);
                    break;
                case 4:
                    color.Set(255, 255, 255);
                    break;
                }
                sparkle++;
            }
            int b = 0;
            if (layers[layer]->BrightnessValueCurve.IsActive())
            {
                b = (int)layers[layer]->BrightnessValueCurve.GetOutputValueAt(offset);
            }
            else
            {
                b = layers[layer]->brightness;
            }
            if (b != 100 || layers[layer]->contrast != 0)
            {
                hsv = color.asHSV();
                hsv.value = hsv.value * ((double)b/(double)100);

                // Apply Contrast
                if (hsv.value< 0.5)
                {
                    // reduce brightness when below 0.5 in the V value or increase if > 0.5
                    hsv.value = hsv.value - (hsv.value* ((double)layers[layer]->contrast/(double)100));
                }
                else
                {
                    hsv.value = hsv.value + (hsv.value* ((double)layers[layer]->contrast/(double)100));
                }

                if (hsv.value < 0.0) hsv.value=0.0;
                if (hsv.value > 1.0) hsv.value=1.0;
                unsigned char alpha = color.Alpha();
                color = hsv;
                color.alpha = alpha;
            }

            if (MixTypeHandlesAlpha(layers[layer]->mixType))
            {
                c = mixColors(x, y, color, c, layer);
            }
            else
            {
                if (cnt == 0 && layers[layer]->fadeFactor != 1.0)
                {
                    //need to fade the first here as we're not mixing anything
                    color.toHSV(hsv);
                    hsv.value *= layers[layer]->fadeFactor;
                    color = hsv;
                }
                if (cnt > 0)
                {
                    //mix with layer below
                    c = mixColors(x, y, color, c, layer);
                }
                else
                {
                    c = color;
                }
            }

            cnt++;
        }
    }
}

void PixelBufferClass::Blur(LayerInfo* layer, float offset)
{
    int b = 0;
    if (layer->BlurValueCurve.IsActive())
    {
        b = (int)layer->BlurValueCurve.GetOutputValueAt(offset);
    }
    else
    {
        b = layer->blur;
    }
    wxASSERT(b <= 15);

    int d = 0;
    int u = 0;
    if (b % 2 == 0)
    {
        d = b / 2;
        u = (b - 1) / 2;
    }
    else
    {
        d = (b - 1) / 2;
        u = (b - 1) / 2;
    }

    xlColor c;
    RenderBuffer orig(layer->buffer);
    for (int x = 0; x < layer->BufferWi; x++)
    {
        for (int y = 0; y < layer->BufferHt; y++)
        {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;
            int sm = 0;
            for (int i = x - d; i <= x + u; i++)
            {
                if (i >= 0 && i < layer->BufferWi)
                {
                    for (int j = y - d; j <= y + u; j++)
                    {
                        if (j >=0 && j < layer->BufferHt)
                        {
                            orig.GetPixel(i, j, c);
                            r += c.Red();
                            g += c.Green();
                            b += c.Blue();
                            a += c.Alpha();
                            ++sm;
                        }
                    }
                }
            }
            xlColor newc(r/sm, g/sm, b/sm, a/sm);
            layer->buffer.SetPixel(x, y, newc);
        }
    }
}

void PixelBufferClass::SetPalette(int layer, xlColorVector& newcolors)
{
    layers[layer]->buffer.SetPalette(newcolors);
}


static const std::string CHOICE_LayerMethod("CHOICE_LayerMethod");
static const std::string SLIDER_EffectLayerMix("SLIDER_EffectLayerMix");
static const std::string CHECKBOX_LayerMorph("CHECKBOX_LayerMorph");
static const std::string TEXTCTRL_Fadein("TEXTCTRL_Fadein");
static const std::string TEXTCTRL_Fadeout("TEXTCTRL_Fadeout");
static const std::string SLIDER_Blur("SLIDER_Blur");
static const std::string SLIDER_Zoom("SLIDER_Zoom");
static const std::string SLIDER_Rotation("SLIDER_Rotation");
static const std::string SLIDER_Rotations("SLIDER_Rotations");
static const std::string SLIDER_ZoomQuality("SLIDER_ZoomQuality");
static const std::string SLIDER_PivotPointX("SLIDER_PivotPointX");
static const std::string SLIDER_PivotPointY("SLIDER_PivotPointY");

static const std::string CHECKBOX_OverlayBkg("CHECKBOX_OverlayBkg");
static const std::string CHOICE_BufferStyle("CHOICE_BufferStyle");
static const std::string CHOICE_BufferTransform("CHOICE_BufferTransform");
static const std::string CUSTOM_SubBuffer("CUSTOM_SubBuffer");
static const std::string VALUECURVE_Blur("VALUECURVE_Blur");
static const std::string VALUECURVE_Brightness("VALUECURVE_Brightness");
static const std::string VALUECURVE_Zoom("VALUECURVE_Zoom");
static const std::string VALUECURVE_Rotation("VALUECURVE_Rotation");
static const std::string VALUECURVE_Rotations("VALUECURVE_Rotations");
static const std::string VALUECURVE_PivotPointX("VALUECURVE_PivotPointX");
static const std::string VALUECURVE_PivotPointY("VALUECURVE_PivotPointY");
static const std::string STR_DEFAULT("Default");
static const std::string STR_EMPTY("");

static const std::string SLIDER_SparkleFrequency("SLIDER_SparkleFrequency");
static const std::string CHECKBOX_MusicSparkles("CHECKBOX_MusicSparkles");
static const std::string SLIDER_Brightness("SLIDER_Brightness");
static const std::string SLIDER_Contrast("SLIDER_Contrast");
static const std::string STR_NORMAL("Normal");
static const std::string STR_NONE("None");
static const std::string STR_FADE("Fade");

static const std::string CHOICE_In_Transition_Type("CHOICE_In_Transition_Type");
static const std::string CHOICE_Out_Transition_Type("CHOICE_Out_Transition_Type");
static const std::string SLIDER_In_Transition_Adjust("SLIDER_In_Transition_Adjust");
static const std::string SLIDER_Out_Transition_Adjust("SLIDER_Out_Transition_Adjust");
static const std::string CHECKBOX_In_Transition_Reverse("CHECKBOX_In_Transition_Reverse");
static const std::string CHECKBOX_Out_Transition_Reverse("CHECKBOX_Out_Transition_Reverse");

void ComputeValueCurve(const std::string& valueCurve, ValueCurve& theValueCurve)
{
    if (valueCurve == STR_EMPTY) {
        theValueCurve.SetDefault();
        return;
    }

    theValueCurve.Deserialise(valueCurve);
}

void ComputeSubBuffer(const std::string &subBuffer, std::vector<NodeBaseClassPtr> &newNodes, int &bufferWi, int &bufferHi) {
    if (subBuffer == STR_EMPTY) {
        return;
    }
    wxArrayString v = wxSplit(subBuffer, 'x');
    float x1 = v.size() > 0 ? wxAtof(v[0]) : 0.0;
    float y1 = v.size() > 1 ? wxAtof(v[1]) : 0.0;
    float x2 = v.size() > 2 ? wxAtof(v[2]) : 100.0;
    float y2 = v.size() > 3 ? wxAtof(v[3]) : 100.0;

    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    
    x1 *= (float)bufferWi;
    x2 *= (float)bufferWi;
    y1 *= (float)bufferHi;
    y2 *= (float)bufferHi;
    x1 /= 100.0;
    x2 /= 100.0;
    y1 /= 100.0;
    y2 /= 100.0;
    
    for (size_t x = 0; x < newNodes.size(); x++) {
        for (auto it2 = newNodes[x]->Coords.begin(); it2 != newNodes[x]->Coords.end(); it2++) {
            it2->bufX -= x1;
            it2->bufY -= y1;
        }
    }
    bufferWi = int(std::ceil(x2 - x1));
    bufferHi = int(std::ceil(y2 - y1));
    if (bufferWi < 1) bufferWi = 1;
    if (bufferHi < 1) bufferHi = 1;
}

void PixelBufferClass::SetLayerSettings(int layer, const SettingsMap &settingsMap) {
    LayerInfo *inf = layers[layer];
    inf->persistent = settingsMap.GetBool(CHECKBOX_OverlayBkg);
    inf->mask.clear();

    inf->fadeInSteps = (int)(settingsMap.GetDouble(TEXTCTRL_Fadein, 0.0)*1000)/frameTimeInMs;
    inf->fadeOutSteps = (int)(settingsMap.GetDouble(TEXTCTRL_Fadeout, 0.0)*1000)/frameTimeInMs;

    inf->inTransitionType = settingsMap.Get(CHOICE_In_Transition_Type, STR_FADE);
    inf->outTransitionType = settingsMap.Get(CHOICE_Out_Transition_Type, STR_FADE);
    inf->inTransitionAdjust = settingsMap.GetInt(SLIDER_In_Transition_Adjust, 0);
    inf->outTransitionAdjust = settingsMap.GetInt(SLIDER_Out_Transition_Adjust, 0);
    inf->inTransitionReverse = settingsMap.GetBool(CHECKBOX_In_Transition_Reverse);
    inf->outTransitionReverse = settingsMap.GetBool(CHECKBOX_Out_Transition_Reverse);

    inf->blur = settingsMap.GetInt(SLIDER_Blur, 1);
    inf->rotation = settingsMap.GetInt(SLIDER_Rotation, 0);
    inf->rotations = (float)settingsMap.GetInt(SLIDER_Rotations, 0) / 10.0f;
    inf->zoom = (float)settingsMap.GetInt(SLIDER_Zoom, 10) / 10.0f;
    inf->zoomquality = settingsMap.GetInt(SLIDER_ZoomQuality, 1);
    inf->pivotpointx = settingsMap.GetInt(SLIDER_PivotPointX, 50);
    inf->pivotpointy = settingsMap.GetInt(SLIDER_PivotPointY, 50);
    inf->sparkle_count = settingsMap.GetInt(SLIDER_SparkleFrequency, 0);
    inf->music_sparkle_count = settingsMap.GetBool(CHECKBOX_MusicSparkles, false);

    inf->brightness = settingsMap.GetInt(SLIDER_Brightness, 100);
    inf->contrast=settingsMap.GetInt(SLIDER_Contrast, 0);

    SetMixType(layer, settingsMap.Get(CHOICE_LayerMethod, STR_NORMAL));

    inf->effectMixThreshold = (float)settingsMap.GetInt(SLIDER_EffectLayerMix, 0)/100.0;
    inf->effectMixVaries = settingsMap.GetBool(CHECKBOX_LayerMorph);


    const std::string &type = settingsMap.Get(CHOICE_BufferStyle, STR_DEFAULT);
    const std::string &transform = settingsMap.Get(CHOICE_BufferTransform, STR_NONE);
    const std::string &subBuffer = settingsMap.Get(CUSTOM_SubBuffer, STR_EMPTY);
    const std::string &blurValueCurve = settingsMap.Get(VALUECURVE_Blur, STR_EMPTY);
    const std::string &brightnessValueCurve = settingsMap.Get(VALUECURVE_Brightness, STR_EMPTY);
    const std::string &rotationValueCurve = settingsMap.Get(VALUECURVE_Rotation, STR_EMPTY);
    const std::string &zoomValueCurve = settingsMap.Get(VALUECURVE_Zoom, STR_EMPTY);
    const std::string &rotationsValueCurve = settingsMap.Get(VALUECURVE_Rotations, STR_EMPTY);
    const std::string &pivotpointxValueCurve = settingsMap.Get(VALUECURVE_PivotPointX, STR_EMPTY);
    const std::string &pivotpointyValueCurve = settingsMap.Get(VALUECURVE_PivotPointY, STR_EMPTY);

    if (inf->bufferType != type || inf->bufferTransform != transform || inf->subBuffer != subBuffer || inf->blurValueCurve != blurValueCurve || inf->zoomValueCurve != zoomValueCurve || inf->rotationValueCurve != rotationValueCurve || inf->rotationsValueCurve != rotationsValueCurve || inf->pivotpointxValueCurve != pivotpointxValueCurve || inf->pivotpointyValueCurve != pivotpointyValueCurve || inf->brightnessValueCurve != brightnessValueCurve)
    {
        inf->Nodes.clear();
        model->InitRenderBufferNodes(type, transform, inf->Nodes, inf->BufferWi, inf->BufferHt);
        ComputeSubBuffer(subBuffer, inf->Nodes, inf->BufferWi, inf->BufferHt);
        ComputeValueCurve(brightnessValueCurve, inf->BrightnessValueCurve);
        ComputeValueCurve(blurValueCurve, inf->BlurValueCurve);
        ComputeValueCurve(rotationValueCurve, inf->RotationValueCurve);
        ComputeValueCurve(zoomValueCurve, inf->ZoomValueCurve);
        ComputeValueCurve(rotationsValueCurve, inf->RotationsValueCurve);
        ComputeValueCurve(pivotpointxValueCurve, inf->PivotPointXValueCurve);
        ComputeValueCurve(pivotpointyValueCurve, inf->PivotPointYValueCurve);
        inf->bufferType = type;
        inf->bufferTransform = transform;
        inf->subBuffer = subBuffer;
        inf->blurValueCurve = blurValueCurve;
        inf->brightnessValueCurve = brightnessValueCurve;
        inf->zoomValueCurve = zoomValueCurve;
        inf->rotationValueCurve = rotationValueCurve;
        inf->rotationsValueCurve = rotationsValueCurve;
        inf->pivotpointxValueCurve = pivotpointxValueCurve;
        inf->pivotpointyValueCurve = pivotpointyValueCurve;
        inf->buffer.InitBuffer(inf->BufferHt, inf->BufferWi);
    }
}
bool PixelBufferClass::IsPersistent(int layer) {
    return layers[layer]->persistent;
}

RenderBuffer& PixelBufferClass::BufferForLayer(int layer)
{
    return layers[layer]->buffer;
}
void PixelBufferClass::SetLayer(int newlayer, int period, bool resetState)
{
    CurrentLayer=newlayer;
    layers[CurrentLayer]->buffer.SetState(period, resetState, modelName);
}

void PixelBufferClass::SetTimes(int layer, int startTime, int endTime)
{
    layers[layer]->buffer.SetEffectDuration(startTime, endTime);
}
void PixelBufferClass::SetColors(int layer, const unsigned char *fdata)
{
    for (size_t n = 0; n < layers[layer]->Nodes.size(); n++)
    {
        int start = NodeStartChannel(n);
        SetNodeChannelValues(n, &fdata[start]);
        xlColor color;
        layers[layer]->Nodes[n]->GetColor(color);
        for (size_t x = 0; x < layers[layer]->Nodes[n]->Coords.size(); x++)
        {
            layers[layer]->buffer.SetPixel(layers[layer]->Nodes[n]->Coords[x].bufX,
                                           layers[layer]->Nodes[n]->Coords[x].bufY, color);
        }
    }
}

void PixelBufferClass::RotoZoom(LayerInfo* layer, float offset)
{
    float zoom = layer->zoom;
    if (layer->ZoomValueCurve.IsActive())
    {
        zoom = layer->ZoomValueCurve.GetOutputValueAt(offset);
    }
    float rotations = layer->rotations;
    float rotationoffset = offset;
    float offsetperrotation = 1.0f;
    if (layer->RotationsValueCurve.IsActive())
    {
        rotations = layer->RotationsValueCurve.GetOutputValueAt(offset);
    }
    if (rotations > 0)
    {
        offsetperrotation = 1.0f / rotations;
    }
    while (rotationoffset > offsetperrotation)
    {
        rotationoffset -= offsetperrotation;
    }
    rotationoffset *= rotations;
    float rotation = (float)layer->rotation / 100.0;
    if (rotations > 0)
    {
        if (layer->RotationValueCurve.IsActive())
        {
            rotation = layer->RotationValueCurve.GetValueAt(rotationoffset);
        }
    }

    if (rotation != 0.0 || zoom != 1.0)
    {
        static const float PI_2 = 6.283185307f;
        xlColor c;
        RenderBuffer orig(layer->buffer);
        int q = layer->zoomquality;
        int cx = layer->pivotpointx;
        if (layer->PivotPointXValueCurve.IsActive())
        {
            cx = layer->PivotPointXValueCurve.GetOutputValueAt(offset);
        }
        int cy = layer->pivotpointy;
        if (layer->PivotPointYValueCurve.IsActive())
        {
            cy = layer->PivotPointYValueCurve.GetOutputValueAt(offset);
        }
        float inc = 1.0 / (float)q;

        float angle = PI_2 * -rotation;
        float xoff = (cx * layer->buffer.BufferWi) / 100.0;
        float yoff = (cy * layer->BufferHt) / 100.0;
        float anglecos = cos(-angle);
        float anglesin = sin(-angle);


        layer->buffer.Clear(xlBLACK);
        for (int x = 0; x < layer->BufferWi; x++)
        {
            for (int i = 0; i < q; i++)
            {
                for (int y = 0; y < layer->BufferHt; y++)
                {
                    orig.GetPixel(x, y, c);
                    for (int j = 0; j < q; j++)
                    {
                        float xx = (float)x + ((float)i * inc) - xoff;
                        float yy = (float)y + ((float)j * inc) - yoff;
                        float u = xoff + anglecos * xx * zoom + anglesin * yy * zoom;
                        if (u >= 0 && u < layer->BufferWi)
                        {
                            float v = yoff + -anglesin * xx * zoom + anglecos * yy * zoom;

                            if (v >= 0 && v < layer->BufferHt)
                            {
                                layer->buffer.SetPixel(u, v, c);
                            }
                        }
                    }
                }
            }
        }
    }
}

void PixelBufferClass::CalcOutput(int EffectPeriod, const std::vector<bool> & validLayers)
{
    xlColor color;
    HSVValue hsv;
    int curStep;

    // blur all the layers if necessary ... before the merge?
    for (int layer = 0; layer < numLayers; layer++)
    {
        int effStartPer, effEndPer;
        layers[layer]->buffer.GetEffectPeriods(effStartPer, effEndPer);
        float offset = ((float)EffectPeriod - (float)effStartPer) / ((float)effEndPer - (float)effStartPer);
        offset = std::min(offset, 1.0f);

        // do gausian blur
        if (layers[layer]->BlurValueCurve.IsActive() || layers[layer]->blur > 1)
        {
            Blur(layers[layer], offset);
        }
        RotoZoom(layers[layer], offset);
    }

    for(int ii=0; ii < numLayers; ii++)
    {
        double fadeInFactor=1, fadeOutFactor=1;
        layers[ii]->fadeFactor = 1.0;
        layers[ii]->inMaskFactor = 1.0;
        layers[ii]->outMaskFactor = 1.0;
        if( layers[ii]->fadeInSteps > 0 || layers[ii]->fadeOutSteps > 0)
        {
            int effStartPer, effEndPer;
            layers[ii]->buffer.GetEffectPeriods(effStartPer, effEndPer);
            if (EffectPeriod < (effStartPer)+layers[ii]->fadeInSteps)
            {
                curStep = EffectPeriod - effStartPer + 1;
                fadeInFactor = (double)curStep/(double)layers[ii]->fadeInSteps;
            }
            if (EffectPeriod > (effEndPer)-layers[ii]->fadeOutSteps)
            {
                curStep = EffectPeriod - (effEndPer-layers[ii]->fadeOutSteps);
                fadeOutFactor = 1-(double)curStep/(double)layers[ii]->fadeOutSteps;
            }
            //calc fades
            if (STR_FADE == layers[ii]->inTransitionType) {
                if (fadeInFactor<1) {
                    layers[ii]->fadeFactor = fadeInFactor;
                }
            } else {
                layers[ii]->inMaskFactor = fadeInFactor;
            }
            if (STR_FADE == layers[ii]->outTransitionType) {
                if (fadeOutFactor<1) {
                    if (STR_FADE == layers[ii]->inTransitionType
                        && fadeInFactor<1) {
                        layers[ii]->fadeFactor = (fadeInFactor+fadeOutFactor)/(double)2.0;
                    } else {
                        layers[ii]->fadeFactor = fadeOutFactor;
                    }
                }
            } else {
                layers[ii]->outMaskFactor = fadeOutFactor;
            }
            layers[ii]->calculateMask();
        } else {
            layers[ii]->mask.clear();
        }
    }

    // layer calculation and map to output
    size_t NodeCount = layers[0]->Nodes.size();
    for(size_t i = 0; i < NodeCount; i++)
    {
        if (!layers[0]->Nodes[i]->IsVisible())
        {
            // unmapped pixel - set to black
            layers[0]->Nodes[i]->SetColor(xlBLACK);
        }
        else
        {
            // get blend of two effects
            GetMixedColor(i,
                          color,
                          validLayers, EffectPeriod);

            // Apply dimming curve
            DimmingCurve *curve = layers[0]->Nodes[i]->model->modelDimmingCurve;
            wxASSERT(layers[0]->Nodes[i]->model != NULL);
            if (curve != nullptr)
            {
                curve->apply(color);
            }

            // set color for physical output
            layers[0]->Nodes[i]->SetColor(color);
        }
    }
}


static int DecodeType(const std::string &type)
{
    if (type == "Wipe")
    {
        return 1;
    }
    else if (type == "Clock")
    {
        return 2;
    }
    else if (type == "From Middle")
    {
        return 3;
    }
    else if (type == "Square Explode")
    {
        return 4;
    }
    else if (type == "Circle Explode")
    {
        return 5;
    }
    else if (type == "Blinds")
    {
        return 6;
    }
    else if (type == "Blend")
    {
        return 7;
    }
    else if (type == "Slide Checks")
    {
        return 8;
    }
    else if (type == "Slide Bars")
    {
        return 9;
    }
    return 1;
}


void PixelBufferClass::LayerInfo::createFromMiddleMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
    }
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    
    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }
    
    float step = ((float)buffer.BufferWi / 2.0) * factor;
   
    int x1 = BufferWi / 2 - step;
    int x2 = BufferWi / 2 + step;
    for (int x = 0; x < BufferWi; x++)
    {
        uint8_t c = m1;
        if (x < x1) {
            c = m1;
        } else if (x < x2) {
            c = m2;
        } else {
            c = m1;
        }
        for (int y = 0; y < BufferHt; y++)
        {
            mask[x * BufferHt + y] = c;
        }
    }
}

void PixelBufferClass::LayerInfo::createCircleExplodeMask(bool out) {
    // distance from centre
    // sqrt((x - buffer.BufferWi / 2) ^ 2 + (y - buffer.BufferHt / 2) ^ 2);
    float maxradius = sqrt(((buffer.BufferWi / 2) * (buffer.BufferWi / 2)) + ((buffer.BufferHt / 2) * (buffer.BufferHt / 2)));
    
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
    }
    
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    
    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }
    
    float rad = maxradius * factor;
    
    for (int x = 0; x < BufferWi; x++)
    {
        for (int y = 0; y < BufferHt; y++)
        {
            float radius = sqrt((x - (BufferWi / 2)) * (x - (BufferWi / 2)) + (y - (BufferHt / 2)) * (y - (BufferHt / 2)));
            mask[x * BufferHt + y] = radius < rad ? m2 : m1;
        }
    }
}
void PixelBufferClass::LayerInfo::createSquareExplodeMask(bool out)
{
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
    }

    uint8_t m1 = 255;
    uint8_t m2 = 0;

    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }

    float xstep = ((float)BufferWi / 2.0) * (float)factor;
    float ystep = ((float)BufferHt / 2.0) * (float)factor;

    int x1 = BufferWi / 2 - xstep;
    int x2 = BufferWi / 2 + xstep;
    int y1 = BufferHt / 2 - ystep;
    int y2 = BufferHt / 2 + ystep;
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            uint8_t c;
            if (x < x1 || x > x2 || y < y1 || y > y2) {
                c = m1;
            } else {
                c = m2;
            }
            mask[x * BufferHt + y] = c;
        }
    }
}


static bool isLeft(const wxPoint &a, const wxPoint &b, const wxPoint &test) {
    return ((b.x - a.x)*(test.y - a.y) - (b.y - a.y)*(test.x - a.x)) > 0;
}

void PixelBufferClass::LayerInfo::createWipeMask(bool out)
{
    int adjust = inTransitionAdjust;
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    if (out) {
        adjust = outTransitionAdjust;
        reverse = outTransitionReverse;
        factor = outMaskFactor;
    }
    
    if (reverse) {
        adjust += 50;
        if (adjust >= 100) {
            adjust -= 100;
        }
    }
    
    float angle = 2.0 * M_PI * (float)adjust / 100.0;

    float slope = tan(angle);

    uint8_t m1 = 255;
    uint8_t m2 = 0;
    
    float curx = std::round(factor * ((float)BufferWi - 1.0));
    float cury = std::round(factor * ((float)BufferHt - 1.0));

    if (angle >= 0 && angle < M_PI_2) {
        curx = BufferWi - curx - 1;
        std::swap(m1, m2);
    } else if (angle >= M_PI_2 && angle < M_PI) {
        curx = BufferWi - curx - 1;
        cury = BufferHt - cury - 1;
    } else if (angle >= M_PI && angle < (M_PI + M_PI_2)) {
        cury = BufferHt - cury - 1;
    } else {
        std::swap(m1, m2);
    }
    float endx = curx == -1 ? -5 : -1;
    float endy = slope * (endx - curx) + cury;
    if (slope > 999) {
        //nearly vertical
        endx = curx;
        endy = cury - 10;
    } else if (slope < -999) {
        //nearly vertical
        endx = curx;
        endy = cury + 10;
    }
    wxPoint start(curx, cury);
    wxPoint end(endx, endy);

    // start bottom left 0, 0
    // y = slope * x + y'
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            mask[x * BufferHt + y] = isLeft(start, end, wxPoint(x, y)) ? m1 : m2;
        }
    }
}

void PixelBufferClass::LayerInfo::createClockMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
    }
    
    float startradians = 2.0 * M_PI * (float)adjust / 100.0;
    float currentradians = 2.0 * M_PI * factor;
    if (reverse) {
        float tmp = startradians;
        startradians = startradians - currentradians;
        currentradians = tmp;
        if (startradians < 0) {
            startradians += 2.0f * (float)M_PI;
            currentradians += 2.0f * (float)M_PI;
        }
    } else {
        currentradians = startradians + currentradians;
    }
    
    for (int x = 0; x < BufferWi; x++)
    {
        for (int y = 0; y < BufferHt; y++)
        {
            float radianspixel;
            if (x - BufferWi / 2 == 0 && y - BufferHt / 2 == 0)
            {
                radianspixel = 0.0;
            }
            else
            {
                radianspixel = atan2(x - BufferWi / 2,
                                     y - BufferHt / 2);
            }
            if (radianspixel < 0)
            {
                radianspixel += 2.0f * (float)M_PI;
            }
            if (currentradians > 2.0f * (float)M_PI && radianspixel < startradians) {
                radianspixel += 2.0f * (float)M_PI;
            }
            
            bool s_lt_p = radianspixel > startradians;
            bool c_gt_p = radianspixel < currentradians;
            mask[x * BufferHt + y] = (s_lt_p && c_gt_p) ? m2 : m1;
        }
    }
    
}


void PixelBufferClass::LayerInfo::createBlindsMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
    }
    if (adjust == 0) {
        adjust = 1;
    }
    adjust = (buffer.BufferWi / 2) * adjust / 100;
    if (adjust == 0) {
        adjust = 1;
    }
    
    int per = buffer.BufferWi / adjust;
    if (per < 1) {
        per = 1;
    }
    int blinds = buffer.BufferWi / per;
    while (blinds * per < buffer.BufferWi) {
        blinds++;
    }
    int step = std::round(((float)per) * factor);
    int x = 0;
    while (x < BufferWi) {
        for (int z = 0; z < per && x < BufferWi; z++, x++) {
            int c = z < step ? m2 : m1;
            if (reverse) {
                c = (per - z - 1) < step ? m2 : m1;
            }
            for (int y = 0; y < BufferHt; y++) {
                mask[x * BufferHt + y] = c;
            }
    
        }
    }
}

void PixelBufferClass::LayerInfo::createBlendMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
    }
    
    std::minstd_rand rng(1234);
    
    int pixels = BufferWi * BufferHt;
    adjust = 10 * adjust / 100;
    if (adjust == 0) {
        adjust = 1;
    }
    int actualpixels = pixels / (adjust * adjust);
    float step = ((float)pixels / (adjust*adjust)) * factor;
    
    int xpixels = BufferWi / adjust;
    while (xpixels * adjust < BufferWi) {
        xpixels++;
    }
    int ypixels = BufferHt / adjust;
    while (ypixels * adjust < BufferHt) {
        ypixels++;
    }
    
    // set all the background first
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            mask[x * BufferHt + y] = m1;
        }
    }
    
    for (int i = 0; i < step; i++)
    {
        int jy = rng() % actualpixels;
        int jx = rng() % actualpixels;
        
        int x = jx % xpixels * adjust;
        int y = jy % ypixels * adjust;
        if (mask[x * BufferHt + y] == m2) {
            i--;
        } else {
            for (int k = 0; k < adjust; k++) {
                if ((x + k) < BufferWi) {
                    for (int l = 0; l < adjust; l++) {
                        if ((y + l) < BufferHt) {
                            mask[(x + k) * BufferHt + y + l] = m2;
                        }
                    }
                }
            }
        }
    }
}

void PixelBufferClass::LayerInfo::createSlideChecksMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
    }

    if (adjust < 2) {
        adjust = 2;
    }
    adjust = (std::max(buffer.BufferWi / 2, buffer.BufferHt /2)) * adjust / 100;
    if (adjust < 2) {
        adjust = 2;
    }
    int xper = buffer.BufferWi * 2 / adjust ;
    if (xper < 1) {
        xper = 1;
    }
    int yper = buffer.BufferHt / adjust;
    if (yper < 1) {
        yper = 1;
    }
    float step = (((float)xper*2.0) * factor);
    for (int x = 0; x < BufferWi; x++) {
        int xb = x / xper;
        int xp = (x - xb * xper) % xper;
        int xpos = x;
        if (reverse) {
            xpos = BufferWi - x - 1;
        }
        for (int y = 0; y < BufferHt; y++) {
            int yb = y / yper;
            if (yb % 2) {
                if (xp >= (xper / 2)) {
                    int xp2 = xp - xper / 2;
                    mask[xpos * BufferHt + y] = xp2 < step ? m2 : m1;
                } else {
                    int step2 = step - (xper / 2);
                    mask[xpos * BufferHt + y] = xp < step2 ? m2 : m1;
                }
            } else {
                mask[xpos * BufferHt + y] = xp < step ? m2 : m1;
            }
        }
    }
}
void PixelBufferClass::LayerInfo::createSlideBarsMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
    }

    if (adjust == 0) {
        adjust = 1;
    }
    adjust = (BufferHt / 2) * adjust / 100;
    if (adjust == 0) {
        adjust = 1;
    }
    
    int per = BufferHt / adjust;
    if (per < 1) {
        per = 1;
    }
    int blinds = BufferHt / per;
    while (blinds * per < BufferHt) {
        blinds++;
    }
    
    float step = (float)BufferWi * factor;
    for (int y = 0; y < BufferHt; y++) {
        int blind = y / per;
        for (int x = 0; x < BufferWi; x++) {
            int xpos = x;
            if ((blind % 2 == 1) == out) {
                xpos = BufferWi - x - 1;
            }
            mask[xpos * BufferHt + y] = x <= step ? m2 : m1;
        }
    }
}

void PixelBufferClass::LayerInfo::calculateMask() {
    bool hasMask = false;
    if (inMaskFactor < 1.0) {
        mask.resize(BufferHt * BufferWi);
        calculateMask(inTransitionType, false);
        hasMask = true;
    }
    if (outMaskFactor < 1.0) {
        mask.resize(BufferHt * BufferWi);
        calculateMask(outTransitionType, true);
        hasMask = true;
    }
    if (!hasMask) {
        mask.clear();
    }
}
void PixelBufferClass::LayerInfo::calculateMask(const std::string &type, bool mode) {
    switch (DecodeType(type)) {
        case 1:
            createWipeMask(mode);
            break;
        case 2:
            createClockMask(mode);
            break;
        case 3:
            createFromMiddleMask(mode);
            break;
        case 4:
            createSquareExplodeMask(mode);
            break;
        case 5:
            createCircleExplodeMask(mode);
            break;
        case 6:
            createBlindsMask(mode);
            break;
        case 7:
            createBlendMask(mode);
            break;
        case 8:
            createSlideChecksMask(mode);
            break;
        case 9:
            createSlideBarsMask(mode);
            break;
        default:
            break;
    }
}
bool PixelBufferClass::LayerInfo::isMasked(int x, int y) {
    int idx = x*BufferHt + y;
    if (idx < mask.size()) {
        return mask[idx];
    }
    return false;
}

