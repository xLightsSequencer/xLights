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

void RgbEffects::RenderPictures(int dir, const wxString& NewPictureName)
{
    const int speedfactor=4;
    if (NewPictureName != PictureName)
    {
        if (!image.LoadFile(NewPictureName))
        {
            //wxMessageBox("Error loading image file: "+NewPictureName);
            image.Clear();
        }
        PictureName=NewPictureName;
    }
    if (!image.IsOk()) return;
    int imgwidth=image.GetWidth();
    int imght=image.GetHeight();
    int yoffset=(BufferHt+imght)/2;
    int xoffset=(imgwidth-BufferWi)/2;
    int limit=(dir < 2) ? imgwidth+BufferWi : imght+BufferHt;
    int movement=(state % (limit*speedfactor)) / speedfactor;

    // copy image to buffer
    wxColour c;
    for(int x=0; x<imgwidth; x++)
    {
        for(int y=0; y<imght; y++)
        {
            if (!image.IsTransparent(x,y))
            {
                c.Set(image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y));
                switch (dir)
                {
                case 0:
                    // left
                    SetPixel(x+BufferWi-movement,yoffset-y,c);
                    break;
                case 1:
                    // right
                    SetPixel(x+movement-imgwidth,yoffset-y,c);
                    break;
                case 2:
                    // up
                    SetPixel(x-xoffset,movement-y,c);
                    break;
                case 3:
                    // down
                    SetPixel(x-xoffset,BufferHt+imght-y-movement,c);
                    break;
                default:
                    // no movement - centered
                    SetPixel(x-xoffset,yoffset-y,c);
                    break;
                }
            }
        }
    }
}
