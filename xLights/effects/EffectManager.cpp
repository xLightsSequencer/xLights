#include "EffectManager.h"


#include "../BitmapCache.h"

#include "OffEffect.h"
#include "OnEffect.h"
#include "BarsEffect.h"
#include "ButterflyEffect.h"
#include "CirclesEffect.h"
#include "ColorWashEffect.h"
#include "CurtainEffect.h"
#include "DMXEffect.h"
#include "FacesEffect.h"
#include "FanEffect.h"
#include "FireEffect.h"
#include "FireworksEffect.h"
#include "GalaxyEffect.h"
#include "GarlandsEffect.h"
#include "GlediatorEffect.h"
#include "LifeEffect.h"
#include "LightningEffect.h"
#include "MarqueeEffect.h"
#include "MeteorsEffect.h"
#include "MorphEffect.h"
#include "OffEffect.h"
#include "OnEffect.h"
#include "PianoEffect.h"
#include "PicturesEffect.h"
#include "PinwheelEffect.h"
#include "PlasmaEffect.h"
#include "RenderableEffect.h"
#include "RippleEffect.h"
#include "ShimmerEffect.h"
#include "ShockwaveEffect.h"
#include "SingleStrandEffect.h"
#include "SnowflakesEffect.h"
#include "SnowstormEffect.h"
#include "SpiralsEffect.h"
#include "SpirographEffect.h"
#include "StrobeEffect.h"
#include "TextEffect.h"
#include "TreeEffect.h"
#include "TwinkleEffect.h"
#include "WaveEffect.h"


EffectManager::EffectManager() : last(-1)
{
    effectsById[BitmapCache::eff_OFF] = new OffEffect(BitmapCache::eff_OFF);
    effectsById[BitmapCache::eff_ON] = new OnEffect(BitmapCache::eff_ON);
    effectsById[BitmapCache::eff_BARS] = new BarsEffect(BitmapCache::eff_BARS);
    effectsById[BitmapCache::eff_BUTTERFLY] = new ButterflyEffect(BitmapCache::eff_BUTTERFLY);
    effectsById[BitmapCache::eff_CIRCLES] = new CirclesEffect(BitmapCache::eff_CIRCLES);
    effectsById[BitmapCache::eff_COLORWASH] = new ColorWashEffect(BitmapCache::eff_COLORWASH);
    effectsById[BitmapCache::eff_CURTAIN] = new CurtainEffect(BitmapCache::eff_CURTAIN);
    effectsById[BitmapCache::eff_DMX] = new DMXEffect(BitmapCache::eff_DMX);
    effectsById[BitmapCache::eff_FACES] = new FacesEffect(BitmapCache::eff_FACES);
    effectsById[BitmapCache::eff_FAN] = new FanEffect(BitmapCache::eff_FAN);
    effectsById[BitmapCache::eff_FIRE] = new FireEffect(BitmapCache::eff_FIRE);
    effectsById[BitmapCache::eff_FIREWORKS] = new FireworksEffect(BitmapCache::eff_FIREWORKS);
    effectsById[BitmapCache::eff_GALAXY] = new GalaxyEffect(BitmapCache::eff_GALAXY);
    effectsById[BitmapCache::eff_GARLANDS] = new GarlandsEffect(BitmapCache::eff_GARLANDS);
    effectsById[BitmapCache::eff_GLEDIATOR] = new GlediatorEffect(BitmapCache::eff_GLEDIATOR);
    effectsById[BitmapCache::eff_LIFE] = new LifeEffect(BitmapCache::eff_LIFE);
    effectsById[BitmapCache::eff_LIGHTNING] = new LightningEffect(BitmapCache::eff_LIGHTNING);
    effectsById[BitmapCache::eff_MARQUEE] = new MarqueeEffect(BitmapCache::eff_MARQUEE);
    effectsById[BitmapCache::eff_METEORS] = new MeteorsEffect(BitmapCache::eff_METEORS);
    effectsById[BitmapCache::eff_MORPH] = new MorphEffect(BitmapCache::eff_MORPH);
    effectsById[BitmapCache::eff_PIANO] = new PianoEffect(BitmapCache::eff_PIANO);
    effectsById[BitmapCache::eff_PICTURES] = new PicturesEffect(BitmapCache::eff_PICTURES);
    effectsById[BitmapCache::eff_PINWHEEL] = new PinwheelEffect(BitmapCache::eff_PINWHEEL);
    effectsById[BitmapCache::eff_PLASMA] = new PlasmaEffect(BitmapCache::eff_PLASMA);
    effectsById[BitmapCache::eff_RIPPLE] = new RippleEffect(BitmapCache::eff_RIPPLE);
    effectsById[BitmapCache::eff_SHIMMER] = new ShimmerEffect(BitmapCache::eff_SHIMMER);
    effectsById[BitmapCache::eff_SHOCKWAVE] = new ShockwaveEffect(BitmapCache::eff_SHOCKWAVE);
    effectsById[BitmapCache::eff_SINGLESTRAND] = new SingleStrandEffect(BitmapCache::eff_SINGLESTRAND);
    effectsById[BitmapCache::eff_SNOWFLAKES] = new SnowflakesEffect(BitmapCache::eff_SNOWFLAKES);
    effectsById[BitmapCache::eff_SNOWSTORM] = new SnowstormEffect(BitmapCache::eff_SNOWSTORM);
    effectsById[BitmapCache::eff_SPIRALS] = new SpiralsEffect(BitmapCache::eff_SPIRALS);
    effectsById[BitmapCache::eff_SPIROGRAPH] = new SpirographEffect(BitmapCache::eff_SPIROGRAPH);
    effectsById[BitmapCache::eff_STROBE] = new StrobeEffect(BitmapCache::eff_STROBE);
    effectsById[BitmapCache::eff_TEXT] = new TextEffect(BitmapCache::eff_TEXT);
    effectsById[BitmapCache::eff_TREE] = new TreeEffect(BitmapCache::eff_TREE);
    effectsById[BitmapCache::eff_TWINKLE] = new TwinkleEffect(BitmapCache::eff_TWINKLE);
    effectsById[BitmapCache::eff_WAVE] = new WaveEffect(BitmapCache::eff_WAVE);

    last = BitmapCache::eff_WAVE;
    
    for (int x = 0; x <= last; x++) {
        RenderableEffect * eff = effectsById[x];
        if (eff != nullptr) {
            effectsByName[eff->Name()] = eff;
        }
    }
}

EffectManager::~EffectManager()
{
    for (int x = 0; x <= last; x++) {
        RenderableEffect * eff = effectsById[x];
        if (eff != nullptr) {
            delete eff;
        }
    }
}


RenderableEffect *EffectManager::GetEffect(int i) const {
    return effectsById[i];
}
RenderableEffect *EffectManager::GetEffect(const std::string &str) const {
    return effectsByName[str];
}

int EffectManager::GetLastEffectId() const {
    return last;
}
