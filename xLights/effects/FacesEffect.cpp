/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <list>

#include "FacesEffect.h"
#include "FacesPanel.h"
#include "../models/Model.h"
#include "../models/SubModel.h"
#include "../models/ModelGroup.h"
#include "../sequencer/SequenceElements.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../xLightsMain.h" 
#include "PicturesEffect.h"
#include "../ExternalHooks.h"

#include <wx/tokenzr.h>

#include "../../include/corofaces.xpm"

#include <log4cpp/Category.hh>

class FacesRenderCache : public EffectRenderCache {
    std::map<std::string, RenderBuffer*> _imageCache;

public:
    int blinkEndTime;
    int nextBlinkTime;
    std::map<std::string, int> nodeNameCache;

    FacesRenderCache(int nextBlinkTime) :
        blinkEndTime(0), nextBlinkTime(nextBlinkTime) {
    }
    virtual ~FacesRenderCache() {
        for (auto it : _imageCache) {
            delete it.second;
        }
        _imageCache.clear();
    }
    void Clear() {
        nodeNameCache.clear();
    }
    RenderBuffer* GetImage(std::string key) {
        if (_imageCache.find(key) != _imageCache.end()) {
            return _imageCache[key];
        }

        return nullptr;
    }
    void AddImage(std::string key, RenderBuffer* crb) {
        _imageCache[key] = crb;
    }
};

FacesEffect::FacesEffect(int id) :
    RenderableEffect(id, "Faces", corofaces, corofaces, corofaces, corofaces, corofaces) {
    //ctor
}

FacesEffect::~FacesEffect() {
    //dtor
}

wxString FacesEffect::GetEffectString() {
    FacesPanel* p = (FacesPanel*)panel;
    std::stringstream ret;

    if (p->CheckBox_Faces_Outline->GetValue()) {
        ret << "E_CHECKBOX_Faces_Outline=1,";

        if (p->Choice1->GetStringSelection() != "") {
            ret << "E_CHOICE_Faces_UseState=";
            ret << p->Choice1->GetStringSelection();
            ret << ",";
        }
    }

    if (p->CheckBox_SuppressShimmer->GetValue()) {
        ret << "E_CHECKBOX_Faces_SuppressShimmer=1,";
    }

    if (p->CheckBox_SuppressWhenNotSinging->GetValue() && !p->RadioButton1->GetValue()) {
        ret << "E_CHECKBOX_Faces_SuppressWhenNotSinging=1,";
        if (p->CheckBox_Fade->GetValue()) {
            ret << "E_CHECKBOX_Faces_Fade=1,";
        }
        if (p->SpinCtrl_LeadFrames->GetValue() > 0) {
            ret << "E_SPINCTRL_Faces_LeadFrames=";
            ret << p->SpinCtrl_LeadFrames->GetValue();
            ret << ",";
        }
    }

    if (p->CheckBox_TransparentBlack->GetValue()) {
        ret << "E_CHECKBOX_Faces_TransparentBlack=1,";
        ret << "E_TEXTCTRL_Faces_TransparentBlack=";
        ret << p->TextCtrl_Faces_TransparentBlack->GetValue().ToStdString();
        ret << ",";
    }

    ret << "E_CHOICE_Faces_Eyes=";
    ret << p->Choice_Faces_Eyes->GetStringSelection().ToStdString();
    ret << ",";

    ret << "E_CHOICE_Faces_EyeBlinkFrequency=";
    ret << p->Choice_Faces_EyeBlinkFrequency->GetStringSelection().ToStdString();
    ret << ",";

    ret << "E_CHOICE_Faces_FaceDefinition=";
    ret << p->Face_FaceDefinitonChoice->GetStringSelection().ToStdString();
    ret << ",";

    if (p->RadioButton1->GetValue()) {
        ret << "E_CHOICE_Faces_Phoneme=";
        ret << p->Choice_Faces_Phoneme->GetStringSelection().ToStdString();
        ret << ",";
    } else {
        ret << "E_CHOICE_Faces_TimingTrack=";
        ret << p->Choice_Faces_TimingTrack->GetStringSelection().ToStdString();
        ret << ",";
    }
    return ret.str();
}

std::list<std::string> FacesEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) {
    wxLogNull logNo; // suppress popups from png images. See http://trac.wxwidgets.org/ticket/15331
    std::list<std::string> res;

    wxString definition = settings.Get("E_CHOICE_Faces_FaceDefinition", "");
    if (definition == "Default" && !model->faceInfo.empty() && model->faceInfo.begin()->first != "") {
        definition = model->faceInfo.begin()->first;
    }
    bool found = true;
    std::map<std::string, std::map<std::string, std::string>>::iterator it = model->faceInfo.find(definition.ToStdString());
    if (it == model->faceInfo.end()) {
        //not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model->faceInfo.find("SingleNode") != model->faceInfo.end()) {
            definition = "SingleNode";
            found = true;
        } else if ("SingleNode" == definition && model->faceInfo.find("Coro") != model->faceInfo.end()) {
            definition = "Coro";
            found = true;
        }
    }

    // check the face exists on the model
    if (definition != "Rendered") {
        if (model->faceInfo.find(definition) == model->faceInfo.end()) {
            res.push_back(wxString::Format("    ERR: Face effect face '%s' does not exist on model '%s'. Start %s", definition, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    wxString modelType = found ? wxString(model->faceInfo[definition.ToStdString()]["Type"].c_str()) : definition;
    if (modelType == "") {
        modelType = definition;
    }

    if (modelType != "Matrix" && modelType != "Rendered") {
        // -Buffer not rotated
        wxString bufferTransform = settings.Get("B_CHOICE_BufferTransform", "None");

        if (bufferTransform != "None") {
            res.push_back(wxString::Format("    WARN: Face effect with transformed buffer '%s' may not render correctly. Model '%s', Start %s", model->GetName(), bufferTransform, FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        if (settings.GetInt("B_SLIDER_Rotation", 0) != 0 ||
            settings.GetInt("B_SLIDER_Rotations", 0) != 0 ||
            settings.GetInt("B_SLIDER_XRotation", 0) != 0 ||
            settings.GetInt("B_SLIDER_YRotation", 0) != 0 ||
            settings.GetInt("B_SLIDER_Zoom", 1) != 1 ||
            settings.Get("B_VALUECURVE_Rotation", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_XRotation", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_YRotation", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_Rotations", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_Zoom", "").find("Active=TRUE") != std::string::npos) {
            res.push_back(wxString::Format("    WARN: Face effect with rotozoom active may not render correctly. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        if (settings.Get("B_CUSTOM_SubBuffer", "") != "") {
            res.push_back(wxString::Format("    WARN: Face effect with subbuffer defined '%s' may not render correctly. Model '%s', Start %s", settings.Get("B_CUSTOM_SubBuffer", ""), model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    if (modelType == "Matrix") {
        auto images = model->faceInfo[definition.ToStdString()];
        for (const auto& it2 : images) {
            if (it2.first.find("Mouth") == 0) {
                std::string picture = it2.second;

                if (picture != "") {
                    if (!FileExists(picture)) {
                        res.push_back(wxString::Format("    ERR: Face effect image file not found '%s'. Model '%s', Start %s", picture, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                    } else if (!IsFileInShowDir(xLightsFrame::CurrentDir, picture)) {
                        res.push_back(wxString::Format("    WARN: Faces effect image file '%s' not under show directory. Model '%s', Start %s", picture, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                    }

                    if (FileExists(picture)) {
                        wxImage i;
                        i.LoadFile(picture);
                        if (i.IsOk()) {
                            int ih = i.GetHeight();
                            int iw = i.GetWidth();

#define IMAGESIZETHRESHOLD 10
                            if (ih > IMAGESIZETHRESHOLD * model->GetDefaultBufferHt() || iw > IMAGESIZETHRESHOLD * model->GetDefaultBufferWi()) {
                                float scale = std::max((float)ih / model->GetDefaultBufferHt(), (float)iw / model->GetDefaultBufferWi());
                                res.push_back(wxString::Format("    WARN: Faces effect image file '%s' is %.1f times the height or width of the model ... xLights is going to need to do lots of work to resize the image. Model '%s', Start %s", picture, scale, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                            }
                        }
                    }
                }
            }
        }
    }

    wxString timing = settings.Get("E_CHOICE_Faces_TimingTrack", "");
    wxString phoneme = settings.Get("E_CHOICE_Faces_Phoneme", "");

    // - Face chosen or specific phoneme
    if (phoneme == "" && timing == "") {
        res.push_back(wxString::Format("    ERR: Face effect with no timing selected. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    } else if (timing != "" && GetTiming(timing) == nullptr) {
        res.push_back(wxString::Format("    ERR: Face effect with unknown timing (%s) selected. Model '%s', Start %s", timing, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

int FacesEffect::GetMaxEyeDelay( std::string& eyeBlinkFreqString ) const {
    int maxEyeDelay = 5500; //Normal
    if( eyeBlinkFreqString  == "Slowest")
        maxEyeDelay = 9000;
    else if( eyeBlinkFreqString  == "Slow")
        maxEyeDelay = 7250;
    else if( eyeBlinkFreqString  == "Fast")
        maxEyeDelay = 3750;
    else if( eyeBlinkFreqString  == "Fastest")
        maxEyeDelay = 2000;
    
    return maxEyeDelay;
}

void FacesEffect::SetPanelStatus(Model* cls) {
    FacesPanel* fp = (FacesPanel*)panel;
    if (fp == nullptr)
        return;

    fp->Choice1->Clear();
    fp->Choice1->Append("");
    fp->Choice_Faces_TimingTrack->Clear();
    fp->Face_FaceDefinitonChoice->Clear();
    for (const auto& it : wxSplit(GetTimingTracks(0, 3), '|')) {
        fp->Choice_Faces_TimingTrack->Append(it);
    }

    bool addRender = true;
    if (cls != nullptr) {
        Model* m = cls;

        if (cls->GetDisplayAs() == "ModelGroup") {
            m = ((ModelGroup*)cls)->GetFirstModel();
        } else if (cls->GetDisplayAs() == "SubModel") {
            m = ((SubModel*)cls)->GetParent();
        }

        if (m != nullptr) {
            for (auto& it : m->faceInfo) {
                if (it.first != "") {
                    fp->Face_FaceDefinitonChoice->Append(it.first);
                    if (it.second["Type"] == "Coro" || it.second["Type"] == "SingleNode" || it.second["Type"] == "NodeRange") {
                        addRender = false;
                    }
                }
            }

            std::list<std::string> used;
            for (const auto& it : m->stateInfo) {
                if (std::find(begin(used), end(used), it.first) == end(used) )
                {
                    fp->Choice1->Append(it.first);
                    used.push_back(it.first);
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

std::list<std::string> FacesEffect::GetFacesUsed(const SettingsMap& SettingsMap) const {
    std::list<std::string> res;
    auto face = SettingsMap.Get("E_CHOICE_Faces_FaceDefinition", "Default");
    if (face != "Default" && face != "Rendered" && face != "") {
        res.emplace_back(face);
    }
    return res;
}

std::list<std::string> FacesEffect::GetFileReferences(Model* model, const SettingsMap& settings) const {
    std::list<std::string> res;

    if (model != nullptr) {
        wxString definition = settings.Get("E_CHOICE_Faces_FaceDefinition", "");
        if (definition == "Default" && !model->faceInfo.empty() && model->faceInfo.begin()->first != "") {
            definition = model->faceInfo.begin()->first;
        }
        bool found = true;
        std::map<std::string, std::map<std::string, std::string>>::iterator it = model->faceInfo.find(definition.ToStdString());
        if (it == model->faceInfo.end()) {
            //not found
            found = false;
        }
        if (!found) {
            if ("Coro" == definition && model->faceInfo.find("SingleNode") != model->faceInfo.end()) {
                definition = "SingleNode";
                found = true;
            } else if ("SingleNode" == definition && model->faceInfo.find("Coro") != model->faceInfo.end()) {
                definition = "Coro";
                found = true;
            }
        }

        wxString modelType = found ? wxString(model->faceInfo[definition.ToStdString()]["Type"].c_str()) : definition;
        if (modelType == "") {
            modelType = definition;
        }

        if (modelType == "Matrix") {
            auto images = model->faceInfo[definition.ToStdString()];
            for (const auto& it2 : images) {
                if (it2.first.find("Mouth") == 0) {
                    if (it2.second != "" && std::find(begin(res), end(res), it2.second) == end(res)) {
                        res.push_back(it2.second);
                    }
                }
            }
        }
    }
    return res;
}

xlEffectPanel* FacesEffect::CreatePanel(wxWindow* parent) {
    return new FacesPanel(parent);
}

void FacesEffect::SetDefaultParameters() {
    FacesPanel* fp = (FacesPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    SetRadioValue(fp->RadioButton1);
    SetChoiceValue(fp->Choice_Faces_Phoneme, "AI");
    SetChoiceValue(fp->Choice_Faces_Eyes, "Auto");
    SetChoiceValue(fp->Choice_Faces_EyeBlinkFrequency, "Normal");
    SetChoiceValue(fp->Choice1, "");

    if (fp->Face_FaceDefinitonChoice->GetCount() > 0) {
        fp->Face_FaceDefinitonChoice->SetSelection(0);
    }

    SetCheckBoxValue(fp->CheckBox_SuppressShimmer, false);
    SetCheckBoxValue(fp->CheckBox_Faces_Outline, false);
    SetCheckBoxValue(fp->CheckBox_SuppressWhenNotSinging, false);
    SetCheckBoxValue(fp->CheckBox_Fade, false);
    SetSpinValue(fp->SpinCtrl_LeadFrames, 0);
    SetCheckBoxValue(fp->CheckBox_TransparentBlack, false);
    SetSliderValue(fp->Slider_Faces_TransparentBlack, 0);
}

void FacesEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) {
    wxString timing = effect->GetSettings().Get("E_CHOICE_Faces_TimingTrack", "");

    if (timing.ToStdString() == oldname) {
        effect->GetSettings()["E_CHOICE_Faces_TimingTrack"] = wxString(newname);
    }
}

uint8_t FacesEffect::CalculateAlpha(SequenceElements* elements, int leadFrames, bool fade, const std::string& timingTrack, RenderBuffer& buffer)
{
    uint8_t res = 0;

    Element* track = elements->GetElement(timingTrack);
    std::recursive_timed_mutex tmpLock;
    std::recursive_timed_mutex* lock = &tmpLock;
    if (track != nullptr) {
        lock = &track->GetChangeLock();
    }
    std::unique_lock<std::recursive_timed_mutex> locker(*lock);

    if (track != nullptr && track->GetEffectLayerCount() == 3) {

        EffectLayer* layer = track->GetEffectLayer(2);
        std::unique_lock<std::recursive_mutex> locker2(layer->GetLock());

        int currentTime = buffer.curPeriod * buffer.frameTimeInMs + 1;

        Effect* currentEffect = layer->GetEffectByTime(currentTime);

        if (currentEffect != nullptr) {
            res = 255;
        }
        else {
            if (leadFrames == 0) {
                res = 0;
            }
            else {
                int leadMS = leadFrames * buffer.frameTimeInMs;
                Effect* afterEffect = layer->GetEffectAfterTime(currentTime);
                uint8_t beforeAlpha = 0;
                if (afterEffect != nullptr) {
                    if (afterEffect->GetStartTimeMS() - currentTime < leadMS) {
                        if (fade) {
                            beforeAlpha = 255 - ((afterEffect->GetStartTimeMS() - currentTime) * 255) / leadMS;
                        }
                        else {
                            beforeAlpha = 255;
                        }
                    }
                }
                Effect* beforeEffect = layer->GetEffectBeforeTime(currentTime);
                uint8_t afterAlpha = 0;
                if (beforeEffect != nullptr) {
                    if (currentTime - beforeEffect->GetEndTimeMS() < leadMS) {
                        if (fade) {
                            afterAlpha = 255 - ((currentTime - beforeEffect->GetEndTimeMS()) * 255) / leadMS;
                        }
                        else {
                            afterAlpha = 255;
                        }
                    }
                }
                res = std::max(beforeAlpha, afterAlpha);
            }
        }
    }

    return res;
}

void FacesEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    //static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //wxStopWatch sw;
    uint8_t alpha = 255;
    if (SettingsMap.GetBool("CHECKBOX_Faces_SuppressWhenNotSinging", false)) {
        if (SettingsMap["CHOICE_Faces_TimingTrack"] != "") {
            alpha = CalculateAlpha(effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(), SettingsMap.GetInt("SPINCTRL_Faces_LeadFrames", 0), SettingsMap.GetBool("CHECKBOX_Faces_Fade", false), SettingsMap["CHOICE_Faces_TimingTrack"], buffer);
        }
    }

    if (SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default") == XLIGHTS_PGOFACES_FILE) {
        RenderCoroFacesFromPGO(buffer,
                               SettingsMap["CHOICE_Faces_Phoneme"],
                               SettingsMap.Get("CHOICE_Faces_Eyes", "Auto"),
                               SettingsMap.Get("CHOICE_Faces_EyeBlinkFrequency", "Normal"),
                               SettingsMap.GetBool("CHECKBOX_Faces_Outline"),
                               alpha, SettingsMap.GetBool("CHECKBOX_Faces_SuppressShimmer", false));
    } else {
        RenderFaces(buffer,
                    effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                    SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default"),
                    SettingsMap["CHOICE_Faces_Phoneme"],
                    SettingsMap["CHOICE_Faces_TimingTrack"],
                    SettingsMap["CHOICE_Faces_Eyes"],
                    SettingsMap["CHOICE_Faces_EyeBlinkFrequency"],
                    SettingsMap.GetBool("CHECKBOX_Faces_Outline"),
                    SettingsMap.GetBool("CHECKBOX_Faces_TransparentBlack", false),
                    SettingsMap.GetInt("TEXTCTRL_Faces_TransparentBlack", 0),
                    alpha,
                    SettingsMap.Get("CHOICE_Faces_UseState", ""),
                    SettingsMap.GetBool("CHECKBOX_Faces_SuppressShimmer", false));
    }

    //if (sw.TimeInMicro() > 2000) {
    //    logger_base.debug("Face effect frame render time: %lldus %s", sw.TimeInMicro(), (const char*)buffer.GetModel()->GetFullName().c_str());
    //}
}

void FacesEffect::RenderFaces(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, const std::string& eyeBlinkFreq, bool outline, uint8_t alpha, bool suppressShimmer) {
    if (alpha == 0)
        return; // 0 alpha means there is nothing to do

    static const std::map<wxString, int> phonemeMap = {
        { "AI", 0 },
        { "E", 1 },
        { "FV", 2 },
        { "L", 3 },
        { "MBP", 4 },
        { "O", 5 },
        { "U", 6 },
        { "WQ", 7 },
        { "etc", 8 },
        { "rest", 9 },
        { "(off)", 10 }
    };

    wxString pp = Phoneme;
    std::string p = pp.BeforeFirst('-');
    bool shimmer = !suppressShimmer && pp.Lower().EndsWith("-shimmer");

    std::map<wxString, int>::const_iterator it = phonemeMap.find(p);
    int PhonemeInt = 0;
    if (it != phonemeMap.end()) {
        PhonemeInt = it->second;
    }

    int Ht = buffer.BufferHt;
    int Wt = buffer.BufferWi;

    // this draws eyes as well
    drawoutline(buffer, PhonemeInt, outline, eyes, eyeBlinkFreq, buffer.BufferHt, buffer.BufferWi);
    mouth(buffer, PhonemeInt, Ht, Wt, shimmer); // draw a mouth syllable
}

bool FacesEffect::ShimmerState(RenderBuffer& buffer) const
{
    //return !((buffer.curPeriod - buffer.curEffStartPer) % 3 == 0);
    // This is frame rate independent
    return !((buffer.curPeriod - buffer.curEffStartPer) * buffer.frameTimeInMs % 200 >= 150);
}

//TODO: add params for eyes, outline
void FacesEffect::mouth(RenderBuffer& buffer, int Phoneme, int BufferHt, int BufferWi, bool shimmer) {
    if (shimmer) {
        // dont draw every third frame
        if (!ShimmerState(buffer))
            return;
    }

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
    switch (Phoneme) {
    case 0: // AI
        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        drawline1(buffer, Phoneme, x1, x2, y1, y4, 0);
        break;
    case 3:
    case 1: // E, L
        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        drawline1(buffer, Phoneme, x1, x2, y1, y3, 0);
        break;
    case 2: // FV
        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        drawline1(buffer, Phoneme, x1, x2, y1, y2 - 1, 0);
        break;
    case 4:
    case 9: //  MBP,rest

        drawline1(buffer, Phoneme, x1, x2, y1, y2, 0);
        break;
    case 5:
    case 6: // O,U,WQ
    case 7: {
        int xc = (int)(0.5 + Wt * 0.50);
        int yc = (int)(y2 - y5) / 2 + y5;
        double radius = (std::min(Wt, Ht)) * 0.15; // O
        if (Phoneme == 6)
            radius = (std::min(Wt, Ht)) * 0.10; // U
        if (Phoneme == 7)
            radius = (std::min(Wt, Ht)) * 0.05; // WQ
        facesCircle(buffer, Phoneme, xc, yc, radius, 0, 360, 0);
    } break;
    case 8: // WQ, etc
        drawline3(buffer, Phoneme, x3, x4, y5, y2, 0);
        break;
    default:
        break;
    }
}

void FacesEffect::drawline1(RenderBuffer& buffer, int Phoneme, int x1, int x2, int y1, int y2, int ColorIdx) {
    HSVValue hsv;

    buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

    for (int x = x1 + 1; x < x2; x++) {
        buffer.SetPixel(x, y2, hsv); // Turn pixel on
    }

    for (int y = y2 + 1; y <= y1; y++) {
        buffer.SetPixel(x1, y, hsv); // Left side of mouyh
        buffer.SetPixel(x2, y, hsv); // rightside
    }
}

void FacesEffect::drawline3(RenderBuffer& buffer, int Phoneme, int x1, int x2, int y6, int y7, int ColorIdx) {
    HSVValue hsv;
    buffer.palette.GetHSV(ColorIdx, hsv);

    for (int y = y6 + 1; y < y7; y++) {
        buffer.SetPixel(x1, y, hsv); // Left side of mouyh
        buffer.SetPixel(x2, y, hsv); // rightside
    }

    for (int x = x1 + 1; x < x2; x++) {
        buffer.SetPixel(x, y6, hsv); // Bottom
        buffer.SetPixel(x, y7, hsv); // Bottom
    }
}

/*
 faces draw circle
 */
void FacesEffect::facesCircle(RenderBuffer& buffer, int Phoneme, int xc, int yc, double radius, int start_degrees, int end_degrees, int colorIdx) {
    HSVValue hsv;
    buffer.palette.GetHSV(colorIdx, hsv);

    for (int degrees = start_degrees; degrees < end_degrees; degrees++) {
        double t = ((double)degrees * PI) / 180.0;
        int x = (int)((double)xc + radius * cos(t));
        int y = (int)((double)yc + radius * sin(t));
        buffer.SetPixel(x, y, hsv); // Bottom
    }
}

void FacesEffect::drawoutline(RenderBuffer& buffer, int Phoneme, bool outline, const std::string& eyes, const std::string& eyeBlinkFreqIn, int BufferHt, int BufferWi) {
    std::string eye = eyes;
    std::string eyeBlinkFreq = eyeBlinkFreqIn;

    FacesRenderCache* cache = (FacesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        int maxEyeDelay = GetMaxEyeDelay(eyeBlinkFreq);
        cache = new FacesRenderCache(intRand(0, maxEyeDelay));
        buffer.infoCache[id] = cache;
    }

    int Ht = BufferHt - 1;
    int Wt = BufferWi - 1;

    size_t colorcnt = buffer.GetColorCount();

    HSVValue hsvOutline;
    buffer.palette.GetHSV(1 % colorcnt, hsvOutline);

    //  DRAW EYES
    int start_degrees = 0;
    int end_degrees = 360;
    if (eye == "Auto") {
        if (Phoneme == 9 || Phoneme == 10) {
            if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                //calculate the blink time taking into account user selection
                int maxEyeDelay = GetMaxEyeDelay(eyeBlinkFreq);
                cache->nextBlinkTime += intRand(maxEyeDelay-1000, maxEyeDelay);
                cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; //100ms blink
                eye = "Closed";
            } else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                eye = "Closed";
            } else {
                eye = "Open";
            }
        } else {
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
        for (int y = 3; y < BufferHt - 3; y++) {
            buffer.SetPixel(0, y, hsvOutline);            // Left side of mouyh
            buffer.SetPixel(BufferWi - 1, y, hsvOutline); // rightside
        }
        for (int x = 3; x < BufferWi - 3; x++) {
            buffer.SetPixel(x, 0, hsvOutline);            // Bottom
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
    if (!FileExists(pgoFile)) return false;
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

void FacesEffect::RenderCoroFacesFromPGO(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, const std::string& eyeBlinkFreq, bool face_outline, uint8_t alpha, bool suppressShimmer)
{
    if (alpha == 0) return;

    //NOTE:
    //PixelBufferClass contains 2 RgbEffects members, which this method is a member of
    //xLightsFrame contains a PixelBufferClass member named buffer, which is derived from Model and gives the name of the model currently being used
    //therefore we can access the model info by going to parent object's buffer member

    if (!buffer.curPeriod) model_xy.clear(); //flush cache once at start of each effect

    static std::unordered_map<std::string, std::string> auto_phonemes {{"a-AI", "AI"}, {"a-E", "E"}, {"a-FV", "FV"}, {"a-L", "L"},
        {"a-MBP", "MBP"}, {"a-O", "O"}, {"a-U", "U"}, {"a-WQ", "WQ"}, {"a-etc", "etc"}, {"a-rest", "rest"}};

    if (auto_phonemes.find((const char*)Phoneme.c_str()) != auto_phonemes.end())
    {
        RenderFaces(buffer, auto_phonemes[(const char*)Phoneme.c_str()], eyes, eyeBlinkFreq, face_outline, alpha, suppressShimmer);
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

    if (!eyes.empty())
    {
        std::string eyesLower(eyes);
        std::transform(eyesLower.begin(), eyesLower.end(), eyesLower.begin(), ::tolower);

        std::string info = map[wxString::Format(wxT("Eyes_%s"), eyesLower.c_str()).ToStdString()];
        Model::ParseFaceElement(info, first_xy);

        info = map[wxString::Format(wxT("Eyes2_%s"), eyesLower.c_str()).ToStdString()];
        Model::ParseFaceElement(info, first_xy);

        info = map[wxString::Format(wxT("Eyes3_%s"), eyesLower.c_str()).ToStdString()];
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

void FacesEffect::RenderFaces(RenderBuffer& buffer,
                              SequenceElements* elements, const std::string& faceDef,
                              const std::string& Phoneme, const std::string& trackName,
                              const std::string& eyesIn, const std::string& eyeBlinkFreqIn, bool face_outline, bool transparentBlack, int transparentBlackLevel, uint8_t alpha, const std::string& outlineState, bool suppressShimmer)
{
    if (alpha == 0)
        return; // if alpha is zero dont bother.

    std::string eyes = eyesIn;
    std::string eyeBlinkFreq = eyeBlinkFreqIn;

    FacesRenderCache* cache = (FacesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        int maxEyeDelay = GetMaxEyeDelay(eyeBlinkFreq);
        cache = new FacesRenderCache(intRand(0, maxEyeDelay));

        buffer.infoCache[id] = cache;
    }

    if (buffer.needToInit) {
        buffer.needToInit = false;
        elements->AddRenderDependency(trackName, buffer.cur_model);
        cache->Clear();
    }

    if (buffer.cur_model == "") {
        return;
    }
    Model* model_info = buffer.GetPermissiveModel();
    if (model_info == nullptr) {
        return;
    }

    bool group = false;
    // if this is a submodel find the parent so we can find the face definition there
    if (model_info->GetDisplayAs() == "SubModel") {
        model_info = dynamic_cast<SubModel*>(model_info)->GetParent();
    } else if (model_info->GetDisplayAs() == "ModelGroup") {
        model_info = dynamic_cast<ModelGroup*>(model_info)->GetFirstModel();
        group = true;
        if (model_info == nullptr) {
            return;
        }
    }

    if (cache->nodeNameCache.empty()) {
        for (size_t x = 0; x < model_info->GetNodeCount(); x++) {
            cache->nodeNameCache[model_info->GetNodeName(x, true)] = x;
            if (model_info->GetNodeName(x, true) != wxString::Format("Node %d", (int)x + 1)) {
                cache->nodeNameCache[wxString::Format("Node %d", (int)x + 1)] = x;
            }
        }
    }

    std::string definition = faceDef;
    if ((definition == "Default" || definition == "") && !model_info->faceInfo.empty() && model_info->faceInfo.begin()->first != "") {
        definition = model_info->faceInfo.begin()->first;
    }

    bool found = true;
    auto it = model_info->faceInfo.find(definition);
    if (it == model_info->faceInfo.end()) {
        //not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model_info->faceInfo.find("SingleNode") != model_info->faceInfo.end()) {
            definition = "SingleNode";
            found = true;
        } else if ("SingleNode" == definition && model_info->faceInfo.find("Coro") != model_info->faceInfo.end()) {
            definition = "Coro";
            found = true;
        } else if (definition != "Default" && definition != "Rendered" && definition != "" && definition != "Matrix") {
            std::string firstFace = "";
            for (const auto& it2 : model_info->faceInfo) {
                if (it2.first != "") {
                    firstFace = it2.first;
                    break;
                }
            }
            if (firstFace != "") {
                definition = firstFace;
                found = true;
            }
        }
    }

    std::string modelType = found ? model_info->faceInfo[definition]["Type"] : definition;
    if (modelType == "") {
        modelType = definition;
    }

    int type = 3;

    if ("Coro" == modelType || "SingleNode" == modelType) {
        type = 0;
    } else if ("NodeRange" == modelType) {
        type = 1;
    } else if ("Rendered" == definition || "Default" == definition || "" == definition) {
        type = 2;
    }

    if (buffer.curEffStartPer == buffer.curPeriod) {
        if (modelType != "Matrix" && modelType != "Rendered" && modelType != "Default") {
            if (buffer.isTransformed) {
                static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.warn("Faces effect starting at %dms until %dms on model %s has a transformed buffer. This may not work as expected.", buffer.curEffStartPer * buffer.frameTimeInMs, buffer.curEffEndPer * buffer.frameTimeInMs, (const char*)buffer.cur_model.c_str());
            }
        }
    }

    if (group && type != 3) {//only picture type on a group make sense
        return;
    }

    std::string phoneme = Phoneme;

    if (phoneme == "") {
        Element* track = elements->GetElement(trackName);
        // GET Phoneme from timing track
        if (track == nullptr || track->GetEffectLayerCount() < 3) {
            phoneme = "rest";
            if ("Auto" == eyes) {
                if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                    //calculate the blink time taking into account user selection
                    int maxEyeDelay = GetMaxEyeDelay( eyeBlinkFreq );
                    cache->nextBlinkTime += intRand(maxEyeDelay-1000, maxEyeDelay);
                    cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; // 100ms blink
                    eyes = "Closed";
                } else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                    eyes = "Closed";
                } else {
                    eyes = "Open";
                }
            }
        } else {
            // Limit the lock for only as long as we access the timing track - this minimises contention ... especially when using faces effect on groups
            std::recursive_timed_mutex* lock = &track->GetChangeLock();
            std::unique_lock<std::recursive_timed_mutex> locker(*lock);

            int startms = -1;
            int endms = -1;

            EffectLayer* layer = track->GetEffectLayer(2);
            if (layer == nullptr) {
                phoneme = "rest";
                eyes = "Open";
            } else {
                std::unique_lock<std::recursive_mutex> locker2(layer->GetLock());
                int time = buffer.curPeriod * buffer.frameTimeInMs + 1;
                Effect* ef = layer->GetEffectByTime(time);
                if (ef == nullptr) {
                    phoneme = "rest";
                } else {
                    startms = ef->GetStartTimeMS();
                    endms = ef->GetEndTimeMS();
                    phoneme = ef->GetEffectName();
                }
                if ("Auto" == eyes && phoneme == "rest" && type != 2) {
                    if (startms == -1) {
                        // need to figure out the time
                        for (int x = 0; x < layer->GetEffectCount() && startms == -1; x++) {
                            ef = layer->GetEffect(x);
                            if (ef->GetStartTimeMS() > buffer.curPeriod * buffer.frameTimeInMs) {
                                endms = ef->GetStartTimeMS();
                                if (x > 0) {
                                    startms = layer->GetEffect(x - 1)->GetEndTimeMS();
                                } else {
                                    startms = 0;
                                }
                            }
                        }
                    }

                    if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                        if ((startms + 150) >= (buffer.curPeriod * buffer.frameTimeInMs)) {
                            // don't want to blink RIGHT at the start of the rest, delay a little bit
                            int tmp = (buffer.curPeriod * buffer.frameTimeInMs) + intRand(150, 549);

                            // also don't want it right at the end
                            if ((tmp + 130) > endms) {
                                cache->nextBlinkTime = (startms + endms) / 2;
                            } else {
                                cache->nextBlinkTime = tmp;
                            }
                        } else {
                            //calculate the blink time taking into account user selection
                            int maxEyeDelay = GetMaxEyeDelay(eyeBlinkFreq);
                            cache->nextBlinkTime += intRand(maxEyeDelay-1000, maxEyeDelay);
                            cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; // 100ms blink
                            eyes = "Closed";
                        }
                    } else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                        eyes = "Closed";
                    } else {
                        eyes = "Open";
                    }
                }
            }
        }
    } else if (phoneme == "rest" || phoneme == "(off)") {
            if ("Auto" == eyes) {
                if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                    //calculate the blink time, taking into account user selection
                    int maxEyeDelay = GetMaxEyeDelay(eyeBlinkFreq);
                    cache->nextBlinkTime += intRand(maxEyeDelay-1000, maxEyeDelay);
                    cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; // 100ms blink
                    eyes = "Closed";
                } else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                    eyes = "Closed";
                } else {
                    eyes = "Open";
                }
            }
    }

    int colorOffset = 0;
    xlColor color;
    buffer.palette.GetColor(0, color); //use first color for mouth; user must make sure it matches model node type

    bool customColor = found ? model_info->faceInfo[definition]["CustomColors"] == "1" : false;

    wxString pp = phoneme;
    std::string p = pp.BeforeFirst('-');
    bool shimmer = !suppressShimmer && pp.Lower().EndsWith("-shimmer");

    std::vector<std::string> todo;
    std::vector<xlColor> colors;
    if (p != "(off)") {
        todo.push_back("Mouth-" + p);
        colorOffset = 1;
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Mouth-" + p + "-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            } else {
                colors.push_back(xlColor(cname));
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            }
        } else {
            colors.push_back(color);
            colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
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
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            } else {
                colors.push_back(xlColor(cname));
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            }
        } else {
            colors.push_back(color);
            colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
        }

        todo.push_back("Eyes-Open2");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Eyes-Open2-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            } else {
                colors.push_back(xlColor(cname));
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            }
        } else {
            if (buffer.palette.Size() > colorOffset + 3) {
                buffer.palette.GetColor(colorOffset + 3, color); //use fifth colour
            }
            colors.push_back(color);
            colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
        }

        todo.push_back("Eyes-Open3");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Eyes-Open3-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            } else {
                colors.push_back(xlColor(cname));
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            }
        } else {
            if (buffer.palette.Size() > colorOffset + 4) {
                buffer.palette.GetColor(colorOffset + 4, color); //use sixth colour
            }
            colors.push_back(color);
            colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
        }
    } else if (eyes == "Closed") {
        todo.push_back("Eyes-Closed");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Eyes-Closed-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            } else {
                colors.push_back(xlColor(cname));
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            }
        } else {
            colors.push_back(color);
            colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
        }

        todo.push_back("Eyes-Closed2");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Eyes-Closed2-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            } else {
                colors.push_back(xlColor(cname));
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            }
        } else {
            if (buffer.palette.Size() > colorOffset + 3) {
                buffer.palette.GetColor(colorOffset + 3, color); //use fifth colour
            }
            colors.push_back(color);
            colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
        }

        todo.push_back("Eyes-Closed3");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition]["Eyes-Closed3-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            } else {
                colors.push_back(xlColor(cname));
                colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
            }
        } else {
            if (buffer.palette.Size() > colorOffset + 4) {
                buffer.palette.GetColor(colorOffset + 4, color); //use sixth colour
            }
            colors.push_back(color);
            colors.back().alpha = ((int)alpha * colors.back().alpha) / 255;
        }
    } else if (eyes == "(off)") {
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
                colors.front().alpha = ((int)alpha * colors.front().alpha) / 255;
            } else {
                colors.insert(colors.begin(), xlColor(cname));
                colors.front().alpha = ((int)alpha * colors.front().alpha) / 255;
            }
        } else {
            colors.insert(colors.begin(), color);
            colors.front().alpha = ((int)alpha * colors.front().alpha) / 255;
        }

        if (buffer.palette.Size() > (2 + colorOffset)) {
            buffer.palette.GetColor((2 + colorOffset), color); //use forth color for outline 2; user must make sure it matches model node type
        }

        todo.insert(todo.begin(), "FaceOutline2");
        if (customColor) {
            std::string const cname = model_info->faceInfo[definition]["FaceOutline2-Color"];
            if (cname == "") {
                colors.insert(colors.begin(), xlWHITE);
                colors.front().alpha = ((int)alpha * colors.front().alpha) / 255;
            } else {
                colors.insert(colors.begin(), xlColor(cname));
                colors.front().alpha = ((int)alpha * colors.front().alpha) / 255;
            }
        } else {
            colors.insert(colors.begin(), color);
            colors.front().alpha = ((int)alpha * colors.front().alpha) / 255;
        }
    }

    if (type == 2) {
        RenderFaces(buffer, phoneme, eyes, eyeBlinkFreq, face_outline, alpha, suppressShimmer);
        return;
    }
    if (type == 3) {
        // picture
        std::string e = eyes;
        if (eyes == "Auto") {
            e = "Open";
        }
        if (eyes == "(off)") {
            e = "Closed";
        }
        std::string key = "Mouth-" + p + "-Eyes";
        std::string picture = "";
        if (model_info->faceInfo[definition].find(key + e) != model_info->faceInfo[definition].end()) {
            picture = model_info->faceInfo[definition][key + e];
            if (shimmer) {
                if (!ShimmerState(buffer)) {
                    picture = model_info->faceInfo[definition]["Mouth-rest-Eyes" + e];
                }
            }
        }
        if (picture == "" && e == "Closed") {
            if (model_info->faceInfo[definition].find(key + "Open") != model_info->faceInfo[definition].end()) {
                picture = model_info->faceInfo[definition][key + "Open"];
                if (shimmer) {
                    if (!ShimmerState(buffer)) {
                        picture = model_info->faceInfo[definition]["Mouth-rest-EyesOpen"];
                    }
                }
            }
        }
        std::string dirstr = "none"; /*RENDER_PICTURE_NONE*/
        std::string stf = "Scale To Fit";
        if (model_info->faceInfo[definition]["ImagePlacement"] == "Centered") {
            stf = "No Scaling";
        } else if (model_info->faceInfo[definition]["ImagePlacement"] == "Scale Keep Aspect Ratio" ||
                   model_info->faceInfo[definition]["ImagePlacement"] == "Scale Keep Aspect Ratio Crop") {
            stf = model_info->faceInfo[definition]["ImagePlacement"];
        }
        RenderBuffer* crb = cache->GetImage(MakeKey(buffer.BufferWi, buffer.BufferHt, dirstr, picture, stf));
        if (crb == nullptr) {
            crb = new RenderBuffer(buffer);
            PicturesEffect::Render(*crb, dirstr, picture, 0, 0, 0, 0, 0, 0, 100, 100, stf, false, false, false, true, false, false, 0); // set for scale to fit
            cache->AddImage(MakeKey(buffer.BufferWi, buffer.BufferHt, dirstr, picture, stf), crb);
        }

        for (int y = 0; y < buffer.BufferHt; y++) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                if (transparentBlack) {
                    auto c = crb->GetPixel(x, y);
                    int level = c.Red() + c.Green() + c.Blue();
                    if (level > transparentBlackLevel) {
                        c.alpha = ((int)alpha * c.alpha) / 255;
                        buffer.SetPixel(x, y, c);
                    }
                } else {
                    auto c = crb->GetPixel(x, y);
                    c.alpha = ((int)alpha * c.alpha) / 255;
                    buffer.SetPixel(x, y, c);
                }
            }
        }
    }

    for (size_t t = 0; t < todo.size(); t++) {
        if (shimmer && StartsWith(todo[t], "Mouth-")) {
            if (!ShimmerState(buffer))
                continue;
        }
        if (type == 1) {
            for (const auto it : model_info->faceInfoNodes[definition][todo[t]]) {
                buffer.SetNodePixel(it, colors[t], true);
            }
        } else {
            std::string channels = model_info->faceInfo[definition][todo[t]];
            wxStringTokenizer wtkz(channels, ",");
            while (wtkz.HasMoreTokens()) {
                wxString valstr = wtkz.GetNextToken();
                if (type == 0) {
                    auto it2 = cache->nodeNameCache.find(valstr.ToStdString());
                    if (it2 != cache->nodeNameCache.end()) {
                        int n = it2->second;
                        buffer.SetNodePixel(n, colors[t], true);
                    }
                }
            }
        }

        if (todo[t] == "FaceOutline" && outlineState != "") {
            auto sts = model_info->stateInfo[outlineState];
            if (sts["CustomColors"] == "1") {
                if (sts["Type"] == "NodeRange") {
                    for (size_t i = 1; i <= 40; i++) {
                        auto r = sts[wxString::Format("s%d", (int)i)];
                        auto c = sts[wxString::Format("s%d-Color", (int)i)];
                        if (r != "") {
                            xlColor colour = xlColor(c);
                            if (c.empty()) {
                                colour = xlWHITE;
                            }
                            colour.alpha = ((int)alpha * colour.alpha) / 255;

                            // use the nodes as it is faster
                            for (const auto it : model_info->stateInfoNodes[outlineState][wxString::Format("s%d", (int)i)]) {
                                buffer.SetNodePixel(it, colour, true);
                            }
                        }
                    }
                }
            }
        }
    }
}
