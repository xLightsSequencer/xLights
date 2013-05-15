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

void RgbEffects::RenderText(int Top, int Left, const wxString& Line1, const wxString& Line2, const wxString& FontString, int dir,int TextRotation)
{
    wxColour c;
    wxBitmap bitmap(BufferWi,BufferHt);
    wxMemoryDC dc(bitmap);
    wxFont font;
    int ColorIdx,itmp;
    size_t colorcnt=GetColorCount();
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"

    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

    font.SetNativeFontInfoUserDesc(FontString);
    dc.SetFont(font);
    palette.GetColor(0,c);
    dc.SetTextForeground(c);
    wxString msg = Line1;



    if (!Line2.IsEmpty())
    {
        if(colorcnt>1)
        {
          //  palette.GetColor(1,c);
        }
        msg+=wxT("\n")+Line2;
        //      dc.SetTextForeground(c);
    }
    wxSize sz1 = dc.GetTextExtent(Line1);
    wxSize sz2 = dc.GetTextExtent(Line2);
    int maxwidth=sz1.GetWidth() > sz2.GetWidth() ? sz1.GetWidth() : sz2.GetWidth();
    int maxht=sz1.GetHeight() > sz2.GetHeight() ? sz1.GetHeight() : sz2.GetHeight();
    if(TextRotation==1)
    {
        itmp=maxwidth;
        maxwidth=maxht;
        maxht=itmp;
    }
    int dctop=Top * BufferHt / 50 - BufferHt/2;
    int xlimit=(BufferWi+maxwidth)*8 + 1;
    int ylimit=(BufferHt+maxht)*8 + 1;
//  int xcentered=(BufferWi-maxwidth)/2;  // original way
    int xcentered=Left * BufferWi / 50 - BufferWi/2;


    TextRotation *=90.0;
    switch (dir)
    {
    case 0:
        // left
        // dc.DrawText(msg,BufferWi-state % xlimit/8,dctop);

        dc.DrawRotatedText(msg,BufferWi-state % xlimit/8,dctop,TextRotation);
        break;
    case 1:
        // right
        // dc.DrawText(msg,state % xlimit/8-BufferWi,dctop);
        dc.DrawRotatedText(msg,state % xlimit/8-BufferWi,dctop,TextRotation);
        break;
    case 2:
        // up
        //  dc.DrawText(msg,xcentered,BufferHt-state % ylimit/8);
        dc.DrawRotatedText(msg,xcentered,BufferHt-state % ylimit/8,TextRotation);
        break;
    case 3:
        // down
        //  dc.DrawText(msg,xcentered,state % ylimit / 8 - BufferHt);
        dc.DrawRotatedText(msg,xcentered,state % ylimit / 8 - BufferHt,TextRotation);
        break;
    default:
        // no movement - centered
        //   dc.DrawText(msg,xcentered,dctop);
        dc.DrawRotatedText(msg,xcentered,dctop,TextRotation);
        break;
    }

    // copy dc to buffer
    for(wxCoord x=0; x<BufferWi; x++)
    {
        for(wxCoord y=0; y<BufferHt; y++)
        {
            dc.GetPixel(x,BufferHt-y-1,&c);
            SetPixel(x,y,c);

//        ColorIdx=(n % BlockHt) / BarHt;
//        palette.GetHSV(ColorIdx, hsv);

        }
    }
}
