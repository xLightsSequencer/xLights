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

#include <string>
#include <vector>

#define MOVING_HEAD_MIN -1800
#define MOVING_HEAD_MAX  1800
#define MOVING_HEAD_DIVISOR 10
#define MOVING_HEAD_GROUP_MIN 1
#define MOVING_HEAD_GROUP_MAX 6
#define MOVING_HEAD_TIME_MIN -1000
#define MOVING_HEAD_TIME_MAX 1000
#define MOVING_HEAD_SCALE_MIN -100
#define MOVING_HEAD_SCALE_MAX 100
#define MOVING_HEAD_PATTERN_ROTATION_MIN 0
#define MOVING_HEAD_PATTERN_ROTATION_MAX 360

class DmxMotor;
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
    virtual FrameParallelism GetFrameParallelism(const SettingsMap& settings) const override { return FrameParallelism::Pure; }
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return false;
    }
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    // Re-keys a single (non-group) moving-head model's "E_TEXTCTRL_MHn_Settings"
    // slots in `settings` so the model's own fixture slot (per
    // DmxMovingHeadComm::GetFixtureVal()) holds the effect's data and every
    // other slot is cleared. Placement-time fix for xLightsSequencer/xLights#7080:
    // an effect authored for one MH model's fixture number renders nothing once
    // it lands on a different single-fixture model (drag/paste/import), because
    // RenderMovingHeads only reads the slot matching the new model's own
    // fixture. Call this wherever an existing effect's settings can land on a
    // different model than the one it was authored for. adjustSettings()
    // (below) calls it too, but only for files saved by a version older than
    // the fix - every live placement path (paste, drag/move, import, clone)
    // must call it directly instead, since none of those are version
    // migrations. No-op for anything other than a
    // single DmxMovingHead/DmxMovingHeadAdv model (ModelGroup placements keep
    // all 8 slots, one per fixture, and are left untouched).
    static void RemapSingleFixtureSettings(SettingsMap& settings, const Model* model);

    // Panel-time snapshot helper (NOT used by Render/RenderMovingHead): computes where a
    // single head (loc, 1-based fixture slot matching "Heads:" command indices) would sit
    // at the very start of an effect (eff_pos is always 0 there, regardless of Cycles) given
    // its raw "TEXTCTRL_MHx_Settings" command string. Returns false if the settings string
    // defines no position at all (mirrors RenderMovingHead's has_position).
    static bool GetHeadStartPosition(const std::string& mh_settings, int loc, long effStartMS, long effEndMS, float& pan, float& tilt);

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

public:
    // Version-gated like any other migration: heals sequences saved by a
    // version older than this fix (xLights#7080), where a Moving Head effect
    // could have been saved with its settings keyed to the wrong fixture. Once
    // a file is re-saved with a fixed version this stops running for it -
    // every *live* placement path (paste, drag/move, import, clone) now calls
    // RemapSingleFixtureSettings directly instead of relying on this, since
    // those aren't version migrations and must run unconditionally.
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
protected:
    void WriteCmdToPixel(DmxMotor* motor, int value, RenderBuffer& buffer);
    void WriteDMXValue(int channel, int value, RenderBuffer& buffer);
    std::list<const Model*> GetModels(const Model* model);
    void UpdateFixturePositions(const Model* cls){};//missing function body
    void RenderMovingHeads(const Model* model_info, const SettingsMap& SettingsMap, RenderBuffer& buffer);
    void RenderMovingHead(std::string mh_settings, int loc, const Model* model_info, RenderBuffer &buffer);
    xlColor GetMultiColorBlend(double eff_pos, const std::vector<std::string>& colors, RenderBuffer &buffer);
    xlColor GetWheelColor(double eff_pos, const std::vector<std::string>& colors);
    static void GetValueCurvePosition(float& position, const std::string& settings, double eff_pos, long startMS, long endMS);
    static void CalculatePosition(int location, float& position, std::vector<std::string>& heads, int groupings, float offset, float& delta );
    static void CalculatePathPositions(bool pan_path_active, bool tilt_path_active, float& pan_pos, float& tilt_pos, float time_offset, float path_scale, float delta, double eff_pos, const std::string& path_def);
public:
    // Public so preset preview UI (e.g. MHPatternPresetBitmapButton) can trace
    // the same shape the renderer produces without duplicating the math.
    static void CalculatePatternPoint(const std::string& algorithm, float iterator, float x_freq, float y_freq, float x_phase, float y_phase, float& x, float& y);
protected:
    static void CalculatePatternPositions(bool pan_path_active, bool tilt_path_active, float& pan_pos, float& tilt_pos, const std::string& algorithm, float width, float height, float x_offset, float y_offset, float rotation, float start_offset, float phase_offset, float x_freq, float y_freq, float x_phase, float y_phase, float delta, double eff_pos);
    void CalculateDimmer(double eff_pos, std::vector<std::string>& dimmers, uint32_t dimmer_channel, RenderBuffer &buffer);
    void CalculateColorWheelShutter(DmxColorAbility* mh_color, double eff_pos, const std::vector<std::string>& colors, int shutter_channel, int shutter_on, RenderBuffer& buffer);
};
