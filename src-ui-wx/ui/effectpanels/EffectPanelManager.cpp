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
#include "JsonEffectPanel.h"
#include "assist/AssistPanel.h"

#include <wx/debug.h>
#include <vector>
#include <spdlog/spdlog.h>

#include "AdjustPanel.h"
#include "DMXPanel.h"
#include "DuplicatePanel.h"
#include "FacesPanel.h"
#include "MorphPanel.h"
#include "MovingHeadPanel.h"
#include "MusicPanel.h"
#include "PianoPanel.h"
#include "PicturesPanel.h"
#include "RipplePanel.h"
#include "ServoPanel.h"
#include "ShaderPanel.h"
#include "ShapePanel.h"
#include "SketchPanel.h"
#include "SnowflakesPanel.h"
#include "StatePanel.h"
#include "TextPanel.h"
#include "VideoPanel.h"
#include "VUMeterPanel.h"
#include "WarpPanel.h"
#include "WavePanel.h"

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "ui/shared/utils/wxUtilities.h"

EffectPanelManager::EffectPanelManager(EffectManager* em)
    : effectManager_(em)
{
    RegisterPanels();
}

const nlohmann::json& EffectPanelManager::GetEffectMetadataFor(EffectManager* em, const std::string& effectName) {
    static const nlohmann::json empty;
    if (em == nullptr) return empty;
    RenderableEffect* eff = em->GetEffect(effectName);
    if (eff == nullptr) {
        spdlog::error("EffectPanelManager: no effect named '{}' in EffectManager", effectName);
        return empty;
    }
    return eff->GetMetadata();
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

std::string EffectPanelManager::GetMetadataDirectory() {
    // Implementation lives in wxUtilities so EffectManager init (which runs
    // before EffectPanelManager is constructed) can reuse it without pulling
    // in the panel manager header.
    return ::GetEffectMetadataDirectory();
}

void EffectPanelManager::RegisterJson(int effectId, const std::string& name) {
    // Panel is constructed lazily. When requested, we look up the effect in the
    // EffectManager and hand it the JSON metadata the manager loaded at startup.
    EffectManager* em = effectManager_;
    RegisterPanel(effectId, name, [em, name](wxWindow* p) -> xlEffectPanel* {
        const nlohmann::json& metadata = GetEffectMetadataFor(em, name);
        if (metadata.is_null() || metadata.empty()) {
            spdlog::error("JsonEffectPanel: no metadata loaded for effect '{}'", name);
            return nullptr;
        }
        return new JsonEffectPanel(p, metadata);
    });
}

void EffectPanelManager::RegisterPanels() {
    using E = EffectManager::RGB_EFFECTS_e;

    // Panels get their JSON metadata from the effect they represent — the
    // EffectManager loaded and attached it at startup, so we never re-parse
    // the file here. The name passed to Register*Json must match the
    // effect's Name() so the EffectManager lookup resolves it.
    RegisterJson(E::eff_OFF, "Off");
    RegisterJson(E::eff_ON, "On");
    RegisterJsonSubclass<AdjustPanel>(E::eff_ADJUST, "Adjust");
    RegisterJson(E::eff_BARS, "Bars");
    RegisterJson(E::eff_BUTTERFLY, "Butterfly");
    RegisterJson(E::eff_CANDLE, "Candle");
    RegisterJson(E::eff_CIRCLES, "Circles");
    RegisterJson(E::eff_COLORWASH, "Color Wash");
    RegisterJson(E::eff_CURTAIN, "Curtain");
    RegisterJsonSubclass<DMXPanel>(E::eff_DMX, "DMX");
    RegisterJsonSubclass<DuplicatePanel>(E::eff_DUPLICATE, "Duplicate");
    RegisterJsonSubclass<FacesPanel>(E::eff_FACES, "Faces");
    RegisterJson(E::eff_FAN, "Fan");
    RegisterJson(E::eff_FILL, "Fill");
    RegisterJson(E::eff_FIRE, "Fire");
    RegisterJson(E::eff_FIREWORKS, "Fireworks");
    RegisterJson(E::eff_GALAXY, "Galaxy");
    RegisterJson(E::eff_GARLANDS, "Garlands");
    RegisterJson(E::eff_GLEDIATOR, "Glediator");
    RegisterJson(E::eff_GUITAR, "Guitar");
    RegisterJson(E::eff_KALEIDOSCOPE, "Kaleidoscope");
    RegisterJson(E::eff_LIFE, "Life");
    RegisterJson(E::eff_LIGHTNING, "Lightning");
    RegisterJson(E::eff_LINES, "Lines");
    RegisterJson(E::eff_LIQUID, "Liquid");
    RegisterJson(E::eff_MARQUEE, "Marquee");
    RegisterJson(E::eff_METEORS, "Meteors");
    RegisterJsonSubclass<MorphPanel>(E::eff_MORPH, "Morph");
    Register<MovingHeadPanel>(E::eff_MOVINGHEAD, "Moving Head");
    RegisterJsonSubclass<MusicPanel>(E::eff_MUSIC, "Music");
    RegisterJsonSubclass<PianoPanel>(E::eff_PIANO, "Piano");
    RegisterJsonSubclass<PicturesPanel>(E::eff_PICTURES, "Pictures");
    RegisterJson(E::eff_PINWHEEL, "Pinwheel");
    RegisterJson(E::eff_PLASMA, "Plasma");
    RegisterJsonSubclass<RipplePanel>(E::eff_RIPPLE, "Ripple");
    RegisterJsonSubclass<ServoPanel>(E::eff_SERVO, "Servo");
    RegisterJsonSubclass<ShaderPanel>(E::eff_SHADER, "Shader");
    RegisterJsonSubclass<ShapePanel>(E::eff_SHAPE, "Shape");
    RegisterJson(E::eff_SHIMMER, "Shimmer");
    RegisterJson(E::eff_SHOCKWAVE, "Shockwave");
    RegisterJson(E::eff_SINGLESTRAND, "SingleStrand");
    RegisterJsonSubclass<SketchPanel>(E::eff_SKETCH, "Sketch");
    RegisterJsonSubclass<SnowflakesPanel>(E::eff_SNOWFLAKES, "Snowflakes");
    RegisterJson(E::eff_SNOWSTORM, "Snowstorm");
    RegisterJson(E::eff_SPIRALS, "Spirals");
    RegisterJson(E::eff_SPIROGRAPH, "Spirograph");
    RegisterJsonSubclass<StatePanel>(E::eff_STATE, "State");
    RegisterJson(E::eff_STROBE, "Strobe");
    RegisterJson(E::eff_TENDRIL, "Tendril");
    RegisterJsonSubclass<TextPanel>(E::eff_TEXT, "Text");
    RegisterJson(E::eff_TREE, "Tree");
    RegisterJson(E::eff_TWINKLE, "Twinkle");
    RegisterJsonSubclass<VideoPanel>(E::eff_VIDEO, "Video");
    RegisterJsonSubclass<VUMeterPanel>(E::eff_VUMETER, "VU Meter");
    RegisterJsonSubclass<WarpPanel>(E::eff_WARP, "Warp");
    RegisterJsonSubclass<WavePanel>(E::eff_WAVE, "Wave");
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

bool EffectPanelManager::HasAssistPanel(int effectId) {
    if (effectId >= 0 && effectId < static_cast<int>(panels.size()) && panels[effectId].panel != nullptr) {
        return panels[effectId].panel->HasAssistPanel();
    }
    return false;
}

AssistPanel* EffectPanelManager::GetAssistPanel(int effectId, wxWindow* parent, xLightsFrame* xl_frame) {
    if (effectId >= 0 && effectId < static_cast<int>(panels.size()) && panels[effectId].panel != nullptr) {
        return panels[effectId].panel->GetAssistPanel(parent, xl_frame);
    }
    return nullptr;
}
