#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <wx/string.h>

#include "JsonEffectPanel.h"

class xlEffectPanel;
class wxWindow;
class Model;
class SequenceElements;

class EffectPanelManager {
public:
    using PanelFactory = std::function<xlEffectPanel*(wxWindow*)>;

    EffectPanelManager();
    ~EffectPanelManager();

    xlEffectPanel* GetPanel(int effectId, wxWindow* parent);
    xlEffectPanel* GetPanel(const std::string& effectName, wxWindow* parent);

    void SetDefaultParameters(int effectId);
    void SetDefaultParameters(const std::string& effectName);
    void SetPanelStatus(int effectId, Model* cls);
    void SetPanelStatus(const std::string& effectName, Model* cls);
    void SetEffectTimeRange(int effectId, int startTimeMs, int endTimeMs);
    void SetSequenceElements(SequenceElements* els);
    wxString GetEffectString(int effectId);

    bool HasAssistPanel(int effectId);
    class AssistPanel* GetAssistPanel(int effectId, wxWindow* parent, class xLightsFrame* xl_frame);

    int GetPanelCount() const { return static_cast<int>(panels.size()); }

private:
    void RegisterPanels();
    void RegisterPanel(int effectId, const std::string& name, PanelFactory factory);

    template<typename PanelT>
    void Register(int effectId, const std::string& name) {
        RegisterPanel(effectId, name, [](wxWindow* p) -> xlEffectPanel* { return new PanelT(p); });
    }

    void RegisterJson(int effectId, const std::string& name, const std::string& jsonBaseName);

    // Register a JsonEffectPanel subclass that takes (wxWindow*, nlohmann::json) constructor
    template<typename PanelT>
    void RegisterJsonSubclass(int effectId, const std::string& name, const std::string& jsonBaseName) {
        RegisterPanel(effectId, name, [jsonBaseName](wxWindow* p) -> xlEffectPanel* {
            std::string metaDir = EffectPanelManager::GetMetadataDirectory();
            if (metaDir.empty()) return nullptr;
            auto metadata = JsonEffectPanel::LoadMetadata(metaDir + "/" + jsonBaseName + ".json");
            if (metadata.empty()) return nullptr;
            return new PanelT(p, metadata);
        });
    }

public:
    static std::string GetMetadataDirectory();

private:

    struct PanelInfo {
        PanelFactory factory;
        std::string name;
        xlEffectPanel* panel = nullptr;
    };

    std::vector<PanelInfo> panels;
    std::map<std::string, int> panelsByName;
};
