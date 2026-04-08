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
    JsonEffectPanel(wxWindow* parent, const nlohmann::json& metadata);
    virtual ~JsonEffectPanel();

    void ValidateWindow() override;
    void SetDefaultParameters() override;

    // Load metadata from a file path and return the parsed JSON.
    static nlohmann::json LoadMetadata(const std::string& jsonPath);

protected:
    // Subclasses with custom code can call this to build the standard
    // property grid, then add their own custom controls on top.
    void BuildFromJson(const nlohmann::json& metadata);

private:
    struct VisibilityRule {
        std::string conditionPropertyId;
        bool conditionEquals = true; // check for equals (true) vs notEquals (false)
        // For bool conditions:
        bool conditionBoolValue = true;
        // For enum conditions:
        std::vector<std::string> conditionOneOf;

        std::vector<std::string> enableIds;
        std::vector<std::string> disableIds;
        std::vector<std::string> showIds;
        std::vector<std::string> hideIds;
    };

    struct PropertyInfo {
        std::string id;
        std::string controlType;
        std::string type;
        nlohmann::json defaultValue;
        int divisor = 1;

        // Pointers to created controls (not all will be set)
        wxSlider* slider = nullptr;
        wxTextCtrl* textCtrl = nullptr;
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

    void BuildPropertyRow(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols);
    void BuildTabGroup(wxSizer* parentSizer, const nlohmann::json& group, const nlohmann::json& metadata);
    void BuildXYCenter(wxWindow* parentWin, wxSizer* parentSizer, const nlohmann::json& group, const nlohmann::json& allProps);
    void ParseVisibilityRules(const nlohmann::json& metadata);
    void ApplyVisibilityRules();

    wxWindow* FindControlForProperty(const std::string& propId);

    std::map<std::string, PropertyInfo> properties_;
    std::vector<VisibilityRule> visibilityRules_;
    nlohmann::json metadata_;
};
