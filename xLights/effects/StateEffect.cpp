#include "StateEffect.h"
#include "StatePanel.h"
#include "../models/Model.h"
#include "../sequencer/SequenceElements.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h" //xLightsFrame
#include "../../include/state.xpm"

#include <wx/tokenzr.h>

StateEffect::StateEffect(int id) : RenderableEffect(id, "State", state_16, state_16, state_16, state_16, state_16)
{
    //ctor
}

StateEffect::~StateEffect()
{
    //dtor
}

void StateEffect::SetDefaultParameters(Model *cls) {
    StatePanel *fp = (StatePanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->Choice_State_TimingTrack->Clear();
    fp->State_StateDefinitonChoice->Clear();
    if (mSequenceElements == nullptr) {
        return;
    }

    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
        if (mSequenceElements->GetElement(i)->GetEffectLayerCount() == 1
            && mSequenceElements->GetElement(i)->GetType() == "timing") {
            fp->Choice_State_TimingTrack->Append(mSequenceElements->GetElement(i)->GetName());
        }
    }

    if (cls != nullptr) {
        for (std::map<std::string, std::map<std::string, std::string> >::iterator it = cls->stateInfo.begin(); it != cls->stateInfo.end(); it++) {
            fp->State_StateDefinitonChoice->Append(it->first);
        }
    }
    if (fp->State_StateDefinitonChoice->GetCount() > 0)
    {
        fp->State_StateDefinitonChoice->SetSelection(0);
    }
}

wxPanel *StateEffect::CreatePanel(wxWindow *parent) {
    return new StatePanel(parent);
}

void StateEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    RenderState(buffer,
                effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                SettingsMap.Get("CHOICE_State_StateDefinition", ""),
                SettingsMap["CHOICE_State_State"],
                SettingsMap["CHOICE_State_TimingTrack"]
    );
}

void StateEffect::RenderState(RenderBuffer &buffer,
                             SequenceElements *elements, const std::string &faceDefinition,
                             const std::string& Phoneme, const std::string &trackName)
{
    if (buffer.needToInit) {
        buffer.needToInit = false;
        elements->AddRenderDependency(trackName, buffer.cur_model);
    }

    Element *track = elements->GetElement(trackName);
    std::recursive_mutex tmpLock;
    std::recursive_mutex *lock = &tmpLock;
    if (track != nullptr) {
        lock = &track->GetRenderLock();
    }
    std::unique_lock<std::recursive_mutex> locker(*lock);

    if (buffer.cur_model == "") {
        return;
    }

    Model* model_info = buffer.frame->AllModels[buffer.cur_model];
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

    std::string phoneme = Phoneme;
    if (phoneme == "") {
        //GET Phoneme from timing track
            int startms = -1;
            int endms = -1;

            EffectLayer *layer = track->GetEffectLayer(2);
            std::unique_lock<std::recursive_mutex> locker(layer->GetLock());
            int time = buffer.curPeriod * buffer.frameTimeInMs + 1;
            Effect *ef = layer->GetEffectByTime(time);
            if (ef == nullptr) {
                phoneme = "rest";
            } else {
                startms = ef->GetStartTimeMS();
                endms = ef->GetEndTimeMS();
                phoneme = ef->GetEffectName();
            }
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
            std::string cname = model_info->faceInfo[definition][ "Mouth-" + phoneme + "-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
            } else {
                colors.push_back(xlColor(cname));
            }
        } else {
            colors.push_back(color);
        }
    }
    if (buffer.palette.Size() > colorOffset) {
        buffer.palette.GetColor(colorOffset, color); //use second color for eyes; user must make sure it matches model node type
    }
    if (buffer.palette.Size() > (1 + colorOffset)) {
        buffer.palette.GetColor((1 + colorOffset), color); //use third color for outline; user must make sure it matches model node type
    }

    for (size_t t = 0; t < todo.size(); t++) {
        std::string channels = model_info->faceInfo[definition][todo[t]];
        wxStringTokenizer wtkz(channels, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();

            if (type == 0) {
                for (size_t n = 0; n < model_info->GetNodeCount(); n++) {
                    wxString nn = model_info->GetNodeName(n, true);
                    if (nn == valstr) {
                        std::vector<wxPoint> pts;
                        model_info->GetNodeCoords(n, pts);
                        for (size_t x = 0; x < pts.size(); x++) {
                            buffer.SetPixel(pts[x].x, pts[x].y, colors[t]);
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
                        for (size_t x = 0; x < pts.size(); x++) {
                            buffer.SetPixel(pts[x].x, pts[x].y, colors[t]);
                        }
                    }
                }
            }
        }
    }
}



