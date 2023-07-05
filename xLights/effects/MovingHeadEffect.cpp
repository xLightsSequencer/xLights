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

    dp->ValueCurve_Pan->SetActive(false);
    dp->ValueCurve_Tilt->SetActive(false);
    SetSliderValue(dp->Slider_Pan, 0);
    SetSliderValue(dp->Slider_Tilt, 0);
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
            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)model_info;
            
            int pan_cmd = (int)mhead->GetPanMotor()->ConvertPostoCmd(pan_pos);
            int tilt_cmd = (int)mhead->GetTiltMotor()->ConvertPostoCmd(tilt_pos);
            
            WriteCmdToPixel((int)mhead->GetPanMotor()->GetChannel()-1, pan_cmd, (int)mhead->GetPanMotor()->Is16Bit(), buffer);
            WriteCmdToPixel((int)mhead->GetTiltMotor()->GetChannel()-1, tilt_cmd, (int)mhead->GetTiltMotor()->Is16Bit(), buffer);
        }
    }
}

void MovingHeadEffect::WriteCmdToPixel(int channel, int value, bool is_16bit, RenderBuffer &buffer)
{
    xlColor lsb_c = xlBLACK;
    xlColor msb_c = xlBLACK;
    
    uint8_t lsb = value & 0xFF;
    uint8_t msb = value >> 8;
    lsb_c.red = lsb;
    lsb_c.green = lsb;
    lsb_c.blue = lsb;
    if( is_16bit ) {
        msb_c.red = msb;
        msb_c.green = msb;
        msb_c.blue = msb;
        buffer.SetPixel(channel, 0, msb_c, false, false, true);
        buffer.SetPixel(channel+1, 0, lsb_c, false, false, true);
    } else {
        buffer.SetPixel(channel, 0, lsb_c, false, false, true);
    }
}

void MovingHeadEffect::SetPanelStatus(Model *cls) {
    MovingHeadPanel *p = (MovingHeadPanel*)panel;
    if (p == nullptr) {
        return;
    }
    if (cls == nullptr) {
        return;
    }

    p->FlexGridSizer_Main->Layout();
    p->Refresh();
}
