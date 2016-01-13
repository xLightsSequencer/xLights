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

PixelBufferClass::PixelBufferClass() {
    numLayers = 0;
    effects = NULL;
    sparkle_count = NULL;
    brightness = NULL;
    contrast = NULL;
    mixType = NULL;
    effectMixThreshold = NULL;
    effectMixVaries = NULL;
    fadeFactor = NULL;
}

PixelBufferClass::~PixelBufferClass() {
    if (effects != NULL) {
        delete [] effects;
    }
    if (sparkle_count != NULL) {
        delete [] sparkle_count;
    }
    if (brightness != NULL) {
        delete [] brightness;
    }
    if (contrast != NULL) {
        delete [] contrast;
    }
    if (mixType != NULL) {
        delete [] mixType;
    }
    if (fadeFactor != NULL) {
        delete [] fadeFactor;
    }
    if (effectMixThreshold != NULL) {
        delete [] effectMixThreshold;
    }
    if (effectMixVaries != NULL) {
        delete [] effectMixVaries;
    }
}


void PixelBufferClass::reset(int layers, int timing) {
    if (effects != NULL) {
        delete [] effects;
    }
    if (fadeFactor != NULL) {
        delete [] fadeFactor;
    }
    if (sparkle_count != NULL) {
        delete [] sparkle_count;
    }
    if (brightness != NULL) {
        delete [] brightness;
    }
    if (contrast != NULL) {
        delete [] contrast;
    }
    if (mixType != NULL) {
        delete [] mixType;
    }
    if (effectMixThreshold != NULL) {
        delete [] effectMixThreshold;
    }
    if (effectMixVaries != NULL) {
        delete [] effectMixVaries;
    }

    frameTimeInMs = timing;

    numLayers = layers;
    effects = new RenderBuffer[numLayers];
    for (int x = 0; x < numLayers; x++) {
        effects[x].SetFrameTimeInMs(frameTimeInMs);
    }
    sparkle_count = new int[numLayers];
    brightness = new int[numLayers];
    contrast = new int[numLayers];
    mixType = new MixTypes[numLayers];
    effectMixThreshold = new float[numLayers];
    fadeFactor = new double[numLayers];
    effectMixVaries = new bool[numLayers]; //allow varying mix threshold -DJ

    for(size_t i = 0; i < numLayers; i++) {
        effects[i].InitBuffer(BufferHt, BufferWi);
    }
}


void PixelBufferClass::InitBuffer(wxXmlNode* ModelNode, int layers, int timing, NetInfoClass &netInfo, bool zeroBased) {
    SetFromXml(ModelNode, netInfo, zeroBased);
    SetDimmingCurve(modelDimmingCurve);
    reset(layers, timing);
}
void PixelBufferClass::InitStrandBuffer(const ModelClass &pbc, int strand, int timing) {
    parm1 = pbc.GetStrandLength(strand);
    parm2 = 1;
    parm3 = 1;
    StringType = pbc.GetStringType();
    rgbOrder = pbc.rgbOrder;
    SingleNode = pbc.SingleNode;
    SingleChannel = pbc.SingleChannel;
    IsLtoR = pbc.IsLtoR;
    customColor = pbc.customColor;

    stringStartChan.resize(parm1);
    for (int x = 0; x < parm1; x++) {
        stringStartChan[x] = pbc.NodeStartChannel(pbc.MapToNodeIndex(strand, x));
    }
    InitLine();
    SetDimmingCurve(pbc.modelDimmingCurve);
    reset(2, timing);
}
void PixelBufferClass::InitNodeBuffer(const ModelClass &pbc, int strand, int node, int timing) {
    parm1 = 1;
    parm2 = 1;
    parm3 = 1;
    StringType = pbc.GetStringType();
    rgbOrder = pbc.rgbOrder;
    SingleNode = pbc.SingleNode;
    SingleChannel = pbc.SingleChannel;
    IsLtoR = pbc.IsLtoR;
    stringStartChan.resize(1);
    customColor = pbc.customColor;
    stringStartChan[0] = pbc.NodeStartChannel(pbc.MapToNodeIndex(strand, node));
    InitLine();

    SetDimmingCurve(pbc.modelDimmingCurve);
    reset(2, timing);
}

void PixelBufferClass::Clear(int which) {
    xlColor bgColor(0, 0, 0, 0);
    if (which != -1) {
        effects[which].Clear(bgColor); //just clear this one
    } else {
        //clear them all
        for (size_t i = 0; i < numLayers; i++) {
            effects[i].Clear(bgColor);
        }
    }
}
bool MixTypeHandlesAlpha(MixTypes mt) {
    switch (mt) {
    case Mix_Normal:
        return true;
    default:
        return false;
    }
}

// convert MixName to MixType enum
void PixelBufferClass::SetMixType(int layer, const wxString& MixName) {
    MixTypes MixType;
    if (MixName == "Effect 1") {
        MixType=Mix_Effect1;
    } else if (MixName == "Effect 2") {
        MixType=Mix_Effect2;
    } else if (MixName == "1 is Mask") {
        MixType=Mix_Mask1;
    } else if (MixName == "2 is Mask") {
        MixType=Mix_Mask2;
    } else if (MixName == "1 is Unmask") {
        MixType=Mix_Unmask1;
    } else if (MixName == "2 is Unmask") {
        MixType=Mix_Unmask2;
    } else if (MixName == "1 reveals 2") {
        MixType=Mix_1_reveals_2;
    } else if (MixName == "2 reveals 1") {
        MixType=Mix_2_reveals_1;
    } else if (MixName == "Shadow 1 on 2") {
        MixType=Mix_Shadow_1on2;
    } else if (MixName == "Shadow 2 on 1") {
        MixType=Mix_Shadow_2on1;
    } else if (MixName == "Layered") {
        MixType=Mix_Layered;
    } else if (MixName == "Normal") {
        MixType=Mix_Normal;
    } else if (MixName == "Average") {
        MixType=Mix_Average;
    } else if (MixName == "Bottom-Top") {
        MixType=Mix_BottomTop;
    } else if (MixName == "Left-Right") {
        MixType=Mix_LeftRight;
    } else {
        MixType=Mix_Effect1;
    }
    mixType[layer] = MixType;
    effects[layer].SetAllowAlphaChannel(MixTypeHandlesAlpha(MixType));
}


xlColor PixelBufferClass::mixColors(const wxCoord &x, const wxCoord &y, const xlColor &fg, const xlColor &c1, int layer) {
    static const int n = 0;  //increase to change the curve of the crossfade
    xlColor c0 = fg;

    HSVValue hsv0;
    HSVValue hsv1;
    bool handlesAlpha = MixTypeHandlesAlpha(mixType[layer]);
    if (!handlesAlpha && fadeFactor[layer] != 1.0) {
        //need to fade the first here as we're not mixing anything
        hsv0 = c0.asHSV();
        hsv0.value *= fadeFactor[layer];
        c0 = hsv0;
    }

    float svthresh = effectMixThreshold[layer];
    if (effectMixVaries[layer]) {
        //vary mix threshold gradually during effect interval -DJ
        effectMixThreshold[layer] = effects[layer].GetEffectTimeIntervalPosition();
    }
    if (effectMixThreshold[layer] < 0) {
        effectMixThreshold[layer] = 0;
    }

    xlColor c;
    double emt, emtNot;
    switch (mixType[layer]) {
    case Mix_Normal:
        c0.alpha = c0.alpha * fadeFactor[layer] * (1.0 - effectMixThreshold[layer]);
        c = c0.AlphaBlend(c1);
        break;
    case Mix_Effect1:
    case Mix_Effect2: {
        if (!effectMixVaries[layer]) {
            emt = effectMixThreshold[layer];
            if ((emt > 0.000001) && (emt < 0.99999)) {
                emtNot = 1-effectMixThreshold[layer];
                //make cross-fade linear
                emt = cos((M_PI/4)*(pow(2*emt-1,2*n+1)+1));
                emtNot = cos((M_PI/4)*(pow(2*emtNot-1,2*n+1)+1));
            } else {
                emtNot = effectMixThreshold[layer];
                emt = 1 - effectMixThreshold[layer];
            }
        } else {
            emt = effectMixThreshold[layer];
            emtNot = 1-effectMixThreshold[layer];
        }

        xlColor c2(c1);
        if (mixType[layer] == Mix_Effect2) {
            c0.Set(c0.Red()*(emtNot) ,c0.Green()*(emtNot), c0.Blue()*(emtNot));
            c2.Set(c1.Red()*(emt) ,c1.Green()*(emt), c1.Blue()*(emt));
        } else {
            c0.Set(c0.Red()*(emt) ,c0.Green()*(emt), c0.Blue()*(emt));
            c2.Set(c1.Red()*(emtNot) ,c1.Green()*(emtNot), c1.Blue()*(emtNot));
        }
        c.Set(c0.Red()+c2.Red(), c0.Green()+c2.Green(), c0.Blue()+c2.Blue());
        break;
    }
    case Mix_Mask1:
        // first masks second
        hsv0 = c0.asHSV();
        if (hsv0.value <= effectMixThreshold[layer]) {
            // only if effect 1 is black
            c=c1;  // then show the color of effect 2
        } else {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Mask2:
        // second masks first
        hsv1 = c1.asHSV();
        if (hsv1.value <= effectMixThreshold[layer]) {
            c=c0;
        } else {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Unmask1:
        // first unmasks second
        c0.toHSV(hsv0);
        c1.toHSV(hsv1);
        if (hsv0.value > effectMixThreshold[layer]) {
            // if effect 1 is non black
            hsv1.value = hsv0.value;
            c = hsv1;
        } else {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Unmask2:
        // second unmasks first
        c0.toHSV(hsv0);
        c1.toHSV(hsv1);
        if (hsv1.value > effectMixThreshold[layer]) {
            // if effect 2 is non black
            hsv0.value = hsv1.value;
            c = hsv0;
        } else {
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
        if (hsv1.value <= effectMixThreshold[layer]) {
            c=c0;
        } else {
            c=c1;
        }
        break;
    case Mix_Average:
        // only average when both colors are non-black
        if (c0 == xlBLACK) {
            c=c1;
        } else if (c1 == xlBLACK) {
            c=c0;
        } else {
            c.Set( (c0.Red()+c1.Red())/2, (c0.Green()+c1.Green())/2, (c0.Blue()+c1.Blue())/2 );
        }
        break;
    case Mix_BottomTop:
        c= y < BufferHt/2 ? c0 : c1;
        break;
    case Mix_LeftRight:
        c= x < BufferWi/2 ? c0 : c1;
        break;
    case Mix_1_reveals_2:
        c0.toHSV(hsv0);
        c = hsv0.value > effectMixThreshold[layer] ? c0 : c1; // if effect 1 is non black
        break;
    case Mix_2_reveals_1:
        c1.toHSV(hsv1);
        c = hsv1.value > effectMixThreshold[layer] ? c1 : c0; // if effect 2 is non black
        break;
    }
    if (effectMixVaries[layer]) {
        effectMixThreshold[layer] = svthresh; //put it back afterwards in case next row didn't change it
    }
    return c;
}


void PixelBufferClass::GetMixedColor(const wxCoord &x, const wxCoord &y, xlColor& c, const std::vector<bool> & validLayers, int &sparkle) {
    HSVValue hsv;
    int cnt = 0;
    xlColor color;
    c = xlBLACK;
    for (int layer = numLayers - 1; layer >= 0; layer--) {
        if (validLayers[layer]) {
            effects[layer].GetPixel(x, y, color);

            // add sparkles
            if (sparkle_count[layer] > 0 && color != xlBLACK) {
                switch (sparkle % (208 - sparkle_count[layer])) {
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
            if (brightness[layer] != 100 || contrast[layer] != 0) {
                hsv = color.asHSV();
                hsv.value = hsv.value * ((double)brightness[layer]/(double)100);

                // Apply Contrast
                if (hsv.value< 0.5) {
                    // reduce brightness when below 0.5 in the V value or increase if > 0.5
                    hsv.value = hsv.value - (hsv.value* ((double)contrast[layer]/(double)100));
                } else {
                    hsv.value = hsv.value + (hsv.value* ((double)contrast[layer]/(double)100));
                }

                if (hsv.value < 0.0) hsv.value=0.0;
                if (hsv.value > 1.0) hsv.value=1.0;
                unsigned char alpha = color.Alpha();
                color = hsv;
                color.alpha = alpha;
            }

            if (MixTypeHandlesAlpha(mixType[layer])) {
                c = mixColors(x, y, color, c, layer);
            } else {
                if (cnt == 0 && fadeFactor[layer] != 1.0) {
                    //need to fade the first here as we're not mixing anything
                    color.toHSV(hsv);
                    hsv.value *= fadeFactor[layer];
                    color = hsv;
                }
                if (cnt > 0) {
                    //mix with layer below
                    c = mixColors(x, y, color, c, layer);
                } else {
                    c = color;
                }
            }
            cnt++;
        }
    }
}
void PixelBufferClass::SetPalette(int layer, xlColorVector& newcolors) {
    effects[layer].SetPalette(newcolors);
}

// 10-200 or so, or 0 for no sparkle
void PixelBufferClass::SetSparkle(int layer, int freq) {
    sparkle_count[layer]=freq;
}

void PixelBufferClass::SetBrightness(int layer, int value) {
    brightness[layer]=value;
}

void PixelBufferClass::SetDimmingCurve(DimmingCurve *value) {
    dimmingCurve=value;
}

void PixelBufferClass::SetContrast(int layer, int value) {
    contrast[layer]=value;
}

void PixelBufferClass::SetMixThreshold(int layer, int value, bool varies) {
    effectMixThreshold[layer] = (float)value/100.0;
    effectMixVaries[layer] = varies;
}
RenderBuffer& PixelBufferClass::BufferForLayer(int layer) {
    return effects[layer];
}
void PixelBufferClass::SetLayer(int newlayer, int period, bool resetState) {
    CurrentLayer=newlayer;
    effects[CurrentLayer].SetState(period, resetState, name);
}
void PixelBufferClass::SetFadeTimes(int layer, float inTime, float outTime) {
    effects[layer].SetFadeTimes(inTime, outTime);
}
void PixelBufferClass::SetTimes(int layer, int startTime, int endTime) {
    effects[layer].SetEffectDuration(startTime, endTime);
}
void PixelBufferClass::SetColors(int layer, const unsigned char *fdata) {
    for (int n = 0; n < Nodes.size(); n++) {
        int start = NodeStartChannel(n);
        SetNodeChannelValues(n, &fdata[start]);
        xlColor color;
        Nodes[n]->GetColor(color);
        for (int x = 0; x < Nodes[n]->Coords.size(); x++) {
            effects[layer].SetPixel(Nodes[n]->Coords[x].bufX, Nodes[n]->Coords[x].bufY, color);
        }
    }
}

void PixelBufferClass::CalcOutput(int EffectPeriod, const std::vector<bool> & validLayers) {
    xlColor color;
    HSVValue hsv;
    int curStep, fadeInSteps, fadeOutSteps;


    for(int ii=0; ii < numLayers; ii++) {
        double fadeInFactor=1, fadeOutFactor=1;
        fadeFactor[ii] = 1.0;
        effects[ii].GetFadeSteps( fadeInSteps, fadeOutSteps);
        if( fadeInSteps > 0 || fadeOutSteps > 0) {
            int effStartPer, effEndPer;
            effects[ii].GetEffectPeriods( effStartPer, effEndPer);
            if (EffectPeriod < (effStartPer)+fadeInSteps) {
                curStep = EffectPeriod - effStartPer;
                fadeInFactor = (double)curStep/(double)fadeInSteps;
            }
            if (EffectPeriod > (effEndPer)-fadeOutSteps) {
                curStep = EffectPeriod - (effEndPer-fadeOutSteps);
                fadeOutFactor = 1-(double)curStep/(double)fadeOutSteps;
            }
            if(fadeInFactor < 1 && fadeOutFactor < 1) {
                fadeFactor[ii] = (fadeInFactor+fadeOutFactor)/(double)2.0;
            } else if (fadeInFactor<1) {
                fadeFactor[ii] = fadeInFactor;
            } else {
                fadeFactor[ii] = fadeOutFactor;
            }
        }
    }
    // layer calculation and map to output
    size_t NodeCount = Nodes.size();
    for(size_t i = 0; i < NodeCount; i++) {
        if (!Nodes[i]->IsVisible()) {
            // unmapped pixel - set to black
            Nodes[i]->SetColor(xlBLACK);
        } else {
            // get blend of two effects
            GetMixedColor(Nodes[i]->Coords[0].bufX, Nodes[i]->Coords[0].bufY, color, validLayers, Nodes[i]->sparkle);

            // Apply dimming curve
            if (dimmingCurve != nullptr) {
                dimmingCurve->apply(color);
            }

            // set color for physical output
            Nodes[i]->SetColor(color);
        }
    }
}








