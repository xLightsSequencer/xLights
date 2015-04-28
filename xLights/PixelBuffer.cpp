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
    effects = new RgbEffects[numLayers];
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


void PixelBufferClass::InitBuffer(wxXmlNode* ModelNode, int layers, int timing, bool zeroBased) {
    SetFromXml(ModelNode, zeroBased);
    SetModelBrightness(wxAtoi(ModelNode->GetAttribute("ModelBrightness","0")));
    reset(layers, timing);
}
void PixelBufferClass::InitStrandBuffer(PixelBufferClass &pbc, int strand) {
    parm1 = 1;
    parm2 = pbc.parm2;
    parm3 = 1;
    StringType = pbc.StringType;
    rgbOrder = pbc.rgbOrder;
    SingleNode = pbc.SingleNode;
    SingleChannel = pbc.SingleChannel;
    IsLtoR = pbc.IsLtoR;
    
    int node = strand;
    stringStartChan.resize(1);
    if ("Custom" != pbc.DisplayAs) {
        node *= pbc.parm2;
        node /= pbc.parm3;
        stringStartChan[0] = pbc.NodeStartChannel(node);
    } else {
        stringStartChan[0] = pbc.stringStartChan[0];
    }
    InitLine();
    SetModelBrightness(pbc.ModelBrightness);
    reset(2, pbc.frameTimeInMs);
}
void PixelBufferClass::InitNodeBuffer(PixelBufferClass &pbc, int strand, int node) {
    parm1 = 1;
    parm2 = 1;
    parm3 = 1;
    StringType = pbc.StringType;
    rgbOrder = pbc.rgbOrder;
    SingleNode = pbc.SingleNode;
    SingleChannel = pbc.SingleChannel;
    IsLtoR = pbc.IsLtoR;
    stringStartChan.resize(1);
    if ("Custom" == pbc.DisplayAs) {
        stringStartChan[0] = pbc.stringStartChan[0] + node * pbc.ChannelsPerNode();
    } else {
        stringStartChan[0] = pbc.NodeStartChannel(strand * pbc.parm2  / pbc.parm3 + node);
    }
    InitLine();

    SetModelBrightness(pbc.ModelBrightness);
    reset(2, pbc.frameTimeInMs);
}

void PixelBufferClass::Clear(int which) {
    xlColour bgColor(0, 0, 0);
    if (which != -1) {
        effects[which].Clear(bgColor); //just clear this one
    } else {
        //clear them all
        for (size_t i = 0; i < numLayers; i++) {
            effects[i].Clear(bgColor);
        }
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
}


xlColour PixelBufferClass::mixColors(const wxCoord &x, const wxCoord &y, xlColour &c0, xlColour &c1, int layer) {
    static const int n = 0;  //increase to change the curve of the crossfade

    float svthresh = effectMixThreshold[layer];
    if (effectMixVaries[layer]) {
        //vary mix threshold gradually during effect interval -DJ
        effectMixThreshold[layer] = effects[layer].GetEffectTimeIntervalPosition();
    }
    if (effectMixThreshold[layer] < 0) {
        effectMixThreshold[layer] = 0;
    }

    wxImage::HSVValue hsv0;
    wxImage::HSVValue hsv1;

    xlColour c;
    double emt, emtNot;
    switch (mixType[layer]) {
    case Mix_Effect1:
    case Mix_Effect2:
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

        if (mixType[layer] == Mix_Effect2) {
            c0.Set(c0.Red()*(emtNot) ,c0.Green()*(emtNot), c0.Blue()*(emtNot));
            c1.Set(c1.Red()*(emt) ,c1.Green()*(emt), c1.Blue()*(emt));
        } else {
            c0.Set(c0.Red()*(emt) ,c0.Green()*(emt), c0.Blue()*(emt));
            c1.Set(c1.Red()*(emtNot) ,c1.Green()*(emtNot), c1.Blue()*(emtNot));
        }
        c.Set(c0.Red()+c1.Red(), c0.Green()+c1.Green(), c0.Blue()+c1.Blue());
        break;
    case Mix_Mask1:
        // first masks second
        hsv0 = wxImage::RGBtoHSV(c0);
        if (hsv0.value <= effectMixThreshold[layer]) {
            // only if effect 1 is black
            c=c1;  // then show the color of effect 2
        } else {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Mask2:
        // second masks first
        hsv1 = wxImage::RGBtoHSV(c1);
        if (hsv1.value <= effectMixThreshold[layer]) {
            c=c0;
        } else {
            c.Set(0, 0, 0);
        }
        break;
    case Mix_Unmask1:
        // first unmasks second
        hsv0 = wxImage::RGBtoHSV(c0);
        hsv1 = wxImage::RGBtoHSV(c1);
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
        hsv0 = wxImage::RGBtoHSV(c0);
        hsv1 = wxImage::RGBtoHSV(c1);
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
        hsv0 = wxImage::RGBtoHSV(c0);
        hsv1 = wxImage::RGBtoHSV(c1);
        if (hsv0.value > effectMixThreshold[layer]) {
            // if effect 1 is non black
            hsv1.value = hsv0.value;
            hsv1.saturation = hsv0.saturation;
            c = hsv1;
        }
        break;
    case Mix_Shadow_2on1:
        // Effect 2 shadows onto effect 1
        hsv0 = wxImage::RGBtoHSV(c0);
        hsv1 = wxImage::RGBtoHSV(c1);
        if (hsv1.value > effectMixThreshold[layer]) {
            // if effect 1 is non black
            hsv0.value = hsv1.value;
            hsv0.saturation = hsv1.saturation;
            c = hsv0;
        }
        break;
    case Mix_Layered:
        hsv1 = wxImage::RGBtoHSV(c1);
        if (hsv1.value <= effectMixThreshold[layer]) {
            c=c0;
        } else {
            c=c1;
        }
        break;
    case Mix_Average:
        // only average when both colors are non-black
        if (c0.GetRGB() == 0) {
            c=c1;
        } else if (c1.GetRGB() == 0) {
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
        hsv0 = wxImage::RGBtoHSV(c0);
        c = hsv0.value > effectMixThreshold[layer] ? c0 : c1; // if effect 1 is non black
        break;
    case Mix_2_reveals_1:
        hsv1 = wxImage::RGBtoHSV(c1);
        c = hsv1.value > effectMixThreshold[layer] ? c1 : c0; // if effect 2 is non black
        break;
    }
    if (effectMixVaries[layer]) {
        effectMixThreshold[layer] = svthresh; //put it back afterwards in case next row didn't change it
    }
    return c;
}

void PixelBufferClass::GetMixedColor(const wxCoord &x, const wxCoord &y, xlColour& c, bool validLayers[]) {
    xlColour *colors = new xlColour[numLayers];
    int layerIdxs[numLayers];

    wxImage::HSVValue hsv;
    int pos = 0;
    for (int layer = 0; layer < numLayers; layer++) {
        if (validLayers[layer]) {
            effects[layer].GetPixel(x, y, colors[pos]);
            layerIdxs[pos] = layer;
            if (fadeFactor[layer] != 1.0) {
                hsv = wxImage::RGBtoHSV(colors[pos]);
                hsv.value *= fadeFactor[layer];
                colors[pos] = hsv;
            }

            if (pos > 0) {
                //mix with layer below
                colors[pos] = mixColors(x, y, colors[pos - 1], colors[pos], layerIdxs[pos - 1]);
            }
            pos++;
        }
    }
    c = colors[pos - 1];
    delete [] colors;
}
void PixelBufferClass::SetPalette(int layer, xlColourVector& newcolors) {
    effects[layer].SetPalette(newcolors);
}

// 10-200 or so, or 0 for no sparkle
void PixelBufferClass::SetSparkle(int layer, int freq) {
    sparkle_count[layer]=freq;
}

void PixelBufferClass::SetBrightness(int layer, int value) {
    brightness[layer]=value;
}

void PixelBufferClass::SetModelBrightness(int value) {
    ModelBrightness=value;
}

void PixelBufferClass::SetContrast(int layer, int value) {
    contrast[layer]=value;
}

void PixelBufferClass::SetMixThreshold(int layer, int value, bool varies) {
    effectMixThreshold[layer] = (float)value/100.0;
    effectMixVaries[layer] = varies;
}

void PixelBufferClass::SetLayer(int newlayer, int period, int speed, bool resetState) {
    CurrentLayer=newlayer;
    effects[CurrentLayer].SetState(period, speed, resetState, name);
}
void PixelBufferClass::SetFadeTimes(int layer, float inTime, float outTime) {
    effects[layer].SetFadeTimes(inTime, outTime);
}
void PixelBufferClass::SetTimes(int layer, int startTime, int endTime) {
    effects[layer].SetEffectDuration(startTime, endTime);
}
void PixelBufferClass::SetFitToTime(int layer, bool fit) {
    effects[layer].SetFitToTime(fit);
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

void PixelBufferClass::CalcOutput(int EffectPeriod, bool validLayers[]) {
    xlColor color;
    wxImage::HSVValue hsv;
    int curStep, fadeInSteps, fadeOutSteps;

    double fadeInFactor=1, fadeOutFactor=1;

    for(int ii=0; ii < numLayers; ii++) {
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
            Nodes[i]->SetColor(0,0,0);
        } else {
            // get blend of two effects
            GetMixedColor(Nodes[i]->Coords[0].bufX, Nodes[i]->Coords[0].bufY, color, validLayers);

            // add sparkles
            if (sparkle_count[0] > 0 && color.GetRGB() != 0) {
                switch (Nodes[i]->sparkle % (208 - sparkle_count[0])) {
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
                Nodes[i]->sparkle++;
            }
            // Apply brightness
            if (ModelBrightness != 0 || brightness[0] != 100 || contrast[0] != 0) {
                wxImage::RGBValue rgb(color.Red(), color.Green(), color.Blue());
                hsv = wxImage::RGBtoHSV(rgb);

                float fModelBrightness=((float)ModelBrightness/100) + 1.0;
                hsv.value = hsv.value * ((double)brightness[0]/(double)100)*fModelBrightness;


                // Apply Contrast

                if (hsv.value< 0.5) {
                    // reduce brightness when below 0.5 in the V value or increase if > 0.5
                    hsv.value = hsv.value - (hsv.value* ((double)contrast[0]/(double)100));
                } else {
                    hsv.value = hsv.value + (hsv.value* ((double)contrast[0]/(double)100));
                }

                if (hsv.value < 0.0) hsv.value=0.0;
                if (hsv.value > 1.0) hsv.value=1.0;

                color = wxImage::HSVtoRGB(hsv);
            }

            // set color for physical output
            Nodes[i]->SetColor(color);
        }
    }
}

void PixelBufferClass::RenderOff(void) {
    effects[CurrentLayer].RenderOff();
}

void PixelBufferClass::RenderOn(int start, int end) {
    effects[CurrentLayer].RenderOn(start, end);
}

void PixelBufferClass::RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D) {
    effects[CurrentLayer].RenderBars(PaletteRepeat,Direction,Highlight,Show3D);
}

void PixelBufferClass::RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip, int ButterflyDirection) {
    effects[CurrentLayer].RenderButterfly(ColorScheme,Style,Chunks,Skip, ButterflyDirection);
}

void PixelBufferClass::RenderCircles(int number,int radius, bool bounce, bool collide, bool random,
                                     bool radial, bool radial_3D, bool bubbles, int start_x, int start_y, bool plasma, bool fade) {
    effects[CurrentLayer].RenderCircles(number, radius, bounce, collide, random, radial, radial_3D, bubbles, start_x, start_y, plasma, fade);
}

void PixelBufferClass::RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount) {
    effects[CurrentLayer].RenderColorWash(HorizFade,VertFade,RepeatCount);
}

void PixelBufferClass::RenderCurtain(int edge, int effect, int swag, bool repeat) {
    effects[CurrentLayer].RenderCurtain(edge,effect,swag,repeat);
}

void PixelBufferClass::RenderFaces(int Phoneme) {
    effects[CurrentLayer].RenderFaces(Phoneme);
}
//void PixelBufferClass::RenderCoroFaces(int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y/*, const wxString& parsed_xy*/)
void PixelBufferClass::RenderCoroFaces(const wxString& Phoneme, const wxString& eyes, bool face_outline) {
//    effects[CurrentLayer].RenderCoroFaces(Phoneme,x_y,Outline_x_y,Eyes_x_y/*, parsed_xy*/);
    effects[CurrentLayer].RenderCoroFaces(Phoneme, eyes, face_outline);
}

void PixelBufferClass::RenderFan(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
                                 int duration, int acceleration, bool reverse_dir, bool blend_edges,
                                 int num_blades, int blade_width, int blade_angle, int num_elements, int element_width )
{
    effects[CurrentLayer].RenderFan( center_x, center_y, start_radius, end_radius, start_angle, revolutions,
                                     duration, acceleration, reverse_dir, blend_edges,
                                     num_blades, blade_width, blade_angle, num_elements, element_width );
}

void PixelBufferClass::RenderFire(int HeightPct,int HueShift,bool GrowFire) {
    effects[CurrentLayer].RenderFire(HeightPct,HueShift,GrowFire);
}

void PixelBufferClass::RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade) {
    effects[CurrentLayer].RenderFireworks(Number_Explosions,Count,Velocity,Fade);
}

void PixelBufferClass::RenderGalaxy(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
                                    int start_width, int end_width, int duration, int acceleration, bool reverse_dir, bool blend_edges, bool inward ) {
    effects[CurrentLayer].RenderGalaxy(center_x, center_y, start_radius, end_radius, start_angle, revolutions,
                                       start_width, end_width, duration, acceleration, reverse_dir, blend_edges, inward );
}

void PixelBufferClass::RenderGarlands(int GarlandType, int Spacing) {
    effects[CurrentLayer].RenderGarlands(GarlandType,Spacing);
}

void PixelBufferClass::RenderGlediator( const wxString& NewPictureName) {
    effects[CurrentLayer].RenderGlediator(NewPictureName);
}


void PixelBufferClass::RenderLife(int Count, int Seed) {
    effects[CurrentLayer].RenderLife(Count,Seed);
}

void PixelBufferClass::RenderMeteors(int MeteorType, int Count, int Length, int MeteorsEffect, int SwirlIntensity) {
    effects[CurrentLayer].RenderMeteors(MeteorType,Count,Length,MeteorsEffect,SwirlIntensity);
}

void PixelBufferClass::RenderMorph(int start_x1, int start_y1, int start_x2, int start_y2, int end_x1, int end_y1, int end_x2, int end_y2,
                                   int start_length, int end_length, bool start_linked, bool end_linked, int duration, int acceleration, int tail_style,
                                   bool useHeadForStartColor, bool useHeadForEndColor, bool showEntireHeadAtStart ) {
    effects[CurrentLayer].RenderMorph(start_x1, start_y1, start_x2, start_y2, end_x1, end_y1, end_x2, end_y2,
                                      start_length, end_length, start_linked, end_linked, duration, acceleration, tail_style,
                                      useHeadForStartColor, useHeadForEndColor, showEntireHeadAtStart);
}


void PixelBufferClass::RenderPiano(int Style, int NumKeys, int NumRows, int DrawMode, bool Clipping, const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename) { //added more controls -DJ
    effects[CurrentLayer].RenderPiano(Style, NumKeys, NumRows, DrawMode, Clipping, CueFilename, MapFilename, ShapeFilename);
}

void PixelBufferClass::RenderPictures(int dir, const wxString& NewPictureName,int GifSpeed, bool is20FPS, int xc_adj, int yc_adj, bool wrap_x) {
    effects[CurrentLayer].RenderPictures(dir,NewPictureName,GifSpeed, is20FPS, xc_adj, yc_adj, wrap_x);
}
void PixelBufferClass::RenderPinwheel(int pinwheel_arms,int pinwheel_twist,int pinwheel_thickness,
                                      bool pinwheel_rotation,int pinwheel_3D,int xc_adj, int yc_adj, int pinwheel_armsize) {
    effects[CurrentLayer].RenderPinwheel(pinwheel_arms,pinwheel_twist,
                                         pinwheel_thickness,pinwheel_rotation,pinwheel_3D,xc_adj,yc_adj,pinwheel_armsize);
}
void PixelBufferClass::RenderRipple(int Object_To_Draw, int Movement, int Ripple_Thickness,int CheckBox_Ripple3D) {
    effects[CurrentLayer].RenderRipple( Object_To_Draw,  Movement, Ripple_Thickness, CheckBox_Ripple3D);
}
void PixelBufferClass::RenderShimmer(int Duty_Factor,bool Use_All_Colors,bool Blink_Timing,int Blinks_Per_Row) {
    effects[CurrentLayer].RenderShimmer(Duty_Factor,Use_All_Colors,Blink_Timing,Blinks_Per_Row );
}
void PixelBufferClass::RenderShockwave(int center_x, int center_y, int start_radius, int end_radius,
                                       int start_width, int end_width, int acceleration, bool blend_edges )
{
    effects[CurrentLayer].RenderShockwave( center_x, center_y, start_radius, end_radius, start_width, end_width, acceleration, blend_edges );
}
void PixelBufferClass::RenderSingleStrandChase(int ColorScheme,int Number_Chases, int Color_Mix1,
        int Chase_Spacing1,int Chase_Type1,bool Chase_3dFade1,bool Chase_Group_All) {
    effects[CurrentLayer].RenderSingleStrandChase( ColorScheme,Number_Chases, Color_Mix1,
            Chase_Spacing1,Chase_Type1, Chase_3dFade1,Chase_Group_All);
}
void PixelBufferClass::RenderSingleStrandSkips(int Skips_BandSize, int Skips_SkipSize, int Skips_StartPos, const wxString &Skips_Direction) {
    effects[CurrentLayer].RenderSingleStrandSkips(Skips_BandSize,  Skips_SkipSize,  Skips_StartPos,  Skips_Direction);
}

void PixelBufferClass::RenderSnowflakes(int Count, int SnowflakeType) {
    effects[CurrentLayer].RenderSnowflakes(Count,SnowflakeType);
}

void PixelBufferClass::RenderSnowstorm(int Count, int Length) {
    effects[CurrentLayer].RenderSnowstorm(Count,Length);
}

void PixelBufferClass::RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness,
                                     bool Blend, bool Show3D, bool grow, bool shrink) {
    effects[CurrentLayer].RenderSpirals(PaletteRepeat,Direction,Rotation,Thickness,Blend,Show3D,grow,shrink);
}

void PixelBufferClass::RenderSpirograph(int R, int r, int d, bool Animate) {
    effects[CurrentLayer].RenderSpirograph( R,  r,  d, Animate);
}
void PixelBufferClass::RenderStrobe(int Number_Strobes,int StrobeDuration,int Strobe_Type) {
    effects[CurrentLayer].RenderStrobe(Number_Strobes,StrobeDuration,Strobe_Type);
}

void PixelBufferClass::RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,bool center1,int Effect1,int Countdown1,
                                  int Position2, const wxString& Line2, const wxString& FontString2,int dir2,bool center2,int Effect2,int Countdown2,
                                  int Position3, const wxString& Line3, const wxString& FontString3,int dir3,bool center3,int Effect3,int Countdown3,
                                  int Position4, const wxString& Line4, const wxString& FontString4,int dir4,bool center4,int Effect4,int Countdown4) {
    effects[CurrentLayer].RenderText(Position1,Line1,FontString1,dir1,center1,Effect1,Countdown1,
                                     Position2,Line2,FontString2,dir2,center2,Effect2,Countdown2,
                                     Position3,Line3,FontString3,dir3,center3,Effect3,Countdown3,
                                     Position4,Line4,FontString4,dir4,center4,Effect4,Countdown4);
}

void PixelBufferClass::RenderTree(int Branches) {
    effects[CurrentLayer].RenderTree(Branches);
}

void PixelBufferClass::RenderTwinkle(int Count,int Steps,bool Strobe) {
    effects[CurrentLayer].RenderTwinkle(Count,Steps,Strobe);
}

void PixelBufferClass::RenderWave(int WaveType,int FillColor,bool MirrorWave,int NumberWaves,int ThicknessWave,
                                  int WaveHeight, int WaveDirection) {
    effects[CurrentLayer].RenderWave(WaveType, FillColor, MirrorWave, NumberWaves, ThicknessWave, WaveHeight, WaveDirection);
}
