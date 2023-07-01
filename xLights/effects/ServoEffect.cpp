/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "../../include/servo-16.xpm"
#include "../../include/servo-24.xpm"
#include "../../include/servo-32.xpm"
#include "../../include/servo-48.xpm"
#include "../../include/servo-64.xpm"

#include "ServoEffect.h"
#include "ServoPanel.h"
#include "../sequencer/Effect.h"
#include "../sequencer/Element.h"
#include "../sequencer/SequenceElements.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../models/DMX/DmxModel.h"
#include "../models/DMX/DmxSkull.h"
#include "../models/DMX/DmxSkulltronix.h"
#include "../models/DMX/DmxServo.h"
#include "../models/DMX/DmxServo3D.h"
#include "../models/DMX/Servo.h"
#include "../models/ModelGroup.h"

ServoEffect::ServoEffect(int id) : RenderableEffect(id, "Servo", servo_16, servo_24, servo_32, servo_48, servo_64)
{
    //ctor
}

ServoEffect::~ServoEffect()
{
    //dtor
}

xlEffectPanel *ServoEffect::CreatePanel(wxWindow *parent) {
    return new ServoPanel(parent);
}

std::list<std::string> ServoEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    wxString timing = settings.Get("E_CHOICE_Servo_TimingTrack", "");

    if (timing == "")
    {
        res.push_back(wxString::Format("    ERR: Face effect with no timing selected. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else if (timing != "" && GetTiming(timing) == nullptr)
    {
        res.push_back(wxString::Format("    ERR: Face effect with unknown timing (%s) selected. Model '%s', Start %s", timing, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void ServoEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_Servo_TimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_Servo_TimingTrack"] = wxString(newname);
    }
}


void ServoEffect::SetDefaultParameters() {
    ServoPanel *dp = (ServoPanel*)panel;
    if (dp == nullptr) {
        return;
    }

    dp->ValueCurve_Servo->SetActive(false);
    SetSliderValue(dp->Slider_Servo, 0);
}

void ServoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    std::string sel_chan = SettingsMap["CHOICE_Channel"];
    float position = GetValueCurveDouble("Servo", 0, SettingsMap, eff_pos, SERVO_MIN, SERVO_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SERVO_DIVISOR);
    bool is_16bit = SettingsMap.GetBool("CHECKBOX_16bit");

    if (buffer.cur_model == "") {
        return;
    }
    Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }

   uint32_t num_channels = model_info->GetNumChannels();

    const std::string& string_type = model_info->GetStringType();

    if (StartsWith(string_type, "Single Color")) {
        // handle channels for single color nodes
        for(uint32_t i = 0; i <= num_channels; ++i) {
            std::string name = model_info->GetNodeName(i);
            if( name == sel_chan ) {
                int min_limit = 0;
                int max_limit = 100;
                if( model_info->GetDisplayAs().rfind("Dmx", 0) == 0 ) {
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
                    }
                    else if (model_info->GetDisplayAs() == "DmxServo3d") {
                        DmxServo3d* servo = (DmxServo3d*)model_info;
                        for (int k = 0; k < servo->GetNumServos(); ++k) {
                            int axis_channel = servo->GetAxis(k)->GetChannel();
                            if (axis_channel == (i + 1)) {
                                min_limit = servo->GetAxis(k)->GetMinLimit();
                                max_limit = servo->GetAxis(k)->GetMaxLimit();
                                break;
                            }
                        }
                    }
                    else if (model_info->GetDisplayAs() == "DmxSkull") {
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
                uint16_t value = min_limit + (max_limit-min_limit) * (position / 100.0f);
                uint8_t lsb = value & 0xFF;
                uint8_t msb = value >> 8;
                lsb_c.red = lsb;
                lsb_c.green = lsb;
                lsb_c.blue = lsb;
                if( is_16bit ) {
                    msb_c.red = msb;
                    msb_c.green = msb;
                    msb_c.blue = msb;
                    buffer.SetPixel(i, 0, msb_c, false, false, true);
                    buffer.SetPixel(i+1, 0, lsb_c, false, false, true);
                } else {
                    buffer.SetPixel(i, 0, lsb_c, false, false, true);
                }
                break;
            }
        }
    }
}

void ServoEffect::SetPanelStatus(Model *cls) {
    ServoPanel *p = (ServoPanel*)panel;
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
    for (const auto& it : wxSplit(GetTimingTracks(0, 3), '|'))
    {
        p->Choice_Servo_TimingTrack->Append(it);
    }

    int num_channels = m->GetNumChannels();

    wxString choice_ctrl = "ID_CHOICE_Channel";
    wxChoice* choice = (wxChoice*)(p->FindWindowByName(choice_ctrl));

    if( choice != nullptr ) {
        choice->Clear();
        for(int i = 0; i <= num_channels; ++i) {
            std::string name = m->GetNodeName(i);
            if( name != "" && name[0] != '-' ) {
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
        {"AI", 90},
        {"E", 70},
        {"FV", 20},
        {"L", 80},
        {"MBP", 10},
        {"O", 50},
        {"U", 15},
        {"WQ", 35},
        {"etc", 100},
        {"rest", 0},
        {"(off)", 0}
    };

    Element* track = elements->GetElement(trackName);
    std::string phoneme = "rest";
    //GET Phoneme from timing track
    if (track == nullptr || track->GetEffectLayerCount() < 3) {
        phoneme = "rest";
    }
    else {
        int startms = -1;
        int endms = -1;

        EffectLayer* layer = track->GetEffectLayer(2);
        std::unique_lock<std::recursive_mutex> locker2(layer->GetLock());
        int time = buffer.curPeriod * buffer.frameTimeInMs + 1;
        Effect* ef = layer->GetEffectByTime(time);
        if (ef == nullptr) {
            phoneme = "rest";
        }
        else {
            startms = ef->GetStartTimeMS();
            endms = ef->GetEndTimeMS();
            phoneme = ef->GetEffectName();
        }
    }

    wxString pp = phoneme;
    std::string p = pp.BeforeFirst('-');
    //bool shimmer = pp.Lower().EndsWith("-shimmer");

    std::map<wxString, int>::const_iterator it = phonemeMap.find(p);
    int PhonemeInt = 0;
    if (it != phonemeMap.end()) {
        PhonemeInt = it->second;
    }
    return PhonemeInt;
}
