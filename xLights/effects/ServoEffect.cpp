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
#include "../models/DmxModel.h"

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

                if( model_info->GetDisplayAs() == "DMX" ) {
                    DmxModel* dmx = (DmxModel*)model_info;
                    int pan_channel = dmx->GetPanChannel();
                    if( pan_channel == (i+1) ) {
                        min_limit = dmx->GetPanMinLimit();
                        max_limit = dmx->GetPanMaxLimit();
                    }
                    int tilt_channel = dmx->GetTiltChannel();
                    if( tilt_channel == (i+1) ) {
                        min_limit = dmx->GetTiltMinLimit();
                        max_limit = dmx->GetTiltMaxLimit();
                    }
                    int nod_channel = dmx->GetNodChannel();
                    if( nod_channel == (i+1) ) {
                        min_limit = dmx->GetNodMinLimit();
                        max_limit = dmx->GetNodMaxLimit();
                    }
                    int jaw_channel = dmx->GetJawChannel();
                    if( jaw_channel == (i+1) ) {
                        min_limit = dmx->GetJawMinLimit();
                        max_limit = dmx->GetJawMaxLimit();
                    }
                    int eye_ud_channel = dmx->GetEyeUDChannel();
                    if( eye_ud_channel == (i+1) ) {
                        min_limit = dmx->GetEyeUDMinLimit();
                        max_limit = dmx->GetEyeUDMaxLimit();
                    }
                    int eye_lr_channel = dmx->GetEyeLRChannel();
                    if( eye_lr_channel == (i+1) ) {
                        min_limit = dmx->GetEyeLRMinLimit();
                        max_limit = dmx->GetEyeLRMaxLimit();
                    }
                    int brt_channel = dmx->GetEyeBrightnessChannel();
                    int red_channel = dmx->GetRedChannel();
                    int grn_channel = dmx->GetGreenChannel();
                    int blu_channel = dmx->GetBlueChannel();
                    int white_channel = dmx->GetWhiteChannel();
                    if( red_channel == (i+1) || grn_channel == (i+1) || blu_channel == (i+1) || white_channel == (i+1) || brt_channel == (i+1) ) {
                        min_limit = 0;
                        max_limit = 255;
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
                    buffer.SetPixel(i, 0, msb_c);
                    buffer.SetPixel(i+1, 0, lsb_c);
                } else {
                    buffer.SetPixel(i, 0, lsb_c);
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
