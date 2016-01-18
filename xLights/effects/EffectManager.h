#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H


#include <map>
#include <string>
#include <vector>

class RenderableEffect;

class EffectManager : public std::vector<RenderableEffect*>
{
public:
    //TODO - get rid of this, the internal id's should be irrelevant
    enum RGB_EFFECTS_e {
        eff_OFF,
        eff_ON,
        eff_BARS,
        eff_BUTTERFLY,
        eff_CIRCLES,
        eff_COLORWASH,
        eff_CURTAIN,
        eff_DMX,
        eff_FACES,
        eff_FAN,
        eff_FIRE,
        eff_FIREWORKS,
        eff_GALAXY,
        eff_GARLANDS,
        eff_GLEDIATOR,
        eff_LIFE,
        eff_LIGHTNING,
        eff_MARQUEE,
        eff_METEORS,
        eff_MORPH,
        eff_PIANO,
        eff_PICTURES,
        eff_PINWHEEL,
        eff_PLASMA,
        eff_RIPPLE,
        eff_SHIMMER,
        eff_SHOCKWAVE,
        eff_SINGLESTRAND,
        eff_SNOWFLAKES,
        eff_SNOWSTORM,
        eff_SPIRALS,
        eff_SPIROGRAPH,
        eff_STROBE,
        eff_TENDRIL,
        eff_TEXT,
        eff_TREE,
        eff_TWINKLE,
        eff_WAVE,
        eff_LASTEFFECT //Always the last entry
    };


    public:
        EffectManager();
        virtual ~EffectManager();


        RenderableEffect *GetEffect(int i) const {
            if (i >= size()) {
                return nullptr;
            }
            return (*this)[i];
        }
        RenderableEffect *GetEffect(const std::string &str) const;
        int GetLastEffectId() const { return size() - 1;};


        int GetEffectIndex(const std::string &effectName) const;
        const std::string &GetEffectName(int idx) const;

    protected:
    private:
        void add(RenderableEffect *eff);

        mutable std::map<std::string, RenderableEffect *> effectsByName;
};

#endif // EFFECTMANAGER_H
