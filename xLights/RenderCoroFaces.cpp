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

#ifdef _MSC_VER
 #include <hash_map>
 #define unordered_map  hash_map //not c++ 2011 compat?
#else
 #include <unordered_map>
#endif


//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG  -99 //unbuffered in case app crashes
//#include "djdebug.cpp"
#ifndef debug_function //dummy defs if debug cpp not included above
 #define debug(level, ...)
 #define debug_more(level, ...)
 #define debug_function(level)
#endif



//TODO: move this to a shared location:
static wxString NoInactive(wxString name)
{
    const wxString InactiveIndicator = "?";
    return name.StartsWith(InactiveIndicator)? name.substr(InactiveIndicator.size()): name;
}

//static const char* parts[] = {"Outline", "AI", "E", "etc", "FV", "L", "MBP", "O", "rest", "U", "WQ", "Open", "Closed", "Left", "Right", "Up", "Down"};

//cached model info:
static std::unordered_map<std::string, std::unordered_map<std::string, /*wxPoint*/ std::string> > model_xy;
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
    for (int compat = 0; compat < 2; ++compat)
    {
        wxXmlNode* Presets = xLightsFrame::FindNode(pgoXml.GetRoot(), compat? wxT("corofaces"): wxT("presets"), wxT("Name"), wxEmptyString, false); //kludge: backwards compatible with current settings
        if (!Presets) continue; //should be there if seq was generated in this folder
//    wxString buf;
//group name is not available, so use first occurrence of model in *any* group:
//NOTE: assumes phoneme/face mapping is consistent for any given model across groups, which should be the case since the lights don't move
        for (wxXmlNode* group = Presets->GetChildren(); group != NULL; group = group->GetNext())
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
                debug(10, "model '%s': using xml info '%s'", (const char*)want_model.c_str(), (const char*)voice->GetContent().c_str());
                for (wxXmlAttribute* attrp = voice->GetAttributes(); attrp; attrp = attrp->GetNext())
                {
                    wxString value = attrp->GetValue();
                    if (!value.empty()) map[(const char*)attrp->GetName().c_str()] = (const char*)value.c_str();
                    debug(10, "has attr '%s' = '%s'", (const char*)attrp->GetName().c_str(), (const char*)attrp->GetValue().c_str());
                }
                return true;
            }
        }
    }
#endif
    return false;
}

//NOTE: params are re-purposed as follows for Coro face mode:
// x_y = list of active elements for this frame
// Outline_x_y = list of persistent/sticky elements (stays on after frame ends)
// Eyes_x_y = list of random elements (intended for eye blinks, etc)

void RgbEffects::RenderCoroFacesFromPGO(const wxString& Phoneme, const wxString& eyes, bool face_outline)
{
//    const wxString& parsed_xy = IsParsed(x_y)? x_y: wxEmptyString;
//NOTE:
//PixelBufferClass contains 2 RgbEffects members, which this method is a member of
//xLightsFrame contains a PixelBufferClass member named buffer, which is derived from ModelClass and gives the name of the model currently being used
//therefore we can access the model info by going to parent object's buffer member
//    wxString model_name = "???";
    if (!curPeriod) model_xy.clear(); //flush cache once at start of each effect

    static std::unordered_map<std::string, int> auto_phonemes {{"a-AI", 0}, {"a-E", 1}, {"a-FV", 2}, {"a-L", 3},
              {"a-MBP", 4}, {"a-O", 5}, {"a-U", 6}, {"a-WQ", 7}, {"a-etc", 8}, {"a-rest", 9}};
//    static const char* auto_eyes[] = {"Open", "Closed", "Left", "Right", "Up", "Down"};

    if (auto_phonemes.find((const char*)Phoneme.c_str()) != auto_phonemes.end())
    {
        RenderFaces(auto_phonemes[(const char*)Phoneme.c_str()]); //TODO: add params for eyes, outline
        return;
    }

    wxImage::HSVValue hsv;
    
        xlColor color;
        palette.GetColor(0, color); //use first color; user must make sure it matches model node type
        color = xlWHITE; //kludge: must use WHITE to get single-color nodes to show correctly
        Color2HSV(color, hsv);

        std::vector<wxPoint> first_xy;
        ModelClass* model_info = xLightsFrame::AllModels[cur_model].get();
        if (!model_info || !parse_model(cur_model))
        {
            return;
        }
        std::unordered_map<std::string, std::string>& map = model_xy[(const char*)cur_model.c_str()];
        if (Phoneme == wxT("(test)"))
        {
            /*static*/ wxString info = eyes;
            ModelClass::ParseFaceElement(info, first_xy);
        }
        if (!Phoneme.empty())
        {
            wxString info = map[(const char*)Phoneme.c_str()];
            ModelClass::ParseFaceElement(info, first_xy);
        }
        if (!eyes.empty())
        {
            wxString info = map[(const char*)wxString::Format(wxT("Eyes_%s"), eyes.Lower()).c_str()];
            ModelClass::ParseFaceElement(info, first_xy);
        }
        if (face_outline)
        {
            wxString info = map["Outline"];
            ModelClass::ParseFaceElement(info, first_xy);
        }
        for (auto it = first_xy.begin(); it != first_xy.end(); ++it)
        {
            --(*it).x; // "A" = 1 = first col
            SetPixel((*it).x, BufferHt - (*it).y, hsv); //only need to turn on first pixel for each face part
        }
}


void RgbEffects::RenderCoroFaces(const wxString& Phoneme, Element *track, const wxString& eyes, bool face_outline)
{
    wxString phoneme = Phoneme;
    if (phoneme == "") {
        //GET Phoneme from timing track
        if (track == nullptr || track->GetEffectLayerCount() < 3) {
            phoneme = "rest";
        } else {
            EffectLayer *layer = track->GetEffectLayer(2);
            int idx = 0;
            int time = curPeriod * frameTimeInMs + 1;
            if (layer->HitTestEffectByTime(time, idx)) {
                phoneme = layer->GetEffect(idx)->GetEffectName();
            } else {
                phoneme = "rest";
            }
        }
    }
    ModelClass* model_info = xLightsFrame::AllModels[cur_model].get();

    xlColor color;
    palette.GetColor(0, color); //use first color; user must make sure it matches model node type
    
    wxArrayString todo;
    todo.push_back("Mouth-" + phoneme);
    if (face_outline) {
        todo.push_back("FaceOutline");
    }
    if (eyes == "Open") {
        todo.push_back("Eyes-Open");
    }
    if (eyes == "Closed") {
        todo.push_back("Eyes-Closed");
    }
    if (eyes == "(off)") {
        //no eyes
    }
    
    for (int x = 0; x < todo.size(); x++) {
        wxString channels = model_info->faceInfo[todo[x]];
        wxStringTokenizer wtkz(channels, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();
            for (int x = 0; x < model_info->GetNodeCount(); x++) {
                if (model_info->GetNodeName(x) == valstr) {
                    std::vector<wxPoint> pts;
                    model_info->GetNodeCoords(x, pts);
                    for (int x = 0; x < pts.size(); x++) {
                        SetPixel(pts[x].x, pts[x].y, color);
                    }
                }
            }
        }
    }
}

