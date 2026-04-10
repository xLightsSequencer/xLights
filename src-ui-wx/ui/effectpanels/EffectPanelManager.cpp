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
    std::error_code ec;

    // Candidate locations to search for effectmetadata. The first existing directory wins.
    // The fallbacks let dev builds find resources/effectmetadata in the source tree
    // without requiring a post-build copy step.
    std::vector<std::string> candidates;
    candidates.push_back(resDir + "/effectmetadata");
#ifdef _WIN32
    // Visual Studio builds run from xLights/x64/<Config>/, so the source resources
    // dir is three levels up.
    candidates.push_back(resDir + "/../../../resources/effectmetadata");
#endif
#ifdef LINUX
    candidates.push_back(resDir + "/../resources/effectmetadata");
#endif

    for (const auto& dir : candidates) {
        if (std::filesystem::is_directory(std::filesystem::path(dir), ec)) {
            cachedDir = dir;
            return cachedDir;
        }
    }
    spdlog::error("Effect metadata directory not found: {}", candidates.front());
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

    RegisterJson(E::eff_OFF, "Off", "Off");
    RegisterJson(E::eff_ON, "On", "On");
    RegisterJsonSubclass<AdjustPanel>(E::eff_ADJUST, "Adjust", "Adjust");
    RegisterJson(E::eff_BARS, "Bars", "Bars");
    RegisterJson(E::eff_BUTTERFLY, "Butterfly", "Butterfly");
    RegisterJson(E::eff_CANDLE, "Candle", "Candle");
    RegisterJson(E::eff_CIRCLES, "Circles", "Circles");
    RegisterJson(E::eff_COLORWASH, "Color Wash", "ColorWash");
    RegisterJson(E::eff_CURTAIN, "Curtain", "Curtain");
    Register<DMXPanel>(E::eff_DMX, "DMX");
    RegisterJsonSubclass<DuplicatePanel>(E::eff_DUPLICATE, "Duplicate", "Duplicate");
    RegisterJsonSubclass<FacesPanel>(E::eff_FACES, "Faces", "Faces");
    RegisterJson(E::eff_FAN, "Fan", "Fan");
    RegisterJson(E::eff_FILL, "Fill", "Fill");
    RegisterJson(E::eff_FIRE, "Fire", "Fire");
    RegisterJson(E::eff_FIREWORKS, "Fireworks", "Fireworks");
    RegisterJson(E::eff_GALAXY, "Galaxy", "Galaxy");
    RegisterJson(E::eff_GARLANDS, "Garlands", "Garlands");
    RegisterJson(E::eff_GLEDIATOR, "Glediator", "Glediator");
    RegisterJson(E::eff_GUITAR, "Guitar", "Guitar");
    RegisterJson(E::eff_KALEIDOSCOPE, "Kaleidoscope", "Kaleidoscope");
    RegisterJson(E::eff_LIFE, "Life", "Life");
    RegisterJson(E::eff_LIGHTNING, "Lightning", "Lightning");
    RegisterJson(E::eff_LINES, "Lines", "Lines");
    RegisterJson(E::eff_LIQUID, "Liquid", "Liquid");
    RegisterJson(E::eff_MARQUEE, "Marquee", "Marquee");
    RegisterJson(E::eff_METEORS, "Meteors", "Meteors");
    RegisterJsonSubclass<MorphPanel>(E::eff_MORPH, "Morph", "Morph");
    Register<MovingHeadPanel>(E::eff_MOVINGHEAD, "Moving Head");
    RegisterJsonSubclass<MusicPanel>(E::eff_MUSIC, "Music", "Music");
    RegisterJsonSubclass<PianoPanel>(E::eff_PIANO, "Piano", "Piano");
    RegisterJsonSubclass<PicturesPanel>(E::eff_PICTURES, "Pictures", "Pictures");
    RegisterJson(E::eff_PINWHEEL, "Pinwheel", "Pinwheel");
    RegisterJson(E::eff_PLASMA, "Plasma", "Plasma");
    RegisterJsonSubclass<RipplePanel>(E::eff_RIPPLE, "Ripple", "Ripple");
    RegisterJsonSubclass<ServoPanel>(E::eff_SERVO, "Servo", "Servo");
    Register<ShaderPanel>(E::eff_SHADER, "Shader");
    RegisterJsonSubclass<ShapePanel>(E::eff_SHAPE, "Shape", "Shape");
    RegisterJson(E::eff_SHIMMER, "Shimmer", "Shimmer");
    RegisterJson(E::eff_SHOCKWAVE, "Shockwave", "Shockwave");
    RegisterJson(E::eff_SINGLESTRAND, "SingleStrand", "SingleStrand");
    RegisterJsonSubclass<SketchPanel>(E::eff_SKETCH, "Sketch", "Sketch");
    RegisterJsonSubclass<SnowflakesPanel>(E::eff_SNOWFLAKES, "Snowflakes", "Snowflakes");
    RegisterJson(E::eff_SNOWSTORM, "Snowstorm", "Snowstorm");
    RegisterJson(E::eff_SPIRALS, "Spirals", "Spirals");
    RegisterJson(E::eff_SPIROGRAPH, "Spirograph", "Spirograph");
    RegisterJsonSubclass<StatePanel>(E::eff_STATE, "State", "State");
    RegisterJson(E::eff_STROBE, "Strobe", "Strobe");
    RegisterJson(E::eff_TENDRIL, "Tendril", "Tendril");
    RegisterJsonSubclass<TextPanel>(E::eff_TEXT, "Text", "Text");
    RegisterJson(E::eff_TREE, "Tree", "Tree");
    RegisterJson(E::eff_TWINKLE, "Twinkle", "Twinkle");
    RegisterJsonSubclass<VideoPanel>(E::eff_VIDEO, "Video", "Video");
    RegisterJsonSubclass<VUMeterPanel>(E::eff_VUMETER, "VU Meter", "VUMeter");
    RegisterJsonSubclass<WarpPanel>(E::eff_WARP, "Warp", "Warp");
    RegisterJsonSubclass<WavePanel>(E::eff_WAVE, "Wave", "Wave");
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
