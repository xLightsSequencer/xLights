/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectManager.h"
#include "OffEffect.h"
#include "OnEffect.h"
#include "AdjustEffect.h"
#include "BarsEffect.h"
#include "ButterflyEffect.h"
#include "CandleEffect.h"
#include "CirclesEffect.h"
#include "ColorWashEffect.h"
#include "CurtainEffect.h"
#include "DMXEffect.h"
#include "DuplicateEffect.h"
#include "FacesEffect.h"
#include "FanEffect.h"
#include "FillEffect.h"
#include "FireEffect.h"
#include "FireworksEffect.h"
#include "GalaxyEffect.h"
#include "GarlandsEffect.h"
#include "GlediatorEffect.h"
#include "GuitarEffect.h"
#include "KaleidoscopeEffect.h"
#include "LifeEffect.h"
#include "LightningEffect.h"
#include "LinesEffect.h"
#include "LiquidEffect.h"
#include "MarqueeEffect.h"
#include "MeteorsEffect.h"
#include "MorphEffect.h"
#include "MusicEffect.h"
#include "PianoEffect.h"
#include "PicturesEffect.h"
#include "PinwheelEffect.h"
#include "PlasmaEffect.h"
#include "RenderableEffect.h"
#include "RippleEffect.h"
#include "ServoEffect.h"
#include "ShaderEffect.h"
#include "ShapeEffect.h"
#include "ShimmerEffect.h"
#include "ShockwaveEffect.h"
#include "SingleStrandEffect.h"
#include "SketchEffect.h"
#include "SnowflakesEffect.h"
#include "SnowstormEffect.h"
#include "SpiralsEffect.h"
#include "SpirographEffect.h"
#include "StateEffect.h"
#include "StrobeEffect.h"
#include "TendrilEffect.h"
#include "TextEffect.h"
#include "TreeEffect.h"
#include "TwinkleEffect.h"
#include "VideoEffect.h"
#include "VUMeterEffect.h"
#include "WarpEffect.h"
#include "WaveEffect.h"


#ifdef __WXOSX__
extern RenderableEffect* CreateMetalEffect(EffectManager::RGB_EFFECTS_e eff);
inline RenderableEffect* CreateGPUEffect(EffectManager::RGB_EFFECTS_e eff) {
    return CreateMetalEffect(eff);
}
#else
inline RenderableEffect* CreateGPUEffect(EffectManager::RGB_EFFECTS_e eff) {
    return nullptr;
}
#endif

EffectManager::EffectManager()
{
    add(createEffect(eff_OFF));
    add(createEffect(eff_ON));
    add(createEffect(eff_ADJUST));
    add(createEffect(eff_BARS));
    add(createEffect(eff_BUTTERFLY));
    add(createEffect(eff_CANDLE));
    add(createEffect(eff_CIRCLES));
    add(createEffect(eff_COLORWASH));
    add(createEffect(eff_CURTAIN));
    add(createEffect(eff_DMX));
    add(createEffect(eff_DUPLICATE));
    add(createEffect(eff_FACES));
    add(createEffect(eff_FAN));
    add(createEffect(eff_FILL));
    add(createEffect(eff_FIRE));
    add(createEffect(eff_FIREWORKS));
    add(createEffect(eff_GALAXY));
    add(createEffect(eff_GARLANDS));
    add(createEffect(eff_GLEDIATOR));
    add(createEffect(eff_GUITAR));
    add(createEffect(eff_KALEIDOSCOPE));
    add(createEffect(eff_LIFE));
    add(createEffect(eff_LIGHTNING));
    add(createEffect(eff_LINES));
    add(createEffect(eff_LIQUID));
    add(createEffect(eff_MARQUEE));
    add(createEffect(eff_METEORS));
    add(createEffect(eff_MUSIC));
    add(createEffect(eff_MORPH));
    add(createEffect(eff_PIANO));
    add(createEffect(eff_PICTURES));
    add(createEffect(eff_PINWHEEL));
    add(createEffect(eff_PLASMA));
    add(createEffect(eff_RIPPLE));
    add(createEffect(eff_SERVO));
    add(createEffect(eff_SHADER));
    add(createEffect(eff_SHAPE));
    add(createEffect(eff_SHIMMER));
    add(createEffect(eff_SHOCKWAVE));
    add(createEffect(eff_SINGLESTRAND));
    add(createEffect(eff_SKETCH));
    add(createEffect(eff_SNOWFLAKES));
    add(createEffect(eff_SNOWSTORM));
    add(createEffect(eff_SPIRALS));
    add(createEffect(eff_SPIROGRAPH));
    add(createEffect(eff_STATE));
    add(createEffect(eff_STROBE));
    add(createEffect(eff_TENDRIL));
    add(createEffect(eff_TEXT));
	add(createEffect(eff_TREE));
	add(createEffect(eff_TWINKLE));
	add(createEffect(eff_VIDEO));
	add(createEffect(eff_VUMETER));
    add(createEffect(eff_WARP));
    add(createEffect(eff_WAVE));

    //Map an old name
    effectsByName["CoroFaces"] = GetEffect("Faces");
}

EffectManager::~EffectManager()
{
    for (auto it = begin(); it != end(); ++it) {
        delete *it;
    }
}

RenderableEffect *EffectManager::createEffect(RGB_EFFECTS_e eff) {
    RenderableEffect *effect = CreateGPUEffect(eff);
    if (effect) {
        return effect;
    }
    switch (eff) {
        case eff_OFF: return new OffEffect(eff_OFF);
        case eff_ON: return new OnEffect(eff_ON);
        case eff_ADJUST:
        return new AdjustEffect(eff_ADJUST);
        case eff_BARS:
        return new BarsEffect(eff_BARS);
        case eff_BUTTERFLY: return new ButterflyEffect(eff_BUTTERFLY);
        case eff_CANDLE: return new CandleEffect(eff_CANDLE);
        case eff_CIRCLES: return new CirclesEffect(eff_CIRCLES);
        case eff_COLORWASH: return new ColorWashEffect(eff_COLORWASH);
        case eff_CURTAIN: return new CurtainEffect(eff_CURTAIN);
        case eff_DMX: return new DMXEffect(eff_DMX);
        case eff_DUPLICATE: return new DuplicateEffect(eff_DUPLICATE);
        case eff_FACES: return new FacesEffect(eff_FACES);
        case eff_FAN: return new FanEffect(eff_FAN);
        case eff_FILL: return new FillEffect(eff_FILL);
        case eff_FIRE: return new FireEffect(eff_FIRE);
        case eff_FIREWORKS: return new FireworksEffect(eff_FIREWORKS);
        case eff_GALAXY: return new GalaxyEffect(eff_GALAXY);
        case eff_GARLANDS: return new GarlandsEffect(eff_GARLANDS);
        case eff_GLEDIATOR: return new GlediatorEffect(eff_GLEDIATOR);
        case eff_GUITAR: return new GuitarEffect(eff_GUITAR);
        case eff_KALEIDOSCOPE: return new KaleidoscopeEffect(eff_KALEIDOSCOPE);
        case eff_LIFE: return new LifeEffect(eff_LIFE);
        case eff_LIGHTNING: return new LightningEffect(eff_LIGHTNING);
        case eff_LINES: return new LinesEffect(eff_LINES);
        case eff_LIQUID: return new LiquidEffect(eff_LIQUID);
        case eff_MARQUEE: return new MarqueeEffect(eff_MARQUEE);
        case eff_METEORS: return new MeteorsEffect(eff_METEORS);
        case eff_MUSIC: return new MusicEffect(eff_MUSIC);
        case eff_MORPH: return new MorphEffect(eff_MORPH);
        case eff_PIANO: return new PianoEffect(eff_PIANO);
        case eff_PICTURES: return new PicturesEffect(eff_PICTURES);
        case eff_PINWHEEL: return new PinwheelEffect(eff_PINWHEEL);
        case eff_PLASMA: return new PlasmaEffect(eff_PLASMA);
        case eff_RIPPLE: return new RippleEffect(eff_RIPPLE);
        case eff_SERVO: return new ServoEffect (eff_SERVO);
        case eff_SHADER: return new ShaderEffect(eff_SHADER);
        case eff_SHAPE: return new ShapeEffect (eff_SHAPE);
        case eff_SHIMMER: return new ShimmerEffect(eff_SHIMMER);
        case eff_SHOCKWAVE: return new ShockwaveEffect(eff_SHOCKWAVE);
        case eff_SINGLESTRAND: return new SingleStrandEffect(eff_SINGLESTRAND);
        case eff_SKETCH: return new SketchEffect(eff_SKETCH);
        case eff_SNOWFLAKES: return new SnowflakesEffect(eff_SNOWFLAKES);
        case eff_SNOWSTORM: return new SnowstormEffect(eff_SNOWSTORM);
        case eff_SPIRALS: return new SpiralsEffect(eff_SPIRALS);
        case eff_SPIROGRAPH: return new SpirographEffect(eff_SPIROGRAPH);
        case eff_STATE: return new StateEffect(eff_STATE);
        case eff_STROBE: return new StrobeEffect(eff_STROBE);
        case eff_TENDRIL: return new TendrilEffect(eff_TENDRIL);
        case eff_TEXT: return new TextEffect(eff_TEXT);
        case eff_TREE: return new TreeEffect(eff_TREE);
        case eff_TWINKLE: return new TwinkleEffect(eff_TWINKLE);
        case eff_VIDEO: return new VideoEffect(eff_VIDEO);
        case eff_VUMETER: return new VUMeterEffect(eff_VUMETER);
        case eff_WARP: return new WarpEffect(eff_WARP);
        case eff_WAVE: return new WaveEffect(eff_WAVE);
        default: return nullptr;
    }
}



void EffectManager::add(RenderableEffect *eff) {
    int id = eff->GetId();
    if (id >= size()) {
        effects.resize(id + 1);
    }
    effects[id] = eff;
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

std::vector<RenderableEffect*>::const_iterator EffectManager::begin() const {
    return effects.begin();
}

std::vector<RenderableEffect*>::const_iterator EffectManager::end() const {
    return effects.end();
}
