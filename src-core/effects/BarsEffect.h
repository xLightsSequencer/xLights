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

class BarsEffect : public RenderableEffect
{
public:
    BarsEffect(int id);
    virtual ~BarsEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    static int sBarCountDefault;
    static int sBarCountMin;
    static int sBarCountMax;
    static double sCyclesDefault;
    static double sCyclesMin;
    static double sCyclesMax;
    static int sCyclesDivisor;
    static std::string sDirectionDefault;
    static double sCenterDefault;
    static double sCenterMin;
    static double sCenterMax;
    static bool sHighlightDefault;
    static bool sUseFirstColorForHighlightDefault;
    static bool s3DDefault;
    static bool sGradientDefault;

protected:
    virtual void OnMetadataLoaded() override;
    void GetSpatialColor(xlColor& color, size_t colorIndex, float x, float y, RenderBuffer& buffer, bool gradient, const xlColor& highlightColor, bool highlight, bool show3d, int BarHt, int n, float pct, int color2Index);
};
