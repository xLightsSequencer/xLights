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

void RgbEffects::RenderFaces(int Phoneme)
{
    int ColorIdx;
    size_t i,idx;
    int n,x,y,s;


    wxColour color;
    wxImage::HSVValue hsv;
    int maxframe=BufferHt*2;
    int frame=(BufferHt * state / 200)%maxframe;
    double offset=double(state)/100.0;
    size_t colorcnt=GetColorCount();


    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi,0);
    wxString html = "<html><body><table border=0>";


    /*
    //  I need this code to work. This should allow me to see what the string and node# are from the custom
    //    model.

      size_t NodeCount=GetNodeCount();
       for(i=0; i<NodeCount; i++)
       {
           idx=Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
           if (idx < chmap.size()) chmap[idx]=GetNodeNumber(i);
       }
       for(y=BufferHt-1; y>=0; y--)
       {
           html+="<tr>";
           for(x=0; x<BufferWi; x++)
           {
               n=chmap[y*BufferWi+x];
               if (n==0)
               {
                   html+="<td></td>";
               }
               else
               {
                   s=Nodes[n-1]->StringNum+1;
                   bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
                   html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
               }
           }
           html+="</tr>";
       }
    */


// the following is just code to generate some "twinkle" lights for now. later
//  we will put in real code
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
            palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            SetPixel(x,y,hsv); // Turn pixel on

        }
    }
}
