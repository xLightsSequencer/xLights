/***************************************************************
 * Name:      RenderPiano.cpp
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
#include <deque> //fifo

#define MIN(a, b)  (((a) < (b))? (a): (b))
#define MAX(a, b)  (((a) > (b))? (a): (b))

//these must match choice list in UI:
#define PIANO_STYLE_CORGAN  0
#define PIANO_STYLE_EQBARS  1
#define PIANO_STYLE_KEYEDGE  2
#define PIANO_STYLE_KEYTOP  3
#define PIANO_STYLE_SCROLLING  4
#define PIANO_STYLE_ICICLES  5

#define NoteVolume(key, time)  ((key + time) & 7) //TODO
#define IsNoteOn(key, time)  (NoteVolume(key, time) >= 2) //TODO

//void djdebug(const char* fmt, ...); //_DJ


void RgbEffects::RenderPiano(int Style, int NumKeys, int KeyWidth, const wxString& NotesFile)
{
    static char KeyShapes[] = {'M' /*A*/, 'L' /*B*/, 'R' /*C*/, 'M' /*D*/, 'L' /*E*/, 'R' /*F*/, 'M' /*G*/}; //L/R = black key on left/right, M = black key on left+right
    static std::deque<std::vector<bool>> WasNoteOn; //keep history for scrolling player piano
//    wxImage::HSVValue hsv;
//    wxColour color;
//    int ColorIdx;
//    int keys_mod;

    size_t colorcnt = GetColorCount();
    if (NumKeys < 3) NumKeys = 12; //default to 1 octave
    if (NumKeys > BufferWi) NumKeys = BufferWi; //each key should be at least 1 wide
    if (KeyWidth < 1) KeyWidth = MAX(BufferWi / NumKeys, 1); //"0" => divide available space into equal widths
    if (KeyWidth > BufferWi / NumKeys) KeyWidth = MAX(BufferWi / NumKeys, 1);
    int height = /*(Style == PIANO_STYLE_SCROLLING)? MIN(KeyWidth, BufferHt):*/ BufferHt; //use square keys for scrolling piano, entire height for all others
    if (Style == PIANO_STYLE_SCROLLING) //add another history row
    {
        WasNoteOn.emplace_back();
        if (WasNoteOn.size() > BufferHt / height) WasNoteOn.pop_front(); //drop oldest row
    }

//    char buf[100];
//    sprintf(buf, "Playback: piano fx, style %d, #keys %d, keyw %d, buf w %d, buf h %d", Style, NumKeys, KeyWidth, BufferWi, BufferHt);
//    StatusBar1->SetStatusText(_(buf));
//    djdebug("[%d] style %d, #keys %d, keyw %d, w %d, h %d, fx h %d, hist len %d", state, Style, NumKeys, KeyWidth, BufferWi, BufferHt, height, WasNoteOn.size());
    for (int xofs = 0; xofs < BufferWi; xofs += KeyWidth) // xofs/KeyWidth == key#
    {
        bool isdown = IsNoteOn(xofs/KeyWidth, state);
//how to handle colors?
//choose different color for each key?
        wxImage::HSVValue hsv;
        wxColour color;
        int ColorIdx = rand() % colorcnt; //TODO: why is this random?
        palette.GetColor(ColorIdx, color);
        Color2HSV(color, hsv);

        switch (Style)
        {
            case PIANO_STYLE_KEYEDGE: //edge of keys go up/down according to whether note is on/off, note color is ????
            case PIANO_STYLE_KEYTOP:
                height = isdown? BufferHt: BufferHt/2;
                hsv.value = isdown? .5: 1.0; //make "on" (down) keys brighter??
//                djdebug("  key notex %d/%d: ht %d, bright %f", xofs/KeyWidth, BufferWi/KeyWidth, height, hsv.value);
                break;
            case PIANO_STYLE_CORGAN: //don't even show keys that are off
//                djdebug("  corgan notex %d/%d: isdown? %d", xofs/KeyWidth, BufferWi/KeyWidth, isdown);
                if (!isdown) continue;
                break;
            case PIANO_STYLE_EQBARS: //height indicates strength/loudness of note
                height = BufferHt * NoteVolume(xofs/KeyWidth, state) / 7;
//                djdebug("  eq notex %d/%d: vol %d, ht %d", xofs/KeyWidth, BufferWi/KeyWidth, NoteVolume(xofs/KeyWidth, state), height);
                break;
            case PIANO_STYLE_SCROLLING: //history scrolling
                WasNoteOn.back()[xofs / KeyWidth] = isdown;
//                djdebug("  scr notex %d/%d: ison %d", xofs/KeyWidth, BufferWi/KeyWidth, isdown);
                break;
//            case PIANO_STYLE_ICICLES: //start drip when note is on
        }

        for (int x = 0; (x < KeyWidth) && (xofs + x < BufferWi); ++x)
            for (int y = 0; y < height; ++y)
            {
                if (Style == PIANO_STYLE_SCROLLING)
                {
                    int row = y / KeyWidth;
                    if (WasNoteOn.size() < row) continue; //row wasn't displayed yet
                    if (!WasNoteOn[row][xofs/KeyWidth]) continue; //note is not on currently
                }
                SetPixel(xofs + x, y, hsv);
            }
    }
#if 0
    switch (Style)
    {
        case PIANO_STYLE_KEYTOP: //view from above; black and white keys have different shapes according to place within octave
//fill one key for now:
            for (int x = 0; x < KeyWidth; ++x)
                for (int y = 0; y < height; ++y)
                    SetPixel(x, y, hsv);
            break;

        case PIANO_STYLE_KEYEDGE:

        case PIANO_STYLE_SCROLLING:

        case PIANO_STYLE_EQBARS:

        case PIANO_STYLE_ICICLES:
    }
                            SetPixel(x,y,hsv);

//#if 1

    //  BufferWi ,BufferHt
    int y_start = (int) ((BufferHt/2.0) - (height/2.0));
    int y_end = y_start+height;

    int x_end,y_end2;
    y_end2 =(int) ( y_start+(height/2));


    hsv.hue=0.0;
    hsv.saturation=1.0;
    hsv.value=1.0;

    //   Keys C,F Type 1
    //  Keys D,G,A Type 2
    //  Keys E,B Type 3
    //  Keys C#,D#,F#,G#,A# Type 4

    int basex,x1,x2;
    for (keys=1; keys<=25; keys++)
    {
        int x_start;
        x_start=1+((keys-1)*width);
        x_end=x_start+width-1;
        keys_mod=keys%12; // 12 notes per octave

        switch (keys_mod)
        {
        case 1: // C
        case 6: // F
        {
            if(keys_mod==1) basex = 1;
            if(keys_mod==6)
            {
                basex = 3*width+1;
            }
            x_start=basex+((int)((keys-1)/12))*width*7;
            // hsv.hue=0.2 + (state%10)/10;
            if(hsv.hue>1.0)  hsv.hue = (state%10)/10;
            hsv.hue=.28;
            hsv.saturation=.50;
            hsv.value=.70;
            if(keys_mod==1)
            {
                hsv.hue=.60;
                hsv.saturation=.43;
                hsv.value=.79;
            }

            if(keys>99 && (int)((state/100)+keys)%2 == 1)
            {
                hsv.hue=.18;
                hsv.saturation=.71;
                hsv.value=.90;
            }
            if(Keyboard==3)
            {
                if(width==7)
                {
                    for(x=x_start; x<=x_start+3; x++)
                    {
                        for (y=y_start; y<=y_end; y++)
                        {
                            SetPixel(x,y,hsv);
                        }
                    }

                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x_start+4,y,hsv);
                        SetPixel(x_start+5,y,hsv);
                    }
                    // black out around accidental key
                    hsv.value=0.0; // black
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_start+3,y,hsv);
                    }
                    for(x=x_start+3; x<=x_start+5; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }

                }
                else
                {


                    for(x=x_start; x<=x_end; x++)
                    {
                        for (y=y_start; y<=y_end; y++)
                        {
                            SetPixel(x,y,hsv);
                        }
                    }

                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_end,y,hsv);
                    }
                }
            }
        }
        break;

        case 2:  // C#
        case 4:  // Eb
        case 7:  // F#
        case 9:  // Ab
        case 11: // Bb
        {
            //  hsv.hue=0.4  + (state%10)/10;
            if(keys_mod==2) basex = width+1;
            if(keys_mod==4) basex = 2*width+1;
            if(keys_mod==7) basex = 4*width+1;
            if(keys_mod==9) basex = 5*width+1;
            if(keys_mod==11) basex = 6*width+1;
            x_start=basex+((int)((keys-1)/12))*width*7;
            hsv.hue=.71;
            hsv.saturation=.43;
            hsv.value=.79;

            if(keys>99 &&  (int)((state/100)+keys)%2 == 0)
            {
                hsv.hue=.18;
                hsv.saturation=.71;
                hsv.value=.90;
            }

            if(width==7)
            {
                for(x=x_start-3 ; x<=x_start+1; x++)
                {
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x,y,hsv);
                    }
                }
                // black out around accidental key
                x1=x_start-4; x2=x_start+1;
                if(keys_mod==4 || keys_mod==9)
                {
                     x1=x_start-3; x2=x_start+2;
                }
                hsv.value=0.0; // black
                for (y=y_end2; y<=y_end; y++)
                {
                    SetPixel(x1,y,hsv);
                    SetPixel(x2,y,hsv);
                }
                for(x=x1; x<=x2; x++)
                {
                    SetPixel(x,y_end2,hsv);
                }
            }
            else
            {
                for(x=x_start-1 ; x<=x_start; x++)
                {
                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x,y,hsv);
                    }
                }
            }



        }
        break;

        case 3:  // D
        case 8:  // G
        case 10: // A
        {
            if(keys_mod==3) basex = 1*width+1;
            if(keys_mod==8) basex = 4*width+1;
            if(keys_mod==10) basex = 5*width+1;
            x_start=basex+((int)((keys-1)/12))*width*7;
            // hsv.hue=0.6  + (state%10)/10;
            hsv.hue=.48;
            hsv.saturation=.73;
            hsv.value=.74;
            if(keys_mod==8)
            {
                hsv.hue=.60;
                hsv.saturation=.43;
                hsv.value=.79;
            }
            if(keys>99 && (int)((state/100)+keys)%3 == 1)
            {
                hsv.hue=.18;
                hsv.saturation=.71;
                hsv.value=.90;
            }
            if(Keyboard==3)
            {
                for (y=y_start; y<=y_end; y++)
                {
                    if(width==7)
                    {
                        SetPixel(x_start+2,y,hsv);
                        SetPixel(x_start+3,y,hsv);
                    }
                    else
                        SetPixel(x_start+1,y,hsv);
                }
                for (y=y_start; y<=y_end2; y++)
                {
                    if(width==7)
                    {
                        SetPixel(x_start,y,hsv);
                        SetPixel(x_start+1,y,hsv);
                        SetPixel(x_start+4,y,hsv);
                        SetPixel(x_start+5,y,hsv);
                    }
                    else
                    {
                        SetPixel(x_start,y,hsv);
                        SetPixel(x_end,y,hsv);
                    }
                }

                if(width==7)
                {
                    // black out around accidental key
                    hsv.value=0.0; // black
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_start+1,y,hsv);
                        SetPixel(x_start+4,y,hsv);
                    }
                    for(x=x_start; x<=x_start+1; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }
                    for(x=x_start+4; x<=x_start+5; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }
                }
            }
        }
        break;

        case 5:   // E
        case 0:   // B
        {
            if(keys_mod==5) basex = 2*width+1;
            if(keys_mod==0) basex = 6*width+1;

            x_start=basex+((int)((keys-1)/12))*width*7;
            // hsv.hue=0.2  + (state%10)/10;
            hsv.hue=.60;
            hsv.saturation=.43;
            hsv.value=.79;

            if(Keyboard==3)
            {
                if(width==7)
                {

                    for (y=y_start; y<=y_end; y++)
                    {
                        SetPixel(x_start+2,y,hsv);
                        SetPixel(x_start+3,y,hsv);
                        SetPixel(x_start+4,y,hsv);
                        SetPixel(x_start+5,y,hsv);
                    }

                    int y_end2 = y_start+height/2;
                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x_start,y,hsv);
                        SetPixel(x_start+1,y,hsv);
                    }

                    // black out around accidental key
                    hsv.value=0.0; // black
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_start+2,y,hsv);
                    }
                    for(x=x_start; x<=x_start+2; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }
                }
                else
                {
                    for(x=x_start+1; x<=x_end; x++)
                    {
                        for (y=y_start; y<=y_end; y++)
                        {
                            SetPixel(x,y,hsv);
                        }
                    }
                    int y_end2 = y_start+height/2;
                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x_start,y,hsv);
                    }
                }

            }
        }
        break;

        default:
        {
            /*
            for(x=1; x<BufferWi; x++)
                for(y=0; y<BufferHt; y++)
                {
                    hsv.hue=0.0;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                    SetPixel(x,y,hsv);
                }
                */
        }
        break;
        } // switch (keys_mod)
    } //  for (keys=1; keys<=14; keys++)
#endif // 0
}
