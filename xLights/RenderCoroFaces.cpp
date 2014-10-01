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
//#include <wx/checklst.h>
#include <wx/xml/xml.h>


//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG  -99 //unbuffered in case app crashes
//#include "djdebug.cpp"
#ifndef debug_function //dummy defs if debug cpp not included above
 #define debug(level, ...)
 #define debug_more(level, ...)
 #define debug_function(level)
#endif


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


#if 0
static wxString prev_model;
static void get_elements(wxCheckListBox* listbox, const wxString& model)
{
    listbox->Clear();
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if ((*it)->name != model) continue;
        if ((*it)->GetChannelCoords(listbox))
        {
            prev_model = model;
            return;
        }
    }
//also list non-preview models:
    for (auto it = xLightsFrame::OtherModels.begin(); it != xLightsFrame::OtherModels.end(); ++it)
    {
        if ((*it)->name != model) continue;
        if ((*it)->GetChannelCoords(listbox))
        {
            prev_model = model;
            return;
        }
    }
}
#endif // 0

//static bool IsParsed(const wxString& settings)
//{
//    return settings.Find('@') != wxNOT_FOUND;
//}

//TODO: move this to a shared location:
static wxString NoInactive(wxString name)
{
    const wxString InactiveIndicator = "?";
    return name.StartsWith(InactiveIndicator)? name.substr(InactiveIndicator.size()): name;
}

//static const char* parts[] = {"Outline", "AI", "E", "etc", "FV", "L", "MBP", "O", "rest", "U", "WQ", "Open", "Closed", "Left", "Right", "Up", "Down"};

//cached model info:
static std::unordered_map<std::string, std::unordered_map<std::string, /*wxPoint*/ std::string>> model_xy;
//static std::unordered_map<std::string, wxXmlNode*> model_xy; //since (X,Y) info is already in settings file, just re-use it

static bool parse_model(const wxString& want_model)
{
    if (model_xy.find((const char*)want_model.c_str()) != model_xy.end()) return true; //already have info
//    std::unordered_map<std::string, wxPoint>& xy_info = model_xy[model];

#if 1
    wxFileName pgoFile;
    wxXmlDocument pgoXml;
    pgoFile.AssignDir(xLightsFrame::CurrentDir);
    pgoFile.SetFullName(_(XLIGHTS_PGOFACES_FILE));
    if (!pgoFile.FileExists()) return false;
    if (!pgoXml.Load(pgoFile.GetFullPath())) return false;
    wxXmlNode* root = pgoXml.GetRoot();
    if (!root || (root->GetName() != "papagayo")) return false;
    wxXmlNode* CoroFaces = xLightsFrame::FindNode(pgoXml.GetRoot(), wxT("corofaces"), wxT("name"), wxEmptyString);
    if (!CoroFaces) return false;
//    wxString buf;
    for (wxXmlNode* group = CoroFaces->GetChildren(); group != NULL; group = group->GetNext())
    {
        wxString grpname = group->GetAttribute(wxT("name"));
        debug(15, "found %s group '%s'", (const char*)group->GetName().c_str(), (const char*)group->GetAttribute(wxT("name"), wxT("??")).c_str());
//        if (group->GetName() != "coro") continue;
//        if (grpname.IsEmpty()) continue;
//        wxXmlNode* voice = FindNode(group, "voice", wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1), true);
        for (wxXmlNode* voice = group->GetChildren(); voice != NULL; voice = voice->GetNext())
        {
            wxString voice_name = NoInactive(voice->GetAttribute(wxT("name")));
            debug(10, "found voice name '%s' vs. '%s'", (const char*)voice_name.c_str(), (const char*)want_model.c_str());
            if (voice_name != want_model) continue;
//            model_xy[(const char*)want_model.c_str()] = voice;
//XmlNode getting trashed later, so save it here
            std::unordered_map<std::string, std::string>& map = model_xy[(const char*)want_model.c_str()];
            map.clear();
            debug(10, "using xml info '%s'", (const char*)voice->GetContent().c_str());
            for (wxXmlAttribute* attrp = voice->GetAttributes(); attrp; attrp = attrp->GetNext())
            {
                wxString value = attrp->GetValue();
                if (!value.empty()) map[(const char*)attrp->GetName().c_str()] = (const char*)value.c_str();
                debug(10, "has attr '%s' = '%s'", (const char*)attrp->GetName().c_str(), (const char*)attrp->GetValue().c_str());
            }
            return true;
        }
    }
#endif
    debug(10, "model '%s' not found", (const char*)want_model.c_str());
    return false; //not found
}

//NOTE: params are re-purposed as follows for Coro face mode:
// x_y = list of active elements for this frame
// Outline_x_y = list of persistent/sticky elements (stays on after frame ends)
// Eyes_x_y = list of random elements (intended for eye blinks, etc)
//void RgbEffects::RenderCoroFaces(int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y/*, const wxString& parsed_xy*/)
void RgbEffects::RenderCoroFaces(const wxString& Phoneme, const wxString& eyes, bool face_outline)
{
//    const wxString& parsed_xy = IsParsed(x_y)? x_y: wxEmptyString;
//NOTE:
//PixelBufferClass contains 2 RgbEffects members, which this method is a member of
//xLightsFrame contains a PixelBufferClass member named buffer, which is derived from ModelClass and gives the name of the model currently being used
//therefore we can access the model info by going to parent object's buffer member
//    wxString model_name = "???";
    if (!state) model_xy.clear(); //flush cache at start
    debug(10, "RenderCoroFaces: state %d, model '%s', mouth/phoneme '%s', eyes '%s', face outline? %d", state, (const char*)cur_model.c_str(), (const char*)Phoneme.c_str(), (const char*)eyes.c_str(), face_outline);
//    if (prev_model != cur_model) get_elements(CheckListBox_CoroFaceElements, curmodel); //update choice list

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
//    std::vector<std::vector<int>> chmap; //array of arrays
//    chmap.resize(BufferHt * BufferWi,0);
//    ModelClass mc;
//    mc.GetChannelCoords(chmap, true); //method is on ModelClass object


//    if (IsParsed(x_y)) //already have (X,Y) info
//    {
//TODO: how is color palette supposed to work with Coro faces?
//        wxImage::HSVValue hsv;
//        size_t colorcnt=GetColorCount();
//        int ColorIdx=0;
//        palette.GetHSV(ColorIdx, hsv);
        hsv.hue=0.0;
        hsv.value=1.0;
        hsv.saturation=1.0;

        wxPoint first_xy;
        ModelClass* model_info = ModelClass::FindModel(cur_model);
        if (!model_info || !parse_model(cur_model))
        {
            debug(10, "model '%s' not found", (const char*)cur_model.c_str());
            return;
        }
//        wxXmlNode* xy_info = model_xy[(const char*)cur_model.c_str()];
        std::unordered_map<std::string, std::string>& map = model_xy[(const char*)cur_model.c_str()];
        if (!Phoneme.empty())
        {
            wxString info = map[(const char*)Phoneme.c_str()];
//            if (xy_info) info = xy_info->GetAttribute(Phoneme);
            bool ok = ModelClass::ParseFaceElement(info, &first_xy);
            if (ok) SetPixel(first_xy.x, BufferHt - first_xy.y, hsv); //only need to turn on first pixel for each face part
            debug(10, "model '%s', phoneme '%s', parsed info '%s', turn on (x %d, y %d)? %d", (const char*)cur_model.c_str(), (const char*)Phoneme.c_str(), (const char*)info.c_str(), first_xy.x, first_xy.y, ok);
        }
        if (!eyes.empty())
        {
            wxString info = map[(const char*)eyes.c_str()];
//            if (xy_info) info = xy_info->GetAttribute(eyes);
            bool ok = ModelClass::ParseFaceElement(info, &first_xy);
            if (ok) SetPixel(first_xy.x, BufferHt - first_xy.y, hsv); //only need to turn on first pixel for each face part
            debug(10, "model '%s', eyes '%s', parsed info '%s', turn on (x %d, y %d)? %d", (const char*)cur_model.c_str(), (const char*)eyes.c_str(), (const char*)info.c_str(), first_xy.x, first_xy.y, ok);
        }
        if (face_outline)
        {
            wxString info = map["Outline"];
//            if (xy_info) info = xy_info->GetAttribute("Outline");
            bool ok = ModelClass::ParseFaceElement(info, &first_xy);
            if (ok) SetPixel(first_xy.x, BufferHt - first_xy.y, hsv); //only need to turn on first pixel for each face part
            debug(10, "model '%s', outline, parsed info '%s', turn on (x %d, y %d)? %d", (const char*)cur_model.c_str(), (const char*)info.c_str(), first_xy.x, first_xy.y, ok);
        }

#if 0 //obsolete
        wxStringTokenizer wtkz(x_y, "+");
        while (wtkz.HasMoreTokens())
        {
            wxString nextstr = wtkz.GetNextToken();
            wxPoint first_xy;
            bool ok = ModelClass::ParseFaceElement(nextstr, &first_xy);
            first_xy.y = BufferHt - first_xy.y; //y is reversed?
            debug(10, "coro faces: turn on '%s'? %d, xy (%d, %x)", (const char*)nextstr.c_str(), ok, first_xy.x, first_xy.y);
            if (!ok) continue;
            SetPixel(first_xy.x, first_xy.y, hsv); //only need to turn on first pixel for each face part
        }
        return;
    }
#endif // 0

//    wxString html = "<html><body><table border=0>";
//    int Ht, Wt;
//    Ht = BufferHt;
//    Wt = BufferWi;

//        coroface( Phoneme, x_y, Outline_x_y, Eyes_x_y); // draw a mouth syllable



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

#if 0 //obsolete
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
#endif //0
