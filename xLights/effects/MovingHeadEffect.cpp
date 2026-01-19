/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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
#include "../models/DMX/DmxDimmerAbility.h"
#include "../models/DMX/DmxMovingHeadAdv.h"
#include "../models/DMX/DmxMovingHead.h"
#include "../models/DMX/DmxMovingHeadComm.h"
#include "../models/DMX/DmxMotor.h"
#include "../models/DMX/DmxShutterAbility.h"
#include "../models/ModelGroup.h"
#include "../models/DMX/DmxColorAbilityWheel.h"

MovingHeadEffect::MovingHeadEffect(int id) : RenderableEffect(id, "Moving Head", moving_head_16, moving_head_24, moving_head_32, moving_head_48, moving_head_64)
{
}

MovingHeadEffect::~MovingHeadEffect()
{
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

    dp->ValueCurve_MHPan->SetActive(false);
    dp->ValueCurve_MHTilt->SetActive(false);
    dp->ValueCurve_MHPanOffset->SetActive(false);
    dp->ValueCurve_MHTiltOffset->SetActive(false);
    dp->ValueCurve_MHGroupings->SetActive(false);
    dp->ValueCurve_MHPathScale->SetActive(false);
    dp->ValueCurve_MHTimeOffset->SetActive(false);

    SetSliderValue(dp->Slider_MHPan, 0.0f);
    SetSliderValue(dp->Slider_MHTilt, 0.0f);
    SetSliderValue(dp->Slider_MHPanOffset, 0.0f);
    SetSliderValue(dp->Slider_MHTiltOffset, 0.0f);
    SetSliderValue(dp->Slider_MHGroupings, 1);
    SetSliderValue(dp->Slider_MHCycles, 10);
    SetSliderValue(dp->Slider_MHPathScale, 0.0f);
    SetSliderValue(dp->Slider_MHTimeOffset, 0.0f);

    dp->CheckBox_MHIgnorePan->SetValue(false);
    dp->CheckBox_MHIgnoreTilt->SetValue(false);
    dp->CheckBoxAutoShutter->SetValue(false);

    dp->CheckAllFixtures();

    SetTextValue(dp->TextCtrl_MH1_Settings, xlEMPTY_STRING);
    SetTextValue(dp->TextCtrl_MH2_Settings, xlEMPTY_STRING);
    SetTextValue(dp->TextCtrl_MH3_Settings, xlEMPTY_STRING);
    SetTextValue(dp->TextCtrl_MH4_Settings, xlEMPTY_STRING);
    SetTextValue(dp->TextCtrl_MH5_Settings, xlEMPTY_STRING);
    SetTextValue(dp->TextCtrl_MH6_Settings, xlEMPTY_STRING);
    SetTextValue(dp->TextCtrl_MH7_Settings, xlEMPTY_STRING);
    SetTextValue(dp->TextCtrl_MH8_Settings, xlEMPTY_STRING);

    dp->UpdateStatusPanel();
}

void MovingHeadEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    if (buffer.cur_model == "") {
        return;
    }
    const Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }

    const std::string& string_type = model_info->GetStringType();

    if (StartsWith(string_type, "Single Color")) {
        if( model_info->GetDisplayAs() == "DmxMovingHeadAdv" ||
            model_info->GetDisplayAs() == "DmxMovingHead") {
            MovingHeadPanel *p = (MovingHeadPanel*)panel;
            if (p == nullptr) {
                return;
            }
            RenderMovingHeads(p, model_info, SettingsMap, buffer);
        }
    }
}

void MovingHeadEffect::RenderMovingHeads(MovingHeadPanel *p, const Model* model_info, const SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    auto models = GetModels(model_info);
    for( int i = 1; i <= 8; ++i ) {
        wxString mh_textbox = wxString::Format("TEXTCTRL_MH%d_Settings", i);
        std::string mh_settings = SettingsMap[mh_textbox];
        if( mh_settings != xlEMPTY_STRING ) {
            RenderMovingHead(mh_settings, i, model_info, buffer);
        }
    }
}

void MovingHeadEffect::RenderMovingHead(std::string mh_settings, int loc, const Model* model_info, RenderBuffer &buffer)
{
    // parse all the commands
    float pan_pos = 0.0f;
    float tilt_pos = 0.0f;
    float pan_offset = 0.0f;
    float tilt_offset = 0.0f;
    float time_offset = 0.0f;
    float path_scale = 0.0f;
    float delta = 0.0f;
    float cycles = 1.0f;
    bool path_parsed = false;
    bool pan_path_active = true;
    bool tilt_path_active = true;
    bool has_position = false;
    bool has_color = false;
    bool has_color_wheel = false;
    bool has_dimmers = false;
    bool auto_shutter = false;
    std::string path_setting = "";
    wxArrayString heads;
    wxArrayString colors;
    wxArrayString dimmers;
    int groupings = 1;
    wxArrayString all_cmds = wxSplit(mh_settings, ';');

    // Need to look for Cycles setting first to calculate effect position
    for (size_t j = 0; j < all_cmds.size(); ++j )
    {
        std::string cmd = all_cmds[j];
        if( cmd == xlEMPTY_STRING ) continue;
        int pos = cmd.find(":");
        std::string cmd_type = cmd.substr(0, pos);
        std::string settings = cmd.substr(pos+2, cmd.length());
        std::replace( settings.begin(), settings.end(), '@', ';');
        if( cmd_type == "Cycles" ) {
            cycles = atof(settings.c_str());
            break;
        }
    }
    double eff_pos = buffer.GetEffectTimeIntervalPosition(cycles);

    for (size_t j = 0; j < all_cmds.size(); ++j )
    {
        std::string cmd = all_cmds[j];
        if( cmd == xlEMPTY_STRING ) continue;
        int pos = cmd.find(":");
        std::string cmd_type = cmd.substr(0, pos);
        std::string settings = cmd.substr(pos+2, cmd.length());
        std::replace( settings.begin(), settings.end(), '@', ';');
    
        if( cmd_type == "Pan" ) {
            pan_pos = atof(settings.c_str());
            has_position = true;
        } else if ( cmd_type == "Tilt" ) {
            tilt_pos = atof(settings.c_str());
            has_position = true;
        } else if ( cmd_type == "Pan VC" ) {
            GetValueCurvePosition(pan_pos, settings, eff_pos, buffer);
            has_position = true;
        } else if ( cmd_type == "Tilt VC" ) {
            GetValueCurvePosition(tilt_pos, settings, eff_pos, buffer);
            has_position = true;
        } else if( cmd_type == "PanOffset" ) {
            pan_offset = atof(settings.c_str());
        } else if( cmd_type == "TiltOffset" ) {
            tilt_offset = atof(settings.c_str());
        } else if( cmd_type == ("PanOffset VC") ) {
            GetValueCurvePosition(pan_offset, settings, eff_pos, buffer);
        } else if( cmd_type == ("TiltOffset VC") ) {
            GetValueCurvePosition(tilt_offset, settings, eff_pos, buffer);
        } else if ( cmd_type == "IgnorePan" ) {
            pan_path_active = false;
        } else if ( cmd_type == "IgnoreTilt" ) {
            tilt_path_active = false;
        } else if ( cmd_type == "Path" ) {
            path_setting = settings;
            path_parsed = true;
        } else if( cmd_type == "Heads" ) {
            heads = wxSplit(settings, ',');
        } else if( cmd_type == "Groupings" ) {
            groupings = atoi(settings.c_str());
        } else if( cmd_type == "Groupings VC" ) {
            ValueCurve vc( settings );
            vc.SetLimits(MOVING_HEAD_GROUP_MIN, MOVING_HEAD_GROUP_MAX);
            groupings = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        } else if( cmd_type == "TimeOffset" ) {
            time_offset = atof(settings.c_str());
        } else if( cmd_type == "PathScale" ) {
            path_scale = atof(settings.c_str());
        } else if( cmd_type == "TimeOffset VC" ) {
            ValueCurve vc( settings );
            vc.SetLimits(MOVING_HEAD_TIME_MIN, MOVING_HEAD_TIME_MAX);
            vc.SetDivisor(MOVING_HEAD_DIVISOR);
            time_offset = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        } else if( cmd_type == "PathScale VC" ) {
            ValueCurve vc( settings );
            vc.SetLimits(MOVING_HEAD_SCALE_MIN, MOVING_HEAD_SCALE_MAX);
            vc.SetDivisor(MOVING_HEAD_DIVISOR);
            path_scale = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        } else if( cmd_type == "Color" ) {
            colors = wxSplit(settings, ',');
            has_color = true;
        } else if( cmd_type == "Wheel" ) {
            colors = wxSplit(settings, ',');
            has_color_wheel = true;
        } else if( cmd_type == "Dimmer" ) {
            dimmers = wxSplit(settings, ',');
            has_dimmers = true;
        } else if (cmd_type == "AutoShutter") {
            auto_shutter = true;
        }
    }

    CalculatePosition( loc, pan_pos, heads, groupings, pan_offset, delta);
    CalculatePosition( loc, tilt_pos, heads, groupings, tilt_offset, delta);
    
    if( path_parsed ) {
        CalculatePathPositions( pan_path_active, tilt_path_active, pan_pos, tilt_pos, time_offset, path_scale, delta, eff_pos, path_setting);
    }

    // find models that map to this moving head position
    auto models = GetModels(model_info);
    for (const auto& it : models) {
        auto mhead = dynamic_cast<const DmxMovingHeadComm*>(it);
        if( mhead->GetFixtureVal() == loc ) {
            
            if (has_position) {
                int pan_cmd = mhead->GetPanMotor()->ConvertPostoCmd(-pan_pos);
                int tilt_cmd = mhead->GetTiltMotor()->ConvertPostoCmd(-tilt_pos);
                WriteCmdToPixel(mhead->GetPanMotor(), pan_cmd, buffer);
                WriteCmdToPixel(mhead->GetTiltMotor(), tilt_cmd, buffer);
            }

            if (has_dimmers && mhead->GetDimmerAbility()->GetDimmerChannel() > 0) {
                uint32_t dimmer_channel = mhead->GetDimmerAbility()->GetDimmerChannel();
                CalculateDimmer(eff_pos, dimmers, dimmer_channel, buffer);
            }

            if( (has_color || has_color_wheel) && mhead->HasColorAbility() ) {
                DmxColorAbility* mh_color = mhead->GetColorAbility();
                if (mh_color != nullptr) {
                    if( colors.size() > 0 ) {
                        if( has_color_wheel ) {
                            xlColor c {GetWheelColor(eff_pos, colors)};
                            buffer.SetPixel(0, 0, c);
                            auto shutter_chan = mhead->GetShutterAbility()->GetShutterChannel();
                            if (0 != shutter_chan && auto_shutter) {
                                auto shutter_on = mhead->GetShutterAbility()->GetShutterOnValue();
                                CalculateColorWheelShutter(mh_color, eff_pos, colors, shutter_chan, shutter_on, buffer);
                            }
                        } else if (has_color) {
                            xlColor c {GetMultiColorBlend(eff_pos, colors, buffer)};
                            buffer.SetPixel(0, 0, c);
                        }
                    }
                }
            }
            buffer.EnableFixedDMXChannels(mhead);
        }
    }
}

xlColor MovingHeadEffect::GetMultiColorBlend(double eff_pos, const wxArrayString& colors, RenderBuffer &buffer)
{
    size_t colorcnt = colors.size() / 3;
    if (colorcnt <= 1)
    {
        double hue {wxAtof(colors[0])};
        double sat {wxAtof(colors[1])};
        double val {wxAtof(colors[2])};
        HSVValue v{hue,sat,val};
        return xlColor(v);
    }
    if (eff_pos >= 1.0) eff_pos = 0.99999f;
    if (eff_pos < 0.0) eff_pos = 0.0f;
    float realidx = eff_pos * (colorcnt - 1);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % colorcnt;
    float ratio = realidx - float(coloridx1);
    coloridx1 *= 3;
    coloridx2 *= 3;
    double h1 {wxAtof(colors[coloridx1])};
    double s1 {wxAtof(colors[coloridx1+1])};
    double v1 {wxAtof(colors[coloridx1+2])};
    double h2 {wxAtof(colors[coloridx2])};
    double s2 {wxAtof(colors[coloridx2+1])};
    double v2 {wxAtof(colors[coloridx2+2])};

    xlColor color;
    HSVValue hsv1(h1,s1,v1);
    HSVValue hsv2(h2,s2,v2);
    xlColor c1(hsv1);
    xlColor c2(hsv2);
    color.red = buffer.ChannelBlend(c1.red, c2.red, ratio);
    color.green = buffer.ChannelBlend(c1.green, c2.green, ratio);
    color.blue = buffer.ChannelBlend(c1.blue, c2.blue, ratio);
    return color;
}

xlColor MovingHeadEffect::GetWheelColor(double eff_pos, const wxArrayString& colors)
{
    size_t colorcnt = colors.size() / 3;
    float colorsize = 1.0f / (float)colorcnt;
    if (colorcnt <= 1)
    {
        double hue {wxAtof(colors[0])};
        double sat {wxAtof(colors[1])};
        double val {wxAtof(colors[2])};
        HSVValue v{hue,sat,val};
        return xlColor(v);
    }
    if (eff_pos >= 1.0) eff_pos = 0.99999f;
    if (eff_pos < 0.0) eff_pos = 0.0f;
    int coloridx1 = (int)(eff_pos / colorsize);
    coloridx1 *= 3;
    double h1 {wxAtof(colors[coloridx1])};
    double s1 {wxAtof(colors[coloridx1+1])};
    double v1 {wxAtof(colors[coloridx1+2])};
    HSVValue hsv1(h1,s1,v1);
    xlColor c1(hsv1);
    return c1;
}

void MovingHeadEffect::CalculateDimmer(double eff_pos, wxArrayString& dimmers, uint32_t dimmer_channel, RenderBuffer &buffer)
{
    ValueCurve vc;
    size_t num_pts = dimmers.size() / 2;
    for (size_t i = 0; i < num_pts; ++i) {
        double x { wxAtof(dimmers[i*2]) };
        double y { wxAtof(dimmers[i*2+1]) };
        vc.SetValueAt( x, y, true);
    }
    vc.SetType("Custom");
    vc.SetLimits(0, 255);
    vc.SetActive(true);
    
    uint8_t value = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    xlColor msb_c = xlBLACK;
    msb_c.red = value;
    msb_c.green = value;
    msb_c.blue = value;

    buffer.SetPixel(dimmer_channel - 1, 0, msb_c, false, false, true);
}

void MovingHeadEffect::GetValueCurvePosition(float& position, const std::string& settings, double eff_pos, RenderBuffer &buffer)
{
    ValueCurve vc( settings );
    vc.SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    vc.SetDivisor(MOVING_HEAD_DIVISOR);
    position = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
}

void MovingHeadEffect::CalculatePosition(int location, float& position, wxArrayString& heads, int groupings, float offset, float& delta )
{
    std::map<int, int> locations;
    for (size_t i = 0; i < heads.size(); ++i )
    {
        int head = wxAtoi(heads[i]);
        locations[head] = i+1;
    }

    // calculate the slot number within the group
    float slot = (float)locations[location];
    float center = (float)(groupings > 1 ? groupings : heads.size()) / 2.0f + 0.5;
    if( groupings > 1 ) {
        slot = (float)((locations[location]-1) % groupings + 1);
    }
    delta = slot - center;
    position = delta * offset + position;
    delta = slot - 1; // normalize to 0 to pass along for time_offset
}

void MovingHeadEffect::CalculatePathPositions(bool pan_path_active, bool tilt_path_active, float& pan_pos, float& tilt_pos, float time_offset, float path_scale, float delta, double eff_pos, const std::string& path_def)
{
    if( path_def != xlEMPTY_STRING ) {
        SketchEffectSketch sketch(SketchEffectSketch::SketchFromString(path_def));
        wxPoint2DDouble pt;
        double progress_pos = eff_pos + ((delta * time_offset) / 100.0f);
        if( abs(progress_pos) > 1.0f ) {
            int prog1 = (int)(abs(progress_pos) * 100.0f);
            prog1 = prog1 % 100;
            progress_pos = (double)(prog1 / 100.0f);
        }
        sketch.getProgressPosition(progress_pos, pt.m_x, pt.m_y);
        glm::vec3 point;
        float scale = 180.0f;
        float new_scale = path_scale;
        if( new_scale >= 0.0f ) {
            new_scale += 1.0f;
        } else {
            new_scale = 1.0f / abs(new_scale);
        }
        point.x = (pt.m_x - 0.5f) * scale * new_scale;
        point.y = scale / 2.0f;
        point.z = (0.5f - pt.m_y) * scale * new_scale;

        glm::vec4 position = glm::vec4(point, 1.0);
        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), glm::radians(pan_pos), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), glm::radians(tilt_pos), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec4 path_position = rotationMatrixPan * rotationMatrixTilt * position;
        
        // find angle from coordinates
        float new_pan = 0.0f;
        float new_tilt = 0.0f;
        
        if( abs(path_position.z) < 0.0001f ) {
            if( path_position.x > 0.0001f ) {
                new_pan = 90.0f;
            } else if( path_position.x < -0.0001f ) {
                new_pan = -90.0f;
            }
        } else if( abs(path_position.x) > 0.0001f ) {
            new_pan = atan2(path_position.x, path_position.z) * 180.0f / PI;
        }

        float hyp = sqrt(path_position.x * path_position.x + path_position.z * path_position.z);
        if( abs(path_position.y) < 0.0001f ) {
            if( path_position.z > 0.0001f ) {
                new_tilt = 90.0f;
            } else if( path_position.z < -0.0001f ) {
                new_tilt = -90.0f;
            }
        } else if( abs(hyp) > 0.0001f ) {
            new_tilt = atan2(hyp, path_position.y) * 180.0f / PI;
        }
        
        // adjust pan if pointed backwards
        if( new_pan < -90.0f ) {
            new_pan = 180.0f + new_pan;
            new_tilt = -new_tilt;
        }
        if( new_pan > 90.0f ) {
            new_pan = new_pan - 180.0f;
            new_tilt = -new_tilt;
        }
        
        if( pan_path_active ) {
            pan_pos = new_pan;
        }
        if( tilt_path_active ) {
            tilt_pos = new_tilt;
        }
    }
}

void MovingHeadEffect::CalculateColorWheelShutter(DmxColorAbility* mh_color, double eff_pos, const wxArrayString& colors, int shutter_channel, int shutter_on, RenderBuffer& buffer) {
    size_t num_colors = colors.size() / 3;
    if (0 == num_colors) {
        return;
    }
    DmxColorAbilityWheel* wheel_head = static_cast<DmxColorAbilityWheel*>(mh_color);
    if (!wheel_head) {
        return;
    }

    auto arrayToColor = [colors](int index) {
        double hue{ wxAtof(colors[index]) };
        double sat{ wxAtof(colors[index + 1]) };
        double val{ wxAtof(colors[index + 2]) };
        HSVValue v{ hue, sat, val };
        return xlColor(v);
    };
    auto wheel_delay = wheel_head->GetWheelDelay();
    if (0u == wheel_delay ) {
        return;
    }
    long effect_len_ms = buffer.GetEndTimeMS() - buffer.GetStartTimeMS();
    int frame_Time = buffer.frameTimeInMs;
    ValueCurve vc;
    int start_ind = wheel_head->GetDMXWheelIndex(arrayToColor(0));

    //off to start
    vc.SetValueAt(0, 0, true);
    vc.SetValueAt((float)wheel_delay / effect_len_ms, shutter_on / 255.0F, true);

    if (1 != num_colors) {
        for (size_t i = 1; i < num_colors; ++i) {
            int next_ind = wheel_head->GetDMXWheelIndex(arrayToColor(i * 3));
            int rot_dist = next_ind - start_ind;
            if (0 == rot_dist) {
                continue;
            }
            if (0 > rot_dist) {//go around the whole wheel
                rot_dist += wheel_head->GetColorWheelColorSize();
            }
            long cur_ms = effect_len_ms * ((float)i / num_colors);
            float delay_end_pre = (float)((cur_ms - frame_Time) + (rot_dist * wheel_delay)) / effect_len_ms;
            float delay_end = (float)(cur_ms + (rot_dist * wheel_delay)) / effect_len_ms;
            delay_end = std::min(delay_end, 1.0F);

            vc.SetValueAt((float)(cur_ms - frame_Time) / effect_len_ms, shutter_on / 255.0F, true);
            vc.SetValueAt((float)cur_ms / effect_len_ms, 0, true);
            vc.SetValueAt(delay_end_pre, 0, true);
            vc.SetValueAt(delay_end, shutter_on / 255.0F, true);
        }
    }
    // on to end
    vc.SetValueAt(1.00, shutter_on / 255.0F, true);
    vc.SetType("Custom");
    vc.SetLimits(0, 255);
    vc.SetActive(true);

    uint8_t value = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    xlColor msb_c = xlBLACK;
    msb_c.red = value;
    msb_c.green = value;
    msb_c.blue = value;

    buffer.SetPixel(shutter_channel - 1, 0, msb_c, false, false, true);

    //vc.SaveXVC(xLightsFrame::CurrentDir.ToStdString() + "//test.xvc");//this changes the point locations for some reason, do after
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
    }
    if (fine_channel >= 0) {
        buffer.SetPixel(fine_channel, 0, lsb_c, false, false, true);
    }
}

std::list<const Model*> MovingHeadEffect::GetModels(const Model* model)
{
    std::list<const Model*> model_list;
    if (model != nullptr) {
        if (model->GetDisplayAs() == "ModelGroup") {
            auto mg = dynamic_cast<const ModelGroup*>(model);
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

    // Commenting out to allow checkboxes to initialize as checked
    // disable all fixtures
    //for( int i = 1; i <= 8; ++i ) {
    //    wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
    //    wxCheckBox* checkbox = (wxCheckBox*)(p->FindWindowByName(checkbox_ctrl));
    //    if( checkbox != nullptr ) {
    //        checkbox->Enable(false);
    //        checkbox->SetValue(false);
    //    }
    //}

    // find fixture numbers to enable
    auto models = GetModels(cls);
    bool single_model = models.size() == 1;
    for (const auto& it : models) {
        if (it->GetDisplayAs() == "DmxMovingHeadAdv" || it->GetDisplayAs() == "DmxMovingHead") {
            DmxMovingHeadComm* mhead = (DmxMovingHeadComm*)it;
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", mhead->GetFixtureVal());
            wxCheckBox* checkbox = (wxCheckBox*)(p->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                checkbox->Enable(true);
                // Commenting out to allow checkboxes to initialize as checked
                //if( single_model ) {
                    checkbox->SetValue(true);
                //}
            }
       }
    }

    if( single_model ) {
        // Hide all the stuff not applicable to a single moving head
        wxButton* button = (wxButton*)(p->FindWindowByName("ID_BUTTON_All"));
        if( button != nullptr ) { button->Hide(); }
        button = (wxButton*)(p->FindWindowByName("ID_BUTTON_None"));
        if( button != nullptr ) { button->Hide(); }
        button = (wxButton*)(p->FindWindowByName("ID_BUTTON_Evens"));
        if( button != nullptr ) { button->Hide(); }
        button = (wxButton*)(p->FindWindowByName("ID_BUTTON_Odds"));
        if( button != nullptr ) { button->Hide(); }
        wxStaticText* text = (wxStaticText*)(p->FindWindowByName("ID_STATICTEXT_Fixtures"));
        if( text != nullptr ) { text->Hide(); }
        for( int i = 1; i <= 8; ++i ) {
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
            wxCheckBox* checkbox = (wxCheckBox*)(p->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                checkbox->Hide();
            }
        }
        wxTextCtrl* groupings = (wxTextCtrl*)(p->FindWindowByName("IDD_TEXTCTRL_MHGroupings"));
        if( groupings != nullptr ) { groupings->Enable(); groupings->SetValue("1"); groupings->Hide(); }
        wxSlider* slider = (wxSlider*)(p->FindWindowByName("ID_SLIDER_MHGroupings"));
        if( slider != nullptr ) { slider->Enable(); slider->Hide(); }
        BulkEditValueCurveButton* curve = (BulkEditValueCurveButton*)(p->FindWindowByName("ID_VALUECURVE_MHGroupings"));
        if( curve != nullptr ) { curve->Hide(); }
        text = (wxStaticText*)(p->FindWindowByName("ID_STATICTEXT_Groupings"));
        if( text != nullptr ) { text->Hide(); }
    } else {
        wxButton* button = (wxButton*)(p->FindWindowByName("ID_BUTTON_All"));
        if( button != nullptr ) { button->Show(); }
        button = (wxButton*)(p->FindWindowByName("ID_BUTTON_None"));
        if( button != nullptr ) { button->Show(); }
        button = (wxButton*)(p->FindWindowByName("ID_BUTTON_Evens"));
        if( button != nullptr ) { button->Show(); }
        button = (wxButton*)(p->FindWindowByName("ID_BUTTON_Odds"));
        if( button != nullptr ) { button->Show(); }
        wxStaticText* text = (wxStaticText*)(p->FindWindowByName("ID_STATICTEXT_Fixtures"));
        if( text != nullptr ) { text->Show(); }
        for( int i = 1; i <= 8; ++i ) {
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
            wxCheckBox* checkbox = (wxCheckBox*)(p->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                checkbox->Show();
            }
        }
        wxTextCtrl* groupings = (wxTextCtrl*)(p->FindWindowByName("IDD_TEXTCTRL_MHGroupings"));
        if( groupings != nullptr ) { groupings->Show(); }
        wxSlider* slider = (wxSlider*)(p->FindWindowByName("ID_SLIDER_MHGroupings"));
        if( slider != nullptr ) { slider->Show(); }
        BulkEditValueCurveButton* curve = (BulkEditValueCurveButton*)(p->FindWindowByName("ID_VALUECURVE_MHGroupings"));
        if( curve != nullptr ) { curve->Show(); }
        text = (wxStaticText*)(p->FindWindowByName("ID_STATICTEXT_Groupings"));
        if( text != nullptr ) { text->Show(); }
   }
    p->FlexGridSizerPosition->Layout();
    p->FlexGridSizer_Main->Layout();
    p->Refresh();
}

void MovingHeadEffect::SetEffectTimeRange(int startTimeMs, int endTimeMs) {
    MovingHeadPanel *p = (MovingHeadPanel*)panel;
    if (p == nullptr) {
        return;
    }
    p->SetEffectTimeRange(startTimeMs, endTimeMs);
}
