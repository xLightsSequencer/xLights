#include <wx/tokenzr.h>

#include "StateEffect.h"
#include "StatePanel.h"
#include "../models/Model.h"
#include "../sequencer/SequenceElements.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../../include/state-16.xpm"
#include "../../include/state-64.xpm"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

StateEffect::StateEffect(int id) : RenderableEffect(id, "State", state_16, state_64, state_64, state_64, state_64)
{
    //ctor
}

StateEffect::~StateEffect()
{
    //dtor
}

std::list<std::string> StateEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    // -Buffer not rotated
    wxString bufferTransform = settings.Get("B_CHOICE_BufferTransform", "None");

    if (bufferTransform != "None")
    {
        res.push_back(wxString::Format("    WARN: State effect with transformed buffer '%s' may not render correctly. Model '%s', Start %s", model->GetName(), bufferTransform, FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    wxString timing = settings.Get("E_CHOICE_State_TimingTrack", "");
    wxString state = settings.Get("E_CHOICE_State_State", "");

    // - Face chosen or specific phoneme
    if (state == "" && timing == "")
    {
        res.push_back(wxString::Format("    ERR: State effect with no timing selected. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

std::list<std::string> StateEffect::GetStatesUsed(const SettingsMap& SettingsMap)
{
    std::list<std::string> res;
    auto state = SettingsMap.Get("E_CHOICE_State_StateDefinition", "");
    if (state != "")
    {
        res.emplace_back(state);
    }
    return res;
}

void StateEffect::SetPanelStatus(Model *cls) {
    StatePanel *fp = (StatePanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->Choice_State_TimingTrack->Clear();
    fp->Choice_StateDefinitonChoice->Clear();
    if (mSequenceElements == nullptr) {
        return;
    }

    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
        if (mSequenceElements->GetElement(i)->GetEffectLayerCount() == 1
            && mSequenceElements->GetElement(i)->GetType() == ELEMENT_TYPE_TIMING) {
            fp->Choice_State_TimingTrack->Append(mSequenceElements->GetElement(i)->GetName());
        }
    }

    if (fp->Choice_State_TimingTrack->GetCount() > 0)
    {
        fp->Choice_State_TimingTrack->SetSelection(0);
    }

    if (cls != nullptr) {
        for (std::map<std::string, std::map<std::string, std::string> >::iterator it = cls->stateInfo.begin(); it != cls->stateInfo.end(); ++it) {
            if (it->second.size() > 30) // actually it should be about 120
            {
                fp->Choice_StateDefinitonChoice->Append(it->first);
            }
        }
    }

    if (fp->Choice_StateDefinitonChoice->GetCount() > 0)
    {
        fp->Choice_StateDefinitonChoice->SetSelection(0);
    }

    fp->SetEffect(this, cls);
}

std::list<std::string> StateEffect::GetStates(Model *cls, std::string model) {

    std::list<std::string> res;

    if (cls != nullptr) {
        for (std::map<std::string, std::map<std::string, std::string> >::iterator it = cls->stateInfo.begin(); it != cls->stateInfo.end(); ++it)
        {
            if (model == it->first)
            {
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    wxString f(it2->first);
                    if (f.EndsWith("-Name") && it2->second != "")
                    {
                        res.push_back(it2->second);
                    }
                }
            }
        }
    }

    return res;
}

wxPanel *StateEffect::CreatePanel(wxWindow *parent) {
    return new StatePanel(parent);
}

void StateEffect::SetDefaultParameters() {
    StatePanel *sp = (StatePanel*)panel;
    if (sp == nullptr) {
        return;
    }

    SetChoiceValue(sp->Choice_State_Mode, "Default");
    SetChoiceValue(sp->Choice_State_Color, "Graduate");
}

void StateEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_State_TimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_State_TimingTrack"] = wxString(newname);
    }
}

void StateEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    RenderState(buffer,
                effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                SettingsMap.Get("CHOICE_State_StateDefinition", ""),
                SettingsMap["CHOICE_State_State"],
                SettingsMap["CHOICE_State_TimingTrack"],
                SettingsMap["CHOICE_State_Mode"],
                SettingsMap["CHOICE_State_Color"]
    );
}

std::string StateEffect::FindState(std::map<std::string, std::string>& map, std::string name)
{
    for (auto it2 : map)
    {
        if (EndsWith(it2.first, "-Name") && it2.second == name)
        {
            return it2.first.substr(0, it2.first.size() - 5);
        }
    }

    return "";
}

void StateEffect::RenderState(RenderBuffer &buffer,
                             SequenceElements *elements, const std::string &faceDefinition,
                             const std::string& Phoneme, const std::string &trackName, const std::string& mode, const std::string& colourmode)
{
    if (buffer.needToInit) {
        buffer.needToInit = false;
        elements->AddRenderDependency(trackName, buffer.cur_model);
    
        if (buffer.isTransformed)
        {
            log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("State effect starting at %dms until %dms on model %s has a transformed buffer. This may not work as expected.", buffer.curEffStartPer * buffer.frameTimeInMs, buffer.curEffEndPer * buffer.frameTimeInMs, (const char *)buffer.cur_model.c_str());
        }
    }

    Element *track = elements->GetElement(trackName);
    std::recursive_mutex tmpLock;
    std::recursive_mutex *lock = &tmpLock;
    if (track != nullptr) {
        lock = &track->GetChangeLock();
    }

    std::unique_lock<std::recursive_mutex> locker(*lock);

    if (buffer.cur_model == "") {
        return;
    }

    Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }

    std::string definition = faceDefinition;
    bool found = true;
    std::map<std::string, std::map<std::string, std::string> >::iterator it = model_info->stateInfo.find(definition);
    if (it == model_info->stateInfo.end()) {
        //not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model_info->stateInfo.find("SingleNode") != model_info->stateInfo.end()) {
            definition = "SingleNode";
            found = true;
        } else if ("SingleNode" == definition && model_info->stateInfo.find("Coro") != model_info->stateInfo.end()) {
            definition = "Coro";
            found = true;
        }
    }

    if (definition == "")
    {
        return;
    }

    std::string modelType = found ? model_info->stateInfo[definition]["Type"] : definition;
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
    //GET label from timing track
    int startms = -1;
    int endms = -1;
    int posms = -1;

    if (tstates == "") {

        // if we dont have a track then exit
        if (track == nullptr)
        {
            return;
        }

        EffectLayer *layer = track->GetEffectLayer(0);
        int time = buffer.curPeriod * buffer.frameTimeInMs + 1;
        posms = buffer.curPeriod * buffer.frameTimeInMs;
        Effect *ef = layer->GetEffectByTime(time);
        if (ef == nullptr) {
            tstates = "";
        }
        else {
            startms = ef->GetStartTimeMS();
            endms = ef->GetEndTimeMS();
            tstates = ef->GetEffectName();
        }

        ef = layer->GetEffectByTime(buffer.curEffStartPer * buffer.frameTimeInMs + 1);
        while (ef != nullptr && ef->GetStartTimeMS() <= time)
        {
            intervalnumber++;
            int endtime = ef->GetEndTimeMS();
            ef = layer->GetEffectByTime(endtime + 1);
            if (ef == nullptr)
            {
                ef = layer->GetEffectAfterTime(endtime + 1);
            }
        }
    }

    std::vector<std::string> sstates;

    if (mode == "Default" || startms == -1)
    {
        wxString ss = wxString(tstates);
        wxStringTokenizer tkz(ss, wxT(" ,;:"));
        while (tkz.HasMoreTokens())
        {
            wxString token = tkz.GetNextToken();
            if (token == "*" || token == "<ALL>")
            {
                for (auto it2 : model_info->stateInfo[definition])
                {
                    if (EndsWith(it2.first, "-Name") && it2.second != "")
                    {
                        sstates.push_back(Lower(it2.second));
                    }
                }
            }
            else
            {
                sstates.push_back(token.Lower().ToStdString());
            }
        }
    }
    else if (mode == "Countdown")
    {
        // tstates should contain the starting number
        int val = wxAtoi(tstates);

        val = val * 1000;
        int subtracttime = (posms - startms);
        val = val - subtracttime;
        val = val / 1000;

        int v = val;
        bool force = false;
        if ((v / 1000) * 1000 > 0)
        {
            sstates.push_back(wxString::Format("%d", (v / 1000) * 1000).ToStdString());
            force = true;
        }
        v = v - (v / 1000) * 1000;
        if ((v / 100) * 100 > 0)
        {
            sstates.push_back(wxString::Format("%d", (v / 100) * 100).ToStdString());
            force = true;
        }
        else
        {
            if (force)
            {
                sstates.push_back("000");
            }
        }
        v = v - (v / 100) * 100;
        if ((v / 10) * 10 > 0)
        {
            sstates.push_back(wxString::Format("%d", (v / 10) * 10).ToStdString());
        }
        else
        {
            if (force)
            {
                sstates.push_back("00");
            }
        }
        v = v - (v / 10) * 10;
        sstates.push_back(wxString::Format("%d", v).ToStdString());
    }
    else if (mode == "Time Countdown")
    {
        wxDateTime dt;
        dt.ParseFormat(tstates.c_str(), "%H:%M:%S");

        if (!dt.IsValid())
        {
            dt.ParseFormat(tstates.c_str(), "%M:%S");
        }

        if (dt.IsValid())
        {
            dt.Subtract(wxTimeSpan(0, 0, 0, (buffer.curPeriod - buffer.curEffStartPer) * buffer.frameTimeInMs));
            int m = dt.GetMinute();
            if ((m / 10) * 1000 > 0)
            {
                sstates.push_back(wxString::Format("%d", (m / 10) * 1000).ToStdString());
            }
            else
            {
                sstates.push_back("0000");
            }
            m = m - (m / 10) * 10;
            if (m * 100 > 0)
            {
                sstates.push_back(wxString::Format("%d", m * 100).ToStdString());
            }
            else
            {
                sstates.push_back("000");
            }
            int s = dt.GetSecond();
            if ((s / 10) * 10 > 0)
            {
                sstates.push_back(wxString::Format("%d", (s / 10) * 10).ToStdString());
            }
            else
            {
                sstates.push_back("00");
            }
            s = s - (s / 10) * 10;
            sstates.push_back(wxString::Format("%d", s).ToStdString());
        }
        sstates.push_back("colon");
    }
    else if (mode == "Number") // used for FM frequencies
    {
        double f = wxAtof(tstates);
        sstates.push_back("dot");
        double f2 = f - int(f);
        f2 = (int)(f2 * 10 + 0.5);
        sstates.push_back(wxString::Format("%d", (int)f2).ToStdString());

        int v = f;
        bool force = false;
        if ((v / 100) * 1000 > 0)
        {
            sstates.push_back(wxString::Format("%d", (v / 100) * 1000).ToStdString());
            force = true;
        }
        v = v - (v / 100) * 100;
        if ((v / 10) * 100 > 0)
        {
            sstates.push_back(wxString::Format("%d", (v / 10) * 100).ToStdString());
        }
        else
        {
            if (force)
            {
                sstates.push_back("000");
            }
        }
        v = v - (v / 10) * 10;
        if (v * 10 > 0)
        {
            sstates.push_back(wxString::Format("%d", v * 10).ToStdString());
        }
        else
        {
            sstates.push_back("00");
        }
    }
    else if (mode == "Iterate")
    {
        float progressthroughtimeinterval = ((float)posms - (float)startms) / ((float)endms - (float)startms);

        std::vector<std::string> tmpstates;
        wxString ss = wxString(tstates);
        wxStringTokenizer tkz(ss, wxT(" ,;:"));
        while (tkz.HasMoreTokens())
        {
            wxString token = tkz.GetNextToken();
            if (token == "*" || token == "<ALL>")
            {
                for (auto it2 : model_info->stateInfo[definition])
                {
                    if (EndsWith(it2.first, "-Name") && it2.second != "")
                    {
                        sstates.push_back(Lower(it2.second));
                    }
                }
            }
            else
            {
                tmpstates.push_back(token.Lower().ToStdString());
            }
        }

        int which = tmpstates.size() * progressthroughtimeinterval;

        if (which < tmpstates.size())
        {
            sstates.push_back(tmpstates[which]);
        }
    }

    bool customColor = found ? model_info->stateInfo[definition]["CustomColors"] == "1" : false;

    // process each token
    for (size_t i = 0; i < sstates.size(); i++)
    {
        // get the channels
        std::string statename = FindState(model_info->stateInfo[definition], sstates[i]);
        std::string channels = model_info->stateInfo[definition][statename];

        if (statename != "" && channels != "")
        {
            xlColor color;
            if (colourmode == "Graduate")
            {
                buffer.GetMultiColorBlend(buffer.GetEffectTimeIntervalPosition(), false, color);
            }
            else if (colourmode == "Cycle")
            {
                buffer.palette.GetColor((intervalnumber - 1) % buffer.GetColorCount(), color);
            }
            else
            {
                // allocate
                int statenum = wxAtoi(statename.substr(1));
                buffer.palette.GetColor((statenum - 1) % buffer.GetColorCount(), color);
            }
            if (customColor) {
                std::string cname = model_info->stateInfo[definition][statename + "-Color"];
                if (cname == "") {
                    color = xlWHITE;
                }
                else {
                    color = xlColor(cname);
                }
            }

            wxStringTokenizer wtkz(channels, ",");
            while (wtkz.HasMoreTokens())
            {
                wxString valstr = wtkz.GetNextToken();

                if (type == 0) {
                    for (size_t n = 0; n < model_info->GetNodeCount(); n++) {
                        wxString nn = model_info->GetNodeName(n, true);
                        if (nn == valstr) {
                            buffer.SetNodePixel(n, color);
                        }
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
                        buffer.SetNodePixel(n, color);
                    }
                }
            }
        }
    }
}