#include "EffectManager.h"

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
#include "TendrilEffect.h"
#include "TextEffect.h"
#include "TreeEffect.h"
#include "TwinkleEffect.h"
#include "WaveEffect.h"


EffectManager::EffectManager() : std::vector<RenderableEffect*>()
{
    add(new OffEffect(eff_OFF));
    add(new OnEffect(eff_ON));
    add(new BarsEffect(eff_BARS));
    add(new ButterflyEffect(eff_BUTTERFLY));
    add(new CirclesEffect(eff_CIRCLES));
    add(new ColorWashEffect(eff_COLORWASH));
    add(new CurtainEffect(eff_CURTAIN));
    add(new DMXEffect(eff_DMX));
    add(new FacesEffect(eff_FACES));
    add(new FanEffect(eff_FAN));
    add(new FireEffect(eff_FIRE));
    add(new FireworksEffect(eff_FIREWORKS));
    add(new GalaxyEffect(eff_GALAXY));
    add(new GarlandsEffect(eff_GARLANDS));
    add(new GlediatorEffect(eff_GLEDIATOR));
    add(new LifeEffect(eff_LIFE));
    add(new LightningEffect(eff_LIGHTNING));
    add(new MarqueeEffect(eff_MARQUEE));
    add(new MeteorsEffect(eff_METEORS));
    add(new MorphEffect(eff_MORPH));
    add(new PianoEffect(eff_PIANO));
    add(new PicturesEffect(eff_PICTURES));
    add(new PinwheelEffect(eff_PINWHEEL));
    add(new PlasmaEffect(eff_PLASMA));
    add(new RippleEffect(eff_RIPPLE));
    add(new ShimmerEffect(eff_SHIMMER));
    add(new ShockwaveEffect(eff_SHOCKWAVE));
    add(new SingleStrandEffect(eff_SINGLESTRAND));
    add(new SnowflakesEffect(eff_SNOWFLAKES));
    add(new SnowstormEffect(eff_SNOWSTORM));
    add(new SpiralsEffect(eff_SPIRALS));
    add(new SpirographEffect(eff_SPIROGRAPH));
    add(new StrobeEffect(eff_STROBE));
    add(new TendrilEffect(eff_TENDRIL));
    add(new TextEffect(eff_TEXT));
    add(new TreeEffect(eff_TREE));
    add(new TwinkleEffect(eff_TWINKLE));
    add(new WaveEffect(eff_WAVE));

    //Map an old name
    effectsByName["CoroFaces"] = GetEffect("Faces");
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
    effectsByName[eff->ToolTip()] = eff;
}

RenderableEffect *EffectManager::GetEffect(const std::string &str) const {
    return effectsByName[str];
}

int EffectManager::GetEffectIndex(const std::string &effectName) const {
    RenderableEffect *eff = GetEffect(effectName);
    if (eff != nullptr) {
        return eff->GetId();
    }
    return -1;
}
const std::string &EffectManager::GetEffectName(int idx) const {
    RenderableEffect *eff = GetEffect(idx);
    if (eff != nullptr) {
        return eff->Name();
    }
    return GetEffect(0)->Name();
}


