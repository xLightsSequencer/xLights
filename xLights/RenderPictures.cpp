/***************************************************************
 * Name:      RenderPictures.cpp
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
#define RENDER_PICTURE_UPONCE  18
#define RENDER_PICTURE_DOWNONCE  19
#define RENDER_PICTURE_VECTOR  20
#define RENDER_PICTURE_TILE_LEFT  21
#define RENDER_PICTURE_TILE_RIGHT  22
#define RENDER_PICTURE_TILE_DOWN  23
#define RENDER_PICTURE_TILE_UP  24


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


//this allows copy/paste from Vixen grid:
void RgbEffects::LoadPixelsFromTextFile(wxFile& debug, const wxString& filename)
{
    wxByte rgb[3];
    imageCount = 0;
    imageIndex = 0;
    if (image.GetWidth() && image.GetHeight()) image.Clear(); //CAUTION: image must be non-empty to clear it (memory error otherwise)

    if (!PictureName.CmpNoCase(filename)) { wrdebug("no change: " + filename); return; }
    if (!wxFileExists(filename)) { wrdebug("not found: " + filename); return; }
    wxTextFile f;
    PixelsByFrame.clear();
    if (!f.Open(filename.c_str())) { wrdebug("can't open: " + filename); return; }

//read channel values from Vixen grid or routine:
//    std::vector<std::vector<std::pair<int, byte>>> ChannelsByFrame; //list of channel#s by frame and their associated value
    int numch = 0, chbase = 0, nodesize = 1;
    for (wxString linebuf = f.GetFirstLine(); !f.Eof(); linebuf = f.GetNextLine())
    {
        std::string::size_type ofs;
        if ((ofs = linebuf.find("#")) != std::string::npos) linebuf.erase(ofs); //remove comments
        while (!linebuf.empty() && isspace(linebuf.Last())) linebuf.RemoveLast(); //trim trailing spaces
        if (linebuf.empty()) continue; //skip blank lines

        wrdebug(wxString::Format("read line '%s'", (const char*)linebuf.c_str()));
        static wxRegEx chbase_re("^\\s*ChannelBase\\s*=\\s*(-?[0-9]+)\\s*$", wxRE_ICASE);
        if (!PixelsByFrame.size() && chbase_re.Matches(linebuf)) //allow channels to be shifted
        {
            chbase = wxAtoi(chbase_re.GetMatch(linebuf, 1));
            wrdebug(wxString::Format("got ch base %d", chbase));
            continue;
        }
        static wxRegEx nodesize_re("^\\s*ChannelsPerNode\\s*=\\s*([13])\\s*$", wxRE_ICASE);
        if (!PixelsByFrame.size() && nodesize_re.Matches(linebuf)) //allow channels to be shifted
        {
            nodesize = wxAtoi(nodesize_re.GetMatch(linebuf, 1));
            wrdebug(wxString::Format("got node size %d", nodesize));
            continue;
        }

        PixelVector frame;
        wrdebug(wxString::Format("load channels for frame %d (%.3f sec): '" + linebuf + "'", PixelsByFrame.size(), PixelsByFrame.size() * 50/1000.));
        wxStringTokenizer tkz(linebuf, " ");
        for (int chnum = 0; tkz.HasMoreTokens(); ++chnum)
        {
            wxByte chval = wxAtoi(tkz.GetNextToken());
            wrdebug(wxString::Format("got chval %d for ch %d, frame %d", (int)chval, chnum, PixelsByFrame.size()));
            if (!chval) continue; //only need to remember channels that are on (assume most channels are off)
            std::pair<wxPoint, xlColor> new_pixel;
            switch (nodesize)
            {
                case 1: //map each Vixen channel to a monochrome pixel
                    new_pixel.second.Set(chval, chval, chval); //grayscale
                    break;
                case 3: //map Vixen triplets to an RGB pixel
                    switch (chnum % 3)
                    {
                        case 0: rgb[0] = chval; continue;
                        case 1: rgb[1] = chval; continue;
                        case 2: rgb[2] = chval; break;
                    }
            }
            new_pixel.second.Set(rgb[0], rgb[1], rgb[2]);
//            for (each wxPoint where chnum + chbase occurs in current model)
                frame.push_back(new_pixel); //build list of pixels that must be set
            if (chnum + 1 > numch) numch = chnum + 1; //vix grid or routine should be rectangular, but in case it isn't, pad out the shorter rows
        }
		PixelsByFrame.push_back(frame); //add new frame, MSVC 2010 doesn't support emplace_back
    }
//now create an image to look like it was loaded like the other picture functions:
//    image.Create(maxcol + 1, pixels.size());
//    for (int y = 0; y < pixels.size(); ++y)
//        for (int x = 0; x < pixels[y].size(); x += 3)
//            image.SetRGB(x, y, pixels[y][x + 0], pixels[y][x + 1], pixels[y][x + 2]);

    wrdebug(wxString::Format("read %d channels (relative to %d) x %d frames from Vixen, channels/node: %d", numch, chbase, PixelsByFrame.size(), nodesize));
//    imageCount = 1; //TODO: allow multiple?
//    imageIndex = 0;
    PictureName = filename;
}

//            image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y));


void RgbEffects::ProcessPixel(int x_pos, int y_pos, const xlColour &color, bool wrap_x, int width)
{
    int x_value = x_pos;
    if( wrap_x )  // if set wrap image at boundary
    {
        x_value %= width;
        x_value = (x_value >= 0) ? (x_value) : (width + x_value);
    }
    SetPixel(x_value,y_pos,color);
}

void RgbEffects::RenderPictures(int dir, const wxString& NewPictureName2,
                                float movementSpeed, float frameRateAdj,
                                int xc_adj, int yc_adj,
                                int xce_adj, int yce_adj,
                                bool pixelOffsets, bool wrap_x)
{
    double position = GetEffectTimeIntervalPosition(movementSpeed);
    wxString suffix,extension,BasePicture,sPicture,NewPictureName,buff;

    wxFile f;
    if(NewPictureName2.length()==0) return;

//  Look at ending of the filename passed in. If we have it ending as *-1.jpg or *-1.png then we will assume
//  we have a bunch of jpg files made by ffmpeg
//  movie files can be converted into jpg frames by this command
//      ffmpeg -i XXXX.mp4 -s 16x50 XXXX-%d.jpg
//      ffmpeg -i XXXX.avi -s 16x50 XXXX-%d.jpg
//      ffmpeg -i XXXX.mov -s 16x50 XXXX-%d.jpg
//      ffmpeg -i XXXX.mts -s 16x50 XXXX-%d.jpg

    sPicture = NewPictureName2;
    suffix = NewPictureName2.substr(NewPictureName2.length()-6,2);
    extension = NewPictureName2.substr(NewPictureName2.length()-3,3);
    if (suffix == "-1")  {// do we have a movie file?
        //    yes
        BasePicture= NewPictureName2.substr(0,NewPictureName2.length()-6) ;

        //  build the next filename. the frame counter is incrementing through all frames
        if (needToInit) { // only once, try 10000 files to find how high is frame count
            needToInit = false;
            maxmovieframes = 1;
            sPicture = wxString::Format("%s-%d.%s",BasePicture,frame,extension);
            for (frame=1; frame<=9999; frame++)
            {
                sPicture = wxString::Format("%s-%d.%s",BasePicture,frame,extension);
                if(wxFileExists(sPicture)) {
                    maxmovieframes=frame;
                } else {
                    break;
                }
            }
            frame=1;
        } else {
            frame = floor((double(curPeriod - curEffStartPer)) * frameRateAdj) + 1;
        }
        if (frame > maxmovieframes) {
            return;
        }
        sPicture = wxString::Format("%s-%d.%s",BasePicture,frame,extension);
    }

    NewPictureName=sPicture;

    if (dir == RENDER_PICTURE_VIXREMAP) //load pre-rendered pixels from file and apply to model -DJ
    {
        LoadPixelsFromTextFile(f, NewPictureName);
        int idx = curPeriod - curEffStartPer;
        if (idx < PixelsByFrame.size()) //TODO: wrap?
            for (auto /*std::vector<std::pair<wxPoint, xlColour>>::iterator*/ it = PixelsByFrame[idx].begin(); it != PixelsByFrame[idx].end(); ++it)
            {
                SetPixel(it->first.x, it->first.y, it->second);
            }
        return;
    }

    if (NewPictureName != PictureName)
    {
        wxLogNull logNo;  // suppress popups from png images. See http://trac.wxwidgets.org/ticket/15331
        imageCount = wxImage::GetImageCount(NewPictureName);
        imageIndex = 0;
        if (!image.LoadFile(NewPictureName,wxBITMAP_TYPE_ANY,0))
        {
            //wxMessageBox("Error loading image file: "+NewPictureName);
            image.Create(5, 5, true);
        }
        PictureName=NewPictureName;
        if (!image.IsOk())
            return;
    }
    if(imageCount > 1) {
        //animated Gif,
        int ii = imageCount * GetEffectTimeIntervalPosition(frameRateAdj) * 0.99;
        if (ii != imageIndex) {
            imageIndex = ii;
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
    int imght   =image.GetHeight();
    int yoffset =(BufferHt+imght)/2; //centered if sizes don't match
    int xoffset =(imgwidth-BufferWi)/2; //centered if sizes don't match
    int waveX, waveY, waveW, waveN; //location of first wave, height adjust, width, wave# -DJ
    float xscale, yscale;
    switch (dir) //prep
    {
        case RENDER_PICTURE_SCALED:
            dir = RENDER_PICTURE_NONE;
            image.Rescale(BufferWi, BufferHt);
            imgwidth=image.GetWidth();
            imght = image.GetHeight();
            yoffset =(BufferHt+imght)/2; //centered if sizes don't match
            xoffset =(imgwidth-BufferWi)/2; //centered if sizes don't match
            break;
        case RENDER_PICTURE_ZOOMIN: //src <- dest scale factor -DJ
            xscale = (imgwidth > 1)? (float)BufferWi / imgwidth: 1;
            yscale = (imght > 1)? (float)BufferHt / imght: 1;
            xscale *= position;
            yscale *= position;
            break;
        case RENDER_PICTURE_PEEKABOO_0: //up+down 1x -DJ
        case RENDER_PICTURE_PEEKABOO_180: //up+down 1x -DJ
            yoffset = (-BufferHt) * (1.0 - position*2.0);
            if (yoffset > 10) yoffset = -yoffset + 10; //reverse direction
            else if (yoffset > 0) yoffset = 0; //pause in middle
            break;
        case RENDER_PICTURE_PEEKABOO_90: //peekaboo 90
        case RENDER_PICTURE_PEEKABOO_270: //peekaboo 270
            yoffset = (imght - BufferWi) / 2; //adjust offsets for other axis
            xoffset =  (-BufferHt) * (1.0-position*2.0); // * speedfactor; //draw_at = (state < BufferHt)? state
            if (xoffset > 10) xoffset = -xoffset + 10; //reverse direction
            else if (xoffset > 0) xoffset = 0; //pause in middle
            break;
        case RENDER_PICTURE_UPONCE:
        case RENDER_PICTURE_DOWNONCE:
            position = GetEffectTimeIntervalPosition() * movementSpeed;
            if (position > 1.0) {
                position = 1.0;
            }
            break;
        case RENDER_PICTURE_WIGGLE: //wiggle left-right -DJ
            if (position >= 0.5) {
                xoffset += BufferWi * ((1.0 - position)*2.0 - 0.5);
            } else {
                xoffset += BufferWi * (position * 2.0 - 0.5);
            }
            break;
        case RENDER_PICTURE_FLAGWAVE: //flag wave -DJ
            waveW = BufferWi;
            waveX = position * 200;
            waveN = waveX / waveW;
            break;
    }

    int xoffset_adj = xc_adj;
    int yoffset_adj = yc_adj;
    if (dir == RENDER_PICTURE_VECTOR) {
        //adjust the picture offset
        dir = RENDER_PICTURE_NONE;
        xoffset_adj = std::round(position * double(xce_adj - xc_adj)) + xc_adj;
        yoffset_adj = std::round(position * double(yce_adj - yc_adj)) + yc_adj;
    }
    if (!pixelOffsets) {
        xoffset_adj = (xoffset_adj*BufferWi)/100.0; // xc_adj is from -100 to 100
        yoffset_adj = (yoffset_adj*BufferHt)/100.0; // yc_adj is from -100 to 100
    }
    // copy image to buffer
    xlColour c;
    bool hasAlpha = image.HasAlpha();
    for(int x=0; x<imgwidth; x++)
    {
        for(int y=0; y<imght; y++)
        {
            if (!image.IsTransparent(x,y))
            {
                unsigned char alpha = hasAlpha ? image.GetAlpha(x, y) : 255;
                c.Set(image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y), alpha);
                if (!allowAlpha && alpha < 64) {
                    //almost transparent, but this mix doesn't support transparent unless it's black;
                    c = xlBLACK;
                }
                switch (dir)
                {
                case RENDER_PICTURE_LEFT: //0:
                    ProcessPixel(x+BufferWi-((imgwidth+BufferWi+xoffset_adj)*position),yoffset-y-yoffset_adj,c, wrap_x, imgwidth);
                    break; // left
                case RENDER_PICTURE_RIGHT: //1:
                    ProcessPixel(x+((imgwidth+BufferWi+xoffset_adj)*position)-imgwidth,yoffset-y-yoffset_adj,c, wrap_x, imgwidth);
                    break; // right
                case RENDER_PICTURE_UP: //2:
                case RENDER_PICTURE_UPONCE: //18
                    ProcessPixel(x-xoffset+xoffset_adj,((imght+BufferHt)*position)-y-yoffset_adj,c, wrap_x, imgwidth);
                    break; // up
                case RENDER_PICTURE_DOWN: //3:
                case RENDER_PICTURE_DOWNONCE: //19
                    ProcessPixel(x-xoffset+xoffset_adj,BufferHt+imght-y-yoffset_adj-((imght+BufferHt)*position),c, wrap_x, imgwidth);
                    break; // down
                case RENDER_PICTURE_UPLEFT: //5:
                    ProcessPixel(x+xoffset_adj+BufferWi-((imgwidth+BufferWi)*position),((imght+BufferHt)*position)-y-yoffset_adj,c, wrap_x, imgwidth);
                    break; // up-left
                case RENDER_PICTURE_DOWNLEFT: //6:
                    ProcessPixel(x+xoffset_adj+BufferWi-((imgwidth+BufferWi)*position),BufferHt+imght-y-yoffset_adj-((imght+BufferHt)*position),c, wrap_x, imgwidth);
                    break; // down-left
                case RENDER_PICTURE_UPRIGHT: //7:
                    ProcessPixel(x+xoffset_adj+((imgwidth+BufferWi)*position)-imgwidth,((imght+BufferHt)*position)-y-yoffset_adj,c, wrap_x, imgwidth);
                    break; // up-right
                case RENDER_PICTURE_DOWNRIGHT: //8:
                    ProcessPixel(x+xoffset_adj+((imgwidth+BufferWi)*position)-imgwidth,BufferHt+imght-y-yoffset_adj-((imght+BufferHt)*position),c, wrap_x, imgwidth);
                    break; // down-right

                case RENDER_PICTURE_PEEKABOO_0: //10: //up+down 1x (peekaboo) -DJ
                    ProcessPixel(x - xoffset+xoffset_adj, BufferHt + yoffset - y - yoffset_adj, c, wrap_x, imgwidth); // - BufferHt, c);
                    break;
                case RENDER_PICTURE_ZOOMIN: //12: //zoom in (explode) -DJ
//TODO: use rescale or resize?
                    ProcessPixel((x+xoffset_adj) * xscale, (BufferHt - 1 - y - yoffset_adj) * yscale, c, wrap_x, imgwidth); //CAUTION: y inverted?; TODO: anti-aliasing, averaging, etc.
                    break;
                case RENDER_PICTURE_PEEKABOO_90: //13: //peekaboo 90 -DJ
                    ProcessPixel(BufferWi + xoffset - y + xoffset_adj, x - yoffset - yoffset_adj, c, wrap_x, imgwidth);
                    break;
                case RENDER_PICTURE_PEEKABOO_180: //14: //peekaboo 180 -DJ
                    ProcessPixel(x - xoffset+xoffset_adj, y - yoffset - yoffset_adj, c, wrap_x, imgwidth);
                    break;
                case RENDER_PICTURE_PEEKABOO_270: //15: //peekabo 270 -DJ
                    ProcessPixel(y - xoffset+xoffset_adj, BufferHt + yoffset + yoffset_adj - x, c, wrap_x, imgwidth);
                    break;
                case RENDER_PICTURE_FLAGWAVE: //17: //flag wave in wind -DJ
                    if (BufferHt < 20) //small grid => small waves
                    {
                        waveN = (x - waveX) / waveW;
                        waveY = !x? 0: (waveN & 1)? -1: 0;
                    }
                    else //larger grid => larger waves
                    {
                        waveY = !x? 0: (waveN & 1)? 0: (waveN & 2)? -1: +1;
                        if (waveX < 0) waveY *= -1;
                    }
                    ProcessPixel(x - xoffset+xoffset_adj, yoffset - y - yoffset_adj + waveY - 1, c, wrap_x, imgwidth);
                    break;
                case RENDER_PICTURE_TILE_LEFT: // 21
                    {
                        for (int renderycount = 0; renderycount < (BufferHt + imght) / imght; renderycount++)
                        {
                            for(int renderxcount = 0; renderxcount < (BufferWi + imgwidth) / imgwidth; renderxcount++)
                            {
                                ProcessPixel((x + xoffset_adj + -1 * ((int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imgwidth)) % imgwidth + renderxcount * imgwidth,
                                             (BufferHt - 1 - (y + 0 + yoffset_adj) % imght + renderycount * imght) % BufferHt,
                                             c, false, 0);
                            }
                        }
                    }
                    break;
                case RENDER_PICTURE_TILE_RIGHT: // 22
                    {
                        for (int renderycount = 0; renderycount < (BufferHt + imght) / imght; renderycount++)
                        {
                            for(int renderxcount = 0; renderxcount < (BufferWi + imgwidth) / imgwidth; renderxcount++)
                            {
                                ProcessPixel((x + xoffset_adj + ((int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imgwidth)) % imgwidth + renderxcount * imgwidth,
                                             (BufferHt - 1 - (y + 0 + yoffset_adj) % imght + renderycount * imght) % BufferHt,
                                             c, false, 0);
                            }
                        }
                    }
                    break;
                case RENDER_PICTURE_TILE_DOWN: // 23
                    {
                        for (int renderycount = 0; renderycount < (BufferHt + imght) / imght; renderycount++)
                        {
                            for(int renderxcount = 0; renderxcount < (BufferWi + imgwidth) / imgwidth; renderxcount++)
                            {
                                ProcessPixel((x + 0 + xoffset_adj) % imgwidth + renderxcount * imgwidth,
                                             (BufferHt - 1 - (y + yoffset_adj + ((int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imght)) % imght + renderycount * imght) % BufferHt,
                                             c, false, 0);
                            }
                        }
                    }
                    break;
                case RENDER_PICTURE_TILE_UP: // 24
                    {
                        for (int renderycount = 0; renderycount < (BufferHt + imght) / imght; renderycount++)
                        {
                            for(int renderxcount = 0; renderxcount < (BufferWi + imgwidth) / imgwidth; renderxcount++)
                            {
                                ProcessPixel((x + 0 + xoffset_adj) % imgwidth + renderxcount * imgwidth,
                                             (BufferHt - 1 - (y + yoffset_adj + -1 * ((int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imght)) % imght + renderycount * imght) % BufferHt,
                                             c, false, 0);
                            }
                        }
                    }
                    break;
                case RENDER_PICTURE_WIGGLE: //11: //back+forth a little (wiggle) -DJ
//                    ProcessPixel(x + xoffset+xoffset_adj, yoffset - y - yoffset_adj, c, wrap_x, imgwidth);
//                    break;
                default:
                    ProcessPixel(x-xoffset+xoffset_adj,yoffset+yoffset_adj-y - 1,c, wrap_x, imgwidth);
                    break; // no movement - centered
                }
            }
        }
    }
}
