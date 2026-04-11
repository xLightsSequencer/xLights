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

class WaveEffect : public RenderableEffect
{
public:
    WaveEffect(int id);
    virtual ~WaveEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanBeRandom() override
    {
        return false;
    }

    // Cached from Wave.json by OnMetadataLoaded().
    static std::string sWaveTypeDefault;
    static std::string sFillColorsDefault;
    static bool sMirrorWaveDefault;
    // Number_Waves is now a float "number of cycles" with divisor 360. The
    // underlying storage stays in degrees (pre-divisor 180..3600) so old
    // sequence value curves keep working; Render multiplies the divided
    // float back up to get the raw degree count used by the wave math.
    static double sNumberWavesDefault;
    static double sNumberWavesMin;
    static double sNumberWavesMax;
    static int sNumberWavesDivisor;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static int sWaveHeightDefault;
    static int sWaveHeightMin;
    static int sWaveHeightMax;
    static double sWaveSpeedDefault;
    static double sWaveSpeedMin;
    static double sWaveSpeedMax;
    static int sWaveSpeedDivisor;
    static std::string sWaveDirectionDefault;
    static int sYOffsetDefault;
    static int sYOffsetMin;
    static int sYOffsetMax;

protected:
    virtual void OnMetadataLoaded() override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
};
