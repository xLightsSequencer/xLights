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
#include "xLightsMain.h" //xLightsFrame


#define WANT_DEBUG_IMPL
#define WANT_DEBUG  99
//#include "djdebug.cpp"


int FindChannelAtXY(int x, int y, const wxString& model)
{
//get list of models:
    wxString buf;
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if (!model.IsEmpty() && model.CmpNoCase((*it)->name)) continue; //don't check this model

        /*
        //        debug(1, "checking model '%s' ...", (const char*)(*it)->name.c_str());
              buf = xLightsFrame::PreviewModels[0]->ChannelLayoutHtml();
               if (buf.size() > 500) buf.resize(500);
               debug(1, "first 500 char of layout html = %s", (const char*)buf);
               wxString buf = (*it)->ChannelLayoutHtml();

                for (size_t n = (*it)->GetNodeCount(); n > 0; --n)
               {
                   Nodes[nodenum]->Coords.size()
               }
               size_t CoordCount=GetCoordCount(n);
                for(size_t c=0; c < CoordCount; c++)
               {
                   Nodes[n]->Coords[c].screenX = Nodes[n]->Coords[c].bufX - xoffset;
                   Nodes[n]->Coords[c].screenY = Nodes[n]->Coords[c].bufY;
               }

        */
        int ch = (*it)->FindChannelAt(x, y);
        if (ch != -1) return ch;
    }
    return -1; //pixel not found
}


void RgbEffects::RenderFaces(int Phoneme)
{


    /*
        FacesPhoneme.Add("AI");     0
        FacesPhoneme.Add("E");      1
        FacesPhoneme.Add("FV");     2
        FacesPhoneme.Add("L");      3
        FacesPhoneme.Add("MBP");    4
        FacesPhoneme.Add("O");      5
        FacesPhoneme.Add("U");      6
        FacesPhoneme.Add("WQ");     7
        FacesPhoneme.Add("etc");    8
        FacesPhoneme.Add("rest");   9
    */
    wxColour color;
    wxImage::HSVValue hsv;
    int maxframe=BufferHt*2;
    int frame=(BufferHt * state / 200)%maxframe;
    double offset=double(state)/100.0;
    size_t colorcnt=GetColorCount();


//    std::vector<int> chmap;
    std::vector<std::vector<int>> chmap; //array of arrays
//    chmap.resize(BufferHt * BufferWi,0);
    ModelClass mc;
    mc.GetChannelCoords(chmap, true); //method is on ModelClass object


    wxString html = "<html><body><table border=0>";
    int Ht, Wt;
    Ht = BufferHt;
    Wt = BufferWi;
    int mode; // 1=auto, 2=coroface, 3=picture,4=movie;

    mode=2;
    switch (mode)
    {


    case 1:
        mouth( Phoneme, Ht,  Wt); // draw a mouth syllable
        break;

    case 2:
        coroface( Phoneme); // draw a mouth syllable
        break;
    }


//size_t NodeCount=GetNodeCount();

//    above is from ModelClass::ChannelLayoutHtml()
#if 1 //sample code for Sean
    std::vector<std::vector<int>> face_channels;
    wxString model_name = "(change this)";
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if (model_name.CmpNoCase((*it)->name)) continue; //don't check this model
        wxSize wh = (*it)->GetChannelCoords(face_channels, true);
//        debug(1, "model '%s' is %d x %d, channel[0,0] = %d, ...", (const char*)(*it)->name.c_str(), wh.x, wh.y, face_channels[0][0]);
        break;
    }
#endif // 1
#if 0 //DEBUG
//get list of models:
    wxString buf;
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
        buf += ", " + (*it)->name; //ModelClassPtr*
    debug(1, "faces: models = %s", (CmpNoCaseconst char*)buf + 2);

//get info about one of the models:
    buf = xLightsFrame::PreviewModels[0]->name;
    debug(1, "first model is %s", (const char*)buf);
    buf = xLightsFrame::PreviewModels[0]->ChannelLayoutHtml();
    if (buf.size() > 500) buf.resize(500);
    debug(1, "first 500 char of layout html = %s", (const char*)buf);
#endif
}

void RgbEffects::coroface(int Phoneme)
{
    /*
       FacesPhoneme.Add("AI");     0 : 2,3
       FacesPhoneme.Add("E");      1 : 6
       FacesPhoneme.Add("FV");     2 : 6
       FacesPhoneme.Add("L");      3 : 6
       FacesPhoneme.Add("MBP");    4 : 6
       FacesPhoneme.Add("O");      5 :4
       FacesPhoneme.Add("U");      6 :4
       FacesPhoneme.Add("WQ");     7 : 5
       FacesPhoneme.Add("etc");    8 : 5
       FacesPhoneme.Add("rest");   9 : 6
    */


#if 0
    int face[5][5] =
    {
        {7,8,4,8,1}, /*  initializers for row indexed by 0 */
        {-1,4,2,1,-1}, /*  initializers for row indexed by 1 */
        {4,2,1,3,-1}, /*  initializers for row indexed by 2 */
        {-1,1,3,5,-1}, /*  initializers for row indexed by 3 */
        {1,-1,5,-1,6}
    }; /*  initializers for row indexed by 4 */
#endif

    int face[51][51] =
    {
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,1,1,1,1,1,1,1,-1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 50 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 49 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 48 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,3,3,-1,-1,-1,3,3,3,3,-1,-1,3,3,3,3,-1,-1,-1,3,3,3,-1,-1,-1,3,3,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 47 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,3,-1,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,-1,3,-1,3,-1,-1,-1,3,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 46 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,3,-1,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,-1,3,-1,3,-1,-1,-1,3,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 45 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,3,-1,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,-1,3,-1,3,-1,-1,-1,3,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 44 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,3,-1,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,3,-1,-1,-1,3,-1,3,-1,-1,-1,3,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 43 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,3,3,3,3,3,-1,-1,3,3,3,3,4,4,3,3,3,3,3,-1,3,3,3,3,3,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 42 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,-1,-1,-1,-1,-1,5,5,5,5,4,4,5,5,4,5,5,5,5,-1,-1,-1,-1,-1,-1,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 41 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,-1,-1,-1,-1,-1,5,-1,-1,4,-1,-1,-1,-1,-1,4,-1,-1,-1,5,5,-1,-1,-1,-1,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 40 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,-1,-1,6,6,5,-1,-1,4,-1,6,6,-1,6,6,6,4,-1,6,6,-1,5,5,6,-1,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 39 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,-1,6,6,-1,5,6,6,6,4,6,-1,-1,6,-1,-1,6,4,6,-1,-1,6,6,6,5,6,-1,3,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 38 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,3,6,-1,-1,5,-1,-1,-1,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1,-1,5,6,-1,3,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 37 */
        {-1,-1,-1,-1,-1,-1,-1,-1,1,-1,6,6,-1,-1,-1,5,-1,-1,-1,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1,-1,-1,5,6,6,6,6,-1,1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 36 */
        {-1,-1,-1,-1,-1,-1,-1,1,1,-1,6,3,-1,-1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1,-1,-1,-1,5,-1,3,-1,-1,6,-1,1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 35 */
        {-1,-1,-1,-1,-1,-1,-1,1,-1,-1,6,3,-1,-1,5,5,5,-1,5,-1,4,5,5,-1,-1,5,-1,5,4,-1,5,5,5,-1,-1,5,5,5,-1,3,-1,6,-1,1,1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 34 */
        {-1,-1,-1,-1,-1,-1,1,-1,-1,-1,6,3,-1,5,5,-1,-1,5,-1,5,5,4,4,5,5,5,5,4,5,5,-1,-1,-1,5,5,-1,-1,5,-1,3,-1,-1,6,-1,1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 33 */
        {-1,-1,-1,-1,-1,1,-1,-1,-1,6,3,-1,5,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,4,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,5,-1,3,-1,-1,6,-1,1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 32 */
        {-1,-1,-1,-1,1,1,-1,-1,6,3,3,-1,5,2,2,2,2,2,-1,-1,2,2,2,2,-1,-1,2,2,2,2,-1,-1,2,2,2,2,2,-1,5,5,-1,3,-1,6,-1,1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 31 */
        {-1,-1,-1,1,1,-1,-1,6,3,3,-1,5,-1,2,-1,-1,-1,2,-1,-1,2,-1,-1,2,-1,-1,2,-1,-1,2,-1,-1,2,-1,-1,-1,2,-1,-1,5,5,3,-1,-1,6,-1,1,-1,-1,-1,-1}, /*  initializers for row indexed by 30 */
        {-1,-1,1,-1,-1,-1,6,3,3,-1,5,5,-1,2,-1,-1,-1,2,2,2,2,-1,-1,2,2,2,2,-1,-1,2,2,2,2,-1,-1,-1,2,-1,-1,-1,5,-1,3,-1,6,-1,-1,1,-1,-1,-1}, /*  initializers for row indexed by 29 */
        {-1,-1,1,-1,-1,-1,6,3,5,5,5,-1,-1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,-1,-1,-1,-1,5,-1,3,-1,6,-1,-1,1,-1,-1}, /*  initializers for row indexed by 28 */
        {-1,1,-1,-1,-1,6,3,5,5,5,-1,-1,-1,2,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,2,-1,-1,-1,-1,5,5,3,-1,6,-1,-1,-1,1,1}, /*  initializers for row indexed by 27 */
        {1,-1,-1,-1,-1,6,3,5,-1,-1,-1,-1,2,2,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,2,2,-1,-1,-1,5,5,3,-1,6,-1,-1,-1,1}, /*  initializers for row indexed by 26 */
        {1,-1,-1,-1,6,3,5,-1,-1,2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2,-1,5,-1,3,-1,6,-1,-1,1}, /*  initializers for row indexed by 25 */
        {1,-1,-1,6,3,3,-1,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2,-1,3,6,-1,-1,1}, /*  initializers for row indexed by 24 */
        {1,-1,-1,6,3,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,6,-1,1}, /*  initializers for row indexed by 23 */
        {-1,1,-1,-1,-1,-1,-1,-1,-1,8,8,8,8,8,8,8,8,8,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,1,-1,-1,-1,8,8,8,8,8,8,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1}, /*  initializers for row indexed by 22 */
        {-1,1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,1,1,-1,-1,-1,-1,1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,8,8,8,8,8,-1,-1,-1,-1,-1,-1,1}, /*  initializers for row indexed by 21 */
        {1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,-1,-1,-1,1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,-1,1,-1}, /*  initializers for row indexed by 20 */
        {1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,-1,-1,-1,1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,1,-1}, /*  initializers for row indexed by 19 */
        {1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,1,-1,-1,-1,1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,1,-1}, /*  initializers for row indexed by 18 */
        {1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,1,-1,-1,-1,1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,1}, /*  initializers for row indexed by 17 */
        {1,-1,-1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,1,-1,-1,-1,1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,1}, /*  initializers for row indexed by 16 */
        {1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,-1,1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,1}, /*  initializers for row indexed by 15 */
        {1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,-1}, /*  initializers for row indexed by 14 */
        {-1,1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,-1}, /*  initializers for row indexed by 13 */
        {-1,1,1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,-1}, /*  initializers for row indexed by 12 */
        {-1,-1,1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,1,-1,-1}, /*  initializers for row indexed by 11 */
        {-1,-1,1,-1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,1,-1,-1}, /*  initializers for row indexed by 10 */
        {-1,-1,-1,1,1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,1,-1,-1,-1}, /*  initializers for row indexed by 9 */
        {-1,-1,-1,-1,-1,1,-1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,1,-1,-1,-1,-1}, /*  initializers for row indexed by 8 */
        {-1,-1,-1,-1,-1,-1,1,1,-1,-1,8,8,8,8,8,8,8,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,8,8,8,8,8,8,8,-1,-1,-1,-1,1,-1,-1,-1,-1}, /*  initializers for row indexed by 7 */
        {-1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 6 */
        {-1,-1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 5 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 4 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 3 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 2 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, /*  initializers for row indexed by 1 */
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    }; /*  initializers for row indexed by 0 */





    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();
    int ColorIdx=0;
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue=0.0;
    hsv.value=1.0;
    hsv.saturation=1.0;
    int v;
    wxString model; //set to target model name (optional)



    for (int y=0; y<BufferHt; y++)
    {
        for (int x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            v=face[x][y];
            switch (Phoneme)
            {

            case 0: // FacesPhoneme.Add("AI");     0 : 2,3
                if(v==2 || v==3)  SetPixel(x,y,hsv);
                break;


            case 1: //FacesPhoneme.Add("E");      1 : 6
                if(v==6)  SetPixel(x,y,hsv);
                break;

            case 2: //FacesPhoneme.Add("FV");     2 : 6
                if(v==6)  SetPixel(x,y,hsv);
                break;

            case 3: //FacesPhoneme.Add("L");      3 : 6
                if(v==6)  SetPixel(x,y,hsv);
                break;

            case 4: //FacesPhoneme.Add("MBP");    4 : 6
                if(v==6)  SetPixel(x,y,hsv);
                break;

            case 5: //FacesPhoneme.Add("O");      5 :4
                if(v==4)  SetPixel(x,y,hsv);
                break;

            case 6: //FacesPhoneme.Add("U");      6 :4
                if(v==4)  SetPixel(x,y,hsv);
                break;

            case 7: //FacesPhoneme.Add("WQ");     7 : 5
                if(v==5)  SetPixel(x,y,hsv);
                break;

            case 8: //FacesPhoneme.Add("etc");    8 : 5
                if(v==5)  SetPixel(x,y,hsv);
                break;

            case 9: //FacesPhoneme.Add("rest");   9 : 6
                if(v==6)  SetPixel(x,y,hsv);
                break;
            }
        }
    }

#if 0

    for (int y=0; y<BufferHt; y++)
    {
        for (int x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            v=face[x][y];
            if(v == 1)
                SetPixel(x,y,hsv);
        }
    }
#endif

    /*
    for (int y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (int x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            if(Phoneme==5 || Phoneme==6) // O, U
            {
                if(x==19 && y==15)  SetPixel(x,y,hsv);
            }
            if(Phoneme==0) // AI
            {
                if(x==6 && y==28)
                    SetPixel(x,y,hsv);  // mouth 2
                if(x==11 && y==18)
                    SetPixel(x,y,hsv);  // mouth 3
            }
            if(Phoneme==1 || Phoneme==2 || Phoneme==3 || Phoneme==4 || Phoneme==9) // E,FV,L,MBP,rest
            {
                if(x==19 && y==13)  SetPixel(x,y,hsv); // Mouth 6
            }
            if(Phoneme==7 || Phoneme==8) // etc, WQ
            {
                if(x==19 && y==10)  SetPixel(x,y,hsv);
            }
            //        if (!color.GetRGB()) continue; //color == BLACK) continue; //pixel is off
        }
    }
    */
}

void RgbEffects::mouth(int Phoneme,int BufferHt, int BufferWi)
{
    /*
    FacesPhoneme.Add("AI");     0
    FacesPhoneme.Add("E");      1
    FacesPhoneme.Add("FV");     2
    FacesPhoneme.Add("L");      3
    FacesPhoneme.Add("MBP");    4
    FacesPhoneme.Add("O");      5
    FacesPhoneme.Add("U");      6
    FacesPhoneme.Add("WQ");     7
    FacesPhoneme.Add("etc");    8
    FacesPhoneme.Add("rest");   9

    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    -1-------------+-------------1-         5% and 95% in x, 52% in y
    .1.............|.............1.         55% in y
    .11111111111111111111111111111.         58% in y
    .2..........55.|.55..........2.
    .2........55...|...55........2.
    .2......55.....|.....55......2.
    .2....55.......|.......55....2.
    .2..55.........|.........55..2.
    .255...........|...........552.
    .2.............|.............2.      y 81%
    .2.............|.............2.
    .255...........|...........552.
    .2..55.........|.........55..2.
    .2....55.......|.......55....2.
    .2......55.....|.....55......2.
    .2........55...|...55........2.
    ............55.|.55............
    */

    int Wt, Ht, x1,x2,x3,x4,y1,y2,y3,y4,y5,y6,y7;
    int xc,yc;
    double radius,offset=0.0;
    Ht = BufferHt-1;
    Wt = BufferWi-1;
    x1=(int)(offset + Wt*0.25);
    x2=(int)(offset + Wt*0.75);
    x3=(int)(offset + Wt*0.30);
    x4=(int)(offset + Wt*0.70);

    y1=(int)(offset + Ht*0.48);
    y2=(int)(offset + Ht*0.40);
    y3=(int)(offset + Ht*0.25);
    y4=(int)(offset + Ht*0.20);
    y5=(int)(offset + Ht*0.30);
    y6=(int)(offset + Ht*0.20);
    y7=(int)(offset + Ht*0.30);

    // eyes


    drawoutline(Phoneme,BufferHt,BufferWi);

    switch (Phoneme)
    {
    case 0:         // AI

        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y4);
        break;
    case 3:
    case 1:       // E, L
        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y3);
        break;
    case 2:       // FV
        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y2-1);
        break;
    case 4:
    case 9:     //  MBP,rest

        drawline1( Phoneme, x1, x2, y1,y2);
        break;
    case 5:
    case 6:       // O,U,WQ
    case 7:
        xc = (int)(0.5 + Wt*0.50);
        yc = (int) (y2-y5)/2 + y5;
        radius = Wt*0.15;  // O
        if(Phoneme==6) radius = Wt*0.10;  // U
        if(Phoneme==7) radius = Wt*0.05;  // WQ
        facesCircle(Phoneme,xc, yc, radius,0,360);
        break;
    case 8:       // WQ, etc
        drawline3( Phoneme, x3, x4, y5, y2);
        break;

    }
}

void RgbEffects::drawline1(int Phoneme, int x1,int x2,int y1,int y2)
{
    wxColour color;
    wxImage::HSVValue hsv;
    int ColorIdx,x=0,y=0;
    size_t colorcnt=GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
    hsv.hue = (float)Phoneme/10.0;
    int ch;
//    ch=FindChannelAtXY( x,  y,'A');
    hsv.value=1.0;


    for(x=x1+1; x<x2; x++)
    {
        SetPixel(x,y2,hsv); // Turn pixel on
    }
    for(y=y2+1; y<=y1; y++)
    {
        SetPixel(x1,y,hsv); // Left side of mouyh
        SetPixel(x2,y,hsv); // rightside
    }
}




void RgbEffects::drawline3 (int Phoneme, int x1,int x2,int y6,int y7)
{
    wxColour color;
    wxImage::HSVValue hsv;

    int ColorIdx,x,y;
    size_t colorcnt=GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;

    for(y=y6+1; y<y7; y++)
    {
        SetPixel(x1,y,hsv); // Left side of mouyh
        SetPixel(x2,y,hsv); // rightside
    }
    for(x=x1+1; x<x2; x++)
    {
        SetPixel(x,y6,hsv); // Bottom
        SetPixel(x,y7,hsv); // Bottom
    }
    /*
    dy = (y7-y6)/2;
    xc = x1+dy;
    yc = y7+dy;
    radius = (double) dy;
    facesCircle(Phoneme,xc, yc, radius,90,270);
    xc = x1-dy;
    facesCircle(Phoneme,xc, yc, radius,270,630);
    */

}
/*
faces draw circle
*/
void RgbEffects::facesCircle(int Phoneme, int xc,int yc,double radius,int start_degrees, int end_degrees)
{
    int x,y,degrees;
    wxImage::HSVValue hsv;
    palette.GetHSV(0, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;
    double t,PI=3.1415926;
    for(degrees=start_degrees; degrees<end_degrees; degrees+=1)
    {
        t = degrees * (PI/180);
        x = (int)xc+radius*cos(t);
        y = (int)yc+radius*sin(t);
        SetPixel(x,y,hsv); // Bottom
    }
}

void RgbEffects::drawoutline(int Phoneme,int BufferHt,int BufferWi)
{
    wxColour color;
    wxImage::HSVValue hsv;
    double radius;
    int ColorIdx,x,y,xc,yc;
    int Ht, Wt;
    size_t colorcnt=GetColorCount();
    Ht = BufferHt-1;
    Wt = BufferWi-1;


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue = 0.0;
    hsv.saturation = 1.0;
    hsv.value=1.0;

//  DRAW EYES
    int start_degrees=0;
    int end_degrees=360;
    if(Phoneme==5 || Phoneme==6 || Phoneme==7)
    {
        start_degrees=180;
        end_degrees=360;
    }
    xc = (int)(0.5 + Wt*0.33); // left eye
    yc = (int)(0.5 + Ht*0.75);
    radius = Wt*0.08;
    facesCircle( Phoneme,xc, yc, radius,start_degrees,end_degrees);
    xc = (int)(0.5 + Wt*0.66); // right eye
    yc = (int)(0.5 + Ht*0.75);
    radius = Wt*0.08;
    facesCircle(Phoneme,xc, yc, radius,start_degrees,end_degrees);


    /*
    ...********...
    ..*
    .*
    *
    *
    *
    */

    for(y=3; y<BufferHt-3; y++)
    {
        SetPixel(0,y,hsv); // Left side of mouyh
        SetPixel(BufferWi-1,y,hsv); // rightside
    }
    for(x=3; x<BufferWi-3; x++)
    {
        SetPixel(x,0,hsv); // Bottom
        SetPixel(x,BufferHt-1,hsv); // Bottom
    }
    SetPixel(2,1,hsv); // Bottom left
    SetPixel(1,2,hsv); //

    SetPixel(BufferWi-3,1,hsv); // Bottom Right
    SetPixel(BufferWi-2,2,hsv); //

    SetPixel(BufferWi-3,BufferHt-2,hsv); // Bottom Right
    SetPixel(BufferWi-2,BufferHt-3,hsv); //

    SetPixel(2,BufferHt-2,hsv); // Bottom Right
    SetPixel(1,BufferHt-3,hsv); //
}

/*
void ModelClass::InitCustomMatrix(const wxString& customModel)
{
    wxString value;
    wxArrayString cols;
    long idx;
    int width=1;
    std::vector<int> nodemap;

    wxArrayString rows=wxSplit(customModel,';');
    int height=rows.size();
    int cpn = ChannelsPerNode();
    for(size_t row=0; row < rows.size(); row++)
    {
        cols=wxSplit(rows[row],',');
        if (cols.size() > width) width=cols.size();
        for(size_t col=0; col < cols.size(); col++)
        {
            value=cols[col];
            if (!value.IsEmpty() && value != "0")
            {
                value.ToLong(&idx);

                // increase nodemap size if necessary
                if (idx > nodemap.size()) {
                    nodemap.resize(idx, -1);
                }
                idx--;  // adjust to 0-based

                // is node already defined in map?
                if (nodemap[idx] < 0) {
                    // unmapped - so add a node
                    nodemap[idx]=Nodes.size();
                    SetNodeCount(1,0);  // this creates a node of the correct class
                    Nodes.back()->StringNum= SingleNode ? idx : 0;
                    Nodes.back()->ActChan=stringStartChan[0] + idx * cpn;
                    Nodes.back()->AddBufCoord(col,height - row - 1);
                } else {
                    // mapped - so add a coord to existing node
                    Nodes[nodemap[idx]]->AddBufCoord(col,height - row - 1);
                }

            }
        }
    }
    SetBufferSize(height,width);
}
*/
