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

// C++ ABI for "xLights Plugins" - general program-level UI extensions
// (dockable panels, Tools-menu entries), distinct from AI service plugins
// (ai/ServiceManager) and effect plugins (effects/EffectManager).
//
// IMPORTANT: a plugin DLL must NOT include or link wxWidgets. xLights.exe
// links wx statically (there is no shared wxmsw*.dll), so a plugin linking
// its own copy of wx would get an independent, non-communicating copy of
// wx's runtime state (window-class registry, the wxTheApp singleton, etc.) -
// constructing a wx window from that copy fails a wx assert
// ("failed to register window class") because it was never initialized as
// the app. Instead, every UI element a plugin can use is one of the
// abstract IPlugin* interfaces below, implemented host-side as a thin
// wrapper around a real wx object the HOST constructs (so window creation
// always happens through xLights.exe's own linked copy of wx). A plugin
// project needs no wx include paths or libraries at all.
//
// A plugin DLL must export exactly two C functions:
//   IXLightsPlugin* xlCreateXLightsPlugin(IXLightsPluginHost* host)
//   void            xlDestroyXLightsPlugin(IXLightsPlugin* plugin)
//
// Use the XL_XLIGHTS_PLUGIN_EXPORT(ClassName) macro in your DLL's .cpp to
// generate these, where ClassName's constructor takes a single
// `IXLightsPluginHost* host` and forwards it to IXLightsPlugin's constructor.
//
// v1 scope: sized to a single concrete plugin (a dockable "Sequence
// Assistant" preset browser that reacts to grid selection) - a label, a
// list box, and sizer layout. Deliberately does NOT yet cover custom
// painting, arbitrary wx controls, grid context-menu injection, or other
// lifecycle hooks (sequence opened, effect added) - add those (as new
// IPlugin* widget types or host methods) when a second real plugin needs
// them, same philosophy as the effects plugin generalization. Every new
// control type requires a host-side wrapper implementation and an
// xLights.exe rebuild - plugins cannot reach for a wx control we haven't
// wrapped.

#include "utils/pluginExport.h"

#include <functional>
#include <string>
#include <vector>

class Effect;

// Base for every abstract UI element a plugin can hold. Host-owned; a
// plugin never constructs one of these directly (or any of its wx-backed
// implementation) - only IXLightsPluginHost/IPluginPanel factory methods do.
class IPluginWidget {
public:
    virtual ~IPluginWidget() = default;
};

class IPluginStaticText : public IPluginWidget {
public:
    virtual void SetText(const std::string& text) = 0;
};

class IPluginListBox : public IPluginWidget {
public:
    virtual void Clear() = 0;
    virtual void Append(const std::string& item) = 0;
    [[nodiscard]] virtual int GetSelection() const = 0;
    [[nodiscard]] virtual std::string GetString(int index) const = 0;
    // Called with the double-clicked item's index.
    virtual void OnDoubleClick(std::function<void(int index)> callback) = 0;
};

class IPluginButton : public IPluginWidget {
public:
    virtual void OnClick(std::function<void()> callback) = 0;
};

// A dockable panel's content area. Widgets are added top-to-bottom in a
// vertical sizer in creation order - sized to what Sequence Assistant needs;
// this is not a general layout system.
class IPluginPanel : public IPluginWidget {
public:
    virtual IPluginStaticText* AddStaticText(const std::string& initialText) = 0;
    virtual IPluginListBox* AddListBox() = 0;
    virtual IPluginButton* AddButton(const std::string& label) = 0;

    // Called every time this panel becomes visible (including the first
    // time it's shown), so a plugin can refresh content that wasn't ready
    // yet at construction time. Plugins are constructed at app startup,
    // right after CreateSequencer() - well before any show/sequence is
    // opened, so anything sourced from the open show (like effect presets,
    // which load per-show) is empty if only fetched in the constructor.
    virtual void OnShown(std::function<void()> callback) = 0;
};

class IXLightsPluginHost {
public:
    virtual ~IXLightsPluginHost() = default;

    // --- Effect presets (EffectPresetManager) ---
    // Flat "\"-separated paths, e.g. "Fireworks\\Big Show Opener".
    [[nodiscard]] virtual std::vector<std::string> GetPresetPaths() const = 0;
    // Applies the named preset at the grid's current selection/cursor
    // (mirrors xLightsFrame::ApplyEffectsPreset(presetName) - the same
    // stateless-w.r.t.-caller-selection behavior the desktop Presets panel
    // uses). Returns false if the preset name wasn't found.
    virtual bool ApplyPreset(const std::string& presetPath) = 0;

    // --- Sequencer grid selection ---
    // Returns nullptr if nothing is selected.
    [[nodiscard]] virtual const Effect* GetSelectedEffect() const = 0;
    // Called with the newly selected effect, or nullptr on deselect.
    using SelectionChangedFn = std::function<void(const Effect* effectOrNull)>;
    virtual void OnSelectionChanged(SelectionChangedFn callback) = 0;

    // --- UI registration ---
    // Creates a new dockable AUI pane (initially hidden) plus a wxITEM_CHECK
    // entry under Tools > Plugins that shows/hides it (checked state kept in
    // sync with the pane's actual visibility), and returns its content area
    // for the plugin to populate via IPluginPanel::Add*(). The host owns the
    // pane and every widget added to it for the life of the process.
    virtual IPluginPanel* CreateDockablePanel(const std::string& paneName,
                                               const std::string& caption,
                                               const std::string& toolsMenuLabel) = 0;

    // Adds a plain (non-toggling) entry under Tools > Plugins that invokes
    // callback when clicked - for plugins whose UI is a modal dialog rather
    // than a dockable panel (CreateDockablePanel above).
    virtual void AddPluginsMenuAction(const std::string& label, std::function<void()> callback) = 0;

    // --- Effects toolbar (EffectsToolBar) ---
    // Every registered effect's name (built-ins + loaded effect plugins), in
    // EffectManager's current order - the toolbar's default order before any
    // user customization.
    [[nodiscard]] virtual std::vector<std::string> GetAllEffectNames() const = 0;
    // Rebuilds the real Effects toolbar to show exactly these effects, in
    // this order (unknown names are skipped). Callable any time after
    // startup - not just once.
    virtual void SetEffectsToolbarEffects(const std::vector<std::string>& orderedVisibleNames) = 0;

    // --- Generic reorder/show-hide dialog ---
    // Host-side wrapper around wx's built-in wxRearrangeDialog (a checklist
    // with move-up/move-down buttons - the standard "choose and order items"
    // UI). `items`/`checked` are both input (initial state) and output (the
    // user's final choice, only updated if this returns true); their order on
    // return reflects what the user arranged, not necessarily the input
    // order. Returns false if the user cancelled (in which case neither
    // vector is touched).
    virtual bool ShowReorderDialog(const std::string& title, const std::string& message,
                                    std::vector<std::string>& items, std::vector<bool>& checked) = 0;

    // Per-user, per-machine directory a plugin can read/write its own config
    // files in (same directory xLights' own settings.json lives in). A
    // plugin can't call wxStandardPaths itself (see this header's top
    // comment), hence this wrapper.
    [[nodiscard]] virtual std::string GetUserDataDir() const = 0;
};

class IXLightsPlugin {
public:
    explicit IXLightsPlugin(IXLightsPluginHost* host) : host_(host) {}
    virtual ~IXLightsPlugin() = default;

    IXLightsPlugin(const IXLightsPlugin&) = delete;
    IXLightsPlugin& operator=(const IXLightsPlugin&) = delete;
    IXLightsPlugin(IXLightsPlugin&&) = delete;
    IXLightsPlugin& operator=(IXLightsPlugin&&) = delete;

    [[nodiscard]] virtual std::string GetName() const = 0;

protected:
    IXLightsPluginHost* host_;
};

extern "C" {
    typedef IXLightsPlugin* (*xlCreateXLightsPluginFn)(IXLightsPluginHost* host);
    typedef void            (*xlDestroyXLightsPluginFn)(IXLightsPlugin* plugin);
}

// Place this macro once in a plugin DLL .cpp file to export the required entry points.
#define XL_XLIGHTS_PLUGIN_EXPORT(ClassName)                                                     \
    extern "C" XL_PLUGIN_EXPORT_ATTR IXLightsPlugin* xlCreateXLightsPlugin(IXLightsPluginHost* host) { \
        return new ClassName(host);                                                             \
    }                                                                                            \
    extern "C" XL_PLUGIN_EXPORT_ATTR void xlDestroyXLightsPlugin(IXLightsPlugin* plugin) {       \
        delete plugin;                                                                           \
    }
