/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectPanelManager.h"
#include "EffectPanelUtils.h"

#include <wx/debug.h>

#include "AdjustPanel.h"
#include "BarsPanel.h"
#include "ButterflyPanel.h"
#include "CandlePanel.h"
#include "CirclesPanel.h"
#include "ColorWashPanel.h"
#include "CurtainPanel.h"
#include "DMXPanel.h"
#include "DuplicatePanel.h"
#include "FacesPanel.h"
#include "FanPanel.h"
#include "FillPanel.h"
#include "FirePanel.h"
#include "FireworksPanel.h"
#include "GalaxyPanel.h"
#include "GarlandsPanel.h"
#include "GlediatorPanel.h"
#include "GuitarPanel.h"
#include "KaleidoscopePanel.h"
#include "LifePanel.h"
#include "LightningPanel.h"
#include "LinesPanel.h"
#include "LiquidPanel.h"
#include "MarqueePanel.h"
#include "MeteorsPanel.h"
#include "MorphPanel.h"
#include "MovingHeadPanel.h"
#include "MusicPanel.h"
#include "OffPanel.h"
#include "OnPanel.h"
#include "PianoPanel.h"
#include "PicturesPanel.h"
#include "PinwheelPanel.h"
#include "PlasmaPanel.h"
#include "RipplePanel.h"
#include "ServoPanel.h"
#include "ShaderPanel.h"
#include "ShapePanel.h"
#include "ShimmerPanel.h"
#include "ShockwavePanel.h"
#include "SingleStrandPanel.h"
#include "SketchPanel.h"
#include "SnowflakesPanel.h"
#include "SnowstormPanel.h"
#include "SpiralsPanel.h"
#include "SpirographPanel.h"
#include "StatePanel.h"
#include "StrobePanel.h"
#include "TendrilPanel.h"
#include "TextPanel.h"
#include "TreePanel.h"
#include "TwinklePanel.h"
#include "VideoPanel.h"
#include "VUMeterPanel.h"
#include "WarpPanel.h"
#include "WavePanel.h"

#include "../../effects/EffectManager.h"

EffectPanelManager::EffectPanelManager() {
    RegisterPanels();
}

EffectPanelManager::~EffectPanelManager() {
}

void EffectPanelManager::RegisterPanel(int effectId, const std::string& name, PanelFactory factory) {
    if (effectId >= static_cast<int>(panels.size())) {
        panels.resize(effectId + 1);
    }
    wxASSERT_MSG(panels[effectId].factory == nullptr, wxString::Format("Duplicate registration for effect id %d (%s)", effectId, name));
    wxASSERT_MSG(panelsByName.find(name) == panelsByName.end(), wxString::Format("Duplicate registration for effect name '%s'", name));
    panels[effectId] = {factory, name, nullptr};
    panelsByName[name] = effectId;
}

void EffectPanelManager::RegisterPanels() {
    using E = EffectManager::RGB_EFFECTS_e;

    Register<OffPanel>(E::eff_OFF, "Off");
    Register<OnPanel>(E::eff_ON, "On");
    Register<AdjustPanel>(E::eff_ADJUST, "Adjust");
    Register<BarsPanel>(E::eff_BARS, "Bars");
    Register<ButterflyPanel>(E::eff_BUTTERFLY, "Butterfly");
    Register<CandlePanel>(E::eff_CANDLE, "Candle");
    Register<CirclesPanel>(E::eff_CIRCLES, "Circles");
    Register<ColorWashPanel>(E::eff_COLORWASH, "Color Wash");
    Register<CurtainPanel>(E::eff_CURTAIN, "Curtain");
    Register<DMXPanel>(E::eff_DMX, "DMX");
    Register<DuplicatePanel>(E::eff_DUPLICATE, "Duplicate");
    Register<FacesPanel>(E::eff_FACES, "Faces");
    Register<FanPanel>(E::eff_FAN, "Fan");
    Register<FillPanel>(E::eff_FILL, "Fill");
    Register<FirePanel>(E::eff_FIRE, "Fire");
    Register<FireworksPanel>(E::eff_FIREWORKS, "Fireworks");
    Register<GalaxyPanel>(E::eff_GALAXY, "Galaxy");
    Register<GarlandsPanel>(E::eff_GARLANDS, "Garlands");
    Register<GlediatorPanel>(E::eff_GLEDIATOR, "Glediator");
    Register<GuitarPanel>(E::eff_GUITAR, "Guitar");
    Register<KaleidoscopePanel>(E::eff_KALEIDOSCOPE, "Kaleidoscope");
    Register<LifePanel>(E::eff_LIFE, "Life");
    Register<LightningPanel>(E::eff_LIGHTNING, "Lightning");
    Register<LinesPanel>(E::eff_LINES, "Lines");
    Register<LiquidPanel>(E::eff_LIQUID, "Liquid");
    Register<MarqueePanel>(E::eff_MARQUEE, "Marquee");
    Register<MeteorsPanel>(E::eff_METEORS, "Meteors");
    Register<MorphPanel>(E::eff_MORPH, "Morph");
    Register<MovingHeadPanel>(E::eff_MOVINGHEAD, "Moving Head");
    Register<MusicPanel>(E::eff_MUSIC, "Music");
    Register<PianoPanel>(E::eff_PIANO, "Piano");
    Register<PicturesPanel>(E::eff_PICTURES, "Pictures");
    Register<PinwheelPanel>(E::eff_PINWHEEL, "Pinwheel");
    Register<PlasmaPanel>(E::eff_PLASMA, "Plasma");
    Register<RipplePanel>(E::eff_RIPPLE, "Ripple");
    Register<ServoPanel>(E::eff_SERVO, "Servo");
    Register<ShaderPanel>(E::eff_SHADER, "Shader");
    Register<ShapePanel>(E::eff_SHAPE, "Shape");
    Register<ShimmerPanel>(E::eff_SHIMMER, "Shimmer");
    Register<ShockwavePanel>(E::eff_SHOCKWAVE, "Shockwave");
    Register<SingleStrandPanel>(E::eff_SINGLESTRAND, "Single Strand");
    Register<SketchPanel>(E::eff_SKETCH, "Sketch");
    Register<SnowflakesPanel>(E::eff_SNOWFLAKES, "Snowflakes");
    Register<SnowstormPanel>(E::eff_SNOWSTORM, "Snowstorm");
    Register<SpiralsPanel>(E::eff_SPIRALS, "Spirals");
    Register<SpirographPanel>(E::eff_SPIROGRAPH, "Spirograph");
    Register<StatePanel>(E::eff_STATE, "State");
    Register<StrobePanel>(E::eff_STROBE, "Strobe");
    Register<TendrilPanel>(E::eff_TENDRIL, "Tendril");
    Register<TextPanel>(E::eff_TEXT, "Text");
    Register<TreePanel>(E::eff_TREE, "Tree");
    Register<TwinklePanel>(E::eff_TWINKLE, "Twinkle");
    Register<VideoPanel>(E::eff_VIDEO, "Video");
    Register<VUMeterPanel>(E::eff_VUMETER, "VU Meter");
    Register<WarpPanel>(E::eff_WARP, "Warp");
    Register<WavePanel>(E::eff_WAVE, "Wave");
}

xlEffectPanel* EffectPanelManager::GetPanel(int effectId, wxWindow* parent) {
    if (effectId < 0 || effectId >= static_cast<int>(panels.size())) {
        return nullptr;
    }
    auto& info = panels[effectId];
    if (info.panel == nullptr && info.factory && parent != nullptr) {
        info.panel = info.factory(parent);
    }
    return info.panel;
}

xlEffectPanel* EffectPanelManager::GetPanel(const std::string& effectName, wxWindow* parent) {
    auto it = panelsByName.find(effectName);
    if (it != panelsByName.end()) {
        return GetPanel(it->second, parent);
    }
    return nullptr;
}

void EffectPanelManager::SetDefaultParameters(int effectId) {
    if (effectId >= 0 && effectId < static_cast<int>(panels.size()) && panels[effectId].panel != nullptr) {
        panels[effectId].panel->SetDefaultParameters();
    }
}

void EffectPanelManager::SetDefaultParameters(const std::string& effectName) {
    auto it = panelsByName.find(effectName);
    if (it != panelsByName.end()) {
        SetDefaultParameters(it->second);
    }
}

void EffectPanelManager::SetPanelStatus(int effectId, Model* cls) {
    if (effectId >= 0 && effectId < static_cast<int>(panels.size()) && panels[effectId].panel != nullptr) {
        panels[effectId].panel->SetPanelStatus(cls);
    }
}

void EffectPanelManager::SetPanelStatus(const std::string& effectName, Model* cls) {
    auto it = panelsByName.find(effectName);
    if (it != panelsByName.end()) {
        SetPanelStatus(it->second, cls);
    }
}

void EffectPanelManager::SetEffectTimeRange(int effectId, int startTimeMs, int endTimeMs) {
    if (effectId >= 0 && effectId < static_cast<int>(panels.size()) && panels[effectId].panel != nullptr) {
        panels[effectId].panel->SetEffectTimeRange(startTimeMs, endTimeMs);
    }
}

void EffectPanelManager::SetSequenceElements(SequenceElements* els) {
    for (auto& info : panels) {
        if (info.panel != nullptr) {
            info.panel->SetSequenceElements(els);
        }
    }
}

wxString EffectPanelManager::GetEffectString(int effectId) {
    if (effectId >= 0 && effectId < static_cast<int>(panels.size()) && panels[effectId].panel != nullptr) {
        return panels[effectId].panel->GetEffectString();
    }
    return wxString();
}
