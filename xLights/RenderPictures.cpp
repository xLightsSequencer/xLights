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


void RgbEffects::RenderPictures(int dir, const wxString& NewPictureName2,int GifSpeed)
{
    const int speedfactor=4;

    //  <== BEGIN NEW CODE
    //  This is the little new code I added Dave;
    //  Note teh original call was
    //  void RgbEffects::RenderPictures(int dir, const wxString& NewPictureName2,int GifSpeed)
    //  i modified NewPictureName and called it NewPictureName2, just trying to see if i could see the
    //  string values. Now you explained why i cant

    int frame = state%900;

    wxString sPicture,NewPictureName;

   sPicture = wxString::Format(wxT("C:\\Vixen.2.1.1\\Sequences\\movies\\o_44e917b9ebba5a8d-%d"),frame);
    NewPictureName=NewPictureName2;
 //   NewPictureName=sPicture;
    //  I was just hard coding path. I am tring to get the frame to increment and be used
    //  <== END NEW CODE


    if (NewPictureName != PictureName)
    {
        imageCount = wxImage::GetImageCount(NewPictureName);
        imageIndex = 0;
        if (!image.LoadFile(NewPictureName,wxBITMAP_TYPE_ANY,0))
        {
            //wxMessageBox("Error loading image file: "+NewPictureName);
            image.Clear();
        }
        PictureName=NewPictureName;
        if (!image.IsOk())
            return;

    }
    if(imageCount>1)
    {
        // The 10 could be animation speed. I did notice that state is jumping numbers
        // so state%someNumber == 0 may not hit every time. There could be a better way.
        if(state%(21-GifSpeed)==0)  // change 1-20 in Gimspeed to be 20 to 1. This makes right hand slider fastest
        {
            if(imageIndex == imageCount-1)
            {
                imageIndex = 0;
            }
            else
            {
                imageIndex++;

            }


            if (!image.LoadFile(PictureName,wxBITMAP_TYPE_ANY,imageIndex))
            {
                //wxMessageBox("Error loading image file: "+NewPictureName);
                image.Clear();
            }
            if (!image.IsOk())
                return;
        }
    }

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


/*void RgbEffects::RenderPictures(int dir, const wxString& NewPictureName)
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
*/
