/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../../include/servo-16.xpm"
#include "../../include/servo-24.xpm"
#include "../../include/servo-32.xpm"
#include "../../include/servo-48.xpm"
#include "../../include/servo-64.xpm"

#include "ServoEffect.h"
#include "ServoPanel.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../models/DMX/DmxModel.h"
#include "../models/DMX/DmxMotor.h"
#include "../models/DMX/DmxMovingHeadAdv.h"
#include "../models/DMX/DmxServo.h"
#include "../models/DMX/DmxServo3D.h"
#include "../models/DMX/DmxSkull.h"
#include "../models/DMX/DmxSkulltronix.h"
#include "../models/DMX/Servo.h"
#include "../models/ModelGroup.h"
#include "../sequencer/Effect.h"
#include "../sequencer/Element.h"
#include "../sequencer/SequenceElements.h"

ServoEffect::ServoEffect(int id) :
    RenderableEffect(id, "Servo", servo_16, servo_24, servo_32, servo_48, servo_64) {
    // ctor
}

ServoEffect::~ServoEffect() {
    // dtor
}

xlEffectPanel* ServoEffect::CreatePanel(wxWindow* parent) {
    return new ServoPanel(parent);
}
bool ServoEffect::needToAdjustSettings(const std::string& version) {
    if (IsVersionOlder("2024.11", version)) {
        return true;
    }
    return RenderableEffect::needToAdjustSettings(version);
}
void ServoEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    RenderableEffect::adjustSettings(version,  effect, removeDefaults);
    
    std::string vct = effect->GetSettings().Get("E_VALUECURVE_Servo", xlEMPTY_STRING);
    if (!vct.empty()) {
        ValueCurve v(vct);
        if (v.IsOk() && v.IsActive()) {
            //map old Ramp and Flat value curves to new start/end sliders
            if (v.GetType() == "Flat") {
                effect->GetSettings().erase("E_VALUECURVE_Servo");
                char buf[16];
                float p1 = v.GetParameter1();
                snprintf(buf, sizeof(buf), "%0.1f", (p1 / 10.0));
                effect->GetSettings()["E_TEXTCTRL_Servo"] = buf;
                effect->GetSettings()["E_TEXTCTRL_EndValue"] = buf;
            } else if (v.GetType() == "Ramp") {
                effect->GetSettings().erase("E_VALUECURVE_Servo");
                char buf[16];
                float p1 = v.GetParameter1();
                snprintf(buf, sizeof(buf), "%0.1f", (p1 / 10.0));
                effect->GetSettings()["E_TEXTCTRL_Servo"] = buf;
                p1 = v.GetParameter2();
                snprintf(buf, sizeof(buf), "%0.1f", (p1 / 10.0));
                effect->GetSettings()["E_TEXTCTRL_EndValue"] = buf;
            }
        }
    }
    
    std::string i = effect->GetSettings().Get("E_TEXTCTRL_EndValue", xlEMPTY_STRING);
    if (i.empty()) {
        i = effect->GetSettings().Get("E_TEXTCTRL_Servo", xlEMPTY_STRING);
        if (!i.empty()) {
            effect->GetSettings()["E_TEXTCTRL_EndValue"] = i;
        } else {
            effect->GetSettings()["E_TEXTCTRL_EndValue"] = "0";
        }
    }
}
void ServoEffect::AdjustSettingsAfterSplit(Effect *first, Effect *second) {
    float total = second->GetEndTimeMS() - first->GetStartTimeMS();
    float pct = (first->GetEndTimeMS() - first->GetStartTimeMS()) / total;
    
    const std::string vn = "E_VALUECURVE_Servo";
    const std::string &vc = first->GetSetting(vn);
    if (vc.empty()) {
        float sv = std::atof(first->GetSetting("E_TEXTCTRL_Servo").c_str());
        float ev = std::atof(first->GetSetting("E_TEXTCTRL_EndValue").c_str());
        float mv = sv + (ev - sv) * pct;
        first->SetSetting("E_TEXTCTRL_EndValue", std::to_string(mv));
        second->SetSetting("E_TEXTCTRL_Servo", std::to_string(mv));
        first->SetSetting("E_TOGGLEBUTTON_End", "1");
        second->SetSetting("E_TOGGLEBUTTON_Start", "1");
    }
}


std::list<std::string> ServoEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) {
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    bool useTiming = settings.GetBool("E_CHECKBOX_Timing_Track");
    if (useTiming) {
        wxString timing = settings.Get("E_CHOICE_Servo_TimingTrack", "");
        if (timing == "") {
            res.push_back(wxString::Format("    ERR: Servo effect with no timing selected. Model '%s', Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        } else if (timing != "" && GetTiming(timing) == nullptr) {
            res.push_back(wxString::Format("    ERR: Servo effect with unknown timing (%s) selected. Model '%s', Start %s", timing, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }
    return res;
}

void ServoEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) {
    wxString timing = effect->GetSettings().Get("E_CHOICE_Servo_TimingTrack", "");

    if (timing.ToStdString() == oldname) {
        effect->GetSettings()["E_CHOICE_Servo_TimingTrack"] = wxString(newname);
    }
}

void ServoEffect::SetDefaultParameters() {
    ServoPanel* dp = (ServoPanel*)panel;
    if (dp == nullptr) {
        return;
    }
    dp->EndLinkedButton->SetValue(false);
    dp->StartLinkedButton->SetValue(false);
    dp->ValueCurve_Servo->SetActive(false);
    SetSliderValue(dp->Slider_Servo, 0);
    SetSliderValue(dp->SliderEndValue, 0);
    dp->Choice_Channel->SetSelection(-1);
    dp->CheckBox_16bit->SetValue(false);
    SetCheckBoxValue(dp->CheckBox_Timing_Track, false);
    dp->Choice_Servo_TimingTrack->SetSelection(-1);
    dp->SyncCheckBox->SetValue(false);
}

void ServoEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    std::string sel_chan = SettingsMap["CHOICE_Channel"];
    float position = GetValueCurveDouble("Servo", 0, SettingsMap, eff_pos, SERVO_MIN, SERVO_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SERVO_DIVISOR);;
    const std::string vn = "VALUECURVE_Servo";
    const std::string &vc = SettingsMap.Get(vn, xlEMPTY_STRING);
    if (vc == xlEMPTY_STRING) {
        //not using the value curve, calculate
        float endPos = SettingsMap.GetDouble("TEXTCTRL_EndValue", 0.0);
        position = position + (endPos - position) * buffer.GetEffectTimeIntervalPosition();
    }
    
    bool is_16bit = SettingsMap.GetBool("CHECKBOX_16bit");

    if (buffer.cur_model == "") {
        return;
    }
    const Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }

    uint32_t num_channels = model_info->GetNumChannels();

    const std::string& string_type = model_info->GetStringType();

    if (StartsWith(string_type, "Single Color")) {
        // handle channels for single color nodes
        int channel_coarse = 0;
        int channel_fine = 0;
        for (uint32_t i = 0; i <= num_channels; ++i) {
            channel_coarse = i;
            channel_fine = i + 1;
            std::string name = model_info->GetNodeName(i);
            if (name == sel_chan) {
                int min_limit = 0;
                int max_limit = 100;
                if (model_info->GetDisplayAs().rfind("Dmx", 0) == 0) {
                    DmxModel* dmx = (DmxModel*)model_info;
                    int brt_channel = -1;
                    if (model_info->GetDisplayAs() == "DmxServo") {
                        DmxServo* servo = (DmxServo*)model_info;
                        for (int k = 0; k < servo->GetNumServos(); ++k) {
                            int axis_channel = servo->GetAxis(k)->GetChannel();
                            if (axis_channel == (i + 1)) {
                                min_limit = servo->GetAxis(k)->GetMinLimit();
                                max_limit = servo->GetAxis(k)->GetMaxLimit();
                                break;
                            }
                        }
                    } else if (model_info->GetDisplayAs() == "DmxServo3d") {
                        DmxServo3d* servo = (DmxServo3d*)model_info;
                        for (int k = 0; k < servo->GetNumServos(); ++k) {
                            int axis_channel = servo->GetAxis(k)->GetChannel();
                            if (axis_channel == (i + 1)) {
                                min_limit = servo->GetAxis(k)->GetMinLimit();
                                max_limit = servo->GetAxis(k)->GetMaxLimit();
                                break;
                            }
                        }
                    } else if (model_info->GetDisplayAs() == "DmxMovingHeadAdv") {
                        DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)model_info;
                        for (int k = 0; k < mhead->GetNumMotors(); ++k) {
                            int axis_channel = mhead->GetAxis(k)->GetChannelCoarse();
                            if (axis_channel == (i + 1)) {
                                min_limit = mhead->GetAxis(k)->GetMinValue();
                                max_limit = mhead->GetAxis(k)->GetMaxValue();
                                channel_coarse = mhead->GetAxis(k)->GetChannelCoarse() - 1;
                                channel_fine = mhead->GetAxis(k)->GetChannelFine() - 1;
                                is_16bit = true;
                                break;
                            }
                        }
                    } else if (model_info->GetDisplayAs() == "DmxSkull") {
                        DmxSkull* skull = (DmxSkull*)model_info;
                        if (skull->HasPan()) {
                            int pan_channel = skull->GetPanChannel();
                            if (pan_channel == (i + 1)) {
                                min_limit = skull->GetPanMinLimit();
                                max_limit = skull->GetPanMaxLimit();
                            }
                        }
                        if (skull->HasTilt()) {
                            int tilt_channel = skull->GetTiltChannel();
                            if (tilt_channel == (i + 1)) {
                                min_limit = skull->GetTiltMinLimit();
                                max_limit = skull->GetTiltMaxLimit();
                            }
                        }
                        if (skull->HasNod()) {
                            int nod_channel = skull->GetNodChannel();
                            if (nod_channel == (i + 1)) {
                                min_limit = skull->GetNodMinLimit();
                                max_limit = skull->GetNodMaxLimit();
                            }
                        }
                        if (skull->HasJaw()) {
                            int jaw_channel = skull->GetJawChannel();
                            if (jaw_channel == (i + 1)) {
                                min_limit = skull->GetJawMinLimit();
                                max_limit = skull->GetJawMaxLimit();
                            }
                        }
                        if (skull->HasEyeUD()) {
                            int eye_ud_channel = skull->GetEyeUDChannel();
                            if (eye_ud_channel == (i + 1)) {
                                min_limit = skull->GetEyeUDMinLimit();
                                max_limit = skull->GetEyeUDMaxLimit();
                            }
                        }
                        if (skull->HasEyeLR()) {
                            int eye_lr_channel = skull->GetEyeLRChannel();
                            if (eye_lr_channel == (i + 1)) {
                                min_limit = skull->GetEyeLRMinLimit();
                                max_limit = skull->GetEyeLRMaxLimit();
                            }
                        }
                        brt_channel = skull->GetEyeBrightnessChannel();
                    }
                    // deprecating soon
                    else if (model_info->GetDisplayAs() == "DmxSkulltronix") {
                        DmxSkulltronix* skull = (DmxSkulltronix*)model_info;
                        if (skull != nullptr) {
                            int pan_channel = skull->GetPanChannel();
                            if (pan_channel == (i + 1)) {
                                min_limit = skull->GetPanMinLimit();
                                max_limit = skull->GetPanMaxLimit();
                            }
                            int tilt_channel = skull->GetTiltChannel();
                            if (tilt_channel == (i + 1)) {
                                min_limit = skull->GetTiltMinLimit();
                                max_limit = skull->GetTiltMaxLimit();
                            }
                            int nod_channel = skull->GetNodChannel();
                            if (nod_channel == (i + 1)) {
                                min_limit = skull->GetNodMinLimit();
                                max_limit = skull->GetNodMaxLimit();
                            }
                            int jaw_channel = skull->GetJawChannel();
                            if (jaw_channel == (i + 1)) {
                                min_limit = skull->GetJawMinLimit();
                                max_limit = skull->GetJawMaxLimit();
                            }
                            int eye_ud_channel = skull->GetEyeUDChannel();
                            if (eye_ud_channel == (i + 1)) {
                                min_limit = skull->GetEyeUDMinLimit();
                                max_limit = skull->GetEyeUDMaxLimit();
                            }
                            int eye_lr_channel = skull->GetEyeLRChannel();
                            if (eye_lr_channel == (i + 1)) {
                                min_limit = skull->GetEyeLRMinLimit();
                                max_limit = skull->GetEyeLRMaxLimit();
                            }
                            brt_channel = skull->GetEyeBrightnessChannel();
                        }
                    }
                    if (dmx->HasColorAbility()) {
                        DmxColorAbility* dmx_color = dmx->GetColorAbility();
                        if (dmx_color != nullptr) {
                            if (dmx_color->IsColorChannel(i + 1) || brt_channel == (i + 1)) {
                                min_limit = 0;
                                max_limit = 255;
                            }
                        }
                    }
                }

                xlColor lsb_c = xlBLACK;
                xlColor msb_c = xlBLACK;
                bool use_lyrics = SettingsMap.GetBool("CHECKBOX_Timing_Track");
                if (use_lyrics) {
                    position = GetPhonemeValue(buffer, effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(), SettingsMap["CHOICE_Servo_TimingTrack"]);
                }
                uint16_t value = min_limit + (max_limit - min_limit) * (position / 100.0f);
                uint8_t lsb = value & 0xFF;
                uint8_t msb = value >> 8;
                lsb_c.red = lsb;
                lsb_c.green = lsb;
                lsb_c.blue = lsb;
                if (is_16bit) {
                    msb_c.red = msb;
                    msb_c.green = msb;
                    msb_c.blue = msb;
                    buffer.SetPixel(channel_coarse, 0, msb_c, false, false, true);
                    if (channel_fine >= 0) {
                        buffer.SetPixel(channel_fine, 0, lsb_c, false, false, true);
                    }
                } else {
                    buffer.SetPixel(channel_coarse, 0, lsb_c, false, false, true);
                }
                break;
            }
        }
    }
    if (effect->IsBackgroundDisplayListEnabled()) {
        std::unique_lock<std::recursive_mutex> lock(effect->GetBackgroundDisplayList().lock);
        effect->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6);
        int total = buffer.curEffEndPer - buffer.curEffStartPer + 1;
        double x1 = double(buffer.curPeriod - buffer.curEffStartPer) / double(total);
        double x2 = (buffer.curPeriod - buffer.curEffStartPer + 1.0) / double(total);
        int idx = (buffer.curPeriod - buffer.curEffStartPer) * 6;
        float pos = 1.0 - (position / 100.0);
        buffer.SetDisplayListVRect(effect, idx, x1, pos - 0.02, x2, pos + 0.02, xlWHITE, xlWHITE);
    }
}

void ServoEffect::SetPanelStatus(Model* cls) {
    ServoPanel* p = (ServoPanel*)panel;
    if (p == nullptr) {
        return;
    }
    if (cls == nullptr) {
        return;
    }

    Model* m = cls;
    if (cls->GetDisplayAs() == "ModelGroup") {
        m = dynamic_cast<ModelGroup*>(cls)->GetFirstModel();
        if (m == nullptr)
            m = cls;
    }

    p->Choice_Servo_TimingTrack->Clear();
    for (const auto& it : wxSplit(GetTimingTracks(0, 3), '|')) {
        p->Choice_Servo_TimingTrack->Append(it);
    }

    int num_channels = m->GetNumChannels();

    wxString choice_ctrl = "ID_CHOICE_Channel";
    wxChoice* choice = (wxChoice*)(p->FindWindowByName(choice_ctrl));

    if (choice != nullptr) {
        choice->Clear();
        for (int i = 0; i <= num_channels; ++i) {
            std::string name = m->GetNodeName(i);
            if (name != "" && name[0] != '-') {
                choice->Append(name);
            }
        }
        choice->SetSelection(0);
    }
    p->FlexGridSizer_Main->Layout();
    p->Refresh();
}

int ServoEffect::GetPhonemeValue(RenderBuffer& buffer, SequenceElements* elements, const std::string& trackName) {
    static const std::map<wxString, int> phonemeMap = {
        { "AI", 90 },
        { "E", 70 },
        { "FV", 20 },
        { "L", 80 },
        { "MBP", 10 },
        { "O", 50 },
        { "U", 15 },
        { "WQ", 35 },
        { "etc", 100 },
        { "rest", 0 },
        { "(off)", 0 }
    };

    Element* track = elements->GetElement(trackName);
    std::string phoneme = "rest";
    // GET Phoneme from timing track
    if (track == nullptr || track->GetEffectLayerCount() < 3) {
        phoneme = "rest";
    } else {
        int startms = -1;
        int endms = -1;

        EffectLayer* layer = track->GetEffectLayer(2);
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
    }

    wxString pp = phoneme;
    std::string p = pp.BeforeFirst('-');
    // bool shimmer = pp.Lower().EndsWith("-shimmer");

    std::map<wxString, int>::const_iterator it = phonemeMap.find(p);
    int PhonemeInt = 0;
    if (it != phonemeMap.end()) {
        PhonemeInt = it->second;
    }
    return PhonemeInt;
}
