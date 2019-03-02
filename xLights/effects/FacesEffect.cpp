#include <list>

#include "FacesEffect.h"
#include "FacesPanel.h"
#include "../models/Model.h"
#include "../models/SubModel.h"
#include "../sequencer/SequenceElements.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../xLightsMain.h" 
#include "PicturesEffect.h"

#include <wx/tokenzr.h>

#include "../../include/corofaces.xpm"

#include <log4cpp/Category.hh>

#define PI 3.1415926

class FacesRenderCache : public EffectRenderCache {
    std::map<std::string, RenderBuffer*> _imageCache;
public:
    int blinkEndTime;
    int nextBlinkTime;
    std::map<std::string, int> nodeNameCache;

    FacesRenderCache() : blinkEndTime(0), nextBlinkTime(0) {
    }
    virtual ~FacesRenderCache() {
        for (auto it = _imageCache.begin(); it != _imageCache.end(); ++it)
        {
            delete it->second;;
        }
    }
    void Clear() {
        nodeNameCache.clear();
    }
    RenderBuffer* GetImage(std::string key)
    {
        if (_imageCache.find(key) != _imageCache.end())
        {
            return _imageCache[key];
        }

        return nullptr;
    }
    void AddImage(std::string key, RenderBuffer* crb)
    {
        _imageCache[key] = crb;
    }
};

FacesEffect::FacesEffect(int id) : RenderableEffect(id, "Faces", corofaces, corofaces, corofaces, corofaces, corofaces)
{
    //ctor
}

FacesEffect::~FacesEffect()
{
    //dtor
}

std::list<std::string> FacesEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    wxString definition = settings.Get("E_CHOICE_Faces_FaceDefinition", "");
    if (definition == "Default" && !model->faceInfo.empty() && model->faceInfo.begin()->first != "") {
        definition = model->faceInfo.begin()->first;
    }
    bool found = true;
    std::map<std::string, std::map<std::string, std::string> >::iterator it = model->faceInfo.find(definition.ToStdString());
    if (it == model->faceInfo.end()) {
        //not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model->faceInfo.find("SingleNode") != model->faceInfo.end()) {
            definition = "SingleNode";
            found = true;
        }
        else if ("SingleNode" == definition && model->faceInfo.find("Coro") != model->faceInfo.end()) {
            definition = "Coro";
            found = true;
        }
    }

    wxString modelType = found ? wxString(model->faceInfo[definition.ToStdString()]["Type"].c_str()) : definition;
    if (modelType == "") {
        modelType = definition;
    }

    if (modelType != "Matrix" && modelType != "Rendered")
    {
        // -Buffer not rotated
        wxString bufferTransform = settings.Get("B_CHOICE_BufferTransform", "None");

        if (bufferTransform != "None")
        {
            res.push_back(wxString::Format("    WARN: Face effect with transformed buffer '%s' may not render correctly. Model '%s', Start %s", model->GetName(), bufferTransform, FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        if (settings.GetInt("B_SLIDER_Rotation", 0) != 0 || 
            settings.GetInt("B_SLIDER_Rotations", 0) != 0 ||
            settings.GetInt("B_SLIDER_XRotation", 0) != 0 ||
            settings.GetInt("B_SLIDER_YRotation", 0) != 0 ||
            settings.GetInt("B_SLIDER_Zoom",1) != 1 || 
            settings.Get("B_VALUECURVE_Rotation", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_XRotation", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_YRotation", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_Rotations", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_Zoom", "").find("Active=TRUE") != std::string::npos
            )
        {
            res.push_back(wxString::Format("    WARN: Face effect with rotozoom active may not render correctly. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        if (settings.Get("B_CUSTOM_SubBuffer", "") != "")
        {
            res.push_back(wxString::Format("    WARN: Face effect with subbuffer defined '%s' may not render correctly. Model '%s', Start %s", settings.Get("B_CUSTOM_SubBuffer", ""), model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    if (modelType == "Matrix")
    {
        auto images = model->faceInfo[definition.ToStdString()];
        for (auto it2 = images.begin(); it2 != images.end(); ++it2)
        {
            if ((*it2).first.find("Mouth") == 0)
            {
                std::string picture = (*it2).second;

                if (picture != "")
                {
                    if (!wxFileExists(picture))
                    {
                        res.push_back(wxString::Format("    ERR: Face effect image file not found '%s'. Model '%s', Start %s", picture, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                    }
                    else if (!IsFileInShowDir(xLightsFrame::CurrentDir, picture))
                    {
                        res.push_back(wxString::Format("    WARN: Faces effect image file '%s' not under show directory. Model '%s', Start %s", picture, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                    }
                }
            }
        }
    }

    wxString timing = settings.Get("E_CHOICE_Faces_TimingTrack", "");
    wxString phoneme = settings.Get("E_CHOICE_Faces_Phoneme", "");

    // - Face chosen or specific phoneme
    if (phoneme == "" && timing == "")
    {
        res.push_back(wxString::Format("    ERR: Face effect with no timing selected. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void FacesEffect::SetPanelStatus(Model *cls) {
    FacesPanel *fp = (FacesPanel*)panel;
    if (fp == nullptr) return;

    fp->Choice_Faces_TimingTrack->Clear();
    fp->Face_FaceDefinitonChoice->Clear();
    if (mSequenceElements == nullptr) {
        return;
    }
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
        if (mSequenceElements->GetElement(i)->GetEffectLayerCount() == 3
            && mSequenceElements->GetElement(i)->GetType() == ELEMENT_TYPE_TIMING) {
            fp->Choice_Faces_TimingTrack->Append(mSequenceElements->GetElement(i)->GetName());
        }
    }
    bool addRender = true;
    if (cls != nullptr) {
        for (std::map<std::string, std::map<std::string, std::string> >::iterator it = cls->faceInfo.begin(); it != cls->faceInfo.end(); ++it) {
            if (it->first != "")
            {
                fp->Face_FaceDefinitonChoice->Append(it->first);
                if (it->second["Type"] == "Coro" || it->second["Type"] == "SingleNode" || it->second["Type"] == "NodeRange") {
                    addRender = false;
                }
            }
        }
    }
    if (fp->Face_FaceDefinitonChoice->GetCount() == 0) {
        fp->Face_FaceDefinitonChoice->Append("Default");
        addRender = false;
    }
    if (addRender) {
        fp->Face_FaceDefinitonChoice->Append("Rendered");
    }
    fp->Face_FaceDefinitonChoice->SetSelection(0);
}

std::list<std::string> FacesEffect::GetFacesUsed(const SettingsMap& SettingsMap)
{
    std::list<std::string> res;
    auto face = SettingsMap.Get("E_CHOICE_Faces_FaceDefinition", "Default");
    if (face != "Default" && face != "Rendered" && face != "")
    {
        res.emplace_back(face);
    }
    return res;
}

wxPanel *FacesEffect::CreatePanel(wxWindow *parent) {
    return new FacesPanel(parent);
}

void FacesEffect::SetDefaultParameters() {
    FacesPanel *fp = (FacesPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    SetRadioValue(fp->RadioButton1);
    SetChoiceValue(fp->Choice_Faces_Phoneme, "AI");
    SetChoiceValue(fp->Choice_Faces_Eyes, "Auto");

    if (fp->Face_FaceDefinitonChoice->GetCount() > 0)
    {
        fp->Face_FaceDefinitonChoice->SetSelection(0);
    }

    SetCheckBoxValue(fp->CheckBox_Faces_Outline, false);
    SetCheckBoxValue(fp->CheckBox_SuppressWhenNotSinging, false);
    SetCheckBoxValue(fp->CheckBox_TransparentBlack, false);
    SetSliderValue(fp->Slider_Faces_TransparentBlack, 0);
}

void FacesEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_Faces_TimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_Faces_TimingTrack"] = wxString(newname);
    }
}

void FacesEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    if (SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default") == "Rendered"
        && SettingsMap.Get("CHECKBOX_Faces_Outline", "") == "") {
        //3.x style Faces effect
        RenderFaces(buffer, 
            SettingsMap["CHOICE_Faces_Phoneme"], 
            "Auto", 
            true, 
            SettingsMap.GetBool("CHECKBOX_Faces_SuppressWhenNotSinging", 
                false));
    } else if (SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default") == XLIGHTS_PGOFACES_FILE) {
        RenderCoroFacesFromPGO(buffer,
                               SettingsMap["CHOICE_Faces_Phoneme"],
                               SettingsMap.Get("CHOICE_Faces_Eyes", "Auto"),
                               SettingsMap.GetBool("CHECKBOX_Faces_Outline"),
                               SettingsMap.GetBool("CHECKBOX_Faces_SuppressWhenNotSinging", false));
    } else {
        RenderFaces(buffer,
                    effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                    SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default"),
                    SettingsMap["CHOICE_Faces_Phoneme"],
                    SettingsMap["CHOICE_Faces_TimingTrack"],
                    SettingsMap["CHOICE_Faces_Eyes"],
                    SettingsMap.GetBool("CHECKBOX_Faces_Outline"),
                    SettingsMap.GetBool("CHECKBOX_Faces_TransparentBlack", false),
                    SettingsMap.GetInt("TEXTCTRL_Faces_TransparentBlack", 0), 
                    SettingsMap.GetBool("CHECKBOX_Faces_SuppressWhenNotSinging", false)
            );
    }
}

void FacesEffect::RenderFaces(RenderBuffer &buffer, const std::string &Phoneme, const std::string &eyes, bool outline, bool suppressIfNotSinging)
{
    static const std::map<wxString, int> phonemeMap = {
        {"AI", 0},
        {"E", 1},
        {"FV", 2},
        {"L", 3},
        {"MBP", 4},
        {"O", 5},
        {"U", 6},
        {"WQ", 7},
        {"etc", 8},
        {"rest", 9},
        {"(off)", 10}
    };

    std::map<wxString, int>::const_iterator it = phonemeMap.find(Phoneme);
    int PhonemeInt = 0;
    if (it != phonemeMap.end()) {
        PhonemeInt = it->second;
    }

    int Ht = buffer.BufferHt;
    int Wt = buffer.BufferWi;

    drawoutline(buffer, PhonemeInt, outline, eyes, buffer.BufferHt, buffer.BufferWi);
    mouth(buffer, PhonemeInt, Ht,  Wt); // draw a mouth syllable
}

//TODO: add params for eyes, outline
void FacesEffect::mouth(RenderBuffer &buffer, int Phoneme, int BufferHt, int BufferWi)
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

    double offset = 0.0;
    int Ht = BufferHt - 1;
    int Wt = BufferWi - 1;
    int x1 = (int)(offset + Wt * 0.25);
    int x2 = (int)(offset + Wt * 0.75);
    int x3 = (int)(offset + Wt * 0.30);
    int x4 = (int)(offset + Wt * 0.70);

    int y1 = (int)(offset + Ht * 0.48);
    int y2 = (int)(offset + Ht * 0.40);
    int y3 = (int)(offset + Ht * 0.25);
    int y4 = (int)(offset + Ht * 0.20);
    int y5 = (int)(offset + Ht * 0.30);

    // eyes
    switch (Phoneme)
    {
    case 0:         // AI
        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        drawline1(buffer, Phoneme, x1, x2, y1, y4, 0);
        break;
    case 3:
    case 1:       // E, L
        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        drawline1(buffer, Phoneme, x1, x2, y1, y3, 0);
        break;
    case 2:       // FV
        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        drawline1(buffer, Phoneme, x1, x2, y1, y2 - 1, 0);
        break;
    case 4:
    case 9:     //  MBP,rest

        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        break;
    case 5:
    case 6:       // O,U,WQ
    case 7:
    {
        int xc = (int)(0.5 + Wt * 0.50);
        int yc = (int)(y2 - y5) / 2 + y5;
        double radius = (std::min(Wt, Ht)) * 0.15;  // O
        if (Phoneme == 6) radius = (std::min(Wt, Ht)) * 0.10;  // U
        if (Phoneme == 7) radius = (std::min(Wt, Ht)) * 0.05;  // WQ
        facesCircle(buffer, Phoneme, xc, yc, radius, 0, 360, 0);
    }
    break;
    case 8:       // WQ, etc
        drawline3(buffer, Phoneme, x3, x4, y5, y2, 0);
        break;
    default:
        break;
    }
}

void FacesEffect::drawline1(RenderBuffer &buffer, int Phoneme, int x1, int x2, int y1, int y2, int ColorIdx)
{
    HSVValue hsv;

    buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

    for (int x = x1 + 1; x < x2; x++)
    {
        buffer.SetPixel(x, y2, hsv); // Turn pixel on
    }

    for (int y = y2 + 1; y <= y1; y++)
    {
        buffer.SetPixel(x1, y, hsv); // Left side of mouyh
        buffer.SetPixel(x2, y, hsv); // rightside
    }
}

void FacesEffect::drawline3(RenderBuffer &buffer, int Phoneme, int x1, int x2, int y6, int y7, int ColorIdx)
{
    HSVValue hsv;
    buffer.palette.GetHSV(ColorIdx, hsv);

    for (int y = y6 + 1; y < y7; y++)
    {
        buffer.SetPixel(x1, y, hsv); // Left side of mouyh
        buffer.SetPixel(x2, y, hsv); // rightside
    }

    for (int x = x1 + 1; x < x2; x++)
    {
        buffer.SetPixel(x, y6, hsv); // Bottom
        buffer.SetPixel(x, y7, hsv); // Bottom
    }
}

/*
 faces draw circle
 */
void FacesEffect::facesCircle(RenderBuffer &buffer, int Phoneme, int xc, int yc, double radius, int start_degrees, int end_degrees, int colorIdx)
{
    HSVValue hsv;
    buffer.palette.GetHSV(colorIdx, hsv);

    for (int degrees = start_degrees; degrees < end_degrees; degrees++)
    {
        double t = ((double)degrees * PI) / 180.0;
        int x = (int)((double)xc + radius * cos(t));
        int y = (int)((double)yc + radius * sin(t));
        buffer.SetPixel(x, y, hsv); // Bottom
    }
}

void FacesEffect::drawoutline(RenderBuffer &buffer, int Phoneme, bool outline, const std::string &eyes, int BufferHt,int BufferWi)
{
    std::string eye = eyes;

    FacesRenderCache *cache = (FacesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FacesRenderCache();
        buffer.infoCache[id] = cache;
    }

    int Ht = BufferHt-1;
    int Wt = BufferWi-1;

    size_t colorcnt = buffer.GetColorCount();

    HSVValue hsvOutline;
    buffer.palette.GetHSV(1 % colorcnt, hsvOutline);
    
    //  DRAW EYES
    int start_degrees = 0;
    int end_degrees = 360;
    if (eye == "Auto") {
        if (Phoneme == 9 || Phoneme == 10)
        {
            if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                //roughly every 5 seconds we'll blink
                cache->nextBlinkTime += (4500 + (rand() % 1000));
                cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; //100ms blink
                eye = "Closed";
            }
            else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                eye = "Closed";
            }
            else {
                eye = "Open";
            }
        }
        else
        {
            eye = "Open";
        }
    }

    if (eye == "Closed") {
        start_degrees = 180;
        end_degrees = 360;
    }

    if (eye != "(off)") {
        int xc = (int)(0.5 + Wt * 0.33); // left eye
        int yc = (int)(0.5 + Ht * 0.75);
        double radius = Wt * 0.08;
        facesCircle(buffer, Phoneme, xc, yc, radius, start_degrees, end_degrees, 2 % colorcnt);
        xc = (int)(0.5 + Wt * 0.66); // right eye
        facesCircle(buffer, Phoneme, xc, yc, radius, start_degrees, end_degrees, 2 % colorcnt);
    }

    /*
     ...********...
     ..*
     .*
     *
     *
     *
     */
    if (outline) {
        for(int y = 3; y < BufferHt - 3; y++)
        {
            buffer.SetPixel(0, y, hsvOutline); // Left side of mouyh
            buffer.SetPixel(BufferWi - 1, y, hsvOutline); // rightside
        }
        for(int x = 3; x < BufferWi - 3; x++)
        {
            buffer.SetPixel(x, 0, hsvOutline); // Bottom
            buffer.SetPixel(x, BufferHt - 1, hsvOutline); // Bottom
        }
        buffer.SetPixel(2, 1, hsvOutline); // Bottom left
        buffer.SetPixel(1, 2, hsvOutline); //

        buffer.SetPixel(BufferWi - 3, 1, hsvOutline); // Bottom Right
        buffer.SetPixel(BufferWi - 2, 2, hsvOutline); //

        buffer.SetPixel(BufferWi - 3, BufferHt - 2, hsvOutline); // Bottom Right
        buffer.SetPixel(BufferWi - 2, BufferHt - 3, hsvOutline); //

        buffer.SetPixel(2, BufferHt - 2, hsvOutline); // Bottom Right
        buffer.SetPixel(1, BufferHt - 3, hsvOutline); //
    }
}

/*----------------------CoroFaces--------------------------*/
//TODO: move this to a shared location:
static wxString NoInactive(wxString name)
{
    const wxString InactiveIndicator = "?";
    return name.StartsWith(InactiveIndicator)? name.substr(InactiveIndicator.size()): name;
}

//cached model info:
static std::unordered_map<std::string, std::unordered_map<std::string, /*wxPoint*/ std::string> > model_xy;

static bool parse_model(const wxString& want_model)
{
    if (model_xy.find((const char*)want_model.c_str()) != model_xy.end()) return true; //already have info

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
        //group name is not available, so use first occurrence of model in *any* group:
        //NOTE: assumes phoneme/face mapping is consistent for any given model across groups, which should be the case since the lights don't move
        for (wxXmlNode* group = Presets->GetChildren(); group != nullptr; group = group->GetNext())
        {
            wxString grpname = group->GetAttribute(wxT("name"));
            for (wxXmlNode* voice = group->GetChildren(); voice != nullptr; voice = voice->GetNext())
            {
                wxString voice_name = NoInactive(voice->GetAttribute(wxT("name")));
                if (voice_name != want_model) continue;
                //XmlNode getting trashed later, so save it here
                std::unordered_map<std::string, std::string>& map = model_xy[(const char*)want_model.c_str()];
                map.clear();
                for (wxXmlAttribute* attrp = voice->GetAttributes(); attrp; attrp = attrp->GetNext())
                {
                    wxString value = attrp->GetValue();
                    if (!value.empty()) map[(const char*)attrp->GetName().c_str()] = (const char*)value.c_str();
                }
                return true;
            }
        }
    }

    return false;
}

//NOTE: params are re-purposed as follows for Coro face mode:
// x_y = list of active elements for this frame
// Outline_x_y = list of persistent/sticky elements (stays on after frame ends)
// Eyes_x_y = list of random elements (intended for eye blinks, etc)

void FacesEffect::RenderCoroFacesFromPGO(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, bool face_outline, bool suppressIfNotSinging)
{
    if (suppressIfNotSinging && Phoneme == "") return;

    //NOTE:
    //PixelBufferClass contains 2 RgbEffects members, which this method is a member of
    //xLightsFrame contains a PixelBufferClass member named buffer, which is derived from Model and gives the name of the model currently being used
    //therefore we can access the model info by going to parent object's buffer member

    if (!buffer.curPeriod) model_xy.clear(); //flush cache once at start of each effect

    static std::unordered_map<std::string, std::string> auto_phonemes {{"a-AI", "AI"}, {"a-E", "E"}, {"a-FV", "FV"}, {"a-L", "L"},
        {"a-MBP", "MBP"}, {"a-O", "O"}, {"a-U", "U"}, {"a-WQ", "WQ"}, {"a-etc", "etc"}, {"a-rest", "rest"}};

    if (auto_phonemes.find((const char*)Phoneme.c_str()) != auto_phonemes.end())
    {
        RenderFaces(buffer, auto_phonemes[(const char*)Phoneme.c_str()], eyes, face_outline, suppressIfNotSinging);
        return;
    }

    xlColor color;
    buffer.palette.GetColor(0, color); //use first color; user must make sure it matches model node type
    color = xlWHITE; //kludge: must use WHITE to get single-color nodes to show correctly
    HSVValue hsv;
    buffer.Color2HSV(color, hsv);

    std::vector<wxPoint> first_xy;
    Model* model_info = buffer.GetModel();

    if (!model_info || !parse_model(buffer.cur_model))
    {
        return;
    }
    std::unordered_map<std::string, std::string>& map = model_xy[(const char*)buffer.cur_model.c_str()];
    if (Phoneme == "(test)")
    {
        std::string info = eyes;
        Model::ParseFaceElement(info, first_xy);
    }
    if (!Phoneme.empty())
    {
        std::string info = map[(const char*)Phoneme.c_str()];
        Model::ParseFaceElement(info, first_xy);
    }
    else if (suppressIfNotSinging)
    {
        return;
    }

    if (!eyes.empty())
    {
        std::string eyesLower(eyes);
        std::transform(eyesLower.begin(), eyesLower.end(), eyesLower.begin(), ::tolower);

        std::string info = map[wxString::Format(wxT("Eyes_%s"), eyesLower.c_str()).ToStdString()];
        Model::ParseFaceElement(info, first_xy);
    }
    if (face_outline)
    {
        std::string info = map["Outline"];
        Model::ParseFaceElement(info, first_xy);
    }
    for (auto it = first_xy.begin(); it != first_xy.end(); ++it)
    {
        --(*it).x; // "A" = 1 = first col
        buffer.SetPixel((*it).x, buffer.BufferHt - (*it).y, hsv); //only need to turn on first pixel for each face part
    }
}

std::string FacesEffect::MakeKey(int bufferWi, int bufferHt, std::string dirstr, std::string picture, std::string stf)
{
    return wxString::Format("%d|%d|%s|%s|%s", bufferWi, bufferHt, dirstr, picture, stf).ToStdString();
}

void FacesEffect::RenderFaces(RenderBuffer &buffer,
    SequenceElements *elements, const std::string &faceDefinition,
    const std::string& Phoneme, const std::string &trackName,
    const std::string& eyesIn, bool face_outline, bool transparentBlack, int transparentBlackLevel, bool suppressIfNotSinging)
{
    FacesRenderCache *cache = (FacesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FacesRenderCache();
        buffer.infoCache[id] = cache;
    }

    if (buffer.needToInit) {
        buffer.needToInit = false;
        elements->AddRenderDependency(trackName, buffer.cur_model);
        cache->Clear();
    }
    std::string eyes = eyesIn;

    Element *track = elements->GetElement(trackName);
    std::recursive_timed_mutex tmpLock;
    std::recursive_timed_mutex *lock = &tmpLock;
    if (track != nullptr) {
        lock = &track->GetChangeLock();
    }
    std::unique_lock<std::recursive_timed_mutex> locker(*lock);

    if (buffer.cur_model == "") {
        return;
    }
    Model* model_info = buffer.GetPermissiveModel();
    if (model_info == nullptr) {
        return;
    }

    // if this is a submodel find the parent so we can find the face definition there
    if (model_info->GetDisplayAs() == "SubModel")
    {
        model_info = ((SubModel*)model_info)->GetParent();
    }
    else if (model_info->GetDisplayAs() == "ModelGroup")
    {
        return;
    }

    if (cache->nodeNameCache.empty()) {
        for (int x = 0; x < model_info->GetNodeCount(); x++) {
            cache->nodeNameCache[model_info->GetNodeName(x, true)] = x;
        }
    }

    std::string definition = faceDefinition;
    if (definition == "Default" && !model_info->faceInfo.empty() && model_info->faceInfo.begin()->first != "") {
        definition = model_info->faceInfo.begin()->first;
    }
    bool found = true;
    std::map<std::string, std::map<std::string, std::string> >::iterator it = model_info->faceInfo.find(definition);
    if (it == model_info->faceInfo.end()) {
        //not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model_info->faceInfo.find("SingleNode") != model_info->faceInfo.end()) {
            definition = "SingleNode";
            found = true;
        }
        else if ("SingleNode" == definition && model_info->faceInfo.find("Coro") != model_info->faceInfo.end()) {
            definition = "Coro";
            found = true;
        }
    }
    std::string modelType = found ? model_info->faceInfo[definition]["Type"] : definition;
    if (modelType == "") {
        modelType = definition;
    }

    int type = 3;

    if ("Coro" == modelType || "SingleNode" == modelType) {
        type = 0;
    }
    else if ("NodeRange" == modelType) {
        type = 1;
    }
    else if ("Rendered" == definition || "Default" == definition) {
        type = 2;
    }

    if (buffer.curEffStartPer == buffer.curPeriod)
    {
        if (modelType != "Matrix" && modelType != "Rendered" && modelType != "Default")
        {
            if (buffer.isTransformed)
            {
                static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.warn("Faces effect starting at %dms until %dms on model %s has a transformed buffer. This may not work as expected.", buffer.curEffStartPer * buffer.frameTimeInMs, buffer.curEffEndPer * buffer.frameTimeInMs, (const char *)buffer.cur_model.c_str());
            }
        }
    }

    std::string phoneme = Phoneme;
    if (phoneme == "") {
        //GET Phoneme from timing track
        if (track == nullptr || track->GetEffectLayerCount() < 3) {
            phoneme = "rest";
            if ("Auto" == eyes) {
                if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                    //roughly every 5 seconds we'll blink
                    cache->nextBlinkTime += (4500 + (rand() % 1000));
                    cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; //100ms blink
                    eyes = "Closed";
                }
                else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                    eyes = "Closed";
                }
                else {
                    eyes = "Open";
                }
            }
        }
        else {
            int startms = -1;
            int endms = -1;

            EffectLayer *layer = track->GetEffectLayer(2);
            std::unique_lock<std::recursive_mutex> locker2(layer->GetLock());
            int time = buffer.curPeriod * buffer.frameTimeInMs + 1;
            Effect *ef = layer->GetEffectByTime(time);
            if (ef == nullptr) {
                if (suppressIfNotSinging)
                {
                    return;
                }

                phoneme = "rest";
            }
            else {
                startms = ef->GetStartTimeMS();
                endms = ef->GetEndTimeMS();
                phoneme = ef->GetEffectName();
            }
            if ("Auto" == eyes && phoneme == "rest" && type != 2) {
                if (startms == -1) {
                    //need to figure out the time
                    for (int x = 0; x < layer->GetEffectCount() && startms == -1; x++) {
                        ef = layer->GetEffect(x);
                        if (ef->GetStartTimeMS() > buffer.curPeriod * buffer.frameTimeInMs) {
                            endms = ef->GetStartTimeMS();
                            if (x > 0) {
                                startms = layer->GetEffect(x - 1)->GetEndTimeMS();
                            }
                            else {
                                startms = 0;
                            }
                        }
                    }
                }

                if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                    if ((startms + 150) >= (buffer.curPeriod * buffer.frameTimeInMs)) {
                        //don't want to blink RIGHT at the start of the rest, delay a little bie
                        int tmp = (buffer.curPeriod * buffer.frameTimeInMs) + 150 + rand() % 400;

                        //also don't want it right at the end
                        if ((tmp + 130) > endms) {
                            cache->nextBlinkTime = (startms + endms) / 2;
                        }
                        else {
                            cache->nextBlinkTime = tmp;
                        }
                    }
                    else {
                        //roughly every 5 seconds we'll blink
                        cache->nextBlinkTime += (4500 + (rand() % 1000));
                        cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; //100ms blink
                        eyes = "Closed";
                    }
                }
                else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                    eyes = "Closed";
                }
                else {
                    eyes = "Open";
                }
            }
        }
    }
    else if (phoneme == "rest" || phoneme == "(off)")
    {
        if ("Auto" == eyes) {
            if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                //roughly every 5 seconds we'll blink
                cache->nextBlinkTime += (4500 + (rand() % 1000));
                cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; //100ms blink
                eyes = "Closed";
            }
            else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                eyes = "Closed";
            }
            else {
                eyes = "Open";
            }
        }
    }

    if (phoneme == "" && suppressIfNotSinging)
    {
        return;
    }

    int colorOffset = 0;
    xlColor color;
    buffer.palette.GetColor(0, color); //use first color for mouth; user must make sure it matches model node type

    bool customColor = found ? model_info->faceInfo[definition]["CustomColors"] == "1" : false;

    std::vector<std::string> todo;
    std::vector<xlColor> colors;
    if (phoneme != "(off)") {
        todo.push_back("Mouth-" + phoneme);
        colorOffset = 1;
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Mouth-" + phoneme + "-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
            }
            else {
                colors.push_back(xlColor(cname));
            }
        }
        else {
            colors.push_back(color);
        }
    }
    if (buffer.palette.Size() > colorOffset) {
        buffer.palette.GetColor(colorOffset, color); //use second color for eyes; user must make sure it matches model node type
    }
    if (eyes == "Open" || eyes == "Auto") {
        todo.push_back("Eyes-Open");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Eyes-Open-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
            }
            else {
                colors.push_back(xlColor(cname));
            }
        }
        else {
            colors.push_back(color);
        }
    }
    else if (eyes == "Closed") {
        todo.push_back("Eyes-Closed");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Eyes-Closed-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
            }
            else {
                colors.push_back(xlColor(cname));
            }
        }
        else {
            colors.push_back(color);
        }
    }
    else if (eyes == "(off)") {
        //no eyes
    }
    if (buffer.palette.Size() > (1 + colorOffset)) {
        buffer.palette.GetColor((1 + colorOffset), color); //use third color for outline; user must make sure it matches model node type
    }
    if (face_outline) {
        todo.insert(todo.begin(), "FaceOutline");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["FaceOutline-Color"];
            if (cname == "") {
                colors.insert(colors.begin(), xlWHITE);
            }
            else {
                colors.insert(colors.begin(), xlColor(cname));
            }
        }
        else {
            colors.insert(colors.begin(), color);
        }
    }

    if (type == 2) {
        RenderFaces(buffer, phoneme, eyes, face_outline, suppressIfNotSinging);
        return;
    }
    if (type == 3) {
        //picture
        std::string e = eyes;
        if (eyes == "Auto") {
            e = "Open";
        }
        if (eyes == "(off)") {
            e = "Closed";
        }
        std::string key = "Mouth-" + phoneme + "-Eyes";
        std::string picture = "";
        if (model_info->faceInfo[definition].find(key + e) != model_info->faceInfo[definition].end())
        {
            picture = model_info->faceInfo[definition][key + e];
        }
        if (picture == "" && e == "Closed") {
            if (model_info->faceInfo[definition].find(key + "Open") != model_info->faceInfo[definition].end())
            {
                picture = model_info->faceInfo[definition][key + "Open"];
            }
        }
        std::string dirstr = "none"; /*RENDER_PICTURE_NONE*/
        std::string stf = "Scale To Fit";
        if (model_info->faceInfo[definition]["ImagePlacement"] == "Centered") {
            dirstr = "none"; /*RENDER_PICTURE_NONE */
            stf = "No Scaling";
        }
        RenderBuffer* crb = cache->GetImage(MakeKey(buffer.BufferWi, buffer.BufferHt, dirstr, picture, stf));
        if (crb == nullptr)
        {
            crb = new RenderBuffer(buffer);
            PicturesEffect::Render(*crb, dirstr, picture, 0, 0, 0, 0, 0, 0, 100, 100, stf, false, false, false, true, false, false, 0);  // set for scale to fit
            cache->AddImage(MakeKey(buffer.BufferWi, buffer.BufferHt, dirstr, picture, stf), crb);
        }

        for (int y = 0; y < buffer.BufferHt; y++)
        {
            for (int x = 0; x < buffer.BufferWi; x++)
            {
                if (transparentBlack)
                {
                    auto c = crb->GetPixel(x, y);
                    int level = c.Red() + c.Green() + c.Blue();
                    if (level > transparentBlackLevel)
                    {
                        buffer.SetPixel(x, y, c);
                    }
                }
                else
                {
                    buffer.SetPixel(x, y, crb->GetPixel(x, y));
                }
            }
        }
    }
    for (size_t t = 0; t < todo.size(); t++) {
        std::string channels = model_info->faceInfo[definition][todo[t]];
        wxStringTokenizer wtkz(channels, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();

            if (type == 0) {
                auto it2 = cache->nodeNameCache.find(valstr.ToStdString());
                if (it2 != cache->nodeNameCache.end()) {
                    int n = it2->second;
                    buffer.SetNodePixel(n, colors[t]);
                }
            }
            else if (type == 1) {
                int start, end;
                if (valstr.Contains("-")) {
                    int idx = valstr.Index('-');
                    start = wxAtoi(valstr.Left(idx));
                    end = wxAtoi(valstr.Right(valstr.size() - idx - 1));
                    if (end < start)
                    {
                        std::swap(start, end);
                    }
                }
                else {
                    start = end = wxAtoi(valstr);
                }
                if (start > end) {
                    start = end;
                }
                start--;
                end--;
                for (int n = start; n <= end; n++) {
                    buffer.SetNodePixel(n, colors[t]);
                }
            }
        }
    }
}