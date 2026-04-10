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
#include "UtilFunctions.h"

class MorphEffect : public RenderableEffect
{
public:
    MorphEffect(int id);
    virtual ~MorphEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool PressButton(const std::string& id, SettingsMap& paletteMap, SettingsMap& settings) override;

    // Cached from Morph.json by OnMetadataLoaded().
    static int sStartX1Default;
    static int sStartY1Default;
    static int sStartX2Default;
    static int sStartY2Default;
    static int sEndX1Default;
    static int sEndY1Default;
    static int sEndX2Default;
    static int sEndY2Default;
    static int sXMin;
    static int sXMax;
    static int sYMin;
    static int sYMax;
    static int sStartLengthDefault;
    static int sStartLengthMin;
    static int sStartLengthMax;
    static int sEndLengthDefault;
    static int sEndLengthMin;
    static int sEndLengthMax;
    static int sDurationDefault;
    static int sDurationMin;
    static int sDurationMax;
    static int sAccelDefault;
    static int sAccelMin;
    static int sAccelMax;
    static int sRepeatCountDefault;
    static int sRepeatCountMin;
    static int sRepeatCountMax;
    static int sRepeatSkipDefault;
    static int sRepeatSkipMin;
    static int sRepeatSkipMax;
    static int sStaggerDefault;
    static int sStaggerMin;
    static int sStaggerMax;
    static bool sStartLinkDefault;
    static bool sEndLinkDefault;
    static bool sShowHeadAtStartDefault;
    static bool sAutoRepeatDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
