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

class FanEffect : public RenderableEffect
{
public:
    FanEffect(int id);
    virtual ~FanEffect();
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                     xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    static int sCenterXDefault;
    static int sCenterXMin;
    static int sCenterXMax;
    static int sCenterYDefault;
    static int sCenterYMin;
    static int sCenterYMax;
    static int sStartRadiusDefault;
    static int sStartRadiusMin;
    static int sStartRadiusMax;
    static int sStartAngleDefault;
    static int sStartAngleMin;
    static int sStartAngleMax;
    static int sEndRadiusDefault;
    static int sEndRadiusMin;
    static int sEndRadiusMax;
    // Fan_Revolutions has divisor 360 in JSON — sRevolutionsDefault is the
    // pre-divisor tick value (JSON default * divisor) so it matches what
    // GetValueCurveInt expects.
    static int sRevolutionsDefault;
    static int sRevolutionsMin;
    static int sRevolutionsMax;
    static int sRevolutionsDivisor;
    static int sNumBladesDefault;
    static int sNumBladesMin;
    static int sNumBladesMax;
    static int sBladeWidthDefault;
    static int sBladeWidthMin;
    static int sBladeWidthMax;
    static int sBladeAngleDefault;
    static int sBladeAngleMin;
    static int sBladeAngleMax;
    static int sNumElementsDefault;
    static int sNumElementsMin;
    static int sNumElementsMax;
    static int sElementWidthDefault;
    static int sElementWidthMin;
    static int sElementWidthMax;
    static int sDurationDefault;
    static int sDurationMin;
    static int sDurationMax;
    static int sAccelDefault;
    static int sAccelMin;
    static int sAccelMax;
    static bool sReverseDefault;
    static bool sBlendEdgesDefault;
    static bool sScaleDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
