/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/tokenzr.h>

#include "StateEffect.h"
#include "StatePanel.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"
#include "../models/SubModel.h"
#include "../sequencer/Effect.h"
#include "../sequencer/SequenceElements.h"

#include "../../include/state-16.xpm"
#include "../../include/state-64.xpm"

#include <log4cpp/Category.hh>

StateEffect::StateEffect(int id) :
    RenderableEffect(id, "State", state_16, state_64, state_64, state_64, state_64) {
    // ctor
}

StateEffect::~StateEffect() {
    // dtor
}

std::list<std::string> StateEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) {
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    SubModel* sm = dynamic_cast<SubModel*>(model);
    if (sm != nullptr) {
        res.push_back(wxString::Format("    ERR: State effect on SubModel will not render properly. Model '%s', Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    // -Buffer not rotated
    wxString bufferTransform = settings.Get("B_CHOICE_BufferTransform", "None");

    if (bufferTransform != "None") {
        res.push_back(wxString::Format("    WARN: State effect with transformed buffer '%s' may not render correctly. Model '%s', Start %s", model->GetFullName(), bufferTransform, FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    wxString timing = settings.Get("E_CHOICE_State_TimingTrack", "");
    wxString state = settings.Get("E_CHOICE_State_State", "");

    // - Face chosen or specific phoneme
    if (state == "" && timing == "") {
        res.push_back(wxString::Format("    ERR: State effect with no timing selected. Model '%s', Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    } else if (timing != "" && GetTiming(timing.ToStdString()) == nullptr) {
        res.push_back(wxString::Format("    ERR: State effect with unknown timing (%s) selected. Model '%s', Start %s", timing, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    return res;
}

std::list<std::string> StateEffect::GetStatesUsed(const SettingsMap& SettingsMap) {
    std::list<std::string> res;
    auto state = SettingsMap.Get("E_CHOICE_State_StateDefinition", "");
    if (state != "") {
        res.emplace_back(state);
    }
    return res;
}

void StateEffect::SetPanelStatus(Model* cls) {
    StatePanel* fp = (StatePanel*)panel;
    if (fp == nullptr) {
        return;
    }

    auto lastTiming = fp->Choice_State_TimingTrack->GetStringSelection();
    auto lastState = fp->Choice_StateDefinitonChoice->GetStringSelection();
    fp->Choice_State_TimingTrack->Clear();
    fp->Choice_StateDefinitonChoice->Clear();

    for (const auto& it : wxSplit(GetTimingTracks(1), '|')) {
        fp->Choice_State_TimingTrack->Append(it);
    }

    if (fp->Choice_State_TimingTrack->GetCount() > 0) {
        fp->Choice_State_TimingTrack->SetSelection(0);
    }

    if (cls != nullptr) {
        Model* m = cls;
        if (cls->GetDisplayAs() == "ModelGroup") {
            m = ((ModelGroup*)cls)->GetFirstModel();
        }

        std::list<std::string> used;
        if (m != nullptr) {
            for (const auto& it : m->GetStateInfo()) {
                if (std::find(begin(used), end(used), it.first) == end(used)) {
                    fp->Choice_StateDefinitonChoice->Append(it.first);
                    used.push_back(it.first);
                }
            }
        }
    }

    if (lastTiming != "")
        fp->Choice_State_TimingTrack->SetStringSelection(lastTiming);
    if (lastState != "") {
        fp->Choice_StateDefinitonChoice->SetStringSelection(lastState);
    }

    if (fp->Choice_StateDefinitonChoice->GetSelection() == -1 && fp->Choice_StateDefinitonChoice->GetCount() > 0) {
        fp->Choice_StateDefinitonChoice->SetSelection(0);
    }

    fp->SetEffect(this, cls);
}

std::list<std::string> StateEffect::GetStates(Model* cls, std::string model) {
    std::list<std::string> res;

    if (cls != nullptr) {
        Model* m = cls;
        if (cls->GetDisplayAs() == "ModelGroup") {
            m = ((ModelGroup*)cls)->GetFirstModel();
        }

        if (m != nullptr) {
            for (const auto& it : m->GetStateInfo()) {
                if (model == it.first) {
                    for (const auto& it2 : it.second) {
                        wxString f(it2.first);
                        if (f.EndsWith("-Name") && it2.second != "" && std::find(begin(res), end(res), it2.second) == end(res)) {
                            res.push_back(it2.second);
                        }
                    }
                }
            }
        }
    }

    return res;
}

xlEffectPanel* StateEffect::CreatePanel(wxWindow* parent) {
    return new StatePanel(parent);
}

void StateEffect::SetDefaultParameters() {
    StatePanel* sp = (StatePanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->SetEffect(nullptr, nullptr);
    SetChoiceValue(sp->Choice_State_Mode, "Default");
    SetChoiceValue(sp->Choice_State_Color, "Graduate");
    sp->Choice_StateDefinitonChoice->SetSelection(0);
    SetRadioValue(sp->RadioButton1);
    sp->ValidateWindow();
}

void StateEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) {
    wxString timing = effect->GetSettings().Get("E_CHOICE_State_TimingTrack", "");

    if (timing.ToStdString() == oldname) {
        effect->GetSettings()["E_CHOICE_State_TimingTrack"] = wxString(newname);
    }
}

void StateEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    RenderState(buffer,
                effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                SettingsMap.Get("CHOICE_State_StateDefinition", ""),
                SettingsMap["CHOICE_State_State"],
                SettingsMap["CHOICE_State_TimingTrack"],
                SettingsMap["CHOICE_State_Mode"],
                SettingsMap["CHOICE_State_Color"],
                SettingsMap.GetInt("SLIDER_State_Fade_Time", 0));
}

std::string StateEffect::FindState(std::map<std::string, std::string>& map, std::string name) {
    for (const auto& it2 : map) {
        if (EndsWith(it2.first, "-Name") && it2.second == name) {
            return it2.first.substr(0, it2.first.size() - 5);
        }
    }

    return "";
}

static const std::string &findKey(const std::map<std::string, std::string> &m, const std::string &k, const std::string &dv = xlEMPTY_STRING) {
    const auto &v = m.find(k);
    if (v == m.end()) {
        return dv;
    }
    return v->second;
}

static std::list<int> const& findNodeKey(std::map<std::string, std::list<int>> const& m, std::string const& k) {
    const auto& v = m.find(k);
    if (v == m.end()) {
        static std::list<int> emptyList;
        return emptyList;
    }
    return v->second;
}

void StateEffect::RenderState(RenderBuffer& buffer,
                              SequenceElements* elements, const std::string& faceDefinition,
                              const std::string& Phoneme, const std::string& trackName, const std::string& mode, const std::string& colourmode, int fadeTime) {
    if (buffer.needToInit) {
        buffer.needToInit = false;
        elements->AddRenderDependency(trackName, buffer.cur_model);

        if (buffer.isTransformed) {
            log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("State effect starting at %dms until %dms on model %s has a transformed buffer. This may not work as expected.", buffer.curEffStartPer * buffer.frameTimeInMs, buffer.curEffEndPer * buffer.frameTimeInMs, (const char*)buffer.cur_model.c_str());
        }
    }

    if (buffer.cur_model.empty()) {
        return;
    }

    const Model* model_info = buffer.GetModel();
    std::string definition = faceDefinition;
    bool found = true;
    std::map<std::string, std::map<std::string, std::string>>::const_iterator it = model_info->GetStateInfo().find(definition);
    if (it == model_info->GetStateInfo().end()) {
        // not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model_info->GetStateInfo().find("SingleNode") != model_info->GetStateInfo().end()) {
            definition = "SingleNode";
            found = true;
        } else if ("SingleNode" == definition && model_info->GetStateInfo().find("Coro") != model_info->GetStateInfo().end()) {
            definition = "Coro";
            found = true;
        }
    }

    if (definition.empty()) {
        return;
    }
    std::map<std::string, std::string> emptyMap;
    const std::map<std::string, std::string>& definitionSi = found ? model_info->GetStateInfo().find(definition)->second : emptyMap;
    std::string modelType = findKey(definitionSi, "Type", definition);
    if (modelType == "") {
        modelType = definition;
    }

    int type = 1;

    if ("SingleNode" == modelType) {
        type = 0;
    } else if ("NodeRange" == modelType) {
        type = 1;
    }

    std::string tstates = Phoneme;
    int intervalnumber = 0;
    // GET label from timing track
    int startms = -1;
    int endms = -1;
    int posms = -1;

    if (tstates == "") {
        Element* track = elements->GetElement(trackName);

        // if we dont have a track then exit
        if (track == nullptr) {
            return;
        }

        std::recursive_timed_mutex* lock = &track->GetChangeLock();
        std::unique_lock<std::recursive_timed_mutex> locker(*lock);

        EffectLayer* layer = track->GetEffectLayer(0);
        int time = buffer.curPeriod * buffer.frameTimeInMs + 1;
        posms = buffer.curPeriod * buffer.frameTimeInMs;
        Effect* ef = layer->GetEffectByTime(time);
        if (ef == nullptr) {
            tstates = "";
        } else {
            startms = ef->GetStartTimeMS();
            endms = ef->GetEndTimeMS();
            tstates = ef->GetEffectName();
        }

        ef = layer->GetEffectByTime(buffer.curEffStartPer * buffer.frameTimeInMs + 1);
        while (ef != nullptr && ef->GetStartTimeMS() <= time) {
            intervalnumber++;
            int endtime = ef->GetEndTimeMS();
            ef = layer->GetEffectByTime(endtime + 1);
            if (ef == nullptr) {
                ef = layer->GetEffectAfterTime(endtime + 1);
            }
        }
    }
    uint8_t const alpha = CalculateAlpha(fadeTime,posms, startms, endms, buffer);
    std::vector<std::string> sstates;

    if (mode == "Default" || startms == -1) {
        wxString ss = wxString(tstates);
        wxStringTokenizer tkz(ss, wxT(" ,;:"));
        while (tkz.HasMoreTokens()) {
            wxString token = tkz.GetNextToken();
            if (token == "*" || token == "<ALL>") {
                for (auto it2 : definitionSi) {
                    if (EndsWith(it2.first, "-Name") && it2.second != "") {
                        sstates.push_back(Lower(it2.second));
                    }
                }
            } else {
                sstates.push_back(token.Lower().ToStdString());
            }
        }
    } else if (mode == "Countdown") {
        // tstates should contain the starting number
        int val = wxAtoi(tstates);

        val = val * 1000;
        int subtracttime = (posms - startms);
        val = val - subtracttime;
        val = val / 1000;

        int v = val;
        bool force = false;
        if ((v / 1000) * 1000 > 0) {
            sstates.push_back(wxString::Format("%d", (v / 1000) * 1000).ToStdString());
            force = true;
        }
        v = v - (v / 1000) * 1000;
        if ((v / 100) * 100 > 0) {
            sstates.push_back(wxString::Format("%d", (v / 100) * 100).ToStdString());
            force = true;
        } else {
            if (force) {
                sstates.push_back("000");
            }
        }
        v = v - (v / 100) * 100;
        if ((v / 10) * 10 > 0) {
            sstates.push_back(wxString::Format("%d", (v / 10) * 10).ToStdString());
        } else {
            if (force) {
                sstates.push_back("00");
            }
        }
        v = v - (v / 10) * 10;
        sstates.push_back(wxString::Format("%d", v).ToStdString());
    } else if (mode == "Time Countdown") {
        wxDateTime dt;
        dt.ParseFormat(tstates.c_str(), "%H:%M:%S");

        if (!dt.IsValid()) {
            dt.ParseFormat(tstates.c_str(), "%M:%S");
        }

        if (dt.IsValid()) {
            dt.Subtract(wxTimeSpan(0, 0, 0, (buffer.curPeriod - buffer.curEffStartPer) * buffer.frameTimeInMs));
            int m = dt.GetMinute();
            if ((m / 10) * 1000 > 0) {
                sstates.push_back(wxString::Format("%d", (m / 10) * 1000).ToStdString());
            } else {
                sstates.push_back("0000");
            }
            m = m - (m / 10) * 10;
            if (m * 100 > 0) {
                sstates.push_back(wxString::Format("%d", m * 100).ToStdString());
            } else {
                sstates.push_back("000");
            }
            int s = dt.GetSecond();
            if ((s / 10) * 10 > 0) {
                sstates.push_back(wxString::Format("%d", (s / 10) * 10).ToStdString());
            } else {
                sstates.push_back("00");
            }
            s = s - (s / 10) * 10;
            sstates.push_back(wxString::Format("%d", s).ToStdString());
        }
        sstates.push_back("colon");
    } else if (mode == "Number") // used for FM frequencies
    {
        double f = wxAtof(tstates);
        sstates.push_back("dot");
        double f2 = f - int(f);
        f2 = (int)(f2 * 10 + 0.5);
        sstates.push_back(wxString::Format("%d", (int)f2).ToStdString());

        int v = f;
        bool force = false;
        if ((v / 100) * 1000 > 0) {
            sstates.push_back(wxString::Format("%d", (v / 100) * 1000).ToStdString());
            force = true;
        }
        v = v - (v / 100) * 100;
        if ((v / 10) * 100 > 0) {
            sstates.push_back(wxString::Format("%d", (v / 10) * 100).ToStdString());
        } else {
            if (force) {
                sstates.push_back("000");
            }
        }
        v = v - (v / 10) * 10;
        if (v * 10 > 0) {
            sstates.push_back(wxString::Format("%d", v * 10).ToStdString());
        } else {
            sstates.push_back("00");
        }
    } else if (mode == "Iterate") {
        float progressthroughtimeinterval = ((float)posms - (float)startms) / ((float)endms - (float)startms);

        std::vector<std::string> tmpstates;
        wxString ss = wxString(tstates);
        wxStringTokenizer tkz(ss, wxT(" ,;:"));
        while (tkz.HasMoreTokens()) {
            wxString token = tkz.GetNextToken();
            if (token == "*" || token == "<ALL>") {
                for (auto it2 : definitionSi) {
                    if (EndsWith(it2.first, "-Name") && it2.second != "") {
                        sstates.push_back(Lower(it2.second));
                    }
                }
            } else {
                tmpstates.push_back(token.Lower().ToStdString());
            }
        }

        int which = tmpstates.size() * progressthroughtimeinterval;

        if (which < tmpstates.size()) {
            sstates.push_back(tmpstates[which]);
        }
    }

    bool customColor = found ? findKey(definitionSi, "CustomColors") == "1" : false;

    // process each token
    for (size_t i = 0; i < sstates.size(); i++) {
        for (const auto& it : definitionSi) {
            if (it.second == sstates[i] && EndsWith(it.first, "-Name")) {
                // get the channels
                std::string statename = BeforeFirst(it.first, '-'); // FindState(model_info->stateInfo[definition], sstates[i]);
                std::string channels = findKey(definitionSi, statename);

                if (statename != "" && channels != "") {
                    xlColor color;
                    if (colourmode == "Graduate") {
                        buffer.GetMultiColorBlend(buffer.GetEffectTimeIntervalPosition(), false, color);
                    } else if (colourmode == "Cycle") {
                        buffer.palette.GetColor((intervalnumber - 1) % buffer.GetColorCount(), color);
                    } else {
                        // allocate
                        int statenum = wxAtoi(statename.substr(1));
                        buffer.palette.GetColor((statenum - 1) % buffer.GetColorCount(), color);
                    }
                    if (customColor) {
                        std::string cname = findKey(definitionSi, statename + "-Color");
                        if (cname == "") {
                            color = xlWHITE;
                        } else {
                            color = xlColor(cname);
                        }
                    }
                    color.alpha = ((int)alpha * color.alpha) / 255;
                    if (type == 1) {
                        for (const auto it : findNodeKey(model_info->GetStateInfoNodes().at(definition), statename)) {
                            buffer.SetNodePixel(it, color, true);
                        }
                    } else {
                        wxStringTokenizer wtkz(channels, ",");
                        while (wtkz.HasMoreTokens()) {
                            wxString valstr = wtkz.GetNextToken();

                            if (type == 0) {
                                for (size_t n = 0; n < model_info->GetNodeCount(); n++) {
                                    wxString nn = model_info->GetNodeName(n, true);
                                    if (nn == valstr) {
                                        buffer.SetNodePixel(n, color, true);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

uint8_t StateEffect::CalculateAlpha(int fadeTime, int currentTime, int startTime, int endTime, RenderBuffer& buffer) {
    if (0 == fadeTime || -1 == currentTime || -1 == startTime || -1 == endTime) {
        return 255;
    }
    uint8_t beforeAlpha = 0;
    uint8_t afterAlpha = 0;
    if (endTime - currentTime < fadeTime) {                       
        beforeAlpha = ((endTime - currentTime) * 255) / fadeTime;                        
    } else if (currentTime + buffer.frameTimeInMs - startTime < fadeTime) { // buffer.frameTimeInMs
        afterAlpha = ((currentTime + buffer.frameTimeInMs - startTime) * 255) / fadeTime;
    } else {
        return 255;
    }         
    return std::max(beforeAlpha, afterAlpha);
}