/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Sample "xLights Plugin": builds xlights_plugins/SampleXLightsPlugin.dll,
// which XLightsPluginManager::loadPlugins() picks up at startup (after
// CreateSequencer(), so the AUI manager + Tools menu already exist).
// Demonstrates the concrete "Sequence Assistant" use case that drove this
// plugin category's design: a dockable panel that browses/applies effect
// presets and reacts to the sequencer grid's current selection.
//
// Deliberately includes NO wxWidgets headers and links no wx library - all
// UI goes through the abstract IPlugin* widgets in xLightsPlugin.h, which
// the host implements on top of its own statically-linked wx. See that
// header's comment for why a plugin can't construct wx objects itself.

#include "plugins/xLightsPlugin.h"
#include "render/Effect.h"

class SequenceAssistantPlugin : public IXLightsPlugin {
public:
    explicit SequenceAssistantPlugin(IXLightsPluginHost* host) : IXLightsPlugin(host)
    {
        IPluginPanel* panel = host_->CreateDockablePanel("SequenceAssistant", "Sequence Assistant", "Sequence Assistant Panel");

        selectionLabel_ = panel->AddStaticText("Selected: (nothing)");

        presetList_ = panel->AddListBox();
        presetList_->OnDoubleClick([this](int index) {
            if (index < 0) return;
            host_->ApplyPreset(presetList_->GetString(index));
        });

        // Presets load per-show, well after plugins are constructed at
        // startup, so the list can't be populated here - refresh it every
        // time the panel is shown instead (also covers presets changing
        // between shows without needing a restart).
        panel->OnShown([this] { RefreshPresets(); });

        host_->OnSelectionChanged([this](const Effect* effect) {
            selectionLabel_->SetText(effect != nullptr
                ? std::string("Selected: ") + effect->GetEffectName()
                : std::string("Selected: (nothing)"));
        });
    }

    [[nodiscard]] std::string GetName() const override { return "Sequence Assistant"; }

private:
    IPluginListBox* presetList_ = nullptr;
    IPluginStaticText* selectionLabel_ = nullptr;

    void RefreshPresets() {
        presetList_->Clear();
        for (const auto& path : host_->GetPresetPaths()) {
            presetList_->Append(path);
        }
    }
};

XL_XLIGHTS_PLUGIN_EXPORT(SequenceAssistantPlugin)
