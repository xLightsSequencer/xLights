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
#include <wx/tokenzr.h>
#include "DimmingCurve.h"
#include "models/ModelManager.h"
#include "models/SingleLineModel.h"
#include "models/ModelGroup.h"
#include "UtilClasses.h"
#include "AudioManager.h"
#include "xLightsMain.h"
#include <log4cpp/Category.hh>

#include <random>


// This is needed for visual studio
#ifdef _MSC_VER
#define M_PI_2 1.57079632679489661923
#endif

PixelBufferClass::PixelBufferClass(xLightsFrame *f) : frame(f)
{
    numLayers = 0;
    zbModel = nullptr;
    ssModel = nullptr;
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
        layers[x] = new LayerInfo(frame);
        layers[x]->buffer.SetFrameTimeInMs(frameTimeInMs);
        model->InitRenderBufferNodes("Default", "None", layers[x]->buffer.Nodes, layers[x]->BufferWi, layers[x]->BufferHt);
        layers[x]->bufferType = "Default";
        layers[x]->bufferTransform = "None";
        layers[x]->outTransitionType = "Fade";
        layers[x]->inTransitionType = "Fade";
        layers[x]->subBuffer = "";
        layers[x]->brightnessValueCurve = "";
        layers[x]->hueAdjustValueCurve = "";
        layers[x]->saturationAdjustValueCurve = "";
        layers[x]->valueAdjustValueCurve = "";
        layers[x]->blurValueCurve = "";
        layers[x]->sparklesValueCurve = "";
        layers[x]->rotationValueCurve = "";
        layers[x]->xrotationValueCurve = "";
        layers[x]->yrotationValueCurve = "";
        layers[x]->zoomValueCurve = "";
        layers[x]->rotationsValueCurve = "";
        layers[x]->pivotpointxValueCurve = "";
        layers[x]->pivotpointyValueCurve = "";
        layers[x]->xpivotValueCurve = "";
        layers[x]->ypivotValueCurve = "";
        layers[x]->ModelBufferHt = layers[x]->BufferHt;
        layers[x]->ModelBufferWi = layers[x]->BufferWi;
        layers[x]->buffer.InitBuffer(layers[x]->BufferHt, layers[x]->BufferWi, layers[x]->ModelBufferHt, layers[x]->ModelBufferWi, layers[x]->bufferTransform);
    }
}

void PixelBufferClass::InitPerModelBuffers(const ModelGroup &model, int layer, int timing) {
    for (auto it = model.Models().begin(); it != model.Models().end(); ++it) {

        Model *m = *it;
        RenderBuffer *buf = new RenderBuffer(frame);
        buf->SetFrameTimeInMs(timing);
        m->InitRenderBufferNodes("Default", "None", buf->Nodes, buf->BufferWi, buf->BufferHt);
        buf->InitBuffer(buf->BufferHt, buf->BufferWi, buf->BufferHt, buf->BufferWi, "None");
        layers[layer]->modelBuffers.push_back(std::unique_ptr<RenderBuffer>(buf));
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
    reset(layers + 1, timing);
}
void PixelBufferClass::InitStrandBuffer(const Model &pbc, int strand, int timing, int layers)
{
    if (ssModel == nullptr) {
        ssModel = new SingleLineModel(pbc.GetModelManager());
    }

    ssModel->Reset(pbc.GetStrandLength(strand), pbc, strand);
    model = ssModel;
    reset(layers + 1, timing);
}
void PixelBufferClass::InitNodeBuffer(const Model &pbc, int strand, int node, int timing)
{
    modelName = pbc.name;
    if (ssModel == nullptr) {
        ssModel = new SingleLineModel(pbc.GetModelManager());
    }
    ssModel->Reset(1, pbc, strand, node);
    model = ssModel;
    reset(2, timing);
}

void PixelBufferClass::Clear(int which)
{
    if (which != -1)
    {
        layers[which]->buffer.Clear(); //just clear this one
        if (layers[which]->usingModelBuffers) {
            for (auto it = layers[which]->modelBuffers.begin();  it != layers[which]->modelBuffers.end(); ++it) {
                (*it)->Clear();
            }
        }
    }
    else
    {
        //clear them all
        for (size_t i = 0; i < numLayers; i++)
        {
            layers[i]->buffer.Clear();
            if (layers[i]->usingModelBuffers) {
                for (auto it = layers[i]->modelBuffers.begin();  it != layers[i]->modelBuffers.end(); ++it) {
                    (*it)->Clear();
                }
            }
        }
    }
}

void PixelBufferClass::GetNodeChannelValues(size_t nodenum, unsigned char *buf)
{
    layers[0]->buffer.Nodes[nodenum]->GetForChannels(buf);
}
void PixelBufferClass::SetNodeChannelValues(size_t nodenum, const unsigned char *buf)
{
    layers[0]->buffer.Nodes[nodenum]->SetFromChannels(buf);
}
xlColor PixelBufferClass::GetNodeColor(size_t nodenum) const
{
    xlColor color;
    layers[0]->buffer.Nodes[nodenum]->GetColor(color);
    return color;
}
xlColor PixelBufferClass::GetNodeMaskColor(size_t nodenum) const
{
    xlColor color;
    layers[0]->buffer.Nodes[nodenum]->GetMaskColor(color);
    return color;
}
int PixelBufferClass::NodeStartChannel(size_t nodenum) const
{
    return layers[0]->buffer.Nodes.size() && nodenum < layers[0]->buffer.Nodes.size() ? layers[0]->buffer.Nodes[nodenum]->ActChan: 0;
}
int PixelBufferClass::GetNodeCount() const
{
    return layers[0]->buffer.Nodes.size();
}
int PixelBufferClass::GetChanCountPerNode() const
{
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0)
    {
        return 0;
    }
    return layers[0]->buffer.Nodes[0]->GetChanCount();
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
    else if (MixName == "Additive")
    {
        MixType=Mix_Additive;
    }
    else if (MixName == "Subtractive")
    {
        MixType=Mix_Subtractive;
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
    else if (MixName == "Max")
    {
        MixType=Mix_Max;
    }
    else if (MixName == "Min")
    {
        MixType=Mix_Min;
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
    case Mix_Additive:
        {
            int r = c0.red + c1.red;
            int g = c0.green + c1.green;
            int b = c0.blue + c1.blue;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            c.Set(r, g, b);
        }
        break;
    case Mix_Subtractive:
        {
            int r = c1.red - c0.red;
            int g = c1.green - c0.green;
            int b = c1.blue - c0.blue;
            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            c.Set(r, g, b);
        }
        break;

    case Mix_Min:
        {
            int r = std::min(c0.red, c1.red);
            int g = std::min(c0.green, c1.green);
            int b = std::min(c0.blue, c1.blue);
            c.Set(r, g, b);
        }
        break;
    case Mix_Max:
        {
            int r = std::max(c0.red, c1.red);
            int g = std::max(c0.green, c1.green);
            int b = std::max(c0.blue, c1.blue);
            c.Set(r, g, b);
        }
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    unsigned short &sparkle = layers[0]->buffer.Nodes[node]->sparkle;
    int cnt = 0;
    c = xlBLACK;

    for (int layer = numLayers - 1; layer >= 0; layer--)
    {
        if (validLayers[layer])
        {
            auto thelayer = layers[layer];

            // TEMPORARY - THIS SHOULD BE REMOVED BUT I WANT TO SEE WHAT IS CAUSING SOME RANDOM CRASHES - KW - 2017.7
            if (thelayer == nullptr)
            {
                logger_base.crit("PixelBufferClass::GetMixedColor thelayer is nullptr ... this is going to crash.");
            }

            if (node >= thelayer->buffer.Nodes.size())
            {
                //logger_base.crit("PixelBufferClass::GetMixedColor thelayer->buffer.Nodes does not contain node %d as it is only %d in size ... this was going to crash.", node, thelayer->buffer.Nodes.size());
            }
            else
            {
                int effStartPer, effEndPer;
                thelayer->buffer.GetEffectPeriods(effStartPer, effEndPer);
                float offset = ((float)(EffectPeriod - effStartPer)) / ((float)(effEndPer - effStartPer));
                offset = std::min(offset, 1.0f);

                int x = thelayer->buffer.Nodes[node]->Coords[0].bufX;
                int y = thelayer->buffer.Nodes[node]->Coords[0].bufY;

                xlColor color;
                if (thelayer->isMasked(x, y)
                    || x < 0
                    || y < 0
                    || x >= thelayer->BufferWi
                    || y >= thelayer->BufferHt
                    ) {
                    color = xlBLACK;
                    color.alpha = 0;
                }
                else {
                    thelayer->buffer.GetPixel(x, y, color);
                }

                
                float ha = 0.0;
                if (thelayer->HueAdjustValueCurve.IsActive())
                {
                    ha = thelayer->HueAdjustValueCurve.GetOutputValueAt(offset) / 100.0;
                }
                else
                {
                    ha = (float)thelayer->hueadjust / 100.0;
                }
                float sa = 0.0;
                if (thelayer->SaturationAdjustValueCurve.IsActive())
                {
                    sa = thelayer->SaturationAdjustValueCurve.GetOutputValueAt(offset) / 100.0;
                }
                else
                {
                    sa = (float)thelayer->saturationadjust / 100.0;
                }
                
                float va = 0.0;
                if (thelayer->ValueAdjustValueCurve.IsActive())
                {
                    va = thelayer->ValueAdjustValueCurve.GetOutputValueAt(offset) / 100.0;
                }
                else
                {
                    va = (float)thelayer->valueadjust / 100.0;
                }
                
                // adjust for HSV adjustments
                if (ha != 0 || sa != 0 || va != 0) {
                    HSVValue hsv = color.asHSV();

                    if (ha != 0)
                    {
                        hsv.hue += ha;
                        if (hsv.hue < 0)
                        {
                            hsv.hue += 1.0;
                        }
                        else if (hsv.hue > 1)
                        {
                            hsv.hue -= 1.0;
                        }
                    }

                    if (sa != 0)
                    {
                        hsv.saturation += sa;
                        if (hsv.saturation < 0)
                        {
                            hsv.saturation = 0.0;
                        }
                        else if (hsv.saturation > 1)
                        {
                            hsv.saturation = 1.0;
                        }
                    }

                    if (va != 0)
                    {
                        hsv.value += va;
                        if (hsv.value < 0)
                        {
                            hsv.value = 0.0;
                        }
                        else if (hsv.value > 1)
                        {
                            hsv.value = 1.0;
                        }
                    }

                    unsigned char alpha = color.Alpha();
                    color = hsv;
                    color.alpha = alpha;
                }

                // add sparkles
                if (color != xlBLACK &&
                    (thelayer->music_sparkle_count ||
                        thelayer->sparkle_count > 0 ||
                        thelayer->SparklesValueCurve.IsActive())
                    )
                {
                    int sc = thelayer->sparkle_count;
                    if (thelayer->SparklesValueCurve.IsActive())
                    {
                        sc = (int)thelayer->SparklesValueCurve.GetOutputValueAt(offset);
                    }

                    if (thelayer->music_sparkle_count &&
                        thelayer->buffer.GetMedia() != nullptr)
                    {
                        float f = 0.0;
                        std::list<float>* pf = thelayer->buffer.GetMedia()->GetFrameData(thelayer->buffer.curPeriod, FRAMEDATA_HIGH, "");
                        if (pf != nullptr)
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
                if (thelayer->BrightnessValueCurve.IsActive())
                {
                    b = (int)thelayer->BrightnessValueCurve.GetOutputValueAt(offset);
                }
                else
                {
                    b = thelayer->brightness;
                }
                if (b != 100 || thelayer->contrast != 0)
                {
                    HSVValue hsv = color.asHSV();
                    hsv.value = hsv.value * ((double)b / 100.0);

                    // Apply Contrast
                    if (hsv.value < 0.5)
                    {
                        // reduce brightness when below 0.5 in the V value or increase if > 0.5
                        hsv.value = hsv.value - (hsv.value* ((double)thelayer->contrast / 100.0));
                    }
                    else
                    {
                        hsv.value = hsv.value + (hsv.value* ((double)thelayer->contrast / 100.0));
                    }

                    if (hsv.value < 0.0) hsv.value = 0.0;
                    if (hsv.value > 1.0) hsv.value = 1.0;
                    unsigned char alpha = color.Alpha();
                    color = hsv;
                    color.alpha = alpha;
                }

                if (MixTypeHandlesAlpha(thelayer->mixType))
                {
                    c = mixColors(x, y, color, c, layer);
                }
                else
                {
                    if (cnt == 0 && thelayer->fadeFactor != 1.0)
                    {
                        //need to fade the first here as we're not mixing anything
                        HSVValue hsv = color.asHSV();
                        hsv.value *= thelayer->fadeFactor;
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
}

//http://blog.ivank.net/fastest-gaussian-blur.html
static void boxesForGauss(int d, int n, std::vector<float> &boxes)  // standard deviation, number of boxes
{
    switch (d) {
        case 2:
        case 3:
            boxes.push_back(1.0);
            break;
        case 4:
        case 5:
        case 6:
            boxes.push_back(3.0);
            break;
        case 7:
        case 8:
        case 9:
            boxes.push_back(5.0);
            break;
        case 10:
        case 11:
        case 12:
            boxes.push_back(7.0);
            break;
        case 13:
        case 14:
        case 15:
            boxes.push_back(9.0);
            break;
    }
    float b = boxes.back();
    switch (d) {
        case 2:
        case 4:
        case 5:
        case 7:
        case 8:
        case 10:
        case 11:
        case 13:
        case 14:
            boxes.push_back(b);
            break;
        default:
            boxes.push_back(b + 2.0);
            break;
    }
    switch (d) {
        case 4:
        case 7:
        case 10:
        case 13:
            boxes.push_back(b);
            break;
        default:
            boxes.push_back(b + 2.0);
    }
}

#define RED(a, b) a[(b)*4]
#define GREEN(a, b) a[(b)*4 + 1]
#define BLUE(a, b) a[(b)*4 + 2]
#define ALPHA(a, b) a[(b)*4 + 3]
static inline void SET(float *ar, int idx, float r, float g, float b, float a) {
    idx *= 4;
    ar[idx++] = r;
    ar[idx++] = g;
    ar[idx++] = b;
    ar[idx] = a;
}

static void boxBlurH_4 (const float  * const scl, float *tcl, int w, int h, float r) {
    float iarr = 1.0f / (r+r+1.0f);
    for(int i=0; i<h; i++) {
        int ti = i*w;
        int li = ti;
        int ri = ti+r;
        int maxri = ti + w - 1;
        int fvIdx = ti;
        int lvIdx = ti+w-1;

        float valr = (r+1.0) * RED(scl,fvIdx);
        float valg = (r+1.0) * GREEN(scl,fvIdx);
        float valb = (r+1.0) * BLUE(scl,fvIdx);
        float vala = (r+1.0) * ALPHA(scl,fvIdx);

        float fvRed = RED(scl, fvIdx);
        float fvGreen = GREEN(scl, fvIdx);
        float fvBlue = BLUE(scl, fvIdx);
        float fvAlpha = ALPHA(scl, fvIdx);
        float lvRed = RED(scl, lvIdx);
        float lvGreen = GREEN(scl, lvIdx);
        float lvBlue = BLUE(scl, lvIdx);
        float lvAlpha = ALPHA(scl, lvIdx);

        for (int j=0; j<r; j++) {
            int idx = j < w ? ti+j : lvIdx;
            valr += RED(scl, idx);
            valg += GREEN(scl, idx);
            valb += BLUE(scl, idx);
            vala += ALPHA(scl, idx);
        }
        for (int j=0  ; j<=r ; j++) {
            int idx = ri <= maxri ? ri++ : lvIdx;
            valr += RED(scl, idx) - fvRed;
            valg += GREEN(scl, idx) - fvGreen;
            valb += BLUE(scl, idx) - fvBlue;
            vala += ALPHA(scl, idx) - fvAlpha;

            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
                ti++;
            }
        }
        for (int j=r+1; j<w-r; j++) {
            int c = ri <= maxri ? ri++ : lvIdx;
            int c2 = li <= maxri ? li++ : lvIdx;
            valr += RED(scl, c) - RED(scl, c2);
            valg += GREEN(scl, c) - GREEN(scl, c2);
            valb += BLUE(scl, c) - BLUE(scl, c2);
            vala += ALPHA(scl, c) - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
                ti++;
            }
        }

        for (int j=w-r; j<w  ; j++) {
            int c2 = li <= maxri ? li++: lvIdx;
            valr += lvRed - RED(scl, c2);
            valg += lvGreen - GREEN(scl, c2);
            valb += lvBlue - BLUE(scl, c2);
            vala += lvAlpha - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
                ti++;
            }
        }
    }
}

static void boxBlurT_4 (const float * const scl, float *tcl, int w, int h, float r) {
    float iarr = 1.0f / (r+r+1.0f);
    for(int i=0; i<w; i++) {
        int ti = i;
        int li = ti;
        int ri = ti+r*w;

        int maxri = ti+w*(h-1);

        int fvIdx = ti;
        int lvIdx = ti+w*(h-1);

        float fvRed = RED(scl, fvIdx);
        float fvGreen = GREEN(scl, fvIdx);
        float fvBlue = BLUE(scl, fvIdx);
        float fvAlpha = ALPHA(scl, fvIdx);
        float lvRed = RED(scl, lvIdx);
        float lvGreen = GREEN(scl, lvIdx);
        float lvBlue = BLUE(scl, lvIdx);
        float lvAlpha = ALPHA(scl, lvIdx);

        float valr = (r+1)*fvRed;
        float valg = (r+1)*fvGreen;
        float valb = (r+1)*fvBlue;
        float vala = (r+1)*fvAlpha;

        for(int j=0; j<r; j++) {
            int idx = j < w ? ti+j*w : lvIdx;
            valr += RED(scl, idx);
            valg += GREEN(scl, idx);
            valb += BLUE(scl, idx);
            vala += ALPHA(scl, idx);
        }
        for(int j=0  ; j<=r ; j++) {
            int idx = ri <= maxri ? ri : lvIdx;
            valr += RED(scl, idx) - fvRed;
            valg += GREEN(scl, idx) - fvGreen;
            valb += BLUE(scl, idx) - fvBlue;
            vala += ALPHA(scl, idx) - fvAlpha;
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
            }
            ri+=w;
            ti+=w;
        }
        for(int j=r+1; j<h-r; j++) {
            int c = ri <= maxri ? ri : lvIdx;
            int c2 = li <= maxri ? li : lvIdx;
            valr += RED(scl, c) - RED(scl, c2);
            valg += GREEN(scl, c) - GREEN(scl, c2);
            valb += BLUE(scl, c) - BLUE(scl, c2);
            vala += ALPHA(scl, c) - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
            }
            li+=w; ri+=w; ti+=w;
        }
        for(int j=h-r; j<h  ; j++) {
            int c2 = li <= maxri ? li : lvIdx;
            valr += lvRed - RED(scl, c2);
            valg += lvGreen - GREEN(scl, c2);
            valb += lvBlue - BLUE(scl, c2);
            vala += lvAlpha - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
            }
            li += w;
            ti += w;
        }
    }
}

static void boxBlur_4(float *scl, float *tcl, int w, int h, float r, int size) {
    memcpy(tcl, scl, sizeof(float)*4*size);
    boxBlurH_4(tcl, scl, w, h, r);
    boxBlurT_4(scl, tcl, w, h, r);
}

static void gaussBlur_4(float *scl, float *tcl, int w, int h, int r, int size) {
    std::vector<float> bxs;
    boxesForGauss(r - 1, 3, bxs);
    boxBlur_4 (scl, tcl, w, h, (bxs[0]-1)/2, size);
    boxBlur_4 (tcl, scl, w, h, (bxs[1]-1)/2, size);
    boxBlur_4 (scl, tcl, w, h, (bxs[2]-1)/2, size);
}

static inline int roundInt(float r) {
    int tmp = static_cast<int> (r);
    tmp += (r-tmp>=.5) - (r-tmp<=-.5);
    return tmp;
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

    if (layer->BufferWi == 1 && layer->BufferHt == 1) {
        return;
    }
    if (b < 2) {
        return;
    } else if (b > 2 && layer->BufferWi > 6 && layer->BufferHt > 6) {
        int pixCount = layer->buffer.pixels.size();
        float * input = new float[pixCount * 4];
        float * tmp = new float[pixCount * 4];
        for (int x = 0; x < pixCount; x++) {
            const xlColor &c = layer->buffer.pixels[x];
            input[x * 4] = c.red;
            input[x * 4 + 1] = c.green;
            input[x * 4 + 2] = c.blue;
            input[x * 4 + 3] = c.alpha;
        }
        gaussBlur_4(input, tmp, layer->BufferWi, layer->BufferHt, b, pixCount);

        for (int x = 0; x < pixCount; x++) {
            layer->buffer.pixels[x].Set(roundInt(tmp[x*4]),
                                        roundInt(tmp[x*4 + 1]),
                                        roundInt(tmp[x*4 + 2]),
                                        roundInt(tmp[x*4 + 3]));
        }
        delete [] input;
        delete [] tmp;
    } else {
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
        RenderBuffer orig(layer->buffer);
        for (int x = 0; x < layer->BufferWi; x++)
        {
            for (int y = 0; y < layer->BufferHt; y++)
            {
                int r = 0;
                int g = 0;
                int b2 = 0;
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
                                const xlColor &c = orig.GetPixel(i, j);
                                r += c.red;
                                g += c.green;
                                b2 += c.blue;
                                a += c.alpha;
                                ++sm;
                            }
                        }
                    }
                }
                layer->buffer.SetPixel(x, y, xlColor(r/sm, g/sm, b2/sm, a/sm));
            }
        }
    }
}

void PixelBufferClass::SetPalette(int layer, xlColorVector& newcolors, xlColorCurveVector& newcc)
{
    RenderBuffer& buf = layers[layer]->buffer;
    buf.SetPalette(newcolors, newcc);
    if (layers[layer]->usingModelBuffers) {
        for (auto it = layers[layer]->modelBuffers.begin(); it != layers[layer]->modelBuffers.end(); it++)  {
            (*it)->SetPalette(newcolors, newcc);
        }
    }
}

static const std::string CHOICE_LayerMethod("CHOICE_LayerMethod");
static const std::string SLIDER_EffectLayerMix("SLIDER_EffectLayerMix");
static const std::string CHECKBOX_LayerMorph("CHECKBOX_LayerMorph");
static const std::string TEXTCTRL_Fadein("TEXTCTRL_Fadein");
static const std::string TEXTCTRL_Fadeout("TEXTCTRL_Fadeout");
static const std::string SLIDER_Blur("SLIDER_Blur");
static const std::string SLIDER_Zoom("SLIDER_Zoom");
static const std::string SLIDER_Rotation("SLIDER_Rotation");
static const std::string SLIDER_XRotation("SLIDER_XRotation");
static const std::string SLIDER_YRotation("SLIDER_YRotation");
static const std::string SLIDER_Rotations("SLIDER_Rotations");
static const std::string SLIDER_ZoomQuality("SLIDER_ZoomQuality");
static const std::string SLIDER_PivotPointX("SLIDER_PivotPointX");
static const std::string SLIDER_PivotPointY("SLIDER_PivotPointY");
static const std::string SLIDER_XPivot("SLIDER_XPivot");
static const std::string SLIDER_YPivot("SLIDER_YPivot");

static const std::string CHECKBOX_OverlayBkg("CHECKBOX_OverlayBkg");
static const std::string CHOICE_BufferStyle("CHOICE_BufferStyle");
static const std::string CHOICE_BufferTransform("CHOICE_BufferTransform");
static const std::string CUSTOM_SubBuffer("CUSTOM_SubBuffer");
static const std::string VALUECURVE_Blur("VALUECURVE_Blur");
static const std::string VALUECURVE_Sparkles("VALUECURVE_SparkleFrequency");
static const std::string VALUECURVE_Brightness("VALUECURVE_Brightness");
static const std::string VALUECURVE_HueAdjust("VALUECURVE_Color_HueAdjust");
static const std::string VALUECURVE_SaturationAdjust("VALUECURVE_Color_SaturationAdjust");
static const std::string VALUECURVE_ValueAdjust("VALUECURVE_Color_ValueAdjust");
static const std::string VALUECURVE_Zoom("VALUECURVE_Zoom");
static const std::string VALUECURVE_Rotation("VALUECURVE_Rotation");
static const std::string VALUECURVE_XRotation("VALUECURVE_XRotation");
static const std::string VALUECURVE_YRotation("VALUECURVE_YRotation");
static const std::string VALUECURVE_Rotations("VALUECURVE_Rotations");
static const std::string VALUECURVE_PivotPointX("VALUECURVE_PivotPointX");
static const std::string VALUECURVE_PivotPointY("VALUECURVE_PivotPointY");
static const std::string VALUECURVE_XPivot("VALUECURVE_XPivot");
static const std::string VALUECURVE_YPivot("VALUECURVE_YPivot");
static const std::string STR_DEFAULT("Default");
static const std::string STR_EMPTY("");

static const std::string SLIDER_SparkleFrequency("SLIDER_SparkleFrequency");
static const std::string CHECKBOX_MusicSparkles("CHECKBOX_MusicSparkles");
static const std::string SLIDER_Brightness("SLIDER_Brightness");
static const std::string SLIDER_HueAdjust("SLIDER_Color_HueAdjust");
static const std::string SLIDER_SaturationAdjust("SLIDER_Color_SaturationAdjust");
static const std::string SLIDER_ValueAdjust("SLIDER_Color_ValueAdjust");
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

void ComputeValueCurve(const std::string& valueCurve, ValueCurve& theValueCurve, int divisor = 1)
{
    if (valueCurve == STR_EMPTY) {
        theValueCurve.SetDefault();
        return;
    }

    theValueCurve.SetDivisor(divisor);
    theValueCurve.Deserialise(valueCurve);
}

// Works out the maximum buffer size reached based on a subbuffer - this may be larger than the model size but never less than the model size
void ComputeMaxBuffer(const std::string& subBuffer, int BufferHt, int BufferWi, int& maxHt, int& maxWi)
{
    if (wxString(subBuffer).Contains("Active=TRUE"))
    {
        // value curve present ... we have work to do
        wxString sb = subBuffer;
        sb.Replace("Max", "yyz");

        wxArrayString v = wxSplit(sb, 'x');

        bool fx1vc = v.size() > 0 && v[0].Contains("Active=TRUE");
        bool fy1vc = v.size() > 1 && v[1].Contains("Active=TRUE");
        bool fx2vc = v.size() > 2 && v[2].Contains("Active=TRUE");
        bool fy2vc = v.size() > 3 && v[3].Contains("Active=TRUE");

        // the larger the number the more fine grained the buffer assessment will be ... makes crashes less likely
        #define VCITERATIONS (10.0 * VC_X_POINTS)

        float maxX = 0;
        if (fx1vc)
        {
            v[0].Replace("yyz", "Max");
            ValueCurve vc(v[0].ToStdString());
            vc.SetLimits(-100, 200);
            for (int i = 0; i < VCITERATIONS; ++i)
            {
                float val = vc.GetOutputValueAt((float)i / VCITERATIONS);
                if (val > maxX)
                {
                    maxX = val;
                }
            }
        }
        if (fx2vc)
        {
            v[2].Replace("yyz", "Max");
            ValueCurve vc(v[2].ToStdString());
            vc.SetLimits(-100, 200);
            for (int i = 0; i < VCITERATIONS; ++i)
            {
                float val = vc.GetOutputValueAt((float)i / VCITERATIONS);
                if (val > maxX)
                {
                    maxX = val;
                }
            }
        }

        float maxY = 0;
        if (fy1vc)
        {
            v[1].Replace("yyz", "Max");
            ValueCurve vc(v[1].ToStdString());
            vc.SetLimits(-100, 200);
            for (int i = 0; i < VCITERATIONS; ++i)
            {
                float val = vc.GetOutputValueAt((float)i / VCITERATIONS);
                if (val > maxY)
                {
                    maxY = val;
                }
            }
        }
        if (fy2vc)
        {
            v[3].Replace("yyz", "Max");
            ValueCurve vc(v[3].ToStdString());
            vc.SetLimits(-100, 200);
            for (int i = 0; i < VCITERATIONS; ++i)
            {
                float val = vc.GetOutputValueAt((float)i / VCITERATIONS);
                if (val > maxY)
                {
                    maxY = val;
                }
            }
        }

        maxX *= (float)BufferWi;
        maxY *= (float)BufferHt;
        maxX /= 100.0;
        maxY /= 100.0;

        maxWi = std::max((int)std::ceil(maxX), BufferWi);
        maxHt = std::max((int)std::ceil(maxY), BufferHt);
    }
    else
    {
        maxHt = BufferHt;
        maxWi = BufferWi;
    }
}

void ComputeSubBuffer(const std::string &subBuffer, std::vector<NodeBaseClassPtr> &newNodes, int &bufferWi, int &bufferHi, float progress) {
    if (subBuffer == STR_EMPTY) {
        return;
    }

    wxString sb = subBuffer;
    sb.Replace("Max", "yyz");

    wxArrayString v = wxSplit(sb, 'x');

    bool fx1vc = v.size() > 0 && v[0].Contains("Active=TRUE");
    bool fy1vc = v.size() > 1 && v[1].Contains("Active=TRUE");
    bool fx2vc = v.size() > 2 && v[2].Contains("Active=TRUE");
    bool fy2vc = v.size() > 3 && v[3].Contains("Active=TRUE");

    float x1;
    if (fx1vc)
    {
        v[0].Replace("yyz", "Max");
        ValueCurve vc(v[0].ToStdString());
        vc.SetLimits(-100, 200);
        x1 = vc.GetOutputValueAt(progress);
    }
    else if (v.size() > 0)
    {
        x1 = wxAtof(v[0]);
    }
    else
    {
        x1 = 0.0;
    }

    float y1;
    if (fy1vc)
    {
        v[1].Replace("yyz", "Max");
        ValueCurve vc(v[1].ToStdString());
        vc.SetLimits(-100, 200);
        y1 = vc.GetOutputValueAt(progress);
    }
    else if (v.size() > 1)
    {
        y1 = wxAtof(v[1]);
    }
    else
    {
        y1 = 0.0;
    }

    float x2;
    if (fx2vc)
    {
        v[2].Replace("yyz", "Max");
        ValueCurve vc(v[2].ToStdString());
        vc.SetLimits(-100, 200);
        x2 = vc.GetOutputValueAt(progress);
    }
    else if (v.size() > 2)
    {
        x2 = wxAtof(v[2]);
    }
    else
    {
        x2 = 100.0;
    }

    float y2;
    if (fy2vc)
    {
        v[3].Replace("yyz", "Max");
        ValueCurve vc(v[3].ToStdString());
        vc.SetLimits(-100, 200);
        y2 = vc.GetOutputValueAt(progress);
    }
    else if (v.size() > 3)
    {
        y2 = wxAtof(v[3]);
    }
    else
    {
        y2 = 100.0;
    }

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
        for (auto &it2 : newNodes[x]->Coords) {
            it2.bufX -= x1;
            it2.bufY -= y1;
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
    inf->xrotation = settingsMap.GetInt(SLIDER_XRotation, 0);
    inf->yrotation = settingsMap.GetInt(SLIDER_YRotation, 0);
    inf->rotations = (float)settingsMap.GetInt(SLIDER_Rotations, 0) / 10.0f;
    inf->zoom = (float)settingsMap.GetInt(SLIDER_Zoom, 10) / 10.0f;
    inf->zoomquality = settingsMap.GetInt(SLIDER_ZoomQuality, 1);
    inf->pivotpointx = settingsMap.GetInt(SLIDER_PivotPointX, 50);
    inf->pivotpointy = settingsMap.GetInt(SLIDER_PivotPointY, 50);
    inf->xpivot = settingsMap.GetInt(SLIDER_XPivot, 50);
    inf->ypivot = settingsMap.GetInt(SLIDER_YPivot, 50);
    inf->sparkle_count = settingsMap.GetInt(SLIDER_SparkleFrequency, 0);
    inf->music_sparkle_count = settingsMap.GetBool(CHECKBOX_MusicSparkles, false);

    inf->brightness = settingsMap.GetInt(SLIDER_Brightness, 100);
    inf->hueadjust = settingsMap.GetInt(SLIDER_HueAdjust, 0);
    inf->saturationadjust = settingsMap.GetInt(SLIDER_SaturationAdjust, 0);
    inf->valueadjust = settingsMap.GetInt(SLIDER_ValueAdjust, 0);
    inf->contrast=settingsMap.GetInt(SLIDER_Contrast, 0);

    SetMixType(layer, settingsMap.Get(CHOICE_LayerMethod, STR_NORMAL));

    inf->effectMixThreshold = (float)settingsMap.GetInt(SLIDER_EffectLayerMix, 0)/100.0;
    inf->effectMixVaries = settingsMap.GetBool(CHECKBOX_LayerMorph);

    inf->type = settingsMap.Get(CHOICE_BufferStyle, STR_DEFAULT);
    inf->transform = settingsMap.Get(CHOICE_BufferTransform, STR_NONE);

    const std::string &type = settingsMap.Get(CHOICE_BufferStyle, STR_DEFAULT);
    const std::string &transform = settingsMap.Get(CHOICE_BufferTransform, STR_NONE);
    const std::string &subBuffer = settingsMap.Get(CUSTOM_SubBuffer, STR_EMPTY);
    const std::string &blurValueCurve = settingsMap.Get(VALUECURVE_Blur, STR_EMPTY);
    const std::string &sparklesValueCurve = settingsMap.Get(VALUECURVE_Sparkles, STR_EMPTY);
    const std::string &brightnessValueCurve = settingsMap.Get(VALUECURVE_Brightness, STR_EMPTY);
    const std::string &hueAdjustValueCurve = settingsMap.Get(VALUECURVE_HueAdjust, STR_EMPTY);
    const std::string &saturationAdjustValueCurve = settingsMap.Get(VALUECURVE_SaturationAdjust, STR_EMPTY);
    const std::string &valueAdjustValueCurve = settingsMap.Get(VALUECURVE_ValueAdjust, STR_EMPTY);
    const std::string &rotationValueCurve = settingsMap.Get(VALUECURVE_Rotation, STR_EMPTY);
    const std::string &xrotationValueCurve = settingsMap.Get(VALUECURVE_XRotation, STR_EMPTY);
    const std::string &yrotationValueCurve = settingsMap.Get(VALUECURVE_YRotation, STR_EMPTY);
    const std::string &zoomValueCurve = settingsMap.Get(VALUECURVE_Zoom, STR_EMPTY);
    const std::string &rotationsValueCurve = settingsMap.Get(VALUECURVE_Rotations, STR_EMPTY);
    const std::string &pivotpointxValueCurve = settingsMap.Get(VALUECURVE_PivotPointX, STR_EMPTY);
    const std::string &pivotpointyValueCurve = settingsMap.Get(VALUECURVE_PivotPointY, STR_EMPTY);
    const std::string &xpivotValueCurve = settingsMap.Get(VALUECURVE_XPivot, STR_EMPTY);
    const std::string &ypivotValueCurve = settingsMap.Get(VALUECURVE_YPivot, STR_EMPTY);

    if (inf->bufferType != type || 
        inf->bufferTransform != transform || 
        inf->subBuffer != subBuffer || 
        inf->blurValueCurve != blurValueCurve || 
        inf->sparklesValueCurve != sparklesValueCurve || 
        inf->zoomValueCurve != zoomValueCurve || 
        inf->rotationValueCurve != rotationValueCurve || 
        inf->xrotationValueCurve != xrotationValueCurve || 
        inf->yrotationValueCurve != yrotationValueCurve || 
        inf->rotationsValueCurve != rotationsValueCurve || 
        inf->pivotpointxValueCurve != pivotpointxValueCurve || 
        inf->pivotpointyValueCurve != pivotpointyValueCurve || 
        inf->xpivotValueCurve != xpivotValueCurve || 
        inf->ypivotValueCurve != ypivotValueCurve || 
        inf->brightnessValueCurve != brightnessValueCurve || 
        inf->hueAdjustValueCurve != hueAdjustValueCurve || 
        inf->saturationAdjustValueCurve != saturationAdjustValueCurve || 
        inf->valueAdjustValueCurve != valueAdjustValueCurve)
    {
        int origNodeCount = inf->buffer.Nodes.size();
        inf->buffer.Nodes.clear();
        model->InitRenderBufferNodes(type, transform, inf->buffer.Nodes, inf->BufferWi, inf->BufferHt);
        if (origNodeCount != 0 && origNodeCount != inf->buffer.Nodes.size()) {
            inf->buffer.Nodes.clear();
            model->InitRenderBufferNodes(type, transform, inf->buffer.Nodes, inf->BufferWi, inf->BufferHt);
        }
        
        ComputeSubBuffer(subBuffer, inf->buffer.Nodes, inf->BufferWi, inf->BufferHt, 0);

        // save away the full model buffer size ... some effects need to know this
        ComputeMaxBuffer(subBuffer, inf->BufferHt, inf->BufferWi, inf->ModelBufferHt, inf->ModelBufferWi);

        ComputeValueCurve(brightnessValueCurve, inf->BrightnessValueCurve);
        ComputeValueCurve(hueAdjustValueCurve, inf->HueAdjustValueCurve);
        ComputeValueCurve(saturationAdjustValueCurve, inf->SaturationAdjustValueCurve);
        ComputeValueCurve(valueAdjustValueCurve, inf->ValueAdjustValueCurve);
        ComputeValueCurve(blurValueCurve, inf->BlurValueCurve);
        ComputeValueCurve(sparklesValueCurve, inf->SparklesValueCurve);
        ComputeValueCurve(rotationValueCurve, inf->RotationValueCurve);
        ComputeValueCurve(xrotationValueCurve, inf->XRotationValueCurve);
        ComputeValueCurve(yrotationValueCurve, inf->YRotationValueCurve);
        ComputeValueCurve(zoomValueCurve, inf->ZoomValueCurve, 10);
        ComputeValueCurve(rotationsValueCurve, inf->RotationsValueCurve, 10);
        ComputeValueCurve(pivotpointxValueCurve, inf->PivotPointXValueCurve);
        ComputeValueCurve(pivotpointyValueCurve, inf->PivotPointYValueCurve);
        ComputeValueCurve(xpivotValueCurve, inf->XPivotValueCurve);
        ComputeValueCurve(ypivotValueCurve, inf->YPivotValueCurve);
        inf->bufferType = type;
        inf->bufferTransform = transform;
        inf->subBuffer = subBuffer;
        inf->blurValueCurve = blurValueCurve;
        inf->sparklesValueCurve = sparklesValueCurve;
        inf->brightnessValueCurve = brightnessValueCurve;
        inf->hueAdjustValueCurve = hueAdjustValueCurve;
        inf->saturationAdjustValueCurve = saturationAdjustValueCurve;
        inf->valueAdjustValueCurve = valueAdjustValueCurve;
        inf->zoomValueCurve = zoomValueCurve;
        inf->rotationValueCurve = rotationValueCurve;
        inf->xrotationValueCurve = xrotationValueCurve;
        inf->yrotationValueCurve = yrotationValueCurve;
        inf->rotationsValueCurve = rotationsValueCurve;
        inf->pivotpointxValueCurve = pivotpointxValueCurve;
        inf->pivotpointyValueCurve = pivotpointyValueCurve;
        inf->xpivotValueCurve = xpivotValueCurve;
        inf->ypivotValueCurve = ypivotValueCurve;

        // we create the buffer oversized to prevent issues
        inf->buffer.InitBuffer(inf->BufferHt, inf->BufferWi, inf->ModelBufferHt, inf->ModelBufferWi, inf->bufferTransform);

        if (type.compare(0, 9, "Per Model") == 0) {
            inf->usingModelBuffers = true;
            const ModelGroup *gp = dynamic_cast<const ModelGroup*>(model);
            int cnt = 0;
            for (auto it = inf->modelBuffers.begin(); it != inf->modelBuffers.end(); ++it, ++cnt) {
                std::string ntype = type.substr(10, type.length() - 10);
                int bw, bh;
                (*it)->Nodes.clear();
                gp->Models()[cnt]->InitRenderBufferNodes(ntype, transform, (*it)->Nodes, bw, bh);
                if (bw == 0) bw = 1; // zero sized buffers are a problem
                if (bh == 0) bh = 1;
                (*it)->InitBuffer(bh, bw, bh, bw, transform);
                (*it)->SetAllowAlphaChannel(inf->buffer.allowAlpha);
            }
        } else {
            inf->usingModelBuffers = false;
        }
    }
}
bool PixelBufferClass::IsPersistent(int layer) {
    return layers[layer]->persistent;
}

RenderBuffer& PixelBufferClass::BufferForLayer(int layer, int idx)
{
    if (idx >= 0 && layers[layer]->usingModelBuffers && idx < layers[layer]->modelBuffers.size()) {
        return *layers[layer]->modelBuffers[idx];
    }
    return layers[layer]->buffer;
}
int PixelBufferClass::BufferCountForLayer(int layer)
{
    if (layers[layer]->usingModelBuffers) {
        return layers[layer]->modelBuffers.size();
    }
    return 1;
}
void PixelBufferClass::MergeBuffersForLayer(int layer) {
    if (layers[layer]->usingModelBuffers) {
        //get all the data
        xlColor color;
        int nc = 0;
        for (auto it = layers[layer]->modelBuffers.begin(); it != layers[layer]->modelBuffers.end(); it++) {
            for (auto node = (*it)->Nodes.begin(); node != (*it)->Nodes.end(); node++, nc++) {
                (*it)->GetPixel((*node)->Coords[0].bufX, (*node)->Coords[0].bufY, color);
                for (auto coord = layers[layer]->buffer.Nodes[nc]->Coords.begin(); coord != layers[layer]->buffer.Nodes[nc]->Coords.end(); coord++) {
                    layers[layer]->buffer.SetPixel(coord->bufX, coord->bufY, color);
                }
            }
        }
    }
}

void PixelBufferClass::SetLayer(int newlayer, int period, bool resetState)
{
    CurrentLayer=newlayer;
    layers[CurrentLayer]->buffer.SetState(period, resetState, modelName);
    if (layers[CurrentLayer]->usingModelBuffers) {
        int cnt = 0;
        const ModelGroup *grp = dynamic_cast<const ModelGroup*>(model);
        for (auto it = layers[CurrentLayer]->modelBuffers.begin(); it != layers[CurrentLayer]->modelBuffers.end(); it++, cnt++)  {
            (*it)->SetState(period, resetState, grp->Models()[cnt]->Name());
        }
    }
}

void PixelBufferClass::SetTimes(int layer, int startTime, int endTime)
{
    layers[layer]->buffer.SetEffectDuration(startTime, endTime);
    if (layers[layer]->usingModelBuffers) {
        for (auto it = layers[layer]->modelBuffers.begin(); it != layers[layer]->modelBuffers.end(); it++)  {
            (*it)->SetEffectDuration(startTime, endTime);
        }
    }

}
static inline bool IsInRange(const std::list<NodeRange> &restrictRange, size_t start) {
    if (restrictRange.empty()) {
        return true;
    }
    for (const auto &r : restrictRange) {
        if (start >= r.start && start <= r.end) {
            return true;
        }
    }
    return false;
}
void PixelBufferClass::GetColors(unsigned char *fdata, const std::list<NodeRange> &restrictRange) {

    // KW ... I think this needs to be optimised

    if (layers[0] != nullptr) // I dont like this ... it should never be null
    {
        for (auto &n : layers[0]->buffer.Nodes) {
            size_t start = n->ActChan;
            if (IsInRange(restrictRange, start)) {
                if (n->model != nullptr) // nor this
                {
                    DimmingCurve *curve = n->model->modelDimmingCurve;
                    if (curve != nullptr) {
                        xlColor color;
                        n->GetColor(color);
                        curve->apply(color);
                        n->SetColor(color);
                    }
                }
                n->GetForChannels(&fdata[start]);
            }

        }
    }
}

void PixelBufferClass::SetColors(int layer, const unsigned char *fdata)
{
    xlColor color;
    for (auto &n : layers[layer]->buffer.Nodes) {
        size_t start = n->ActChan;
        
        n->SetFromChannels(&fdata[start]);
        n->GetColor(color);
        
        DimmingCurve *curve = n->model->modelDimmingCurve;
        if (curve != nullptr) {
            curve->reverse(color);
        }
        for (auto &a : n->Coords) {
            layers[layer]->buffer.SetPixel(a.bufX,
                                           a.bufY,
                                           color);

        }
    }
}

void PixelBufferClass::RotoZoom(LayerInfo* layer, float offset)
{
    if (std::isinf(offset)) offset = 1.0;

    // Do the Z axis rotate and zoom first

    float zoom = layer->zoom;
    if (layer->ZoomValueCurve.IsActive())
    {
        zoom = layer->ZoomValueCurve.GetOutputValueAtDivided(offset);
    }
    float rotations = layer->rotations;
    float rotationoffset = offset;
    float offsetperrotation = 1.0f;
    if (layer->RotationsValueCurve.IsActive())
    {
        rotations = layer->RotationsValueCurve.GetOutputValueAtDivided(offset);
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

        layer->buffer.Clear();
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

    // Now do the rotation around a point on the x axis

    float xrotation = layer->xrotation;
    if (layer->XRotationValueCurve.IsActive())
    {
        xrotation = layer->XRotationValueCurve.GetOutputValueAt(offset);
    }

    if (xrotation != 0 && xrotation != 360)
    {
        int xpivot = layer->xpivot;
        if (layer->XPivotValueCurve.IsActive())
        {
            xpivot = layer->XPivotValueCurve.GetOutputValueAt(offset);
        }

        RenderBuffer orig(layer->buffer);
        layer->buffer.Clear();

        float sine = sin((xrotation + 90) * M_PI / 180);
        float pivot = xpivot * layer->buffer.BufferWi / 100;

        for (int x = pivot; x < layer->buffer.BufferWi; ++x)
        {
            float tox = sine * (x - pivot) + pivot;
            for (int y = 0; y < layer->buffer.BufferHt; ++y)
            {
                layer->buffer.SetPixel(tox, y, orig.GetPixel(x, y));
            }
        }

        for (int x = pivot-1; x >= 0; --x)
        {
            float tox = -1 * sine * (pivot - x) + pivot;
            for (int y = 0; y < layer->buffer.BufferHt; ++y)
            {
                layer->buffer.SetPixel(tox, y, orig.GetPixel(x, y));
            }
        }
    }

    // Now do the rotation around a point on the y axis

    float yrotation = layer->yrotation;
    if (layer->YRotationValueCurve.IsActive())
    {
        yrotation = layer->YRotationValueCurve.GetOutputValueAt(offset);
    }

    if (yrotation != 0 && yrotation != 360)
    {
        int ypivot = layer->ypivot;
        if (layer->YPivotValueCurve.IsActive())
        {
            ypivot = layer->YPivotValueCurve.GetOutputValueAt(offset);
        }

        RenderBuffer orig(layer->buffer);
        layer->buffer.Clear();

        float sine = sin((yrotation + 90) * M_PI / 180);
        float pivot = ypivot * layer->buffer.BufferHt / 100;

        for (int y = pivot; y < layer->buffer.BufferHt; ++y)
        {
            float toy = sine * (y - pivot) + pivot;
            for (int x = 0; x < layer->buffer.BufferWi; ++x)
            {
                layer->buffer.SetPixel(x, toy, orig.GetPixel(x, y));
            }
        }

        for (int y = pivot-1; y >= 0; --y)
        {
            float toy = -1 * sine * (pivot - y) + pivot;
            for (int x = 0; x < layer->buffer.BufferWi; ++x)
            {
                layer->buffer.SetPixel(x, toy, orig.GetPixel(x, y));
            }
        }
    }
}

bool PixelBufferClass::IsVariableSubBuffer(int layer) const
{
    const std::string &subBuffer = layers[layer]->subBuffer;
    return subBuffer.find("Active=TRUE") != std::string::npos;
}
    
void PixelBufferClass::PrepareVariableSubBuffer(int EffectPeriod, int layer)
{
    if (!IsVariableSubBuffer(layer)) return;

    const std::string &subBuffer = layers[layer]->subBuffer;

    int effStartPer, effEndPer;
    layers[layer]->buffer.GetEffectPeriods(effStartPer, effEndPer);
    float offset = 0.0;
    if (effEndPer != effStartPer) {
        offset = ((float)EffectPeriod - (float)effStartPer) / ((float)effEndPer - (float)effStartPer);
    }
    offset = std::min(offset, 1.0f);
    const std::string &type = layers[layer]->type;
    const std::string &transform = layers[layer]->transform;
    layers[layer]->buffer.Nodes.clear();
    model->InitRenderBufferNodes(type, transform, layers[layer]->buffer.Nodes, layers[layer]->BufferWi, layers[layer]->BufferHt);
    ComputeSubBuffer(subBuffer, layers[layer]->buffer.Nodes, layers[layer]->BufferWi, layers[layer]->BufferHt, offset);
    layers[layer]->buffer.BufferWi = layers[layer]->BufferWi;
    layers[layer]->buffer.BufferHt = layers[layer]->BufferHt;
    
    if (layers[layer]->buffer.BufferWi == 0) layers[layer]->buffer.BufferWi = 1;
    if (layers[layer]->buffer.BufferHt == 0) layers[layer]->buffer.BufferHt = 1;
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
        float offset = 0.0f;
        if (effEndPer != effStartPer) {
            offset = ((float)EffectPeriod - (float)effStartPer) / ((float)effEndPer - (float)effStartPer);
        }
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
            bool isFirstFrame = (effStartPer == EffectPeriod);

            if (EffectPeriod < (effStartPer)+layers[ii]->fadeInSteps && layers[ii]->fadeInSteps != 0)
            {
                curStep = EffectPeriod - effStartPer + 1;
                fadeInFactor = (double)curStep/(double)layers[ii]->fadeInSteps;
            }
            if (EffectPeriod > (effEndPer)-layers[ii]->fadeOutSteps && layers[ii]->fadeOutSteps != 0)
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
            layers[ii]->calculateMask(isFirstFrame);
        } else {
            layers[ii]->mask.clear();
        }
    }

    // layer calculation and map to output
    size_t NodeCount = layers[0]->buffer.Nodes.size();
    for(size_t i = 0; i < NodeCount; i++)
    {
        if (!layers[0]->buffer.Nodes[i]->IsVisible())
        {
            // unmapped pixel - set to black
            layers[0]->buffer.Nodes[i]->SetColor(xlBLACK);
        }
        else
        {
            // get blend of two effects
            GetMixedColor(i,
                          color,
                          validLayers, EffectPeriod);


            // set color for physical output
            layers[0]->buffer.Nodes[i]->SetColor(color);
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

    return 0;
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
        reverse = !outTransitionReverse;
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
        reverse = !outTransitionReverse;
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
    if (actualpixels == 0) actualpixels = 1;
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

        int x = (jx % xpixels) * adjust;
        int y = (jy % ypixels) * adjust;
        if (mask[x * BufferHt + y] == m2) {

            // check if there is anything left to mask
            bool undone = false;
            for (int tx = 0; tx < std::min(xpixels, actualpixels) && undone == false; ++tx)
            {
                for (int ty = 0; ty < std::min(ypixels, actualpixels) && undone == false; ++ty)
                {
                    if (mask[tx * adjust * BufferHt + ty * adjust] == m1)
                    {
                        undone = true;
                    }
                }
            }

            if (undone)
            {
                i--;
            }
            else
            {
                break;
            }
        } else {
            for (int k = 0; k < adjust; ++k) {
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

void PixelBufferClass::LayerInfo::calculateMask(bool isFirstFrame) {
    bool hasMask = false;
    if (inMaskFactor < 1.0) {
        mask.resize(BufferHt * BufferWi);
        calculateMask(inTransitionType, false, isFirstFrame);
        hasMask = true;
    }
    if (outMaskFactor < 1.0) {
        mask.resize(BufferHt * BufferWi);
        calculateMask(outTransitionType, true, isFirstFrame);
        hasMask = true;
    }
    if (!hasMask) {
        mask.clear();
    }
}
void PixelBufferClass::LayerInfo::calculateMask(const std::string &type, bool mode, bool isFirstFrame) {
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
            if (isFirstFrame)
            {
                static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.warn("Unrecognised transition type '%s'.", (const char *)type.c_str());
            }
            break;
    }
}
bool PixelBufferClass::LayerInfo::isMasked(int x, int y) {
    int idx = x*BufferHt + y;
    if (idx < mask.size()) {
        return mask[idx] > 0;
    }
    return false;
}
