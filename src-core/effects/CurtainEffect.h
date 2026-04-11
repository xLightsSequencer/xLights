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

#include <vector>

class CurtainEffect : public RenderableEffect
{
public:
    CurtainEffect(int id);
    virtual ~CurtainEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;

    // Cached from Curtain.json. Speed uses divisor=10.
    static int sSwagDefault;
    static int sSwagMin;
    static int sSwagMax;
    static double sSpeedDefault;
    static double sSpeedMin;
    static double sSpeedMax;
    static int sSpeedDivisor;
    static std::string sEdgeDefault;
    static std::string sEffectDefault;
    static bool sRepeatDefault;

protected:
    virtual void OnMetadataLoaded() override;
private:
    void DrawCurtain(RenderBuffer& buffer, bool LeftEdge, int xlimit, const std::vector<int>& SwagArray);
    void DrawCurtainVertical(RenderBuffer& buffer, bool topEdge, int ylimit, const std::vector<int>& SwagArray);
};
