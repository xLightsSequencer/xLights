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

PixelBufferClass::PixelBufferClass() {
    numLayers = 0;
	_audio = nullptr;
    zbModel = nullptr;
}

PixelBufferClass::~PixelBufferClass() {
    if (zbModel != nullptr) {
        delete zbModel;
    }
    for (int x = 0; x < numLayers; x++) {
        delete layers[x];
    }
}


void PixelBufferClass::reset(int nlayers, int timing) {
    for (int x = 0; x < numLayers; x++) {
        delete layers[x];
    }
    layers.clear();
    frameTimeInMs = timing;

    numLayers = nlayers;
    layers.resize(nlayers);
    
    for (int x = 0; x < numLayers; x++) {
        layers[x] = new LayerInfo();
        layers[x]->buffer.SetFrameTimeInMs(frameTimeInMs);
        model->InitRenderBufferNodes("Default", layers[x]->Nodes, layers[x]->BufferWi, layers[x]->BufferHt);
        layers[x]->bufferType = "Default";
        layers[x]->buffer.InitBuffer(layers[x]->BufferHt, layers[x]->BufferWi, _audio);
    }
}


void PixelBufferClass::InitBuffer(const Model &pbc, int layers, int timing, NetInfoClass &netInfo, AudioManager* audio, bool zeroBased) {
	_audio = audio;
	modelName = pbc.name;
    if (zeroBased) {
        zbModel = ModelManager::CreateModel(pbc.GetModelXml(), netInfo, zeroBased);
        model = zbModel;
    } else {
        model = &pbc;
    }
    reset(layers, timing);
}
void PixelBufferClass::InitStrandBuffer(const Model &pbc, int strand, int timing) {
    ssModel.Reset(pbc.GetStrandLength(strand), pbc, strand);
    model = &ssModel;
    reset(2, timing);
}
void PixelBufferClass::InitNodeBuffer(const Model &pbc, int strand, int node, int timing) {
    ssModel.Reset(1, pbc, strand, node);
    model = &ssModel;
    reset(2, timing);
}

void PixelBufferClass::Clear(int which) {
    xlColor bgColor(0, 0, 0, 0);
    if (which != -1) {
        layers[which]->buffer.Clear(bgColor); //just clear this one
    } else {
        //clear them all
        for (size_t i = 0; i < numLayers; i++) {
            layers[i]->buffer.Clear(bgColor);
        }
    }
}

void PixelBufferClass::GetNodeChannelValues(size_t nodenum, unsigned char *buf) {
    layers[0]->Nodes[nodenum]->GetForChannels(buf);
}
void PixelBufferClass::SetNodeChannelValues(size_t nodenum, const unsigned char *buf) {
    layers[0]->Nodes[nodenum]->SetFromChannels(buf);
}
xlColor PixelBufferClass::GetNodeColor(size_t nodenum) const {
    xlColor color;
    layers[0]->Nodes[nodenum]->GetColor(color);
    return color;
}
int PixelBufferClass::NodeStartChannel(size_t nodenum) const {
    return layers[0]->Nodes.size() && nodenum < layers[0]->Nodes.size() ? layers[0]->Nodes[nodenum]->ActChan: 0;
}
int PixelBufferClass::GetNodeCount() const {
    return layers[0]->Nodes.size();
}
int PixelBufferClass::GetChanCountPerNode() const {
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    return layers[0]->Nodes[0]->GetChanCount();
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
void PixelBufferClass::SetMixType(int layer, const std::string& MixName) {
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
    layers[layer]->mixType = MixType;
    layers[layer]->buffer.SetAllowAlphaChannel(MixTypeHandlesAlpha(MixType));
}


xlColor PixelBufferClass::mixColors(const wxCoord &x, const wxCoord &y, const xlColor &fg, const xlColor &c1, int layer) {
    static const int n = 0;  //increase to change the curve of the crossfade
    xlColor c0 = fg;

    HSVValue hsv0;
    HSVValue hsv1;
    bool handlesAlpha = MixTypeHandlesAlpha(layers[layer]->mixType);
    if (!handlesAlpha && layers[layer]->fadeFactor != 1.0) {
        //need to fade the first here as we're not mixing anything
        hsv0 = c0.asHSV();
        hsv0.value *= layers[layer]->fadeFactor;
        c0 = hsv0;
    }

    float svthresh = layers[layer]->effectMixThreshold;
    if (layers[layer]->effectMixVaries) {
        //vary mix threshold gradually during effect interval -DJ
        layers[layer]->effectMixThreshold = layers[layer]->buffer.GetEffectTimeIntervalPosition();
    }
    if (layers[layer]->effectMixThreshold < 0) {
        layers[layer]->effectMixThreshold = 0;
    }

    xlColor c;
    double emt, emtNot;
    switch (layers[layer]->mixType) {
    case Mix_Normal:
        c0.alpha = c0.alpha * layers[layer]->fadeFactor * (1.0 - layers[layer]->effectMixThreshold);
        c = c0.AlphaBlend(c1);
        break;
    case Mix_Effect1:
    case Mix_Effect2: {
        if (!layers[layer]->effectMixVaries) {
            emt = layers[layer]->effectMixThreshold;
            if ((emt > 0.000001) && (emt < 0.99999)) {
                emtNot = 1-layers[layer]->effectMixThreshold;
                //make cross-fade linear
                emt = cos((M_PI/4)*(pow(2*emt-1,2*n+1)+1));
                emtNot = cos((M_PI/4)*(pow(2*emtNot-1,2*n+1)+1));
            } else {
                emtNot = layers[layer]->effectMixThreshold;
                emt = 1 - layers[layer]->effectMixThreshold;
            }
        } else {
            emt = layers[layer]->effectMixThreshold;
            emtNot = 1-layers[layer]->effectMixThreshold;
        }

        xlColor c2(c1);
        if (layers[layer]->mixType == Mix_Effect2) {
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
        if (hsv0.value <= layers[layer]->effectMixThreshold) {
            // only if effect 1 is black
            c=c1;  // then show the color of effect 2
        } else {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Mask2:
        // second masks first
        hsv1 = c1.asHSV();
        if (hsv1.value <= layers[layer]->effectMixThreshold) {
            c=c0;
        } else {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Unmask1:
        // first unmasks second
        c0.toHSV(hsv0);
        c1.toHSV(hsv1);
        if (hsv0.value > layers[layer]->effectMixThreshold) {
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
        if (hsv1.value > layers[layer]->effectMixThreshold) {
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
        if (hsv1.value <= layers[layer]->effectMixThreshold) {
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
    if (layers[layer]->effectMixVaries) {
        layers[layer]->effectMixThreshold = svthresh; //put it back afterwards in case next row didn't change it
    }
    return c;
}


void PixelBufferClass::GetMixedColor(int node, xlColor& c, const std::vector<bool> & validLayers) {
    
    unsigned short &sparkle = layers[0]->Nodes[node]->sparkle;
    HSVValue hsv;
    int cnt = 0;
    xlColor color;
    c = xlBLACK;
    for (int layer = numLayers - 1; layer >= 0; layer--) {
        if (validLayers[layer]) {
            int x = layers[layer]->Nodes[node]->Coords[0].bufX;
            int y = layers[layer]->Nodes[node]->Coords[0].bufY;
            
            layers[layer]->buffer.GetPixel(x, y, color);

            // add sparkles
            if (layers[layer]->sparkle_count > 0 && color != xlBLACK) {
                switch (sparkle % (208 - layers[layer]->sparkle_count)) {
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
            if (layers[layer]->brightness != 100 || layers[layer]->contrast != 0) {
                hsv = color.asHSV();
                hsv.value = hsv.value * ((double)layers[layer]->brightness/(double)100);

                // Apply Contrast
                if (hsv.value< 0.5) {
                    // reduce brightness when below 0.5 in the V value or increase if > 0.5
                    hsv.value = hsv.value - (hsv.value* ((double)layers[layer]->contrast/(double)100));
                } else {
                    hsv.value = hsv.value + (hsv.value* ((double)layers[layer]->contrast/(double)100));
                }

                if (hsv.value < 0.0) hsv.value=0.0;
                if (hsv.value > 1.0) hsv.value=1.0;
                unsigned char alpha = color.Alpha();
                color = hsv;
                color.alpha = alpha;
            }

            if (MixTypeHandlesAlpha(layers[layer]->mixType)) {
                c = mixColors(x, y, color, c, layer);
            } else {
                if (cnt == 0 && layers[layer]->fadeFactor != 1.0) {
                    //need to fade the first here as we're not mixing anything
                    color.toHSV(hsv);
                    hsv.value *= layers[layer]->fadeFactor;
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
    layers[layer]->buffer.SetPalette(newcolors);
}

// 10-200 or so, or 0 for no sparkle
void PixelBufferClass::SetSparkle(int layer, int freq) {
    layers[layer]->sparkle_count=freq;
}

void PixelBufferClass::SetBrightness(int layer, int value) {
    layers[layer]->brightness=value;
}

void PixelBufferClass::SetContrast(int layer, int value) {
    layers[layer]->contrast=value;
}

void PixelBufferClass::SetMixThreshold(int layer, int value, bool varies) {
    layers[layer]->effectMixThreshold = (float)value/100.0;
    layers[layer]->effectMixVaries = varies;
}
void PixelBufferClass::SetBufferType(int layer, const std::string &type) {
    if (layers[layer]->bufferType != type) {
        layers[layer]->Nodes.clear();
        model->InitRenderBufferNodes(type, layers[layer]->Nodes, layers[layer]->BufferWi, layers[layer]->BufferHt);
        layers[layer]->bufferType = type;
        layers[layer]->buffer.InitBuffer(layers[layer]->BufferHt, layers[layer]->BufferWi, _audio);
    }
}

RenderBuffer& PixelBufferClass::BufferForLayer(int layer) {
    return layers[layer]->buffer;
}
void PixelBufferClass::SetLayer(int newlayer, int period, bool resetState) {
    CurrentLayer=newlayer;
    layers[CurrentLayer]->buffer.SetState(period, resetState, modelName);
}
void PixelBufferClass::SetFadeTimes(int layer, float inTime, float outTime) {
    layers[layer]->buffer.SetFadeTimes(inTime, outTime);
}
void PixelBufferClass::SetTimes(int layer, int startTime, int endTime) {
    layers[layer]->buffer.SetEffectDuration(startTime, endTime);
}
void PixelBufferClass::SetColors(int layer, const unsigned char *fdata) {
    for (int n = 0; n < layers[layer]->Nodes.size(); n++) {
        int start = NodeStartChannel(n);
        SetNodeChannelValues(n, &fdata[start]);
        xlColor color;
        layers[layer]->Nodes[n]->GetColor(color);
        for (int x = 0; x < layers[layer]->Nodes[n]->Coords.size(); x++) {
            layers[layer]->buffer.SetPixel(layers[layer]->Nodes[n]->Coords[x].bufX,
                                          layers[layer]->Nodes[n]->Coords[x].bufY, color);
        }
    }
}

void PixelBufferClass::CalcOutput(int EffectPeriod, const std::vector<bool> & validLayers) {
    xlColor color;
    HSVValue hsv;
    int curStep, fadeInSteps, fadeOutSteps;


    for(int ii=0; ii < numLayers; ii++) {
        double fadeInFactor=1, fadeOutFactor=1;
        layers[ii]->fadeFactor = 1.0;
        layers[ii]->buffer.GetFadeSteps( fadeInSteps, fadeOutSteps);
        if( fadeInSteps > 0 || fadeOutSteps > 0) {
            int effStartPer, effEndPer;
            layers[ii]->buffer.GetEffectPeriods( effStartPer, effEndPer);
            if (EffectPeriod < (effStartPer)+fadeInSteps) {
                curStep = EffectPeriod - effStartPer;
                fadeInFactor = (double)curStep/(double)fadeInSteps;
            }
            if (EffectPeriod > (effEndPer)-fadeOutSteps) {
                curStep = EffectPeriod - (effEndPer-fadeOutSteps);
                fadeOutFactor = 1-(double)curStep/(double)fadeOutSteps;
            }
            if(fadeInFactor < 1 && fadeOutFactor < 1) {
                layers[ii]->fadeFactor = (fadeInFactor+fadeOutFactor)/(double)2.0;
            } else if (fadeInFactor<1) {
                layers[ii]->fadeFactor = fadeInFactor;
            } else {
                layers[ii]->fadeFactor = fadeOutFactor;
            }
        }
    }
    // layer calculation and map to output
    size_t NodeCount = layers[0]->Nodes.size();
    for(size_t i = 0; i < NodeCount; i++) {
        if (!layers[0]->Nodes[i]->IsVisible()) {
            // unmapped pixel - set to black
            layers[0]->Nodes[i]->SetColor(xlBLACK);
        } else {
            // get blend of two effects
            GetMixedColor(i,
                          color,
                          validLayers);

            // Apply dimming curve
            DimmingCurve *curve = layers[0]->Nodes[i]->model->modelDimmingCurve;
            if (curve != nullptr) {
                curve->apply(color);
            }

            // set color for physical output
            layers[0]->Nodes[i]->SetColor(color);
        }
    }
}



