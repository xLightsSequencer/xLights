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
#include "../render/RenderBuffer.h"

class LinesEffect : public RenderableEffect
{
public:
    LinesEffect(int id);
    virtual ~LinesEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    void Render(RenderBuffer& buffer,
                int objects, int segments, int thickness, double speed, int trails, bool fadeTrails);

    // Cached from Lines.json by OnMetadataLoaded().
    static int sObjectsDefault;
    static int sSegmentsDefault;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static double sSpeedDefault;
    static double sSpeedMin;
    static double sSpeedMax;
    static int sSpeedDivisor;
    static int sTrailsDefault;
    static bool sFadeTrailsDefault;

protected:
    virtual void OnMetadataLoaded() override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) override;
};
