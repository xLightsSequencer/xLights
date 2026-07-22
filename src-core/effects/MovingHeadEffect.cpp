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
#include "render/ValueCurve.h"
#include "SketchEffectDrawing.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <spdlog/fmt/fmt.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../utils/xlPoint.h"

#include "../utils/string_utils.h"
#include "../render/Effect.h"
#include "../render/Element.h"
#include "../render/SequenceElements.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "UtilFunctions.h"
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

std::list<std::string> MovingHeadEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    return res;
}

void MovingHeadEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    std::string timing = effect->GetSettings().Get("E_CHOICE_Servo_TimingTrack", "");

    if (timing == oldname)
    {
        effect->GetSettings()["E_CHOICE_Servo_TimingTrack"] = newname;
    }
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
        if( model_info->GetDisplayAs() == DisplayAsType::DmxMovingHeadAdv ||
            model_info->GetDisplayAs() == DisplayAsType::DmxMovingHead) {
            RenderMovingHeads(model_info, SettingsMap, buffer);
        }
    }
}

void MovingHeadEffect::RenderMovingHeads(const Model* model_info, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    auto models = GetModels(model_info);
    for( int i = 1; i <= 8; ++i ) {
        std::string mh_textbox = "TEXTCTRL_MH" + std::to_string(i) + "_Settings";
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
    bool pattern_parsed = false;
    std::string pattern_algorithm = "Circle";
    float pattern_width = 90.0f;
    float pattern_height = 45.0f;
    float pattern_xoffset = 0.0f;
    float pattern_yoffset = 0.0f;
    float pattern_rotation = 0.0f;
    float pattern_start_offset = 0.0f;
    float pattern_phase_offset = 0.0f;
    float pattern_xfreq = 2.0f;
    float pattern_yfreq = 3.0f;
    float pattern_xphase = 90.0f;
    float pattern_yphase = 0.0f;
    bool has_position = false;
    bool has_color = false;
    bool has_color_wheel = false;
    bool has_dimmers = false;
    bool auto_shutter = false;
    bool shutter_open = false;
    std::string path_setting = "";
    std::vector<std::string> heads;
    std::vector<std::string> colors;
    std::vector<std::string> dimmers;
    int groupings = 1;
    auto all_cmds = Split(mh_settings, ';');

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
            GetValueCurvePosition(pan_pos, settings, eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
            has_position = true;
        } else if ( cmd_type == "Tilt VC" ) {
            GetValueCurvePosition(tilt_pos, settings, eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
            has_position = true;
        } else if( cmd_type == "PanOffset" ) {
            pan_offset = atof(settings.c_str());
        } else if( cmd_type == "TiltOffset" ) {
            tilt_offset = atof(settings.c_str());
        } else if( cmd_type == ("PanOffset VC") ) {
            GetValueCurvePosition(pan_offset, settings, eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        } else if( cmd_type == ("TiltOffset VC") ) {
            GetValueCurvePosition(tilt_offset, settings, eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        } else if ( cmd_type == "IgnorePan" ) {
            pan_path_active = false;
        } else if ( cmd_type == "IgnoreTilt" ) {
            tilt_path_active = false;
        } else if ( cmd_type == "Path" ) {
            path_setting = settings;
            path_parsed = true;
        } else if ( cmd_type == "Pattern" ) {
            pattern_algorithm = settings;
            pattern_parsed = true;
        } else if ( cmd_type == "PatternWidth" ) {
            pattern_width = atof(settings.c_str());
        } else if ( cmd_type == "PatternHeight" ) {
            pattern_height = atof(settings.c_str());
        } else if ( cmd_type == "PatternXOffset" ) {
            pattern_xoffset = atof(settings.c_str());
        } else if ( cmd_type == "PatternYOffset" ) {
            pattern_yoffset = atof(settings.c_str());
        } else if ( cmd_type == "PatternRotation" ) {
            pattern_rotation = atof(settings.c_str());
        } else if ( cmd_type == "PatternRotation VC" ) {
            ValueCurve vc( settings );
            vc.SetLimits(MOVING_HEAD_PATTERN_ROTATION_MIN, MOVING_HEAD_PATTERN_ROTATION_MAX);
            pattern_rotation = vc.GetOutputValueAtDivided(eff_pos, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        } else if ( cmd_type == "PatternStartOffset" ) {
            pattern_start_offset = atof(settings.c_str());
        } else if ( cmd_type == "PatternPhaseOffset" ) {
            pattern_phase_offset = atof(settings.c_str());
        } else if ( cmd_type == "PatternXFreq" ) {
            pattern_xfreq = atof(settings.c_str());
        } else if ( cmd_type == "PatternYFreq" ) {
            pattern_yfreq = atof(settings.c_str());
        } else if ( cmd_type == "PatternXPhase" ) {
            pattern_xphase = atof(settings.c_str());
        } else if ( cmd_type == "PatternYPhase" ) {
            pattern_yphase = atof(settings.c_str());
        } else if( cmd_type == "Heads" ) {
            heads = Split(settings, ',');
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
            colors = Split(settings, ',');
            has_color = true;
        } else if( cmd_type == "Wheel" ) {
            colors = Split(settings, ',');
            has_color_wheel = true;
        } else if( cmd_type == "Dimmer" ) {
            dimmers = Split(settings, ',');
            has_dimmers = true;
        } else if (cmd_type == "AutoShutter") {
            auto_shutter = true;
        } else if (cmd_type == "Shutter") {
            shutter_open = true;
        }
    }

    CalculatePosition( loc, pan_pos, heads, groupings, pan_offset, delta);
    CalculatePosition( loc, tilt_pos, heads, groupings, tilt_offset, delta);
    
    if( path_parsed ) {
        CalculatePathPositions( pan_path_active, tilt_path_active, pan_pos, tilt_pos, time_offset, path_scale, delta, eff_pos, path_setting);
    }

    if( pattern_parsed ) {
        CalculatePatternPositions( pan_path_active, tilt_path_active, pan_pos, tilt_pos, pattern_algorithm,
                                   pattern_width, pattern_height, pattern_xoffset, pattern_yoffset,
                                   pattern_rotation, pattern_start_offset, pattern_phase_offset,
                                   pattern_xfreq, pattern_yfreq, pattern_xphase, pattern_yphase, delta, eff_pos);
        has_position = true;
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

            // Hold the shutter open at the model's "on" value. Written first so that when
            // Auto Shutter is active (color wheel) its pulsing curve overwrites this below,
            // preserving the old behavior; otherwise the shutter stays open.
            if (shutter_open && mhead->HasShutterAbility()) {
                auto sa = mhead->GetShutterAbility();
                if (sa->GetShutterChannel() > 0) {
                    WriteDMXValue(sa->GetShutterChannel(), sa->GetShutterOnValue(), buffer);
                }
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

void MovingHeadEffect::WriteDMXValue(int channel, int value, RenderBuffer& buffer)
{
    if (channel <= 0) {
        return;
    }
    uint8_t v = (uint8_t)std::clamp(value, 0, 255);
    xlColor c = xlBLACK;
    c.red = v;
    c.green = v;
    c.blue = v;
    buffer.SetPixel(channel - 1, 0, c, false, false, true);
}

xlColor MovingHeadEffect::GetMultiColorBlend(double eff_pos, const std::vector<std::string>& colors, RenderBuffer &buffer)
{
    size_t colorcnt = colors.size() / 3;
    if (colorcnt <= 1)
    {
        double hue {std::strtod(colors[0].c_str(), nullptr)};
        double sat {std::strtod(colors[1].c_str(), nullptr)};
        double val {std::strtod(colors[2].c_str(), nullptr)};
        HSVValue v(hue, sat, val);
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
    double h1 {std::strtod(colors[coloridx1].c_str(), nullptr)};
    double s1 {std::strtod(colors[coloridx1+1].c_str(), nullptr)};
    double v1 {std::strtod(colors[coloridx1+2].c_str(), nullptr)};
    double h2 {std::strtod(colors[coloridx2].c_str(), nullptr)};
    double s2 {std::strtod(colors[coloridx2+1].c_str(), nullptr)};
    double v2 {std::strtod(colors[coloridx2+2].c_str(), nullptr)};

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

xlColor MovingHeadEffect::GetWheelColor(double eff_pos, const std::vector<std::string>& colors)
{
    size_t colorcnt = colors.size() / 3;
    float colorsize = 1.0f / (float)colorcnt;
    if (colorcnt <= 1)
    {
        double hue {std::strtod(colors[0].c_str(), nullptr)};
        double sat {std::strtod(colors[1].c_str(), nullptr)};
        double val {std::strtod(colors[2].c_str(), nullptr)};
        HSVValue v(hue, sat, val);
        return xlColor(v);
    }
    if (eff_pos >= 1.0) eff_pos = 0.99999f;
    if (eff_pos < 0.0) eff_pos = 0.0f;
    int coloridx1 = (int)(eff_pos / colorsize);
    coloridx1 *= 3;
    double h1 {std::strtod(colors[coloridx1].c_str(), nullptr)};
    double s1 {std::strtod(colors[coloridx1+1].c_str(), nullptr)};
    double v1 {std::strtod(colors[coloridx1+2].c_str(), nullptr)};
    HSVValue hsv1(h1,s1,v1);
    xlColor c1(hsv1);
    return c1;
}

void MovingHeadEffect::CalculateDimmer(double eff_pos, std::vector<std::string>& dimmers, uint32_t dimmer_channel, RenderBuffer &buffer)
{
    ValueCurve vc;
    size_t num_pts = dimmers.size() / 2;
    for (size_t i = 0; i < num_pts; ++i) {
        double x { std::strtod(dimmers[i*2].c_str(), nullptr) };
        double y { std::strtod(dimmers[i*2+1].c_str(), nullptr) };
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

void MovingHeadEffect::GetValueCurvePosition(float& position, const std::string& settings, double eff_pos, long startMS, long endMS)
{
    ValueCurve vc( settings );
    vc.SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    vc.SetDivisor(MOVING_HEAD_DIVISOR);
    position = vc.GetOutputValueAtDivided(eff_pos, startMS, endMS);
}

bool MovingHeadEffect::GetHeadStartPosition(const std::string& mh_settings, int loc, long effStartMS, long effEndMS, float& pan, float& tilt)
{
    // Mirrors RenderMovingHead's parsing/position-calculation, but kept as an independent
    // pass (rather than factored into RenderMovingHead) so this panel-time-only helper can
    // never accidentally change the render-critical path. eff_pos is always 0 at the very
    // start of an effect regardless of Cycles, so unlike RenderMovingHead we don't need the
    // separate "find Cycles first" pass.
    const double eff_pos = 0.0;

    float pan_pos = 0.0f;
    float tilt_pos = 0.0f;
    float pan_offset = 0.0f;
    float tilt_offset = 0.0f;
    float time_offset = 0.0f;
    float path_scale = 0.0f;
    float delta = 0.0f;
    bool path_parsed = false;
    bool pan_path_active = true;
    bool tilt_path_active = true;
    bool pattern_parsed = false;
    std::string pattern_algorithm = "Circle";
    float pattern_width = 90.0f;
    float pattern_height = 45.0f;
    float pattern_xoffset = 0.0f;
    float pattern_yoffset = 0.0f;
    float pattern_rotation = 0.0f;
    float pattern_start_offset = 0.0f;
    float pattern_phase_offset = 0.0f;
    float pattern_xfreq = 2.0f;
    float pattern_yfreq = 3.0f;
    float pattern_xphase = 90.0f;
    float pattern_yphase = 0.0f;
    bool has_position = false;
    std::string path_setting;
    std::vector<std::string> heads;
    int groupings = 1;

    auto all_cmds = Split(mh_settings, ';');
    for (size_t j = 0; j < all_cmds.size(); ++j) {
        std::string cmd = all_cmds[j];
        if (cmd == xlEMPTY_STRING) continue;
        int pos = cmd.find(":");
        std::string cmd_type = cmd.substr(0, pos);
        std::string settings = cmd.substr(pos + 2, cmd.length());
        std::replace(settings.begin(), settings.end(), '@', ';');

        if (cmd_type == "Pan") {
            pan_pos = atof(settings.c_str());
            has_position = true;
        } else if (cmd_type == "Tilt") {
            tilt_pos = atof(settings.c_str());
            has_position = true;
        } else if (cmd_type == "Pan VC") {
            GetValueCurvePosition(pan_pos, settings, eff_pos, effStartMS, effEndMS);
            has_position = true;
        } else if (cmd_type == "Tilt VC") {
            GetValueCurvePosition(tilt_pos, settings, eff_pos, effStartMS, effEndMS);
            has_position = true;
        } else if (cmd_type == "PanOffset") {
            pan_offset = atof(settings.c_str());
        } else if (cmd_type == "TiltOffset") {
            tilt_offset = atof(settings.c_str());
        } else if (cmd_type == ("PanOffset VC")) {
            GetValueCurvePosition(pan_offset, settings, eff_pos, effStartMS, effEndMS);
        } else if (cmd_type == ("TiltOffset VC")) {
            GetValueCurvePosition(tilt_offset, settings, eff_pos, effStartMS, effEndMS);
        } else if (cmd_type == "IgnorePan") {
            pan_path_active = false;
        } else if (cmd_type == "IgnoreTilt") {
            tilt_path_active = false;
        } else if (cmd_type == "Path") {
            path_setting = settings;
            path_parsed = true;
        } else if (cmd_type == "Pattern") {
            pattern_algorithm = settings;
            pattern_parsed = true;
        } else if (cmd_type == "PatternWidth") {
            pattern_width = atof(settings.c_str());
        } else if (cmd_type == "PatternHeight") {
            pattern_height = atof(settings.c_str());
        } else if (cmd_type == "PatternXOffset") {
            pattern_xoffset = atof(settings.c_str());
        } else if (cmd_type == "PatternYOffset") {
            pattern_yoffset = atof(settings.c_str());
        } else if (cmd_type == "PatternRotation") {
            pattern_rotation = atof(settings.c_str());
        } else if (cmd_type == "PatternRotation VC") {
            ValueCurve vc(settings);
            vc.SetLimits(MOVING_HEAD_PATTERN_ROTATION_MIN, MOVING_HEAD_PATTERN_ROTATION_MAX);
            pattern_rotation = vc.GetOutputValueAtDivided(eff_pos, effStartMS, effEndMS);
        } else if (cmd_type == "PatternStartOffset") {
            pattern_start_offset = atof(settings.c_str());
        } else if (cmd_type == "PatternPhaseOffset") {
            pattern_phase_offset = atof(settings.c_str());
        } else if (cmd_type == "PatternXFreq") {
            pattern_xfreq = atof(settings.c_str());
        } else if (cmd_type == "PatternYFreq") {
            pattern_yfreq = atof(settings.c_str());
        } else if (cmd_type == "PatternXPhase") {
            pattern_xphase = atof(settings.c_str());
        } else if (cmd_type == "PatternYPhase") {
            pattern_yphase = atof(settings.c_str());
        } else if (cmd_type == "Heads") {
            heads = Split(settings, ',');
        } else if (cmd_type == "Groupings") {
            groupings = atoi(settings.c_str());
        } else if (cmd_type == "Groupings VC") {
            ValueCurve vc(settings);
            vc.SetLimits(MOVING_HEAD_GROUP_MIN, MOVING_HEAD_GROUP_MAX);
            groupings = vc.GetOutputValueAtDivided(eff_pos, effStartMS, effEndMS);
        } else if (cmd_type == "TimeOffset") {
            time_offset = atof(settings.c_str());
        } else if (cmd_type == "PathScale") {
            path_scale = atof(settings.c_str());
        } else if (cmd_type == "TimeOffset VC") {
            ValueCurve vc(settings);
            vc.SetLimits(MOVING_HEAD_TIME_MIN, MOVING_HEAD_TIME_MAX);
            vc.SetDivisor(MOVING_HEAD_DIVISOR);
            time_offset = vc.GetOutputValueAtDivided(eff_pos, effStartMS, effEndMS);
        } else if (cmd_type == "PathScale VC") {
            ValueCurve vc(settings);
            vc.SetLimits(MOVING_HEAD_SCALE_MIN, MOVING_HEAD_SCALE_MAX);
            vc.SetDivisor(MOVING_HEAD_DIVISOR);
            path_scale = vc.GetOutputValueAtDivided(eff_pos, effStartMS, effEndMS);
        }
        // Color/Wheel/Dimmer/AutoShutter/Shutter commands don't affect Pan/Tilt; skipped.
    }

    CalculatePosition(loc, pan_pos, heads, groupings, pan_offset, delta);
    CalculatePosition(loc, tilt_pos, heads, groupings, tilt_offset, delta);

    if (path_parsed) {
        CalculatePathPositions(pan_path_active, tilt_path_active, pan_pos, tilt_pos, time_offset, path_scale, delta, eff_pos, path_setting);
    }

    if (pattern_parsed) {
        CalculatePatternPositions(pan_path_active, tilt_path_active, pan_pos, tilt_pos, pattern_algorithm,
                                   pattern_width, pattern_height, pattern_xoffset, pattern_yoffset,
                                   pattern_rotation, pattern_start_offset, pattern_phase_offset,
                                   pattern_xfreq, pattern_yfreq, pattern_xphase, pattern_yphase, delta, eff_pos);
        has_position = true;
    }

    if (!has_position) {
        return false;
    }

    pan = pan_pos;
    tilt = tilt_pos;
    return true;
}

void MovingHeadEffect::CalculatePosition(int location, float& position, std::vector<std::string>& heads, int groupings, float offset, float& delta )
{
    std::map<int, int> locations;
    for (size_t i = 0; i < heads.size(); ++i )
    {
        int head = std::strtol(heads[i].c_str(), nullptr, 10);
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
        double ptx = 0.0, pty = 0.0;
        double progress_pos = eff_pos + ((delta * time_offset) / 100.0f);
        if( abs(progress_pos) > 1.0f ) {
            int prog1 = (int)(abs(progress_pos) * 100.0f);
            prog1 = prog1 % 100;
            progress_pos = (double)(prog1 / 100.0f);
        }
        sketch.getProgressPosition(progress_pos, ptx, pty);
        glm::vec3 point;
        float scale = 180.0f;
        float new_scale = path_scale;
        if( new_scale >= 0.0f ) {
            new_scale += 1.0f;
        } else {
            new_scale = 1.0f / abs(new_scale);
        }
        point.x = (ptx - 0.5f) * scale * new_scale;
        point.y = scale / 2.0f;
        point.z = (0.5f - pty) * scale * new_scale;

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

// Parametric shape generator.
// Produces a normalized point in [-1, 1] for a given iterator (0 .. 2*PI).
void MovingHeadEffect::CalculatePatternPoint(const std::string& algorithm, float iterator, float x_freq, float y_freq, float x_phase, float y_phase, float& x, float& y)
{
    const float half_pi = (float)(PI / 2.0);
    const float pi = (float)PI;

    if (algorithm == "Eight") {
        x = std::cos((iterator * 2.0f) + half_pi);
        y = std::cos(iterator);
    } else if (algorithm == "Line") {
        x = std::cos(iterator);
        y = std::cos(iterator);
    } else if (algorithm == "Line2") {
        x = iterator / pi - 1.0f;
        y = iterator / pi - 1.0f;
    } else if (algorithm == "Diamond") {
        x = std::pow(std::cos(iterator - half_pi), 3.0f);
        y = std::pow(std::cos(iterator), 3.0f);
    } else if (algorithm == "Square") {
        if (iterator < pi / 2.0f) {
            x = (iterator * 2.0f / pi) * 2.0f - 1.0f;
            y = 1.0f;
        } else if (iterator < pi) {
            x = 1.0f;
            y = (1.0f - (iterator - pi / 2.0f) * 2.0f / pi) * 2.0f - 1.0f;
        } else if (iterator < pi * 3.0f / 2.0f) {
            x = (1.0f - (iterator - pi) * 2.0f / pi) * 2.0f - 1.0f;
            y = -1.0f;
        } else {
            x = -1.0f;
            y = ((iterator - pi * 3.0f / 2.0f) * 2.0f / pi) * 2.0f - 1.0f;
        }
    } else if (algorithm == "SquareChoppy") {
        x = std::round(std::cos(iterator));
        y = std::round(std::sin(iterator));
    } else if (algorithm == "SquareTrue") {
        if (iterator < pi / 2.0f) {
            x = 1.0f; y = 1.0f;
        } else if (iterator < pi) {
            x = 1.0f; y = -1.0f;
        } else if (iterator < pi * 3.0f / 2.0f) {
            x = -1.0f; y = -1.0f;
        } else {
            x = -1.0f; y = 1.0f;
        }
    } else if (algorithm == "Leaf") {
        x = std::pow(std::cos(iterator + half_pi), 5.0f);
        y = std::cos(iterator);
    } else if (algorithm == "Lissajous") {
        x = std::cos((x_freq * iterator) - x_phase);
        y = std::cos((y_freq * iterator) - y_phase);
    } else { // Circle (default)
        x = std::cos(iterator + half_pi);
        y = std::cos(iterator);
    }
}

void MovingHeadEffect::CalculatePatternPositions(bool pan_path_active, bool tilt_path_active, float& pan_pos, float& tilt_pos, const std::string& algorithm, float width, float height, float x_offset, float y_offset, float rotation, float start_offset, float phase_offset, float x_freq, float y_freq, float x_phase, float y_phase, float delta, double eff_pos)
{
    // Each head can be phase-shifted along the pattern using its slot index (delta).
    double progress = eff_pos + (double)(delta * phase_offset) / 360.0;
    progress = progress - std::floor(progress); // wrap to [0, 1)

    float iterator = (float)(progress * 2.0 * PI);
    iterator += (float)(start_offset * PI / 180.0);
    while (iterator >= (float)(2.0 * PI)) {
        iterator -= (float)(2.0 * PI);
    }
    while (iterator < 0.0f) {
        iterator += (float)(2.0 * PI);
    }

    float x = 0.0f;
    float y = 0.0f;
    CalculatePatternPoint(algorithm, iterator, x_freq, y_freq, (float)(x_phase * PI / 180.0), (float)(y_phase * PI / 180.0), x, y);

    float cosR = std::cos((float)(rotation * PI / 180.0));
    float sinR = std::sin((float)(rotation * PI / 180.0));

    float new_pan = x_offset + x * cosR * width + y * sinR * height;
    float new_tilt = y_offset - x * sinR * width + y * cosR * height;

    if (pan_path_active) {
        pan_pos = new_pan;
    }
    if (tilt_path_active) {
        tilt_pos = new_tilt;
    }
}

void MovingHeadEffect::CalculateColorWheelShutter(DmxColorAbility* mh_color, double eff_pos, const std::vector<std::string>& colors, int shutter_channel, int shutter_on, RenderBuffer& buffer) {
    size_t num_colors = colors.size() / 3;
    if (0 == num_colors) {
        return;
    }
    DmxColorAbilityWheel* wheel_head = static_cast<DmxColorAbilityWheel*>(mh_color);
    if (!wheel_head) {
        return;
    }

    auto arrayToColor = [&colors](int index) {
        double hue{ std::strtod(colors[index].c_str(), nullptr) };
        double sat{ std::strtod(colors[index + 1].c_str(), nullptr) };
        double val{ std::strtod(colors[index + 2].c_str(), nullptr) };
        HSVValue v( hue, sat, val );
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
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
            auto mg = dynamic_cast<const ModelGroup*>(model);
            if (mg != nullptr) {
                for (const auto& it : mg->GetFlatModels(true, false)) {
                    if (it->GetDisplayAs() != DisplayAsType::ModelGroup && it->GetDisplayAs() != DisplayAsType::SubModel) {
                        model_list.push_back(it);
                    }
                }
            }
        }
        else if (model->GetDisplayAs() == DisplayAsType::SubModel) {
            // don't add SubModels
        }
        else {
            model_list.push_back(model);
        }
    }

    return model_list;
}

