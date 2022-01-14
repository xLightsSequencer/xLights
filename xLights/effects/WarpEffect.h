#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

class WarpEffect : public RenderableEffect
{
    public:
        enum WarpType {
            WATER_DROPS = 0,
            SINGLE_WATER_DROP,
            CIRCLE_REVEAL,
            BANDED_SWIRL,
            CIRCULAR_SWIRL,
            DISSOLVE,
            RIPPLE,
            DROP,
            WAVY,
            SAMPLE_ON,
            MIRROR,
            COPY,
            
            COUNT_WARP_STYLES
        };
        WarpType mapWarpType(const std::string &wt);
    
    
        WarpEffect(int id);
        virtual ~WarpEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) const override { return false; }
        virtual void SetDefaultParameters() override;
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    protected:
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual xlEffectPanel *CreatePanel(wxWindow *parent) override;
};
