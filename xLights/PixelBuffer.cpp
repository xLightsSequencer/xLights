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

PixelBufferClass::PixelBufferClass()
{
}

PixelBufferClass::~PixelBufferClass()
{
}

void PixelBufferClass::InitBuffer(wxXmlNode* ModelNode, bool zeroBased)
{
    SetFromXml(ModelNode, zeroBased);
    for(size_t i=0; i<2; i++)
    {
        Effect[i].InitBuffer(BufferHt, BufferWi);
    }
}

void PixelBufferClass::Clear(int which)
{
    wxColour bgColor=*wxBLACK;
    if (which != -1) Effect[which].Clear(bgColor); //just clear this one
    else //clear them all
        for(size_t i=0; i<2; i++) //why does this clear canvas twice? isn't it the same set of pixels for each2? -DJ
        {
            Effect[i].Clear(bgColor);
        }
}

// convert MixName to MixType enum
void PixelBufferClass::SetMixType(const wxString& MixName)
{
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
    else if (MixName == "Layered")
    {
        MixType=Mix_Layered;
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

}

void PixelBufferClass::GetMixedColor(wxCoord x, wxCoord y, wxColour& c)
{
    wxColour c0,c1;
    wxImage::HSVValue hsv,hsv0,hsv1;
    wxImage::RGBValue rgbVal;
    double emt, emtNot;
    int n =0; //increase to change the curve of the crossfade

    Effect[0].GetPixel(x,y,c0);
    Effect[1].GetPixel(x,y,c1);
    hsv0 = wxImage::RGBtoHSV( wxImage::RGBValue( c0.Red(), c0.Green(), c0.Blue()));
    hsv1 = wxImage::RGBtoHSV(wxImage::RGBValue( c1.Red(), c1.Green(), c1.Blue()));

    hsv0.value *= fadeFactor[0];
    hsv1.value *= fadeFactor[1];

    rgbVal = wxImage::HSVtoRGB(hsv0);
    c0.Set(rgbVal.red, rgbVal.green, rgbVal.blue);
    rgbVal = wxImage::HSVtoRGB(hsv1);
    c1.Set(rgbVal.red, rgbVal.green, rgbVal.blue);

    float svthresh = effectMixThreshold;
    if (effectMixVaries) //vary mix threshold gradually during effect interval -DJ
//        effectMixThreshold = Effect[0].GetEffectPeriodPosition();
        effectMixThreshold = 1 - Effect[0].GetEffectPeriodPosition(); //seems to be backwards, so reverse it
//    debug(1, "get mixed color: varies? %d, mix thresh %f", effectMixVaries, effectMixThreshold);
//    wxColour svc0 = c0, svc1 = c1;

    switch (MixType)
    {
    case Mix_Effect1:
    case Mix_Effect2:
        emt = effectMixThreshold;
        emtNot = 1-effectMixThreshold;
        if (!effectMixVaries) //make cross-fade linear; this inverts it? -DJ
        {
            emt = cos((M_PI/4)*(pow(2*emt-1,2*n+1)+1));
            emtNot = cos((M_PI/4)*(pow(2*emtNot-1,2*n+1)+1));
        }

        if (MixType == Mix_Effect2)
        {
            c0.Set(c0.Red()*(emtNot) ,c0.Green()*(emtNot), c0.Blue()*(emtNot));
            c1.Set(c1.Red()*(emt) ,c1.Green()*(emt), c1.Blue()*(emt));
        }
        else
        {
            c0.Set(c0.Red()*(emt) ,c0.Green()*(emt), c0.Blue()*(emt));
            c1.Set(c1.Red()*(emtNot) ,c1.Green()*(emtNot), c1.Blue()*(emtNot));
        }
        c.Set(c0.Red()+c1.Red(), c0.Green()+c1.Green(), c0.Blue()+c1.Blue());
//        if (effectMixVaries) debug(1, "get mixed color: varies? %d, mix thresh %f, c0 %d,%d,%d + c1 %d,%d,%d => c %d,%d,%d", effectMixVaries, effectMixThreshold, svc0.Red(), svc0.Green(), svc0.Blue(), svc1.Red(), svc1.Green(), svc1.Blue(), c.Red(), c.Green(), c.Blue());
        break;
    case Mix_Mask1:
        // first masks second
        if (hsv0.value <= effectMixThreshold) // only if effect 1 is black
        {
            c=c1;  // then show the color of effect 2
        }
        else
        {
            c.Set(0);
        }
        break;
    case Mix_Mask2:
        // second masks first
        if (hsv1.value <= effectMixThreshold)
        {
            c=c0;
        }
        else
        {
            c.Set(0);
        }
        break;
    case Mix_Unmask1:
        // first unmasks second
        if (hsv0.value > effectMixThreshold) // if effect 1 is non black
        {

            hsv1.value = hsv0.value;
            rgbVal = wxImage::HSVtoRGB(hsv1);
            c.Set(rgbVal.red, rgbVal.green, rgbVal.blue);
        }
        else
        {
            c.Set(0);
        }
        break;
    case Mix_Unmask2:
        // second unmasks first
        if (hsv1.value > effectMixThreshold)  // if effect 2 is non black
        {
            hsv0.value = hsv1.value;
            rgbVal = wxImage::HSVtoRGB(hsv0);
            c.Set(rgbVal.red, rgbVal.green, rgbVal.blue);
        }
        else
        {
            c.Set(0);
        }
        break;
    case Mix_Layered:
        if (hsv1.value <= effectMixThreshold)
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
        if (c0.GetRGB() == 0)
        {
            c=c1;
        }
        else if (c1.GetRGB() == 0)
        {
            c=c0;
        }
        else
        {
            c.Set( (c0.Red()+c1.Red())/2, (c0.Green()+c1.Green())/2, (c0.Blue()+c1.Blue())/2 );
        }
        break;
    case Mix_BottomTop:
        c=y < BufferHt/2 ? c0 : c1;
        break;
    case Mix_LeftRight:
        c=x < BufferWi/2 ? c0 : c1;
        break;
    case Mix_1_reveals_2:
        c = hsv0.value > effectMixThreshold ? c0 : c1; // if effect 1 is non black
        break;
    case Mix_2_reveals_1:
        c = hsv1.value > effectMixThreshold ? c1 : c0; // if effect 2 is non black
        break;
    }
    if (effectMixVaries) effectMixThreshold = svthresh; //put it back afterwards in case next row didn't change it
}

void PixelBufferClass::SetPalette(int layer, wxColourVector& newcolors)
{
    Effect[layer].SetPalette(newcolors);
}

// Not currently used...
//size_t PixelBufferClass::GetColorCount(int layer)
//{
//    return Effect[layer].GetColorCount();
//}

// 10-200 or so, or 0 for no sparkle
void PixelBufferClass::SetSparkle(int freq)
{
    sparkle_count=freq;
}

void PixelBufferClass::SetBrightness(int value)
{
    brightness=value;
}

void PixelBufferClass::SetModelBrightness(int value)
{
    ModelBrightness=value;
}

void PixelBufferClass::SetContrast(int value)
{
    contrast=value;
}

void PixelBufferClass::SetMixThreshold(int value, bool varies)
{
    effectMixThreshold= (float)value/100.0;
    effectMixVaries = varies;
}

void PixelBufferClass::SetLayer(int newlayer, int period, int speed, bool ResetState)
{
    CurrentLayer=newlayer & 1;  // only 0 or 1 is allowed
    Effect[CurrentLayer].SetState(period,speed,ResetState, name);
}
void PixelBufferClass::SetFadeTimes(int layer, float inTime, float outTime)
{
    Effect[layer].SetFadeTimes(inTime, outTime);
}
void PixelBufferClass::SetTimes(int layer, int startTime, int endTime, int nextTime)
{
    Effect[layer].SetEffectDuration(startTime, endTime, nextTime);
}
void PixelBufferClass::SetFitToTime(int layer, bool fit)
{
    Effect[layer].SetFitToTime(fit);
}

void PixelBufferClass::CalcOutput(int EffectPeriod)
{
    wxColour color;
    wxImage::HSVValue hsv;
    int curStep, fadeInSteps, fadeOutSteps;

    double fadeInFactor=1, fadeOutFactor=1;

    for(int ii=0; ii<2; ii++)
    {
        fadeFactor[ii] = 1.0;
        Effect[ii].GetFadeSteps( fadeInSteps, fadeOutSteps);
        if( fadeInSteps > 0 || fadeOutSteps > 0)
        {
            int effStartPer, effNextPer, effEndPer;
            Effect[ii].GetEffectPeriods( effStartPer, effNextPer, effEndPer);
            if (EffectPeriod < (effStartPer)+fadeInSteps)
            {
                curStep = EffectPeriod - effStartPer;
                fadeInFactor = (double)curStep/(double)fadeInSteps;
            }
            if (EffectPeriod > (effNextPer)-fadeOutSteps)
            {
                curStep = EffectPeriod - (effNextPer-fadeOutSteps);
                fadeOutFactor = 1-(double)curStep/(double)fadeOutSteps;
            }
            if(fadeInFactor < 1 && fadeOutFactor < 1)
            {
                fadeFactor[ii] = (fadeInFactor+fadeOutFactor)/(double)2.0;
            }
            else if (fadeInFactor<1)
            {
                fadeFactor[ii] = fadeInFactor;
            }
            else
            {
                fadeFactor[ii] = fadeOutFactor;
            }
        }
    }
    // layer calculation and map to output
    size_t NodeCount=Nodes.size();
    for(size_t i=0; i<NodeCount; i++)
    {
        if (!Nodes[i]->IsVisible())
        {
            // unmapped pixel - set to black
            Nodes[i]->SetColor(0,0,0);
        }
        else
        {
            // get blend of two effects
            GetMixedColor(Nodes[i]->Coords[0].bufX, Nodes[i]->Coords[0].bufY, color);

            // add sparkles
            if (sparkle_count > 0 && color.GetRGB()!=0)
            {
                switch (Nodes[i]->sparkle%sparkle_count)
                {
                case 1:
                case 7:
                    // too dim
                    //color.Set("#444444");
                    break;
                case 2:
                case 6:
                    color.Set("#888888");
                    break;
                case 3:
                case 5:
                    color.Set("#BBBBBB");
                    break;
                case 4:
                    color.Set("#FFFFFF");
                    break;
                }
                Nodes[i]->sparkle++;
            }
            // Apply brightness
            wxImage::RGBValue rgb(color.Red(),color.Green(),color.Blue());
            hsv = wxImage::RGBtoHSV(rgb);
            ModelBrightness=1.0;    // <SCM> we will use this until we figure how to pass in Model brightness
            hsv.value = hsv.value * ((double)brightness/(double)100)*ModelBrightness;


            // Apply Contrast

            if(hsv.value< 0.5) // reduce brightness when below 0.5 in the V value or increase if > 0.5
            {
                hsv.value = hsv.value - (hsv.value* ((double)contrast/(double)100));
            }
            else
            {

                hsv.value = hsv.value + (hsv.value* ((double)contrast/(double)100));
            }


            if(hsv.value < 0.0) hsv.value=0.0;
            if(hsv.value > 1.0) hsv.value=1.0;


            rgb = wxImage::HSVtoRGB(hsv);
            color = wxColor(rgb.red,rgb.green,rgb.blue);

            // set color for physical output
            Nodes[i]->SetColor(color);
        }
    }
}


void PixelBufferClass::RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D)
{
    Effect[CurrentLayer].RenderBars(PaletteRepeat,Direction,Highlight,Show3D);
}

void PixelBufferClass::RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip, int ButterflyDirection)
{
    Effect[CurrentLayer].RenderButterfly(ColorScheme,Style,Chunks,Skip, ButterflyDirection);
}

void PixelBufferClass::RenderCircles(int number,int radius, bool bounce, bool collide, bool random,
                                     bool radial, bool radial_3D,  int start_x, int start_y, bool plasma)
{
    Effect[CurrentLayer].RenderCircles(number, radius, bounce, collide, random, radial, radial_3D, start_x, start_y, plasma);
}

void PixelBufferClass::RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount)
{
    Effect[CurrentLayer].RenderColorWash(HorizFade,VertFade,RepeatCount);
}

void PixelBufferClass::RenderCurtain(int edge, int effect, int swag, bool repeat)
{
    Effect[CurrentLayer].RenderCurtain(edge,effect,swag,repeat);
}

void PixelBufferClass::RenderFaces(int Phoneme)
{
    Effect[CurrentLayer].RenderFaces(Phoneme);
}
//void PixelBufferClass::RenderCoroFaces(int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y/*, const wxString& parsed_xy*/)
void PixelBufferClass::RenderCoroFaces(const wxString& Phoneme, const wxString& eyes, bool face_outline)
{
//    Effect[CurrentLayer].RenderCoroFaces(Phoneme,x_y,Outline_x_y,Eyes_x_y/*, parsed_xy*/);
    Effect[CurrentLayer].RenderCoroFaces(Phoneme, eyes, face_outline);
}


void PixelBufferClass::RenderFire(int HeightPct,int HueShift,bool GrowFire)
{
    Effect[CurrentLayer].RenderFire(HeightPct,HueShift,GrowFire);
}

void PixelBufferClass::RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade)
{
    Effect[CurrentLayer].RenderFireworks(Number_Explosions,Count,Velocity,Fade);
}

void PixelBufferClass::RenderGarlands(int GarlandType, int Spacing)
{
    Effect[CurrentLayer].RenderGarlands(GarlandType,Spacing);
}

void PixelBufferClass::RenderGlediator( const wxString& NewPictureName)
{
    Effect[CurrentLayer].RenderGlediator(NewPictureName);
}


void PixelBufferClass::RenderLife(int Count, int Seed)
{
    Effect[CurrentLayer].RenderLife(Count,Seed);
}

void PixelBufferClass::RenderMeteors(int MeteorType, int Count, int Length, int MeteorsEffect, int SwirlIntensity)
{
    Effect[CurrentLayer].RenderMeteors(MeteorType,Count,Length,MeteorsEffect,SwirlIntensity);
}

void PixelBufferClass::RenderPiano(int Style, int NumKeys, int NumRows, int DrawMode, bool Clipping, const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename) //added more controls -DJ
{
    Effect[CurrentLayer].RenderPiano(Style, NumKeys, NumRows, DrawMode, Clipping, CueFilename, MapFilename, ShapeFilename);
}

void PixelBufferClass::RenderPictures(int dir, const wxString& NewPictureName,int GifSpeed)
{
    Effect[CurrentLayer].RenderPictures(dir,NewPictureName,GifSpeed);
}

void PixelBufferClass::RenderSingleStrand(int ColorScheme,int Number_Chases, int Color_Mix1,
        int Chase_Spacing1,int Chase_Type1,bool Chase_3dFade1,bool Chase_Group_All)
{
    Effect[CurrentLayer].RenderSingleStrand( ColorScheme,Number_Chases, Color_Mix1,
            Chase_Spacing1,Chase_Type1, Chase_3dFade1,Chase_Group_All);
}

void PixelBufferClass::RenderSnowflakes(int Count, int SnowflakeType)
{
    Effect[CurrentLayer].RenderSnowflakes(Count,SnowflakeType);
}

void PixelBufferClass::RenderSnowstorm(int Count, int Length)
{
    Effect[CurrentLayer].RenderSnowstorm(Count,Length);
}

void PixelBufferClass::RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness,
                                     bool Blend, bool Show3D, bool grow, bool shrink)
{
    Effect[CurrentLayer].RenderSpirals(PaletteRepeat,Direction,Rotation,Thickness,Blend,Show3D,grow,shrink);
}

void PixelBufferClass::RenderSpirograph(int R, int r, int d, bool Animate)
{
    Effect[CurrentLayer].RenderSpirograph( R,  r,  d, Animate);
}

void PixelBufferClass::RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,bool center1,int Effect1,int Countdown1,
                                  int Position2, const wxString& Line2, const wxString& FontString2,int dir2,bool center2,int Effect2,int Countdown2,
                                  int Position3, const wxString& Line3, const wxString& FontString3,int dir3,bool center3,int Effect3,int Countdown3,
                                  int Position4, const wxString& Line4, const wxString& FontString4,int dir4,bool center4,int Effect4,int Countdown4)
{
    Effect[CurrentLayer].RenderText(Position1,Line1,FontString1,dir1,center1,Effect1,Countdown1,
                                    Position2,Line2,FontString2,dir2,center2,Effect2,Countdown2,
                                    Position3,Line3,FontString3,dir3,center3,Effect3,Countdown3,
                                    Position4,Line4,FontString4,dir4,center4,Effect4,Countdown4);
}

void PixelBufferClass::RenderTree(int Branches)
{
    Effect[CurrentLayer].RenderTree(Branches);
}

void PixelBufferClass::RenderTwinkle(int Count,int Steps,bool Strobe)
{
    Effect[CurrentLayer].RenderTwinkle(Count,Steps,Strobe);
}

void PixelBufferClass::RenderWave(int WaveType,int FillColor,bool MirrorWave,int NumberWaves,int ThicknessWave,
                                  int WaveHeight, int WaveDirection)
{
    Effect[CurrentLayer].RenderWave(WaveType, FillColor, MirrorWave, NumberWaves, ThicknessWave, WaveHeight, WaveDirection);
}
