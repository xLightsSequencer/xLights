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

    SetSliderValue(dp->Slider_MHPan, 0);
    SetSliderValue(dp->Slider_MHTilt, 0);

    dp->ValueCurve_MHPan->SetActive(false);
    dp->ValueCurve_MHTilt->SetActive(false);
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

            for( int i = 1; i <= 8; ++i ) {
                wxString mh_textbox = wxString::Format("TEXTCTRL_MH%d_Settings", i);
                std::string mh_settings = SettingsMap[mh_textbox];
                if( mh_settings != "" ) {
                    wxArrayString all_cmds = wxSplit(mh_settings, ';');
                    for (size_t j = 0; j < all_cmds.size(); ++j )
                    {
                        std::string cmd = all_cmds[j];
                        int pos = cmd.find(":");
                        std::string cmd_type = cmd.substr(0, pos);
                        std::string settings = cmd.substr(pos+2, cmd.length());
                        std::replace( settings.begin(), settings.end(), '@', ';');

                        if( cmd_type == "Pan" ) {
                            pan_pos = atof(settings.c_str());
                        } else if ( cmd_type == "Tilt" ) {
                            tilt_pos = atof(settings.c_str());
                        } else if ( cmd_type == "Pan VC" ) {
                            ValueCurve vc( settings );
                            vc.SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
                            vc.SetDivisor(MOVING_HEAD_DIVISOR);
                            pan_pos = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
                        } else if ( cmd_type == "Tilt VC" ) {
                            ValueCurve vc( settings );
                            vc.SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
                            vc.SetDivisor(MOVING_HEAD_DIVISOR);
                            tilt_pos = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
                        } else if ( cmd_type == "FanPan" ) {
                            float fan_pos = atof(settings.c_str());
                            CalculateFanPosition( "Pan", i, pan_pos, fan_pos, all_cmds, models, eff_pos, buffer);
                        } else if ( cmd_type == "FanPan VC" ) {
                            ValueCurve vc( settings );
                            vc.SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
                            vc.SetDivisor(MOVING_HEAD_DIVISOR);
                            float fan_pos = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
                            CalculateFanPosition( "Pan", i, pan_pos, fan_pos, all_cmds, models, eff_pos, buffer);
                        } else if ( cmd_type == "FanTilt VC" ) {
                            ValueCurve vc( settings );
                            vc.SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
                            vc.SetDivisor(MOVING_HEAD_DIVISOR);
                            float fan_pos = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
                            CalculateFanPosition( "Tilt", i, tilt_pos, fan_pos, all_cmds, models, eff_pos, buffer);
                        }
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

void MovingHeadEffect::CalculateFanPosition(const std::string& name, int location, float& position, float fan, wxArrayString& all_cmds, std::list<Model*> models, double eff_pos, RenderBuffer &buffer)
{
    float offset = 0.0f;
    int groupings = 1;
    wxArrayString heads;

    for (size_t i = 0; i < all_cmds.size(); ++i )
    {
        std::string cmd = all_cmds[i];
        int pos = cmd.find(":");
        std::string cmd_type = cmd.substr(0, pos);
        std::string settings = cmd.substr(pos+2, cmd.length());

        if( cmd_type == (name + "Offset") ) {
            offset = atof(settings.c_str());
        } else if( cmd_type == "Heads" ) {
            heads = wxSplit(settings, ',');
        } else if( cmd_type == "Groupings" ) {
            groupings = atoi(settings.c_str());
        } else if( cmd_type == (name + "Offset VC") ) {
            ValueCurve vc( settings );
            vc.SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
            vc.SetDivisor(MOVING_HEAD_DIVISOR);
            offset = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        } else if( cmd_type == "Groupings VC" ) {
            ValueCurve vc( settings );
            vc.SetLimits(MOVING_HEAD_GROUP_MIN, MOVING_HEAD_GROUP_MAX);
            groupings = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        }
    }
 
    std::map<int, int> locations;
    for (size_t i = 0; i < heads.size(); ++i )
    {
        int head = wxAtoi(heads[i]);
        locations[head] = i+1;
    }

    // calculate the slot number within the group
    float slot = (float)locations[location];
    if( groupings > 1 ) {
        slot = (float)((locations[location]-1) % groupings + 1);
    }
    float center = (float)(groupings > 1 ? groupings : heads.size()) / 2.0f + 0.5;

    position = (slot - center) * fan + offset;
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
