/***************************************************************
 * Name:      RgbEffects.cpp
 * Purpose:   Implements RGB effects
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-12-23
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
#include <cmath>
#include "RgbEffects.h"


/*
#include "bass/bass.h";
#include <wx/textfile.h>
#include <wx/string.h>
void bass()
{
    DWORD chan, p;
    QWORD pos;
    wxString fileout=wxT("bass.txt");
    wxString input_name=wxT("bass.mp3");
    wxFile f;

    if (!f.Create(fileout,true))
    {
        return;
    }
    //  f.Write(wxString::Format(wxT("\t\t\t<TimeInterval eff=\"3\" dat=\"&lt;?xml version=&quot;1.0&quot; encoding=&quot;utf-16&quot;?&gt;&#xD;&#xA;&lt;ec&gt;&#xD;&#xA;  &lt;in&gt;100&lt;/in&gt;&#xD;&#xA;  &lt;out&gt;100&lt;/out&gt;&#xD;&#xA;&lt;/ec&gt;\" gui=\"{DA98BD5D-9C00-40fe-A11C-AD3242573443}\" in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" bst=\"%ld\" ben=\"%ld\" />\n"),pos,bst,ben));
    f.Write(wxT("BASS Spectrum writer example : MOD/MPx/OGG -> FILE.TXT\n-------------------------------------------------\n"));
    BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);    // no audio output, therefore no update period needed
    BASS_Init(0, 44100, 0, 0, NULL);                // null device, 44100hz, stereo, 16 bits

    chan = BASS_StreamCreateFile(FALSE, input_name, 0, 0, BASS_STREAM_DECODE);  //streaming the file
    pos = BASS_ChannelGetLength(chan, BASS_POS_BYTE);
    f.Write(wxString::Format(wxT("streaming file [%llu bytes]"),pos));
    p = (DWORD) BASS_ChannelBytes2Seconds(chan, pos);  //length of file in seconds
    f.Write(wxString::Format(wxT(" %u:%02u\n"), p / 60, p % 60));
    float time = 0;

    while (BASS_ChannelIsActive(chan))
    {
        long byte_pos = BASS_ChannelSeconds2Bytes(chan, time);
        BASS_ChannelSetPosition(chan, byte_pos, BASS_POS_BYTE);
        float fft[1024];
        int b0 = 0;
        BASS_ChannelGetData(chan, fft, BASS_DATA_FFT2048);  //get the fft data, in this case there are 2048 samples

        //binning the fft, modified from bass spectum.c example in sdk.
        for (int i = 0; i < BANDS; i++)
        {
            float peak = 0;
            int b1 = pow(2, i * 10.0 / (BANDS - 1)); //determine size of the bin

            if (b1 > 1023)
            {
                b1 = 1023;   //upper bound on bin size
            }

            if (b1 <= b0)
            {
                b1 = b0 + 1;   //make sure atleast one bin is used
            }

            //loop over every bin
            for (; b0 < b1; b0++)
            {
                if (peak < fft[1 + b0])
                {
                    peak = fft[1 + b0];
                }
            }
            //write each column to file
            f.Write(wxString::Format(wxT("%f,"), sqrt(peak)));
        }
        //endline after every row
        f.Write(wxT("\n");
        pos = BASS_ChannelGetPosition(chan, BASS_POS_BYTE);
        p = (DWORD) BASS_ChannelBytes2Seconds(chan, pos);
       f.Write(wxString::Format(wxT(" %u:%02u\n"), p / 60, p % 60));  //print current time
        time += 1 / fps;   //increment time
    }

    f.Write(wxT("DONE!"));
    f.Close();
    BASS_Free();
}
*/

void RgbEffects::RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness,
                               bool Blend, bool Show3D, bool grow, bool shrink)
{
    int strand_base,strand,thick,x,y,ColorIdx;
    size_t colorcnt=GetColorCount();
    int SpiralCount=colorcnt * PaletteRepeat;
    int deltaStrands=BufferWi / SpiralCount;
    int SpiralThickness=(deltaStrands * Thickness / 100) + 1;
    int spiralGap = deltaStrands - SpiralThickness;
    long SpiralState=state*Direction;
    long ThicknessState = state/10;
    wxImage::HSVValue hsv;
    wxColour color;


    if (grow && (!shrink || ((ThicknessState/spiralGap)%2)==0))
    {
        SpiralThickness += ThicknessState%(spiralGap);
    }
    else if (shrink && (!grow || ((ThicknessState/spiralGap)%2)==1))
    {
        SpiralThickness +=spiralGap-ThicknessState%(spiralGap);
    }

    for(int ns=0; ns < SpiralCount; ns++)
    {
        strand_base=ns * deltaStrands;
        ColorIdx=ns % colorcnt;
        palette.GetColor(ColorIdx,color);
        for(thick=0; thick < SpiralThickness; thick++)
        {
            strand = (strand_base + thick) % BufferWi;
            for(y=0; y < BufferHt; y++)
            {
                x=(strand + SpiralState/10 + y*Rotation/BufferHt) % BufferWi;
                if (x < 0) x += BufferWi;
                if (Blend)
                {
                    GetMultiColorBlend(double(BufferHt-y-1)/double(BufferHt), false, color);
                }
                if (Show3D)
                {
                    Color2HSV(color,hsv);
                    if (Rotation < 0)
                    {
                        hsv.value*=double(thick+1)/SpiralThickness;
                    }
                    else
                    {
                        hsv.value*=double(SpiralThickness-thick)/SpiralThickness;
                    }
                    SetPixel(x,y,hsv);
                }
                else
                {
                    SetPixel(x,y,color);
                }
            }
        }
    }
}
