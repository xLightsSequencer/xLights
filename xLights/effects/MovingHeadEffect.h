#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#define MOVING_HEAD_MIN -1800
#define MOVING_HEAD_MAX  1800
#define MOVING_HEAD_DIVISOR 10
#define MOVING_HEAD_GROUP_MIN 1
#define MOVING_HEAD_GROUP_MAX 6
#define MOVING_HEAD_TIME_MIN -1000
#define MOVING_HEAD_TIME_MAX 1000
#define MOVING_HEAD_SCALE_MIN -100
#define MOVING_HEAD_SCALE_MAX 100

enum class DMXPathType {
    Circle,
    Square,
    Diamond,
    Line,
    Leaf,
    Eight,
    Unknown
};

class DmxMotorBase;
class MovingHeadPanel;
class DmxColorAbility;

class MovingHeadEffect : public RenderableEffect
{
public:
    MovingHeadEffect(int id);
    virtual ~MovingHeadEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual void SetPanelStatus(Model* cls) override;
    virtual void SetEffectTimeRange(int startTimeMs, int endTimeMs) override;
    virtual void SetDefaultParameters() override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return false;
    }
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_MHPan")
            return MOVING_HEAD_MIN;
        if (name == "E_VALUECURVE_MHTilt")
            return MOVING_HEAD_MIN;
        if (name == "E_VALUECURVE_MHFanPan")
            return MOVING_HEAD_MIN;
        if (name == "E_VALUECURVE_MHFanTilt")
            return MOVING_HEAD_MIN;
        if (name == "E_VALUECURVE_MHPanOffset")
            return MOVING_HEAD_MIN;
        if (name == "E_VALUECURVE_MHTiltOffset")
            return MOVING_HEAD_MIN;
        if (name == "E_VALUECURVE_MHGroupings")
            return MOVING_HEAD_GROUP_MIN;
        if (name == "E_VALUECURVE_MHTimeOffset")
            return MOVING_HEAD_TIME_MIN;
        if (name == "E_VALUECURVE_MHPathScale")
            return MOVING_HEAD_SCALE_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_MHPan")
            return MOVING_HEAD_MAX;
        if (name == "E_VALUECURVE_MHTilt")
            return MOVING_HEAD_MAX;
        if (name == "E_VALUECURVE_MHFanPan")
            return MOVING_HEAD_MAX;
        if (name == "E_VALUECURVE_MHFanTilt")
            return MOVING_HEAD_MAX;
        if (name == "E_VALUECURVE_MHPanOffset")
            return MOVING_HEAD_MAX;
        if (name == "E_VALUECURVE_MHTiltOffset")
            return MOVING_HEAD_MAX;
        if (name == "E_VALUECURVE_MHGroupings")
            return MOVING_HEAD_GROUP_MAX;
        if (name == "E_VALUECURVE_MHTimeOffset")
            return MOVING_HEAD_TIME_MAX;
        if (name == "E_VALUECURVE_MHPathScale")
            return MOVING_HEAD_SCALE_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }
    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_MHPan")
            return MOVING_HEAD_DIVISOR;
        if (name == "E_VALUECURVE_MHTilt")
            return MOVING_HEAD_DIVISOR;
        if (name == "E_VALUECURVE_MHFanPan")
            return MOVING_HEAD_DIVISOR;
        if (name == "E_VALUECURVE_MHFanTilt")
            return MOVING_HEAD_DIVISOR;
        if (name == "E_VALUECURVE_MHPanOffset")
            return MOVING_HEAD_DIVISOR;
        if (name == "E_VALUECURVE_MHTiltOffset")
            return MOVING_HEAD_DIVISOR;
        if (name == "E_VALUECURVE_MHTimeOffset")
            return MOVING_HEAD_DIVISOR;
        if (name == "E_VALUECURVE_MHPathScale")
            return MOVING_HEAD_DIVISOR;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

    static std::pair<int, int> RenderPath(DMXPathType effectType, double eff_pos, int height, int width, int rot);
    static DMXPathType DecodeType(const std::string& shape);
    static std::pair<float, float> CalcLocation(DMXPathType effectType, float degpos);

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override
    {
        return false;
    }
    void WriteCmdToPixel(DmxMotorBase* motor, int value, RenderBuffer& buffer);
    std::list<const Model*> GetModels(const Model* model);
    void UpdateFixturePositions(const Model* cls){};//missing function body
    void RenderMovingHeads(MovingHeadPanel *p, const Model* model_info, const SettingsMap &SettingsMap, RenderBuffer &buffer);
    void RenderMovingHead(std::string mh_settings, int loc, const Model* model_info, RenderBuffer &buffer);
    xlColor GetMultiColorBlend(double eff_pos, const wxArrayString& colors, RenderBuffer &buffer);
    xlColor GetWheelColor(double eff_pos, const wxArrayString& colors);
    void GetValueCurvePosition(float& position, const std::string& settings, double eff_pos, RenderBuffer &buffer);
    void CalculatePosition(int location, float& position, wxArrayString& heads, int groupings, float offset, float& delta );
    void CalculatePathPositions(bool pan_path_active, bool tilt_path_active, float& pan_pos, float& tilt_pos, float time_offset, float path_scale, float delta, double eff_pos, const std::string& path_def);
    void CalculateDimmer(double eff_pos, wxArrayString&dimmers, uint32_t dimmer_channel, RenderBuffer &buffer);
    void CalculateColorWheelShutter(DmxColorAbility* mh_color, double eff_pos, const wxArrayString& colors, int shutter_channel, int shutter_on, RenderBuffer& buffer);
};
