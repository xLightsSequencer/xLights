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
            std::pair<wxPoint, wxColor> new_pixel;
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

#if 0
void RgbEffects::LoadPixelsFromTextFile(wxFile& debug, const wxString& filename)
{
    wxTextFile f;

    if (!PictureName.CmpNoCase(filename)) { wrdebug("no change: " + filename); return; }
    if (!wxFileExists(filename)) { wrdebug("not found: " + filename); return; }
    if (!f.Open(filename.c_str())) { wrdebug("can't open: " + filename); return; }

//read pixel values 1-to-1 from file:
//for ease of access, wrap them as an image
//this should preobably be rewritten into a wx image handler, or maybe reuse one of the existing handlers
    image.Clear();
//        imageCount = 0;
//        imageIndex = 0;
    int maxcol = 0; //vix routine should be rectangular, but in case it isn't, pad out the shorter rows
    std::vector<std::vector</*wxColor or WXCOLORREF*/ wxUint32>> pixels; //wxImage appears to have a static size, so we need to collect all the pixels first before creating the wxImage
    for (wxString linebuf = f.GetFirstLine(); !f.Eof(); linebuf = f.GetNextLine())
    {
        std::string::size_type ofs;
        if ((ofs = linebuf.find("#")) != std::string::npos) linebuf.erase(ofs); //remove comments
        while (!linebuf.empty() && isspace(linebuf.Last())) linebuf.RemoveLast(); //trim trailing spaces
        if (linebuf.empty()) continue; //skip blank lines

        pixels.emplace_back(); //add new row
        wrdebug("got line '" + linebuf + "'");
        wxStringTokenizer tkz(linebuf, " ");
#ifdef ASRGB //use channel triplets to make each pixel
        for (int col = tkz.CountTokens(); col > 0; col -= 3)
        {
            if (col > maxcol) maxcol = col;
//for now assume channels are in R, G, B order:
//            wxColour c;
//            c.Set(gray, gray, gray);
            byte r = wxAtoi(tkz.GetNextToken());
            byte g = wxAtoi(tkz.GetNextToken());
            byte b = wxAtoi(tkz.GetNextToken());
//            c.Set(r, g, b);
            pixels.back().push_back(r); //c.GetRGB()); //image.Set(x, y, c);
            pixels.back().push_back(g);
            pixels.back().push_back(b);
//            image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y));
        }
#endif // ASRGB
#ifdef ASMONO //use one channel for each pixel; color can be added later
        for (int col = tkz.CountTokens(); col > 0; --col)
        {
            if (col > maxcol) maxcol = col;
            byte gray = wxAtoi(tkz.GetNextToken());
//            wxColour c;
//            c.Set(gray, gray, gray);
            pixels.back().push_back(gray); //c.GetRGB()); //image.Set(x, y, c);
            pixels.back().push_back(gray);
            pixels.back().push_back(gray);
        }
#endif // ASMONO
    }
//now create an image to look like it was loaded like the other picture functions:
    image.Create(maxcol + 1, pixels.size());
    for (int y = 0; y < pixels.size(); ++y)
        for (int x = 0; x < pixels[y].size(); x += 3)
            image.SetRGB(x, y, pixels[y][x + 0], pixels[y][x + 1], pixels[y][x + 2]);

    wrdebug(wxString::Format("read %d x %d pixels", maxcol, pixels.size()));
    imageCount = 1; //TODO: allow multiple?
    imageIndex = 0;
    PictureName = filename;
}
#endif

//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG 100
//#include "djdebug.cpp"

void RgbEffects::RenderPictures(int dir, const wxString& NewPictureName2,int GifSpeed, bool is20fps)
{
    const int speedfactor=4;
    wxString suffix,extension,BasePicture,sPicture,NewPictureName,buff;
    wxString filename = "RenderPictures.log";

	// --------------
	// doesn't work when creating DLL (MHB 26 Jan 2014)
	//int createlog= xLightsApp::WantDebug; // use command-line switch to log variables to a log file. this is becaus debug in wxWidgets doesnt display strings
	//  -------------

    wxFile f;
#define debug f //shim; need to rework debug
//#undef wrdebug
//#define wrdebug(x)

    if(NewPictureName2.length()==0) return;

//  Look at ending of the filename passed in. If we have it ending as *-1.jpg or *-1.png then we will assume
//  we have a bunch of jpg files made by ffmpeg
//  movie files can be converted into jpg frames by this command
//      ffmpeg -i XXXX.mp4 -s 16x50 XXXX-%d.jpg
//      ffmpeg -i XXXX.avi -s 16x50 XXXX-%d.jpg
//      ffmpeg -i XXXX.mov -s 16x50 XXXX-%d.jpg
//      ffmpeg -i XXXX.mts -s 16x50 XXXX-%d.jpg

    sPicture = NewPictureName2;
    suffix = NewPictureName2.substr (NewPictureName2.length()-6,2);
    extension = NewPictureName2.substr (NewPictureName2.length()-3,3);
    if( suffix =="-1") // do we have a movie file?
    {
        //    yes
        BasePicture= NewPictureName2.substr (0,NewPictureName2.length()-6) ;

        //  build the next filename. the frame counter is incrementing through all frames


        if(state==0) // only once, try 10000 files to find how high is frame count
        {
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
        } else if (is20fps) {
            frame++;
        } else {
            frame = (state / 10) % maxmovieframes;  //10 is "normal" speed.  < 10 is slow.  > 10 is fast.
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
        wrdebug(wxString::Format("vix remap render: frame %d vs. %d", state, PixelsByFrame.size()));
        if (state < PixelsByFrame.size()) //TODO: wrap?
            for (auto /*std::vector<std::pair<wxPoint, wxColour>>::iterator*/ it = PixelsByFrame[state].begin(); it != PixelsByFrame[state].end(); ++it)
            {
                wrdebug(wxString::Format("set pixel (%d, %d) to color [%d. %d. %d]", it->first.x, it->first.y, it->second.Red(), it->second.Green(), it->second.Blue()));
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
    int imght   =image.GetHeight();
    int yoffset =(BufferHt+imght)/2; //centered if sizes don't match
    int xoffset =(imgwidth-BufferWi)/2; //centered if sizes don't match
    int waveX, waveY, waveW, waveN; //location of first wave, height adjust, width, wave# -DJ
    wrdebug(wxString::Format("pic: state %d, img w/h %d/%d, buf w/h %d/%d, x/y ofs %d/%d", state, imgwidth, imght, BufferWi, BufferHt, xoffset, yoffset));
    float xscale, yscale;
    switch (dir) //prep
    {
        case RENDER_PICTURE_SCALED:
        case RENDER_PICTURE_ZOOMIN: //src <- dest scale factor -DJ
            xscale = (imgwidth > 1)? (float)BufferWi / imgwidth: 1;
            yscale = (imght > 1)? (float)BufferHt / imght: 1;
            if (dir == 12) //zoom in/explode -DJ
            {
                xscale += .001 * state * speedfactor;
                yscale += .001 * state * speedfactor;
//              xscale = log(xscale); xscale *= state * speedfactor / 4; xscale = exp(xscale); //raise to power
//              yscale = log(yscale); yscale *= state * speedfactor / 4; yscale = exp(yscale);
//              wrdebug(1, "zoom: state %d, speed %d, scale %f %f", state, speedfactor, xscale, yscale);
            }
            break;
        case RENDER_PICTURE_PEEKABOO_0: //up+down 1x -DJ
        case RENDER_PICTURE_PEEKABOO_180: //up+down 1x -DJ
            yoffset = state / speedfactor - BufferHt; // * speedfactor; //draw_at = (state < BufferHt)? state
            if (yoffset > 10) yoffset = -yoffset + 10; //reverse direction
            else if (yoffset > 0) yoffset = 0; //pause in middle
//            wrdebug(1, "peekaboo: state %d, speed %d, draw at %d", state, speedfactor, yoffset);
            break;
        case RENDER_PICTURE_WIGGLE: //wiggle left-right -DJ
            xoffset = state % (BufferWi / 4 * speedfactor);
            if (xoffset > BufferWi / 8 * speedfactor) xoffset = BufferWi / 4 * speedfactor - xoffset; //reverse direction
            xoffset -= BufferWi / 4; //* speedfactor; //center it on mid value
            xoffset += (imgwidth-BufferWi) / 2; //add in original xoffset from above
            break;
        case RENDER_PICTURE_PEEKABOO_90: //peekaboo 90
        case RENDER_PICTURE_PEEKABOO_270: //peekaboo 270
            yoffset = (imght - BufferWi) / 2; //adjust offsets for other axis
//            xoffset = (imgwidth - BufferHt) / 2;
            xoffset = state / speedfactor - BufferHt; // * speedfactor; //draw_at = (state < BufferHt)? state
            if (xoffset > 10) xoffset = -xoffset + 10; //reverse direction
            else if (xoffset > 0) xoffset = 0; //pause in middle
            break;
        case RENDER_PICTURE_FLAGWAVE: //flag wave -DJ
            if (!GifSpeed) GifSpeed = 3; //KLUDGE: GifSpeed broken during Scheduler Playback, so default to reasonable value
            waveW = GifSpeed? BufferWi / GifSpeed: BufferWi; //avoid /0; re-use slider as wave count
            if (waveW < 2) waveW = BufferWi; //too many waves
//            waveW += BufferWi / 10; //leave a little gap between waves
            waveX = state / speedfactor; // % (2 * BufferWi) + 1; //location of first wave
            waveN = waveX / waveW;
//            if (waveX > BufferWi) waveX = BufferWi - waveX; //alternate: 1..W,-1..-W
//            debug(1, "wave: gifsp %d, bufwi %d => wavew %d => %d, wavex %d, wave# %d", GifSpeed, BufferWi, GifSpeed? BufferWi / GifSpeed: -1, waveW, waveX, waveN);
            break;
    }
//    if (state < 4) wrdebug(1, "pic: state %d, style %d, img (%d, %d), wnd (%d, %d)", state, dir, imgwidth, imght, BufferWi, BufferHt);
//    if (state < 4) wxMessageBox(xLightsApp::WantDebug? "DEBUG ON": "debug off");

// copy image to buffer
    xlColour c;
    int debug_count = 0;
    for(int x=0; x<imgwidth; x++)
    {
        for(int y=0; y<imght; y++)
        {
            if (!image.IsTransparent(x,y))
            {
                c.Set(image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y));
                switch (dir)
                {
                case RENDER_PICTURE_LEFT: //0:
                    SetPixel(x+BufferWi-(state % ((imgwidth+BufferWi)*speedfactor)) / speedfactor,yoffset-y,c);
                    break; // left
                case RENDER_PICTURE_RIGHT: //1:
                    SetPixel(x+(state % ((imgwidth+BufferWi)*speedfactor)) / speedfactor-imgwidth,yoffset-y,c);
                    break; // right
                case RENDER_PICTURE_UP: //2:
                    SetPixel(x-xoffset,(state % ((imght+BufferHt)*speedfactor)) / speedfactor-y,c);
                    break; // up
                case RENDER_PICTURE_DOWN: //3:
                    SetPixel(x-xoffset,BufferHt+imght-y-(state % ((imght+BufferHt)*speedfactor)) / speedfactor,c);
                    break; // down
                case RENDER_PICTURE_UPLEFT: //5:
                    SetPixel(x+BufferWi-(state % ((imgwidth+BufferWi)*speedfactor)) / speedfactor,(state % ((imght+BufferHt)*speedfactor)) / speedfactor-y,c);
                    break; // up-left
                case RENDER_PICTURE_DOWNLEFT: //6:
                    SetPixel(x+BufferWi-(state % ((imgwidth+BufferWi)*speedfactor)) / speedfactor,BufferHt+imght-y-(state % ((imght+BufferHt)*speedfactor)) / speedfactor,c);
                    break; // down-left
                case RENDER_PICTURE_UPRIGHT: //7:
                    SetPixel(x+(state % ((imgwidth+BufferWi)*speedfactor)) / speedfactor-imgwidth,(state % ((imght+BufferHt)*speedfactor)) / speedfactor-y,c);
                    break; // up-right
                case RENDER_PICTURE_DOWNRIGHT: //8:
                    SetPixel(x+(state % ((imgwidth+BufferWi)*speedfactor)) / speedfactor-imgwidth,BufferHt+imght-y-(state % ((imght+BufferHt)*speedfactor)) / speedfactor,c);
                    break; // down-right
                case RENDER_PICTURE_SCALED: //9: //scaled, no motion -DJ
//TODO: use rescale or resize?
//                    wrdebug(1, "zoom[%d]: pic (x, y) (%d, %d) of (%d, %d) -> wnd (%d, %d) of (%d, %d), color 0x%x", state, x, y, imgwidth, imght, (int)(x * xscale), (int)(BufferHt - 1 - y * yscale), BufferWi, BufferHt, c.GetRGB());
                    SetPixel(x * xscale, BufferHt - 1 - y * yscale, c); //CAUTION: y inverted?; TODO: anti-aliasing, averaging, etc.
                    break;
                case RENDER_PICTURE_PEEKABOO_0: //10: //up+down 1x (peekaboo) -DJ
                    SetPixel(x - xoffset, BufferHt + yoffset - y, c); // - BufferHt, c);
                    break;
                case RENDER_PICTURE_WIGGLE: //11: //back+forth a little (wiggle) -DJ
                    SetPixel(x + xoffset, yoffset - y, c);
                    break;
                case RENDER_PICTURE_ZOOMIN: //12: //zoom in (explode) -DJ
//TODO: use rescale or resize?
                    SetPixel(x * xscale, (BufferHt - 1 - y) * yscale, c); //CAUTION: y inverted?; TODO: anti-aliasing, averaging, etc.
                    break;
//NOTE: a Rotation option should probably be added to all effects rather than just doing it here -DJ
                case RENDER_PICTURE_PEEKABOO_90: //13: //peekaboo 90 -DJ
//                    wrdebug(1, "peeka 90[%d] xofs %d, yofs %d, (x, y) (%d, %d) of (%d, %d) -> wnd (%d, %d) of (%d, %d), color 0x%x", state, xoffset, yoffset, x, y, imgwidth, imght, BufferWi + xoffset - y, x - yoffset, BufferWi, BufferHt, c.GetRGB());
                    SetPixel(BufferWi + xoffset - y, x - yoffset, c);
                    break;
                case RENDER_PICTURE_PEEKABOO_180: //14: //peekaboo 180 -DJ
                    SetPixel(x - xoffset, y - yoffset, c);
                    break;
                case RENDER_PICTURE_PEEKABOO_270: //15: //peekabo 270 -DJ
                    SetPixel(y - xoffset, BufferHt + yoffset - x, c);
//                    SetPixel(y - yoffset - BufferHt, x - xoffset, c);
                    break;
                case RENDER_PICTURE_FLAGWAVE: //17: //flag wave in wind -DJ
//                    if (!x) rippleY = 0; //flag pole edge never moves?
//                    else if (x < abs(rippleX)) rippleY = (rippleX > 0)? +1: -1;
//                    else
                    if (BufferHt < 20) //small grid => small waves
                    {
                        waveN = (x - waveX) / waveW;
                        waveY = !x? 0: (waveN & 1)? -1: 0;
//                        waveN = (x - waveX) / (waveW / 2); //use half-wave to skew waves more down than up
//                        waveY = !x? 0: (waveN & 3)? 0: -1;
                    }
                    else //larger grid => larger waves
                    {
                        waveY = !x? 0: (waveN & 1)? 0: (waveN & 2)? -1: +1;
                        if (waveX < 0) waveY *= -1;
                    }
//                    if (y == 5) debug(1, "draw: x %d, wavex %d => wave# %d, wavey %d", x, waveX, waveN, waveY);
                    SetPixel(x - xoffset, yoffset - y + waveY - 1, c);
                    break;
                default:
                    if (debug_count++ < 2100) wrdebug(wxString::Format("pic: c 0x%2x%2x%2x (%d,%d) -> (%d,%d)", c.Red(), c.Green(), c.Blue(), x, y, x-xoffset, yoffset-y - 1)); //NOTE: wxColor is BGR internally
                    SetPixel(x-xoffset,yoffset-y - 1,c);
                    break; // no movement - centered
                }
            }
        }
    }
}
