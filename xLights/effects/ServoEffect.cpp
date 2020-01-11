#include "../../include/servo-16.xpm"
#include "../../include/servo-24.xpm"
#include "../../include/servo-32.xpm"
#include "../../include/servo-48.xpm"
#include "../../include/servo-64.xpm"

#include "ServoEffect.h"
#include "ServoPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/DMX/DmxModel.h"
#include "../models/DMX/DmxSkulltronix.h"
#include "../models/DMX/DmxServo.h"
#include "../models/DMX/DmxServo3d.h"

ServoEffect::ServoEffect(int id) : RenderableEffect(id, "Servo", servo_16, servo_24, servo_32, servo_48, servo_64)
{
    //ctor
}

ServoEffect::~ServoEffect()
{
    //dtor
}

wxPanel *ServoEffect::CreatePanel(wxWindow *parent) {
    return new ServoPanel(parent);
}

void ServoEffect::SetDefaultParameters() {
    ServoPanel *dp = (ServoPanel*)panel;
    if (dp == nullptr) {
        return;
    }

    dp->ValueCurve_Servo->SetActive(false);
    SetSliderValue(dp->Slider_Servo, 0);
}

void ServoEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
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

    int num_channels = model_info->GetNumChannels();

    const std::string& string_type = model_info->GetStringType();

    if (string_type.find("Single Color") != std::string::npos) {
        // handle channels for single color nodes
        for(int i = 0; i <= num_channels; ++i) {
            std::string name = model_info->GetNodeName(i);
            if( name == sel_chan ) {
                int min_limit = 0;
                int max_limit = 100;
                if( model_info->GetDisplayAs().rfind("Dmx", 0) == 0 ) {
                    DmxModel* dmx = (DmxModel*)model_info;
                    int brt_channel = -1;
                    if (model_info->GetDisplayAs() == "DmxServo") {
                        DmxServo* servo = (DmxServo*)model_info;
                        min_limit = servo->GetMinLimit();
                        max_limit = servo->GetMaxLimit();
                    }
                    else if (model_info->GetDisplayAs() == "DmxServo3d") {
                        DmxServo* servo = (DmxServo3d*)model_info;
                        min_limit = servo->GetMinLimit();
                        max_limit = servo->GetMaxLimit();
                    }
                    else if (model_info->GetDisplayAs() == "DmxSkulltronix") {
                        DmxSkulltronix* skull = (DmxSkulltronix*)model_info;
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
                    if (dmx->HasColorAbility()) {
                        DmxColorAbility* dmx_color = dmx->GetColorAbility();
                        int red_channel = dmx_color->GetRedChannel();
                        int grn_channel = dmx_color->GetGreenChannel();
                        int blu_channel = dmx_color->GetBlueChannel();
                        int white_channel = dmx_color->GetWhiteChannel();
                        if (red_channel == (i + 1) || grn_channel == (i + 1) || blu_channel == (i + 1) || white_channel == (i + 1) || brt_channel == (i + 1)) {
                            min_limit = 0;
                            max_limit = 255;
                        }
                    }
                }

                xlColor lsb_c = xlBLACK;
                xlColor msb_c = xlBLACK;
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

    int num_channels = cls->GetNumChannels();

    wxString choice_ctrl = "ID_CHOICE_Channel";
    wxChoice* choice = (wxChoice*)(p->FindWindowByName(choice_ctrl));

    if( choice != nullptr ) {
        choice->Clear();
        for(int i = 0; i <= num_channels; ++i) {
            std::string name = cls->GetNodeName(i);
            if( name != "" && name[0] != '-' ) {
                choice->Append(name);
            }
        }
    }
    p->FlexGridSizer_Main->Layout();
    p->Refresh();
}
