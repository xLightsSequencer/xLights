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
#include <wx/tokenzr.h>


#define WANT_DEBUG_IMPL
#define WANT_DEBUG  99
//#include "djdebug.cpp"

#if 0 //obsolete
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
#endif // 0


//NOTE: params are re-purposed as follows for Coro face mode:
// x_y = list of active elements for this frame
// Outline_x_y = list of persistent/sticky elements (stays on after frame ends)
// Eyes_x_y = list of random elements (intended for eye blinks, etc)
void RgbEffects::RenderCoroFaces(int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y)
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
//    ModelClass mc;
//    mc.GetChannelCoords(chmap, true); //method is on ModelClass object



    wxString html = "<html><body><table border=0>";
    int Ht, Wt;
    Ht = BufferHt;
    Wt = BufferWi;

        coroface( Phoneme, x_y, Outline_x_y, Eyes_x_y); // draw a mouth syllable



//size_t NodeCount=GetNodeCount();

//    above is from ModelClass::ChannelLayoutHtml()
#if 0 //sample code for Sean
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

void RgbEffects::coroface(int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y)
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


    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();
    int ColorIdx=0;
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue=0.0;
    hsv.value=1.0;
    hsv.saturation=1.0;
    int i,j,indx,x[10],y[10],v;
    int x_Outline,y_Outline,x_Eyes,y_Eyes;
    wxString model,s_Phoneme; //set to target model name (optional)
    switch (Phoneme)
    {

    case 0 :
        s_Phoneme="AI";
        break;

    case 1 :
        s_Phoneme="E";
        break;

    case 2 :
        s_Phoneme="FV";
        break;

    case 3 :
        s_Phoneme="L";
        break;

    case 4 :
        s_Phoneme="MBP";
        break;

    case 5 :
        s_Phoneme="O";
        break;

    case 6 :
        s_Phoneme="U";
        break;

    case 7 :
        s_Phoneme="WQ";
        break;

    case 8 :
        s_Phoneme="etc";
        break;

    case 9 :
        s_Phoneme="rest";
        break;
    }

    //  Now lests parse the coordinates for the Mouth, face outline and the eyes
    /*  we can have more than one pair of numbers
    E1_TEXTCTRL_X_Y=5:28:4:28,
    E1_TEXTCTRL_Outline_X_Y=15:51,
    E1_TEXTCTRL_Eyes_X_Y=10:44
    */
    wxStringTokenizer tkz(x_y, ":");
    i=0;
//    x=y=-1;
    while ( tkz.HasMoreTokens() )
    {
        i++;
        wxString token = tkz.GetNextToken();
        // process token here

        indx=(i-1)/2; // counter to count up by 2's
        if(i==1) x[indx]=wxAtoi(token);
        if(i==2) y[indx]=wxAtoi(token);
    }

    wxStringTokenizer tkz_Outline(Outline_x_y, ":");
    i=0;
    x_Outline=y_Outline=-1;
    while ( tkz_Outline.HasMoreTokens() )
    {
        i++;
        wxString token = tkz_Outline.GetNextToken();
        // process token here
        if(i==1) x_Outline=wxAtoi(token);
        if(i==2) y_Outline=wxAtoi(token);
    }


    wxStringTokenizer tkz_Eyes(Eyes_x_y, ":");
    i=0;
    x_Eyes=y_Eyes=-1;
    while ( tkz_Eyes.HasMoreTokens() )
    {
        i++;
        wxString token = tkz_Eyes.GetNextToken();
        // process token here
        if(i==1) x_Eyes=wxAtoi(token);
        if(i==2) y_Eyes=wxAtoi(token);
    }

    for(j=0; j<=indx; j++)
    {
        if(x[j]>=0 && x[j]<BufferWi && y[j]>=0 && y[j]<=BufferHt)  SetPixel(x[j],y[j],hsv);
    }
    if(x_Outline>=0 && x_Outline<BufferWi && y_Outline>=0 && y_Outline<=BufferHt)  SetPixel(x_Outline,y_Outline,hsv);
    if(x_Eyes>=0 && x_Eyes<BufferWi && y_Eyes>=0 && y_Eyes<=BufferHt)  SetPixel(x_Eyes,y_Eyes,hsv);

}

