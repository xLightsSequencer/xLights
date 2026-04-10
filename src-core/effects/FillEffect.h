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

class FillEffect : public RenderableEffect
{
public:
    FillEffect(int id);
    virtual ~FillEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    static int sPositionDefault;
    static int sPositionMin;
    static int sPositionMax;
    static int sBandSizeDefault;
    static int sBandSizeMin;
    static int sBandSizeMax;
    static int sSkipSizeDefault;
    static int sSkipSizeMin;
    static int sSkipSizeMax;
    static int sOffsetDefault;
    static int sOffsetMin;
    static int sOffsetMax;
    static bool sOffsetInPixelsDefault;
    static bool sColorTimeDefault;
    static bool sWrapDefault;
    static std::string sDirectionDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
