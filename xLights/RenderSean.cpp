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

void RgbEffects::RenderSean(int Count)
{
     #define PI 3.14159265
    int i,x,y,k,xc,yc,ColorIdx;
    int mod1440,state360,d_mod;
    srand(1);
    float R,r,d,d_orig,t;
    double hyp,x2,y2;
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=GetColorCount();

    xc= (int)(BufferWi/2); // 20x100 flex strips with 2 fols per strip = 40x50
    yc= (int)(BufferHt/2);

      mod1440=state%1440;
    state360 = state%360;
    d_orig=d;
    for(i=1; i<=360; i++)
    {
      // if(Animate) d = (int)(d_orig+state/2)%100; // should we modify the distance variable each pass through?
        t = (i+mod1440)*PI/180;
        x = xc * cos (t) +  xc;
        y = yc * sin (t) +  yc;
  palette.GetHSV(0, hsv0);
        ColorIdx=(state+rand()) % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
        palette.GetHSV(ColorIdx, hsv1); // Now go and get the hsv value for this ColorIdx

        SetPixel(x,y,hsv);
    }

}


//  code for explosions/fireworks
//    const float velocity = 0.2;
//    const int maxFlakes = 262144;
//    const int maxCycle = 4096;
//    const int maxNewBurstFlakes = 10;
//
//    int i, totalFlakes=0, idxFlakes=0;
//    int omoux=0, omouy=0;
//    int moux=0, mouy=0;
//    int events=0;
//
//    int MAXX = 640;
//    int MAXY = 480;
//
//    int x,y,ColorIdx;
//    int lights = (BufferHt*BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
//    int step=BufferHt*BufferWi/lights;
//    if(step<1) step=1;
//    double damping=0.8;
//    srand(1); // always have the same random numbers for each frame (state)
//    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
//    wxColour color;
//    int color1,color2,color3,color4,new_color,old_color;
//    size_t colorcnt=GetColorCount();
//    i=0;
//    palette.GetHSV(0, hsv0); // hsv0.hue, hsv0.saturation, hsv0.value
//    palette.GetHSV(1, hsv1);
//
//
//    if (totalFlakes < maxFlakes-maxNewBurstFlakes)
//    {
//        for(int j=0; j<maxNewBurstFlakes; j++)
//        {
//            totalFlakes++;
//            do
//            {
//                idxFlakes = (idxFlakes + 1) % maxFlakes;
//            }
//            while (flakes[idxFlakes]._bActive);
//            flakes[idxFlakes].reset(moux, mouy, true);
//            //printf("Active: %d\n", totalFlakes);
//        }
//    }
//
//
//    // Process all...
//    //#pragma omp parallel for default(none) shared(flakes, MAXX, MAXY, totalFlakes)
//    for (i=0; i<maxFlakes; i++)
//    {
//
//        // ... active flakes:
//        if (flakes[i]._bActive)
//        {
//
//            // Erase old pixel, if within frame (e.g. don't clear the roof-exceeding ones)
//            if (flakes[i]._x>=0. && flakes[i]._x<MAXX && flakes[i]._y>=0. && flakes[i]._y<MAXY)
//                i=i; //  qplot((int) flakes[i]._x, (int) flakes[i]._y, 0);
//
//            // Update position
//            flakes[i]._x += flakes[i]._dx;
//            flakes[i]._y += flakes[i]._dy + flakes[i]._cycles*flakes[i]._cycles/10000000.0;
//
//            // If this flake run for more than maxCycle, time to switch it off
//            if (maxCycle == flakes[i]._cycles++)
//            {
//                flakes[i]._bActive = false;
//                //#pragma omp atomic
//                totalFlakes--;
//                continue;
//            }
//
//            // If this flake hit the earth, time to switch it off
//            if (flakes[i]._y>=MAXY)
//            {
//                flakes[i]._bActive = false;
//                //#pragma omp atomic
//                totalFlakes--;
//                continue;
//            }
//
//            // Draw the flake, if its X-pos is within frame
//            if (flakes[i]._x>=0. && flakes[i]._x<MAXX)
//            {
//
//                // But only if it is "under" the roof!
//                if (flakes[i]._y>=0.)
//                {
//
//                    // Oh, and "dim" it towards red, and eventually, black:
//                    int color = 255 - int(255*(flakes[i]._cycles*2)/maxCycle);
//                    if (color<0) color=0;
//                    color=color; //  qplot((int) flakes[i]._x, (int) flakes[i]._y, color);
//                }
//            }
//            else
//            {
//                // otherwise it just got outside the valid X-pos, so switch it off
//                flakes[i]._bActive = false;
//                //#pragma omp atomic
//                totalFlakes--;
//                continue;
//            }
//        }
//
//    }
//}
//
