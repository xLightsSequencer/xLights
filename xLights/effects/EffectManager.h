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

#include <map>
#include <string>
#include <vector>

class RenderableEffect;

class EffectManager
{
public:
    //TODO - get rid of this, the internal id's should be irrelevant
    enum RGB_EFFECTS_e {
        eff_OFF,
        eff_ON,
        eff_ADJUST,
        eff_BARS,
        eff_BUTTERFLY,
        eff_CANDLE,
        eff_CIRCLES,
        eff_COLORWASH,
        eff_CURTAIN,
        eff_DMX,
        eff_DUPLICATE,
        eff_FACES,
        eff_FAN,
        eff_FILL,
        eff_FIRE,
        eff_FIREWORKS,
        eff_GALAXY,
        eff_GARLANDS,
        eff_GLEDIATOR,
        eff_GUITAR,
        eff_KALEIDOSCOPE,
        eff_LIFE,
        eff_LIGHTNING,
        eff_LINES,
		eff_LIQUID,
        eff_MARQUEE,
        eff_METEORS,
        eff_MORPH,
        eff_MUSIC,
        eff_PIANO,
        eff_PICTURES,
        eff_PINWHEEL,
        eff_PLASMA,
        eff_RIPPLE,
        eff_SERVO,
        eff_SHADER,
        eff_SHAPE,
        eff_SHIMMER,
        eff_SHOCKWAVE,
        eff_SINGLESTRAND,
        eff_SKETCH,
        eff_SNOWFLAKES,
        eff_SNOWSTORM,
        eff_SPIRALS,
        eff_SPIROGRAPH,
        eff_STATE,
        eff_STROBE,
		eff_TENDRIL,
		eff_TEXT,
		eff_TREE,
		eff_TWINKLE,
		eff_VIDEO,
		eff_VUMETER,
        eff_WARP,
        eff_WAVE,
        eff_LASTEFFECT //Always the last entry
    };


    public:
        EffectManager();
        virtual ~EffectManager();


        RenderableEffect *GetEffect(int i) const {
            if (i >= (int)size() || i < 0) {
                return nullptr;
            }
            return effects[i];
        }

        RenderableEffect *GetEffect(const std::string &str) const;
        int GetLastEffectId() const { return (int)size() - 1; }

        int GetEffectIndex(const std::string &effectName) const;
        const std::string &GetEffectName(int idx) const;

        std::vector<RenderableEffect*>::const_iterator begin() const;
        std::vector<RenderableEffect*>::const_iterator end() const;
        RenderableEffect *operator[](int i) const {
            return GetEffect(i);
        }
        size_t size() const {
            return effects.size();
        }
    protected:
        RenderableEffect *createEffect(RGB_EFFECTS_e eff);
        void add(RenderableEffect *eff);
    private:

        mutable std::map<std::string, RenderableEffect *> effectsByName;
        std::vector<RenderableEffect *> effects;
};
