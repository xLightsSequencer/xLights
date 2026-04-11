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

class OnEffect : public RenderableEffect
{
public:
    OnEffect(int id);
    virtual ~OnEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    // Cached from On.json by OnMetadataLoaded(). Exposed as statics so any
    // cold-path code (imports, Metal subclass, tests) can read them without
    // needing a pointer to the singleton effect instance.
    static int sStartDefault;
    static int sEndDefault;
    static bool sShimmerDefault;
    static double sCyclesDefault;
    static int sTransparencyDefault;
    static int sTransparencyMin;
    static int sTransparencyMax;

protected:
    // Pull defaults and value curve bounds out of On.json once at startup so
    // Render() never touches the metadata. Min/max for VC upgrade still come
    // from the base class's JSON-backed GetSettingVCMin/Max (cold path).
    virtual void OnMetadataLoaded() override;
};
