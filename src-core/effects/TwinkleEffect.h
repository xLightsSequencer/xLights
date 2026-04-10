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

class TwinkleEffect : public RenderableEffect
{
public:
    TwinkleEffect(int id);
    virtual ~TwinkleEffect();
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;

    // Cached from Twinkle.json by OnMetadataLoaded().
    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static int sStepsDefault;
    // Twinkle_Steps slider goes 2..400 but the VC is clamped to 2..100 via the
    // JSON `vcMax` field. sStepsVCMin/Max are the bounds passed to
    // GetValueCurveInt at render time (so existing VC data in [2, 100]
    // stays compatible). The base class GetSettingVCMin/Max also honors
    // vcMin/vcMax so UpgradeValueCurve gets the same answer.
    static int sStepsVCMin;
    static int sStepsVCMax;
    static bool sStrobeDefault;
    static bool sReRandomDefault;
    static std::string sStyleDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
