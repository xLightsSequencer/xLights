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
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

#include "AdjustPanel.h"
#include "DMXPanel.h"
#include "DuplicatePanel.h"
#include "FacesPanel.h"
#include "FireworksPanel.h"
#include "GuitarPanel.h"
#include "KaleidoscopePanel.h"
#include "LiquidPanel.h"
#include "MorphPanel.h"
#include "MovingHeadPanel.h"
#include "MusicPanel.h"
#include "OffPanel.h"
#include "OnPanel.h"
#include "PianoPanel.h"
#include "PicturesPanel.h"
#include "RipplePanel.h"
#include "ServoPanel.h"
#include "ShaderPanel.h"
#include "ShapePanel.h"
#include "SingleStrandPanel.h"
#include "SketchPanel.h"
#include "SnowflakesPanel.h"
#include "StatePanel.h"
#include "TextPanel.h"
#include "VideoPanel.h"
#include "VUMeterPanel.h"
#include "WarpPanel.h"
#include "WavePanel.h"

#include "effects/EffectManager.h"
#include "ui/shared/utils/wxUtilities.h"

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

std::string EffectPanelManager::GetMetadataDirectory() {
    static std::string cachedDir;
    if (!cachedDir.empty()) return cachedDir;

    std::string resDir = GetResourcesDirectory();
    std::string metaDir = resDir + "/effectmetadata";

    std::error_code ec;
    if (std::filesystem::is_directory(std::filesystem::path(metaDir), ec)) {
        cachedDir = metaDir;
        return cachedDir;
    }
    spdlog::error("Effect metadata directory not found: {}", metaDir);
    return "";
}

void EffectPanelManager::RegisterJson(int effectId, const std::string& name, const std::string& jsonBaseName) {
    // Capture jsonBaseName by value; JSON is loaded lazily when the panel is first created.
    // This avoids depending on GetResourcesDirectory() at registration time.
    RegisterPanel(effectId, name, [jsonBaseName](wxWindow* p) -> xlEffectPanel* {
        std::string metaDir = EffectPanelManager::GetMetadataDirectory();
        if (metaDir.empty()) {
            spdlog::error("JsonEffectPanel: metadata directory not found for {}", jsonBaseName);
            return nullptr;
        }
        std::string path = metaDir + "/" + jsonBaseName + ".json";
        auto metadata = JsonEffectPanel::LoadMetadata(path);
        if (metadata.empty()) {
            spdlog::error("JsonEffectPanel: failed to load {}", path);
            return nullptr;
        }
        return new JsonEffectPanel(p, metadata);
    });
}

void EffectPanelManager::RegisterPanels() {
    using E = EffectManager::RGB_EFFECTS_e;

    Register<OffPanel>(E::eff_OFF, "Off");
    Register<OnPanel>(E::eff_ON, "On");
    Register<AdjustPanel>(E::eff_ADJUST, "Adjust");
    RegisterJson(E::eff_BARS, "Bars", "Bars");
    RegisterJson(E::eff_BUTTERFLY, "Butterfly", "Butterfly");
    RegisterJson(E::eff_CANDLE, "Candle", "Candle");
    RegisterJson(E::eff_CIRCLES, "Circles", "Circles");
    RegisterJson(E::eff_COLORWASH, "Color Wash", "ColorWash");
    RegisterJson(E::eff_CURTAIN, "Curtain", "Curtain");
    Register<DMXPanel>(E::eff_DMX, "DMX");
    Register<DuplicatePanel>(E::eff_DUPLICATE, "Duplicate");
    Register<FacesPanel>(E::eff_FACES, "Faces");
    RegisterJson(E::eff_FAN, "Fan", "Fan");
    RegisterJson(E::eff_FILL, "Fill", "Fill");
    RegisterJson(E::eff_FIRE, "Fire", "Fire");
    Register<FireworksPanel>(E::eff_FIREWORKS, "Fireworks");
    RegisterJson(E::eff_GALAXY, "Galaxy", "Galaxy");
    RegisterJson(E::eff_GARLANDS, "Garlands", "Garlands");
    RegisterJson(E::eff_GLEDIATOR, "Glediator", "Glediator");
    Register<GuitarPanel>(E::eff_GUITAR, "Guitar");
    Register<KaleidoscopePanel>(E::eff_KALEIDOSCOPE, "Kaleidoscope");
    RegisterJson(E::eff_LIFE, "Life", "Life");
    RegisterJson(E::eff_LIGHTNING, "Lightning", "Lightning");
    RegisterJson(E::eff_LINES, "Lines", "Lines");
    Register<LiquidPanel>(E::eff_LIQUID, "Liquid");
    RegisterJson(E::eff_MARQUEE, "Marquee", "Marquee");
    RegisterJson(E::eff_METEORS, "Meteors", "Meteors");
    Register<MorphPanel>(E::eff_MORPH, "Morph");
    Register<MovingHeadPanel>(E::eff_MOVINGHEAD, "Moving Head");
    Register<MusicPanel>(E::eff_MUSIC, "Music");
    Register<PianoPanel>(E::eff_PIANO, "Piano");
    Register<PicturesPanel>(E::eff_PICTURES, "Pictures");
    RegisterJson(E::eff_PINWHEEL, "Pinwheel", "Pinwheel");
    RegisterJson(E::eff_PLASMA, "Plasma", "Plasma");
    Register<RipplePanel>(E::eff_RIPPLE, "Ripple");
    Register<ServoPanel>(E::eff_SERVO, "Servo");
    Register<ShaderPanel>(E::eff_SHADER, "Shader");
    Register<ShapePanel>(E::eff_SHAPE, "Shape");
    RegisterJson(E::eff_SHIMMER, "Shimmer", "Shimmer");
    RegisterJson(E::eff_SHOCKWAVE, "Shockwave", "Shockwave");
    Register<SingleStrandPanel>(E::eff_SINGLESTRAND, "SingleStrand");
    Register<SketchPanel>(E::eff_SKETCH, "Sketch");
    Register<SnowflakesPanel>(E::eff_SNOWFLAKES, "Snowflakes");
    RegisterJson(E::eff_SNOWSTORM, "Snowstorm", "Snowstorm");
    RegisterJson(E::eff_SPIRALS, "Spirals", "Spirals");
    RegisterJson(E::eff_SPIROGRAPH, "Spirograph", "Spirograph");
    Register<StatePanel>(E::eff_STATE, "State");
    RegisterJson(E::eff_STROBE, "Strobe", "Strobe");
    RegisterJson(E::eff_TENDRIL, "Tendril", "Tendril");
    Register<TextPanel>(E::eff_TEXT, "Text");
    RegisterJson(E::eff_TREE, "Tree", "Tree");
    RegisterJson(E::eff_TWINKLE, "Twinkle", "Twinkle");
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
