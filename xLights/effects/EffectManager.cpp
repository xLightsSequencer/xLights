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


EffectManager::EffectManager() : std::vector<RenderableEffect*>()
{
    add(new OffEffect(BitmapCache::eff_OFF));
    add(new OnEffect(BitmapCache::eff_ON));
    add(new BarsEffect(BitmapCache::eff_BARS));
    add(new ButterflyEffect(BitmapCache::eff_BUTTERFLY));
    add(new CirclesEffect(BitmapCache::eff_CIRCLES));
    add(new ColorWashEffect(BitmapCache::eff_COLORWASH));
    add(new CurtainEffect(BitmapCache::eff_CURTAIN));
    add(new DMXEffect(BitmapCache::eff_DMX));
    add(new FacesEffect(BitmapCache::eff_FACES));
    add(new FanEffect(BitmapCache::eff_FAN));
    add(new FireEffect(BitmapCache::eff_FIRE));
    add(new FireworksEffect(BitmapCache::eff_FIREWORKS));
    add(new GalaxyEffect(BitmapCache::eff_GALAXY));
    add(new GarlandsEffect(BitmapCache::eff_GARLANDS));
    add(new GlediatorEffect(BitmapCache::eff_GLEDIATOR));
    add(new LifeEffect(BitmapCache::eff_LIFE));
    add(new LightningEffect(BitmapCache::eff_LIGHTNING));
    add(new MarqueeEffect(BitmapCache::eff_MARQUEE));
    add(new MeteorsEffect(BitmapCache::eff_METEORS));
    add(new MorphEffect(BitmapCache::eff_MORPH));
    add(new PianoEffect(BitmapCache::eff_PIANO));
    add(new PicturesEffect(BitmapCache::eff_PICTURES));
    add(new PinwheelEffect(BitmapCache::eff_PINWHEEL));
    add(new PlasmaEffect(BitmapCache::eff_PLASMA));
    add(new RippleEffect(BitmapCache::eff_RIPPLE));
    add(new ShimmerEffect(BitmapCache::eff_SHIMMER));
    add(new ShockwaveEffect(BitmapCache::eff_SHOCKWAVE));
    add(new SingleStrandEffect(BitmapCache::eff_SINGLESTRAND));
    add(new SnowflakesEffect(BitmapCache::eff_SNOWFLAKES));
    add(new SnowstormEffect(BitmapCache::eff_SNOWSTORM));
    add(new SpiralsEffect(BitmapCache::eff_SPIRALS));
    add(new SpirographEffect(BitmapCache::eff_SPIROGRAPH));
    add(new StrobeEffect(BitmapCache::eff_STROBE));
    add(new TextEffect(BitmapCache::eff_TEXT));
    add(new TreeEffect(BitmapCache::eff_TREE));
    add(new TwinkleEffect(BitmapCache::eff_TWINKLE));
    add(new WaveEffect(BitmapCache::eff_WAVE));
}

EffectManager::~EffectManager()
{
    for (auto it = begin(); it != end(); it++) {
        delete *it;
    }
}

void EffectManager::add(RenderableEffect *eff) {
    int id = eff->GetId();
    if (id >= size()) {
        resize(id + 1);
    }
    (*this)[id] = eff;
    effectsByName[eff->Name()] = eff;
}

RenderableEffect *EffectManager::GetEffect(const std::string &str) const {
    return effectsByName[str];
}

