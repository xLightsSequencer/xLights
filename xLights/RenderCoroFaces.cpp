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

    static std::unordered_map<std::string, wxString> auto_phonemes {{"a-AI", "AI"}, {"a-E", "E"}, {"a-FV", "FV"}, {"a-L", "L"},
              {"a-MBP", "MBP"}, {"a-O", "O"}, {"a-U", "U"}, {"a-WQ", "WQ"}, {"a-etc", "etc"}, {"a-rest", "rest"}};
//    static const char* auto_eyes[] = {"Open", "Closed", "Left", "Right", "Up", "Down"};

    if (auto_phonemes.find((const char*)Phoneme.c_str()) != auto_phonemes.end())
    {
        RenderFaces(auto_phonemes[(const char*)Phoneme.c_str()], eyes, face_outline);
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


void RgbEffects::RenderFaces(SequenceElements *elements, const wxString &faceDefinition,
                             const wxString& Phoneme, const wxString &trackName, const wxString& eyes, bool face_outline)
{
    
    if (needToInit) {
        needToInit = false;
        elements->AddRenderDependency(trackName, cur_model);
    }

    Element *track = elements->GetElement(trackName);
    wxMutex tmpLock;
    wxMutex *lock = &tmpLock;
    if (track != nullptr) {
        lock = &track->GetRenderLock();
    }
    wxMutexLocker locker(*lock);
    
    wxString phoneme = Phoneme;
    if (phoneme == "") {
        //GET Phoneme from timing track
        if (track == nullptr || track->GetEffectLayerCount() < 3) {
            phoneme = "rest";
        } else {
            EffectLayer *layer = track->GetEffectLayer(2);
            wxMutexLocker locker(layer->GetLock());
            int time = curPeriod * frameTimeInMs + 1;
            Effect *ef = layer->GetEffectByTime(time);
            if (ef == nullptr) {
                phoneme = "rest";
            } else {
                phoneme = ef->GetEffectName();
            }
        }
    }
    ModelClass* model_info = xLightsFrame::AllModels[cur_model].get();

    xlColor color;
    palette.GetColor(0, color); //use first color for mouth; user must make sure it matches model node type
    
    wxArrayString todo;
    std::vector<xlColor> colors;
    todo.push_back("Mouth-" + phoneme);
    colors.push_back(color);
    if (palette.Size() > 1) {
        palette.GetColor(1, color); //use second color for eyes; user must make sure it matches model node type
    }
    if (eyes == "Open" || eyes == "Auto") {
        todo.push_back("Eyes-Open");
        colors.push_back(color);
    }
    if (eyes == "Closed") {
        todo.push_back("Eyes-Closed");
        colors.push_back(color);
    }
    if (eyes == "(off)") {
        //no eyes
    }
    if (palette.Size() > 2) {
        palette.GetColor(2, color); //use third color for outline; user must make sure it matches model node type
    }
    if (face_outline) {
        todo.push_back("FaceOutline");
        colors.push_back(color);
    }
    wxString definition = faceDefinition;
    if (definition == "Default" && !model_info->faceInfo.empty()) {
        definition = model_info->faceInfo.begin()->first;
    }
    bool found = true;
    std::map<wxString, std::map<wxString, wxString> >::iterator it = model_info->faceInfo.find(definition);
    if (it == model_info->faceInfo.end()) {
        //not found
        found = false;
    }
    
    int type = 3;
    if ("Coro" == definition || "SingleNode" == definition) {
        type = 0;
    } else if ("NodeRange" == definition) {
        type = 1;
    } else if ("Rendered" == definition || "Default" == definition) {
        type = 2;
    }
    
    
    if (type == 2) {
        RenderFaces(phoneme, eyes, face_outline);
        return;
    }
    if (type == 3) {
        //picture
        wxString e = eyes;
        if (eyes == "Auto") {
            e = "Open";
        }
        if (eyes == "(off)") {
            e = "Closed";
        }
        wxString key = "Mouth-" + phoneme + "-Eyes";
        wxString picture = model_info->faceInfo[definition][key + e];
        if (picture == "" && e == "Closed") {
            picture = model_info->faceInfo[definition][key + "Open"];
        }
        int i = 9; /*RENDER_PICTURE_SCALED*/
        if (model_info->faceInfo[definition]["ImagePlacement"] == "Centered") {
            i = 4; /*RENDER_PICTURE_NONE */
        }
        RenderPictures(i, picture, 0, 0, 0, 0, 0, 0, 0, false);
    }
    for (int t = 0; t < todo.size(); t++) {
        wxString channels = model_info->faceInfo[definition][todo[t]];
        wxStringTokenizer wtkz(channels, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();
            
            if (type == 0) {
                for (int n = 0; n < model_info->GetNodeCount(); n++) {
                    wxString nn = model_info->GetNodeName(n, true);
                    if (nn == valstr) {
                        std::vector<wxPoint> pts;
                        model_info->GetNodeCoords(n, pts);
                        for (int x = 0; x < pts.size(); x++) {
                            SetPixel(pts[x].x, pts[x].y, colors[t]);
                        }
                    }
                }
            } else if (type == 1) {
                int start, end;
                if (valstr.Contains("-")) {
                    int idx = valstr.Index('-');
                    start = wxAtoi(valstr.Left(idx));
                    end = wxAtoi(valstr.Right(valstr.size() - idx - 1));
                } else {
                    start = end = wxAtoi(valstr);
                }
                if (start > end) {
                    start = end;
                }
                start--;
                end--;
                for (int n = start; n <= end; n++) {
                    std::vector<wxPoint> pts;
                    if (n < model_info->GetNodeCount()) {
                        model_info->GetNodeCoords(n, pts);
                        for (int x = 0; x < pts.size(); x++) {
                            SetPixel(pts[x].x, pts[x].y, colors[t]);
                        }
                    }
                }
            }
        }
    }
}

