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

class MarqueeEffect : public RenderableEffect
{
public:
    MarqueeEffect(int id);
    virtual ~MarqueeEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    // Cached from Marquee.json by OnMetadataLoaded().
    static int sBandSizeDefault;
    static int sBandSizeMin;
    static int sBandSizeMax;
    static int sSkipSizeDefault;
    static int sSkipSizeMin;
    static int sSkipSizeMax;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static int sStaggerDefault;
    static int sStaggerMin;
    static int sStaggerMax;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;
    static int sStartDefault;
    static int sStartMin;
    static int sStartMax;
    static bool sReverseDefault;
    static int sScaleXDefault;
    static int sScaleXMin;
    static int sScaleXMax;
    static int sScaleYDefault;
    static int sScaleYMin;
    static int sScaleYMax;
    static bool sPixelOffsetsDefault;
    static int sXCDefault;
    static int sXCMin;
    static int sXCMax;
    static bool sWrapXDefault;
    static int sYCDefault;
    static int sYCMin;
    static int sYCMax;
    static bool sWrapYDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
