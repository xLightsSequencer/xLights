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

class SingleStrandEffect : public RenderableEffect
{
public:
    SingleStrandEffect(int id);
    virtual ~SingleStrandEffect();
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    std::vector<std::string> GetSettingOptions(const std::string& setting) const override;

    static double sRotationsDefault;
    static double sRotationsMin;
    static double sRotationsMax;
    static int sRotationsDivisor;
    static int sChasesDefault;
    static int sChasesMin;
    static int sChasesMax;
    static int sColourMixDefault;
    static int sColourMixMin;
    static int sColourMixMax;
    static double sOffsetDefault;
    static double sOffsetMin;
    static double sOffsetMax;
    static int sOffsetDivisor;
    static int sFXIntensityDefault;
    static int sFXIntensityMin;
    static int sFXIntensityMax;
    static int sFXSpeedDefault;
    static int sFXSpeedMin;
    static int sFXSpeedMax;
    static std::string sColorsDefault;
    static std::string sChaseTypeDefault;
    static std::string sFadeTypeDefault;
    static bool sGroupAllDefault;
    static int sSkipsBandSizeDefault;
    static int sSkipsSkipSizeDefault;
    static int sSkipsStartPosDefault;
    static int sSkipsAdvanceDefault;

protected:
    virtual void OnMetadataLoaded() override;

private:
    void RenderSingleStrandChase(RenderBuffer& buffer, Effect* eff,
                                 const std::string& ColorScheme, int Number_Chases, int chaseSize,
                                 const std::string& Chase_Type1,
                                 const std::string& Fade_Type, bool Chase_Group_All,
                                 float chaseSpeed, float offset);
    void RenderSingleStrandSkips(RenderBuffer& buffer, Effect* eff, int Skips_BandSize,
                                 int Skips_SkipSize, int Skips_StartPos, const std::string& Skips_Direction, int advances);
    void RenderSingleStrandFX(RenderBuffer& buffer, Effect* eff, int intensity, int speed, const std::string& fx, const std::string& palette);
    void draw_chase(RenderBuffer& buffer,
                    int x, bool group, int ColorScheme, int Number_Chases, bool autoReverse, int width,
                    int Color_Mix1, const std::string& Fade_Type, int ChaseDirection, bool mirror);
};
