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
#include "wx/log.h"
#include <wx/file.h>
#include <wx/utils.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include "xLightsApp.h"

//CAUTION: these must match EffectDirections exactly:
#define RENDER_PICTURE_LEFT  0
#define RENDER_PICTURE_RIGHT  1
#define RENDER_PICTURE_UP  2
#define RENDER_PICTURE_DOWN  3
#define RENDER_PICTURE_NONE  4
#define RENDER_PICTURE_UPLEFT  5
#define RENDER_PICTURE_DOWNLEFT  6
#define RENDER_PICTURE_UPRIGHT  7
#define RENDER_PICTURE_DOWNRIGHT  8
#define RENDER_PICTURE_SCALED  9
#define RENDER_PICTURE_PEEKABOO_0  10
#define RENDER_PICTURE_WIGGLE  11
#define RENDER_PICTURE_ZOOMIN  12
#define RENDER_PICTURE_PEEKABOO_90  13
#define RENDER_PICTURE_PEEKABOO_180  14
#define RENDER_PICTURE_PEEKABOO_270  15
#define RENDER_PICTURE_VIXREMAP  16
#define RENDER_PICTURE_FLAGWAVE  17


#define wrdebug(msg)  if (debug.IsOpened()) debug.Write(msg + "\n")


//Vixen channel remap from Vixen 2.x back to xLights:
//for use when you have cell-by-cell Vixen 2.x sequencing that you want to preserve in an xLights sequence
//how it works:
//1. look at which channels are on in Vixen during each frame (fixed time intervals)
//2. using the current elapsed time from start of xLights effect to select a Vixen frame,
//     reverse lookup thru the current xLights model to determine which screen pixels must be turned on to generate the same results
//3. set those pixels as the effective output from the xLights effect
//4. xLights will remap those pixels into target channels
//net result is that the output of any effects from Vixen will be duplicated in the xLights sequence
//however, using xLights they can be further manipulated or blended with addition effects to make variations of the original sequence patterns
//NOTE: channels should be in same order between Vixen and xLights; use Vixen Reorder functions to accomplish that, since xLights only reorders within the model



void RgbEffects::RenderGlediator( const wxString& GledFilename)
{
    wxFile f;
    //int maxframes=wxAtoi( MaxFrames ); // get max frames the user has passed in
    int frame,maxframes;
    wxFileOffset fileLength;
    wxColour color;
    wxImage::HSVValue hsv;
    int x,y,p,bytes_per_period,i,j,period; // for now hard code matrix to be 32x32. after we get this working, we will prompt for this info during convert
    unsigned int ch,byte,byte1,byte2;
    wxString filename=wxString::Format(_("01 - Carol of the Bells.mp3")); // hard code a mp3 file for now
    size_t readcnt;
    off_t offset;
    int SeqNumPeriods,SeqDataLen,SeqNumChannels;
    wxString suffix,extension,BasePicture,sPicture,NewPictureName,buff;


    if (!wxFileExists(GledFilename)) // if it doesnt exist, just return
    {
        return;
    }

    if (!f.Open(GledFilename.c_str())) // open the *.gled file
    {
        //   PlayerError(_("Unable to load sequence:\n")+FileName);
        return;
    }

    fileLength=f.Length();
    SeqNumChannels=(BufferWi*3*BufferHt); // 3072 = 32*32*3
    //   char row[1024000];
    char frameBuffer[SeqNumChannels];
    // Get File size

    SeqNumPeriods=(int)(fileLength/(BufferWi*3*BufferHt));
    SeqDataLen=SeqNumPeriods * SeqNumChannels;
//   SetMediaFilename(filename);
//SeqData.resize(fileLength);
    SeqDataLen=fileLength;

    wxYield();
    period = (state/10)%SeqNumPeriods;
    offset = period*SeqNumChannels;
    f.Seek(offset, wxFromStart);
    readcnt=f.Read(frameBuffer,SeqNumChannels); // Read one period of channels
    i=0;
    for(j=0; j<readcnt; j+=3)
    {
        // Loop thru all channels
        color = wxColor(frameBuffer[j],frameBuffer[j+1],frameBuffer[j+2]);
        x=(j%(BufferWi*3))/3;
        y=(BufferHt-1) - (j/(BufferWi*3));
        if(x<BufferWi and y<BufferHt and y>= 0)
        {
            SetPixel(x,y,color);
        }

    }
}
