/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "../../include/moving-head-16.xpm"
#include "../../include/moving-head-24.xpm"
#include "../../include/moving-head-32.xpm"
#include "../../include/moving-head-48.xpm"
#include "../../include/moving-head-64.xpm"

#include "MovingHeadEffect.h"
#include "MovingHeadPanel.h"
#include "../sequencer/Effect.h"
#include "../sequencer/Element.h"
#include "../sequencer/SequenceElements.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../models/DMX/DmxMovingHeadAdv.h"
#include "../models/DMX/DmxMotor.h"
#include "../models/ModelGroup.h"

MovingHeadEffect::MovingHeadEffect(int id) : RenderableEffect(id, "Moving Head", moving_head_16, moving_head_24, moving_head_32, moving_head_48, moving_head_64)
{
    //ctor
}

MovingHeadEffect::~MovingHeadEffect()
{
    //dtor
}

xlEffectPanel *MovingHeadEffect::CreatePanel(wxWindow *parent) {
    return new MovingHeadPanel(parent);
}

std::list<std::string> MovingHeadEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    return res;
}

void MovingHeadEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_Servo_TimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_Servo_TimingTrack"] = wxString(newname);
    }
}

void MovingHeadEffect::SetDefaultParameters() {
    MovingHeadPanel *dp = (MovingHeadPanel*)panel;
    if (dp == nullptr) {
        return;
    }

    SetSliderValue(dp->Slider_Pan, 0);
    SetSliderValue(dp->Slider_Tilt, 0);

    dp->ValueCurve_Pan->SetActive(false);
    dp->ValueCurve_Tilt->SetActive(false);
}

void MovingHeadEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    float pan_pos = GetValueCurveDouble("Pan", 0, SettingsMap, eff_pos, MOVING_HEAD_MIN, MOVING_HEAD_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), MOVING_HEAD_DIVISOR);
    float tilt_pos = GetValueCurveDouble("Tilt", 0, SettingsMap, eff_pos, MOVING_HEAD_MIN, MOVING_HEAD_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), MOVING_HEAD_DIVISOR);

    if (buffer.cur_model == "") {
        return;
    }
    Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }

    const std::string& string_type = model_info->GetStringType();

    if (StartsWith(string_type, "Single Color")) {
        //float pan_pos = 0.0f;
        //float tilt_pos = 0.0f;

        if( model_info->GetDisplayAs() == "DmxMovingHeadAdv" ) {
            MovingHeadPanel *p = (MovingHeadPanel*)panel;
            if (p == nullptr) {
                return;
            }
            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)model_info;

            int head_count = 0;
            for( int i = 1; i <= 8; ++i ) {
                wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
                wxCheckBox* checkbox = (wxCheckBox*)(p->FindWindowByName(checkbox_ctrl));
                if( checkbox != nullptr ) {
                    if( checkbox->IsEnabled() ) {
                        head_count++;
                    }
                }
            }

            auto models = GetModels(model_info);

            if( head_count == 1 ) {
                //pan_pos = SettingsMap.GetFloat("SLIDER_Pan", 0.0) / 10.0;
                //tilt_pos = SettingsMap.GetFloat("SLIDER_Tilt", 0.0) / 10.0;

                int pan_cmd = (int)mhead->GetPanMotor()->ConvertPostoCmd(-pan_pos);
                int tilt_cmd = (int)mhead->GetTiltMotor()->ConvertPostoCmd(-tilt_pos);

                WriteCmdToPixel(mhead->GetPanMotor(), pan_cmd, buffer);
                WriteCmdToPixel(mhead->GetTiltMotor(), tilt_cmd, buffer);
            } else {
                for( int i = 1; i <= 8; ++i ) {
                    wxString mh_textbox = wxString::Format("TEXTCTRL_MH%d", i);
                    std::string mh_settings = SettingsMap[mh_textbox];
                    if( mh_settings != "" ) {
                        int pos = mh_settings.find("Pan:");
                        if( pos >= 0 ) {
                            std::string num = mh_settings.substr(pos+5, mh_settings.length());
                            pan_pos = atof(num.c_str());
                        }
                        pos = mh_settings.find("Tilt:");
                        if( pos >= 0 ) {
                            std::string num = mh_settings.substr(pos+6, mh_settings.length());
                            tilt_pos = atof(num.c_str());
                        }

                        // find models that map to this moving head position
                        for (const auto& it : models) {
                            if( it->GetDisplayAs() == "DmxMovingHeadAdv" ) {
                                DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)it;
                                if( mhead->GetFixtureVal() == i ) {
                                    int pan_cmd = (int)mhead->GetPanMotor()->ConvertPostoCmd(-pan_pos);
                                    int tilt_cmd = (int)mhead->GetTiltMotor()->ConvertPostoCmd(-tilt_pos);
                                    
                                    WriteCmdToPixel(mhead->GetPanMotor(), pan_cmd, buffer);
                                    WriteCmdToPixel(mhead->GetTiltMotor(), tilt_cmd, buffer);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MovingHeadEffect::WriteCmdToPixel(DmxMotor* motor, int value, RenderBuffer &buffer)
{
    xlColor lsb_c = xlBLACK;
    xlColor msb_c = xlBLACK;
    
    uint8_t lsb = value & 0xFF;
    uint8_t msb = value >> 8;
    lsb_c.red = lsb;
    lsb_c.green = lsb;
    lsb_c.blue = lsb;
    msb_c.red = msb;
    msb_c.green = msb;
    msb_c.blue = msb;
    int coarse_channel = motor->GetChannelCoarse() - 1;
    int fine_channel = motor->GetChannelFine() - 1;

    if( coarse_channel >= 0 ) {
        buffer.SetPixel(coarse_channel, 0, msb_c, false, false, true);
        if( fine_channel >= 0 ) {
            buffer.SetPixel(fine_channel, 0, lsb_c, false, false, true);
        }
    }
}

std::list<Model*> MovingHeadEffect::GetModels(Model* model)
{
    std::list<Model*> model_list;
    if (model != nullptr) {
        if (model->GetDisplayAs() == "ModelGroup") {
            auto mg = dynamic_cast<ModelGroup*>(model);
            if (mg != nullptr) {
                for (const auto& it : mg->GetFlatModels(true, false)) {
                    if (it->GetDisplayAs() != "ModelGroup" && it->GetDisplayAs() != "SubModel") {
                        model_list.push_back(it);
                    }
                }
            }
        }
        else if (model->GetDisplayAs() == "SubModel") {
            // don't add SubModels
        }
        else {
            model_list.push_back(model);
        }
    }

    return model_list;
}

void MovingHeadEffect::SetPanelStatus(Model *cls) {
    MovingHeadPanel *p = (MovingHeadPanel*)panel;
    if (p == nullptr) {
        return;
    }
    if (cls == nullptr) {
        return;
    }

    // disable all fixtures
    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(p->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            checkbox->Enable(false);
            checkbox->SetValue(false);
        }
    }

    // find fixture numbers to enable
    auto models = GetModels(cls);
    bool single_model = models.size() == 1;
    for (const auto& it : models) {
        if( it->GetDisplayAs() == "DmxMovingHeadAdv" ) {
            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)it;
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", mhead->GetFixtureVal());
            wxCheckBox* checkbox = (wxCheckBox*)(p->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                checkbox->Enable(true);
                if( single_model ) {
                    checkbox->SetValue(true);
               }
            }
       }
    }

    p->FlexGridSizer_Main->Layout();
    p->Refresh();
}

void MovingHeadEffect::UpdateFixturePositions(Model *cls)
{
    MovingHeadPanel *p = (MovingHeadPanel*)panel;
    if (p == nullptr) {
        return;
    }

    auto models = GetModels(cls);
    bool single_model = models.size() == 1;
    for (const auto& it : models) {
        if( it->GetDisplayAs() == "DmxMovingHeadAdv" ) {
            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)it;
            wxString label_ctrl = wxString::Format("ID_STATICTEXT_MH%d", mhead->GetFixtureVal());
            wxStaticText* label = (wxStaticText*)(p->FindWindowByName(label_ctrl));

       }
    }

}
