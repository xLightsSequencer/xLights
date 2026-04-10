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

#include "EffectPanelUtils.h"
#include <string>
#include <vector>
#include <map>

#include <nlohmann/json.hpp>

class wxSlider;
class wxTextCtrl;
class wxComboBox;
class wxCheckBox;
class wxChoice;
class wxSpinCtrl;
class wxNotebook;
class wxFilePickerCtrl;
class wxFontPickerCtrl;
class wxColourPickerCtrl;
class wxRadioButton;
class ValueCurveButton;

// A generic effect panel that builds its UI from a JSON metadata file.
// This replaces wxSmith-generated panels for effects whose panels are
// purely data-driven (no custom event handlers or special controls).
class JsonEffectPanel : public xlEffectPanel {
public:
    // Construct from a JSON metadata object (already parsed).
    // Set deferBuild=true when subclass overrides CreateCustomControl (virtual dispatch
    // doesn't work during base construction). Subclass must call BuildFromJson() itself.
    JsonEffectPanel(wxWindow* parent, const nlohmann::json& metadata, bool deferBuild = false);
    virtual ~JsonEffectPanel();

    void ValidateWindow() override;
    void SetDefaultParameters() override;
    void SetPanelStatus(Model* cls) override;
    void SetRenderableEffect(RenderableEffect* eff) override;
    wxString GetEffectString() override;

    // Load metadata from a file path and return the parsed JSON.
    static nlohmann::json LoadMetadata(const std::string& jsonPath);

protected:
    // Subclasses with custom code can call this to build the standard
    // property grid, then add their own custom controls on top.
    void BuildFromJson(const nlohmann::json& metadata);

    // Parent window that framework-built controls should use. Defaults to
    // the panel itself; when the metadata has "scrollable": true the
    // framework creates a wxScrolledWindow child and points this at it so
    // controls scroll with their content. Subclasses that build custom
    // widgets should use this instead of `this` for the parent argument.
    wxWindow* contentParent_ = nullptr;

    // When scrollable is active we override the panel's best size so that
    // wxAUI and parent sizers don't compute the pane's minimum from the
    // natural size of all the embedded controls — the inner scroll window
    // should handle overflow, and the user should be free to drag the pane
    // smaller than the content. Returns a small default when scrollable,
    // otherwise delegates to the base class.
    wxSize DoGetBestClientSize() const override;

    // Info about a built property row. Made protected so subclasses can
    // reach into the framework-built controls for custom serialization
    // (e.g. TimingPanel::GetTimingString needs to walk the map with the
    // T_ prefix instead of the default E_).
    struct PropertyInfo {
        std::string id;
        std::string controlType;
        std::string type;
        nlohmann::json defaultValue;
        int divisor = 1;
        bool suppressIfDefault = false;
        std::string settingPrefix; // Override setting key prefix (e.g. "TEXTCTRL" instead of "SLIDER")
        std::string dynamicOptions; // Source for dynamically populated options (e.g. "timingTracks")

        // Pointers to created controls (not all will be set)
        wxSlider* slider = nullptr;
        wxTextCtrl* textCtrl = nullptr;
        wxComboBox* comboBox = nullptr;  // Used when "text" controlType has "commonValues"
        wxCheckBox* checkBox = nullptr;
        wxChoice* choice = nullptr;
        wxSpinCtrl* spinCtrl = nullptr;
        wxFilePickerCtrl* filePicker = nullptr;
        wxFontPickerCtrl* fontPicker = nullptr;
        wxColourPickerCtrl* colourPicker = nullptr;
        wxRadioButton* radioButton = nullptr;
        ValueCurveButton* valueCurveBtn = nullptr;
        wxWindow* buddySlider = nullptr;   // IDD_SLIDER for float props
        wxTextCtrl* buddyText = nullptr;   // IDD_TEXTCTRL for int slider props
    };

    // Subclasses can look up the built control info for a property by id.
    // Returns nullptr if the property doesn't exist.
    PropertyInfo* GetPropertyInfo(const std::string& propId);
    const PropertyInfo* GetPropertyInfo(const std::string& propId) const;

    // Subclasses (especially shared panels loading their metadata
    // internally) need direct access to the parsed metadata object so
    // they can call BuildFromJson(metadata_) without re-reading the file.
    nlohmann::json metadata_;

private:
    struct VisibilityRule {
        std::string conditionPropertyId;
        bool conditionEquals = true; // check for equals (true) vs notEquals (false)
        // For bool conditions:
        bool conditionBoolValue = true;
        // For enum/choice conditions:
        std::vector<std::string> conditionOneOf;
        bool conditionOneOfInverted = false; // treat conditionOneOf as notOneOf when true
        std::string conditionStartsWith;
        std::string conditionStringEquals;
        // For numeric-text conditions:
        bool conditionHasGreaterThan = false;
        double conditionGreaterThan = 0.0;
        // For "any" conditions (OR of multiple checkbox properties):
        std::vector<std::string> conditionAnyIds;

        std::vector<std::string> enableIds;
        std::vector<std::string> disableIds;
        std::vector<std::string> showIds;
        std::vector<std::string> hideIds;
    };

    // Repopulate any choices that source their options from the sequence's
    // timing tracks. Called from SetPanelStatus and from the rename-timing
    // event handler so the dropdowns always reflect current state.
    void RepopulateTimingTrackChoices();

    void BuildPropertyRow(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols);

    // Override in subclasses to create custom controls for properties with controlType "custom".
    // Should create the control(s), add them to sizer, and return the primary control.
    // parentWin is the parent window, prop is the JSON property definition, cols is the grid column count.
    virtual wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols) { return nullptr; }
    void BuildTabGroup(wxSizer* parentSizer, const nlohmann::json& group, const nlohmann::json& metadata);
    void BuildXYCenter(wxWindow* parentWin, wxSizer* parentSizer, const nlohmann::json& group, const nlohmann::json& allProps);
    // Build a section group: a wxStaticBoxSizer (if label is present) or a plain
    // sub-sizer containing an inner grid with the listed properties. Used for
    // logical grouping like "Mouth Movements" or "Eyes" on the Faces panel.
    void BuildSection(wxSizer* parentSizer, const nlohmann::json& group, const nlohmann::json& allProps);
    void ParseVisibilityRules(const nlohmann::json& metadata);
    void ApplyVisibilityRules();

    wxWindow* FindControlForProperty(const std::string& propId);

    std::map<std::string, PropertyInfo> properties_;
    std::vector<VisibilityRule> visibilityRules_;
};
