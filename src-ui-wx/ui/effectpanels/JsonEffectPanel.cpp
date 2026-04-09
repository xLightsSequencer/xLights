/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "JsonEffectPanel.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/clrpicker.h>
#include <wx/radiobut.h>
#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/utils/xlLockButton.h"
#include "ui/sequencer/TimingPanel.h"
#include "ui/shared/utils/wxUtilities.h"
#include "effects/RenderableEffect.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "render/SequenceElements.h"
#include "xLightsApp.h"
#include "xLightsMain.h"

#include <fstream>
#include <spdlog/spdlog.h>

nlohmann::json JsonEffectPanel::LoadMetadata(const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        spdlog::error("JsonEffectPanel: Failed to open metadata file: {}", jsonPath);
        return {};
    }
    try {
        return nlohmann::json::parse(f);
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("JsonEffectPanel: JSON parse error in {}: {}", jsonPath, e.what());
        return {};
    }
}

JsonEffectPanel::JsonEffectPanel(wxWindow* parent, const nlohmann::json& metadata, bool deferBuild)
    : xlEffectPanel(), metadata_(metadata) {
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    if (!deferBuild) {
        // Sanity check: if the metadata declares any "custom" controlType properties,
        // the panel must be a subclass that overrides CreateCustomControl, which only
        // works if the subclass passes deferBuild=true (otherwise virtual dispatch from
        // this base constructor resolves to the no-op base method). Detect that mistake
        // here so it fails loudly rather than silently producing a broken panel.
        if (metadata.contains("properties")) {
            for (const auto& prop : metadata["properties"]) {
                if (prop.value("controlType", "") == "custom") {
                    spdlog::error("JsonEffectPanel: effect '{}' has a 'custom' controlType property "
                                  "('{}') but the panel was constructed with deferBuild=false. "
                                  "Subclasses with CreateCustomControl overrides must pass deferBuild=true "
                                  "and call BuildFromJson() themselves, otherwise the override will not run.",
                                  metadata.value("effectName", "?"),
                                  prop.value("id", "?"));
                    break;
                }
            }
        }
        BuildFromJson(metadata);
    }
}

JsonEffectPanel::~JsonEffectPanel() {
}

void JsonEffectPanel::BuildFromJson(const nlohmann::json& metadata) {
    std::string effectName = metadata.value("effectName", "Unknown");

    // Collect property ids that are owned by groups (tabs, xyCenter, section).
    std::set<std::string> groupedPropIds;
    nlohmann::json tabGroup;
    bool hasTabs = false;
    std::vector<nlohmann::json> xyCenterGroups;
    // Sections keyed by their insertion index — the position of the first listed
    // property in the overall property list — so sections can be interleaved with
    // flat-grid properties in source order.
    std::map<int, nlohmann::json> sectionsByFirstIndex;

    if (metadata.contains("groups")) {
        for (const auto& group : metadata["groups"]) {
            std::string gtype = group.value("type", "");
            if (gtype == "tabs") {
                hasTabs = true;
                tabGroup = group;
                for (const auto& tab : group["tabs"]) {
                    for (const auto& pid : tab["properties"]) {
                        groupedPropIds.insert(pid.get<std::string>());
                    }
                }
            } else if (gtype == "xyCenter") {
                xyCenterGroups.push_back(group);
                if (group.contains("xProperty")) groupedPropIds.insert(group["xProperty"].get<std::string>());
                if (group.contains("yProperty")) groupedPropIds.insert(group["yProperty"].get<std::string>());
                if (group.contains("wrapX")) groupedPropIds.insert(group["wrapX"].get<std::string>());
                if (group.contains("wrapY")) groupedPropIds.insert(group["wrapY"].get<std::string>());
            } else if (gtype == "section" && group.contains("properties")) {
                // Record the section's property ids and locate its first
                // property in the overall list; that index becomes the
                // section's insertion point in the flat layout.
                std::set<std::string> sectionPropIds;
                for (const auto& pid : group["properties"]) {
                    sectionPropIds.insert(pid.get<std::string>());
                    groupedPropIds.insert(pid.get<std::string>());
                }
                int firstIdx = -1;
                if (metadata.contains("properties")) {
                    int idx = 0;
                    for (const auto& prop : metadata["properties"]) {
                        if (sectionPropIds.count(prop.value("id", ""))) {
                            firstIdx = idx;
                            break;
                        }
                        idx++;
                    }
                }
                if (firstIdx >= 0) {
                    sectionsByFirstIndex[firstIdx] = group;
                }
            }
        }
    }

    // Add description text at the top if present
    std::string descText;
    if (metadata.contains("description")) {
        descText = metadata["description"].get<std::string>();
    }

    if (hasTabs) {
        // Tabbed layout
        auto* outerSizer = new wxFlexGridSizer(0, 1, 0, 0);
        outerSizer->AddGrowableCol(0);
        if (!descText.empty()) {
            auto* desc = new wxStaticText(this, wxID_ANY, wxString(descText));
            desc->Wrap(300);
            outerSizer->Add(desc, 0, wxALL | wxEXPAND, 5);
        }
        BuildTabGroup(outerSizer, tabGroup, metadata);
        SetSizer(outerSizer);
    } else {
        // Flat or mixed layout
        // Check if we need a 1-column outer sizer (for XY groups, sections, separators, or description)
        bool hasXY = !xyCenterGroups.empty();
        bool hasSections = !sectionsByFirstIndex.empty();
        bool hasSeparator = false;
        for (const auto& prop : metadata["properties"]) {
            if (prop.value("separator", false)) { hasSeparator = true; break; }
        }
        bool needsOuter = hasXY || hasSections || hasSeparator || !descText.empty();

        auto* outerSizer = new wxFlexGridSizer(0, 1, 0, 0);
        outerSizer->AddGrowableCol(0);

        // Description appears at the top, matching tabbed layout.
        if (!descText.empty()) {
            auto* descLabel = new wxStaticText(this, wxID_ANY, wxString(descText));
            descLabel->Wrap(300);
            outerSizer->Add(descLabel, 0, wxALL | wxEXPAND, 5);
        }

        auto* gridSizer = new wxFlexGridSizer(0, 4, 0, 0);
        gridSizer->AddGrowableCol(1);

        // Helper: flush the current flat grid into the outer sizer (if it has
        // anything) and hand back a fresh grid to continue with.
        auto flushGrid = [&]() {
            if (gridSizer->GetItemCount() > 0) {
                outerSizer->Add(gridSizer, 0, wxALL | wxEXPAND, 0);
            } else {
                delete gridSizer;
            }
            gridSizer = new wxFlexGridSizer(0, 4, 0, 0);
            gridSizer->AddGrowableCol(1);
        };

        // Build properties into the grid, flushing and emitting sections at
        // the position of each section's first property. Skips properties
        // that are owned by groups (tabs/xyCenter/section).
        int propIndex = 0;
        for (const auto& prop : metadata["properties"]) {
            // Insert any section that anchors at this position before emitting
            // the next flat property.
            auto secIt = sectionsByFirstIndex.find(propIndex);
            if (secIt != sectionsByFirstIndex.end()) {
                flushGrid();
                BuildSection(outerSizer, secIt->second, metadata["properties"]);
            }
            propIndex++;

            std::string pid = prop.value("id", "");
            if (groupedPropIds.find(pid) != groupedPropIds.end()) continue;

            if (prop.value("separator", false) && needsOuter) {
                flushGrid();
                auto* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
                outerSizer->Add(line, 0, wxALL | wxEXPAND, 5);
            }
            BuildPropertyRow(this, gridSizer, prop, 4);
        }

        if (needsOuter) {
            // Flush any trailing flat grid contents into the outer sizer.
            if (gridSizer->GetItemCount() > 0) {
                outerSizer->Add(gridSizer, 0, wxALL | wxEXPAND, 0);
            } else {
                delete gridSizer;
            }
            for (const auto& xyGroup : xyCenterGroups) {
                BuildXYCenter(this, outerSizer, xyGroup, metadata["properties"]);
            }
            SetSizer(outerSizer);
        } else {
            delete outerSizer;
            SetSizer(gridSizer);
        }
    }

    // Parse visibility rules
    ParseVisibilityRules(metadata);

    // Connect change events on controls that are visibility rule conditions
    // so that ValidateWindow is called when they change
    std::set<std::string> conditionIds;
    for (const auto& rule : visibilityRules_) {
        if (!rule.conditionPropertyId.empty()) conditionIds.insert(rule.conditionPropertyId);
        for (const auto& id : rule.conditionAnyIds) conditionIds.insert(id);
    }
    for (const auto& cid : conditionIds) {
        auto it = properties_.find(cid);
        if (it != properties_.end()) {
            if (it->second.choice) {
                it->second.choice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
            }
            if (it->second.checkBox) {
                it->second.checkBox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
            }
            if (it->second.slider) {
                it->second.slider->Bind(wxEVT_SLIDER, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
            }
            if (it->second.spinCtrl) {
                it->second.spinCtrl->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& e) { ValidateWindow(); e.Skip(); });
            }
            if (it->second.radioButton) {
                it->second.radioButton->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
            }
            if (it->second.textCtrl) {
                it->second.textCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
            }
        }
    }

    // Connect standard events
    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&JsonEffectPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&JsonEffectPanel::OnValidateWindow, 0, this);

    // Refresh timing-track choices when a track is renamed elsewhere in the UI.
    // We ignore the event payload (which sends an unfiltered name list) and
    // re-derive the lists from mSequenceElements ourselves so layer-count
    // filtering stays consistent with SetPanelStatus.
    bool hasTimingTrackChoices = false;
    for (const auto& [id, info] : properties_) {
        if (info.dynamicOptions == "timingTracks" || info.dynamicOptions == "lyricTimingTracks") {
            hasTimingTrackChoices = true;
            break;
        }
    }
    if (hasTimingTrackChoices) {
        Bind(EVT_SETTIMINGTRACKS, [this](wxCommandEvent& event) {
            RepopulateTimingTrackChoices();
            event.Skip();
        });
    }

    // Set panel name for identification
    SetName(wxString::Format("ID_PANEL_%s", wxString(effectName).Upper()));

    ValidateWindow();
}

void JsonEffectPanel::BuildTabGroup(wxSizer* parentSizer, const nlohmann::json& group,
                                     const nlohmann::json& metadata) {
    const auto& allProps = metadata["properties"];

    // Create the notebook
    std::string settingKey = group.value("settingKey", "");
    std::string notebookName = settingKey.empty() ? "ID_NOTEBOOK" : ("ID_" + settingKey.substr(2)); // E_NOTEBOOK_X -> ID_NOTEBOOK_X
    wxWindowID notebookId = wxNewId();
    auto* notebook = new wxNotebook(this, notebookId, wxDefaultPosition, wxDefaultSize, 0, wxString(notebookName));
    notebook->SetName(wxString(notebookName));

    // Build a lookup of property id -> property json
    std::map<std::string, nlohmann::json> propLookup;
    for (const auto& prop : allProps) {
        propLookup[prop.value("id", "")] = prop;
    }

    // Collect xyCenter groups and their property ids
    std::vector<nlohmann::json> xyCenterGroups;
    std::set<std::string> xyCenterPropIds;
    if (metadata.contains("groups")) {
        for (const auto& g : metadata["groups"]) {
            if (g.value("type", "") == "xyCenter") {
                xyCenterGroups.push_back(g);
                if (g.contains("xProperty")) xyCenterPropIds.insert(g["xProperty"].get<std::string>());
                if (g.contains("yProperty")) xyCenterPropIds.insert(g["yProperty"].get<std::string>());
                if (g.contains("wrapX")) xyCenterPropIds.insert(g["wrapX"].get<std::string>());
                if (g.contains("wrapY")) xyCenterPropIds.insert(g["wrapY"].get<std::string>());
            }
        }
    }

    // Build a set of all properties that appear in tabs
    std::set<std::string> tabbedPropIds;
    for (const auto& tab : group["tabs"]) {
        for (const auto& propId : tab["properties"]) {
            tabbedPropIds.insert(propId.get<std::string>());
        }
    }

    // Create pages
    for (const auto& tab : group["tabs"]) {
        std::string tabLabel = tab.value("label", "");
        auto* page = new wxPanel(notebook, wxID_ANY);

        // Check if this tab contains xyCenter properties
        // If so, use a 1-col outer sizer to stack grid + XY
        std::vector<nlohmann::json> tabXYGroups;
        for (const auto& xyg : xyCenterGroups) {
            std::string xp = xyg.value("xProperty", "");
            // Check if xProperty is in this tab's properties
            for (const auto& pid : tab["properties"]) {
                if (pid.get<std::string>() == xp) {
                    tabXYGroups.push_back(xyg);
                    break;
                }
            }
        }

        if (!tabXYGroups.empty()) {
            // Mixed tab: grid properties + XY groups (+ possible separators)
            auto* pageOuterSizer = new wxFlexGridSizer(0, 1, 0, 0);
            pageOuterSizer->AddGrowableCol(0);
            auto* pageSizer = new wxFlexGridSizer(0, 4, 0, 0);
            pageSizer->AddGrowableCol(1);

            for (const auto& propId : tab["properties"]) {
                std::string id = propId.get<std::string>();
                if (xyCenterPropIds.find(id) != xyCenterPropIds.end()) continue;
                auto it = propLookup.find(id);
                if (it != propLookup.end()) {
                    if (it->second.value("separator", false)) {
                        pageOuterSizer->Add(pageSizer, 0, wxALL | wxEXPAND, 0);
                        auto* line = new wxStaticLine(page, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
                        pageOuterSizer->Add(line, 0, wxALL | wxEXPAND, 5);
                        pageSizer = new wxFlexGridSizer(0, 4, 0, 0);
                        pageSizer->AddGrowableCol(1);
                    }
                    BuildPropertyRow(page, pageSizer, it->second, 4);
                }
            }
            pageOuterSizer->Add(pageSizer, 0, wxALL | wxEXPAND, 0);

            for (const auto& xyg : tabXYGroups) {
                BuildXYCenter(page, pageOuterSizer, xyg, allProps);
            }
            page->SetSizer(pageOuterSizer);
        } else {
            // Simple tab: all grid properties
            auto* pageSizer = new wxFlexGridSizer(0, 4, 0, 0);
            pageSizer->AddGrowableCol(1);

            for (const auto& propId : tab["properties"]) {
                std::string id = propId.get<std::string>();
                auto it = propLookup.find(id);
                if (it != propLookup.end()) {
                    BuildPropertyRow(page, pageSizer, it->second, 4);
                }
            }
            page->SetSizer(pageSizer);
        }

        notebook->AddPage(page, wxString(tabLabel));
    }

    parentSizer->Add(notebook, 1, wxALL | wxEXPAND, 0);

    // Add any properties that aren't in any tab (e.g., properties below the notebook)
    bool hasUntabbed = false;
    for (const auto& prop : allProps) {
        std::string id = prop.value("id", "");
        if (tabbedPropIds.find(id) == tabbedPropIds.end()) {
            hasUntabbed = true;
            break;
        }
    }

    if (hasUntabbed) {
        auto* belowSizer = new wxFlexGridSizer(0, 4, 0, 0);
        belowSizer->AddGrowableCol(1);
        for (const auto& prop : allProps) {
            std::string id = prop.value("id", "");
            if (tabbedPropIds.find(id) == tabbedPropIds.end()) {
                BuildPropertyRow(this, belowSizer, prop, 4);
            }
        }
        parentSizer->Add(belowSizer, 0, wxALL | wxEXPAND, 0);
    }
}

void JsonEffectPanel::BuildXYCenter(wxWindow* parentWin, wxSizer* parentSizer,
                                     const nlohmann::json& group, const nlohmann::json& allProps) {
    // Build a lookup of property id -> property json
    std::map<std::string, nlohmann::json> propLookup;
    for (const auto& prop : allProps) {
        propLookup[prop.value("id", "")] = prop;
    }

    std::string xId = group.value("xProperty", "");
    std::string yId = group.value("yProperty", "");
    std::string wrapXId = group.value("wrapX", "");
    std::string wrapYId = group.value("wrapY", "");

    auto xIt = propLookup.find(xId);
    auto yIt = propLookup.find(yId);
    if (xIt == propLookup.end() || yIt == propLookup.end()) return;

    const auto& xProp = xIt->second;
    const auto& yProp = yIt->second;
    int xMin = xProp.value("min", 0);
    int xMax = xProp.value("max", 100);
    int xDef = xProp.value("default", 0);
    int yMin = yProp.value("min", 0);
    int yMax = yProp.value("max", 100);
    int yDef = yProp.value("default", 0);
    bool xHasVC = xProp.value("valueCurve", false);
    bool yHasVC = yProp.value("valueCurve", false);

    // Layout: 2-column outer sizer
    // Left column: X label, X slider row, optional Wrap X
    // Right column: Y label, Y VC+text, Y vertical slider, optional Wrap Y
    auto* xySizer = new wxFlexGridSizer(0, 2, 0, 0);
    xySizer->AddGrowableCol(0);

    // === Left column (X) ===
    auto* xSizer = new wxFlexGridSizer(0, 1, 0, 0);
    xSizer->AddGrowableCol(0);

    // X label
    auto* xLabel = new wxStaticText(parentWin, wxID_ANY, wxString(xProp.value("label", "X-axis Center")));
    xSizer->Add(xLabel, 0, wxALL | wxALIGN_LEFT, 5);

    // X slider row
    auto* xSliderRow = new wxFlexGridSizer(0, 3, 0, 0);
    xSliderRow->AddGrowableCol(0);

    wxWindowID xSliderId = wxNewId();
    std::string xSliderName = "ID_SLIDER_" + xId;
    auto* xSlider = new BulkEditSlider(parentWin, xSliderId, xDef, xMin, xMax,
                                        wxDefaultPosition, wxDefaultSize, 0,
                                        wxDefaultValidator, wxString(xSliderName));
    xSliderRow->Add(xSlider, 1, wxALL | wxEXPAND, 5);

    PropertyInfo xInfo;
    xInfo.id = xId;
    xInfo.controlType = "slider";
    xInfo.type = "int";
    xInfo.defaultValue = xDef;
    xInfo.slider = xSlider;

    if (xHasVC) {
        std::string xVCName = "ID_VALUECURVE_" + xId;
        wxWindowID xVCId = wxNewId();
        auto* xVCBtn = new BulkEditValueCurveButton(parentWin, xVCId, GetValueCurveNotSelectedBitmap(),
                                                      wxDefaultPosition, wxDefaultSize,
                                                      wxBU_AUTODRAW | wxBORDER_NONE,
                                                      wxDefaultValidator, wxString(xVCName));
        xVCBtn->GetValue()->SetLimits(static_cast<float>(xMin), static_cast<float>(xMax));
        xInfo.valueCurveBtn = xVCBtn;
        xSliderRow->Add(xVCBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
        Connect(xVCId, wxEVT_COMMAND_BUTTON_CLICKED,
                (wxObjectEventFunction)&JsonEffectPanel::OnVCButtonClick);
    }

    // X buddy text
    wxWindowID xTextId = wxNewId();
    std::string xTextName = "IDD_TEXTCTRL_" + xId;
    auto* xText = new BulkEditTextCtrl(parentWin, xTextId, wxString::Format("%d", xDef),
                                        wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                        0, wxDefaultValidator, wxString(xTextName));
    xText->SetMaxLength(5);
    xInfo.buddyText = xText;
    xSliderRow->Add(xText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    xSizer->Add(xSliderRow, 1, wxALL | wxEXPAND, 0);

    // Wrap X checkbox (optional)
    if (!wrapXId.empty()) {
        auto wrapXIt = propLookup.find(wrapXId);
        if (wrapXIt != propLookup.end()) {
            const auto& wrapXProp = wrapXIt->second;
            std::string wrapXName = "ID_CHECKBOX_" + wrapXId;
            wxWindowID wrapXCtrlId = wxNewId();
            std::string wrapLabel = wrapXProp.value("label", "Wrap X");
            auto* wrapXCb = new BulkEditCheckBox(parentWin, wrapXCtrlId, wxString(wrapLabel),
                                                   wxDefaultPosition, wxDefaultSize, 0,
                                                   wxDefaultValidator, wxString(wrapXName));
            wrapXCb->SetValue(wrapXProp.value("default", false));
            xSizer->Add(wrapXCb, 0, wxALL, 5);

            PropertyInfo wrapXInfo;
            wrapXInfo.id = wrapXId;
            wrapXInfo.controlType = "checkbox";
            wrapXInfo.type = "bool";
            wrapXInfo.defaultValue = wrapXProp.value("default", false);
            wrapXInfo.checkBox = wrapXCb;
            properties_[wrapXId] = wrapXInfo;
        }
    }

    xySizer->Add(xSizer, 1, wxALL | wxEXPAND, 0);
    properties_[xId] = xInfo;

    // === Right column (Y) ===
    auto* ySizer = new wxFlexGridSizer(0, 2, 0, 0);

    // Y label + VC + text (stacked vertically in left sub-column)
    auto* yInfoSizer = new wxFlexGridSizer(0, 1, 0, 0);

    auto* yLabel = new wxStaticText(parentWin, wxID_ANY, wxString(yProp.value("label", "Y-axis Center")));
    yInfoSizer->Add(yLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    auto* yVCTextSizer = new wxFlexGridSizer(0, 2, 0, 0);

    PropertyInfo yInfo;
    yInfo.id = yId;
    yInfo.controlType = "slider";
    yInfo.type = "int";
    yInfo.defaultValue = yDef;

    if (yHasVC) {
        std::string yVCName = "ID_VALUECURVE_" + yId;
        wxWindowID yVCId = wxNewId();
        auto* yVCBtn = new BulkEditValueCurveButton(parentWin, yVCId, GetValueCurveNotSelectedBitmap(),
                                                      wxDefaultPosition, wxDefaultSize,
                                                      wxBU_AUTODRAW | wxBORDER_NONE,
                                                      wxDefaultValidator, wxString(yVCName));
        yVCBtn->GetValue()->SetLimits(static_cast<float>(yMin), static_cast<float>(yMax));
        yInfo.valueCurveBtn = yVCBtn;
        yVCTextSizer->Add(yVCBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
        Connect(yVCId, wxEVT_COMMAND_BUTTON_CLICKED,
                (wxObjectEventFunction)&JsonEffectPanel::OnVCButtonClick);
    }

    // Y buddy text
    wxWindowID yTextId = wxNewId();
    std::string yTextName = "IDD_TEXTCTRL_" + yId;
    auto* yText = new BulkEditTextCtrl(parentWin, yTextId, wxString::Format("%d", yDef),
                                        wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                        0, wxDefaultValidator, wxString(yTextName));
    yText->SetMaxLength(5);
    yInfo.buddyText = yText;
    yVCTextSizer->Add(yText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    yInfoSizer->Add(yVCTextSizer, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

    // Wrap Y checkbox (optional)
    if (!wrapYId.empty()) {
        auto wrapYIt = propLookup.find(wrapYId);
        if (wrapYIt != propLookup.end()) {
            const auto& wrapYProp = wrapYIt->second;
            std::string wrapYName = "ID_CHECKBOX_" + wrapYId;
            wxWindowID wrapYCtrlId = wxNewId();
            std::string wrapLabel = wrapYProp.value("label", "Wrap Y");
            auto* wrapYCb = new BulkEditCheckBox(parentWin, wrapYCtrlId, wxString(wrapLabel),
                                                   wxDefaultPosition, wxDefaultSize, 0,
                                                   wxDefaultValidator, wxString(wrapYName));
            wrapYCb->SetValue(wrapYProp.value("default", false));
            yInfoSizer->Add(wrapYCb, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

            PropertyInfo wrapYInfo;
            wrapYInfo.id = wrapYId;
            wrapYInfo.controlType = "checkbox";
            wrapYInfo.type = "bool";
            wrapYInfo.defaultValue = wrapYProp.value("default", false);
            wrapYInfo.checkBox = wrapYCb;
            properties_[wrapYId] = wrapYInfo;
        }
    }

    ySizer->Add(yInfoSizer, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 1);

    // Y vertical slider (right sub-column)
    wxWindowID ySliderId = wxNewId();
    std::string ySliderName = "ID_SLIDER_" + yId;
    auto* ySlider = new BulkEditSlider(parentWin, ySliderId, yDef, yMin, yMax,
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSL_VERTICAL | wxSL_INVERSE,
                                        wxDefaultValidator, wxString(ySliderName));
    yInfo.slider = ySlider;
    ySizer->Add(ySlider, 1, wxALL | wxEXPAND, 5);

    xySizer->Add(ySizer, 0, wxALL | wxEXPAND, 0);
    properties_[yId] = yInfo;

    parentSizer->Add(xySizer, 0, wxALL | wxEXPAND, 0);
}

void JsonEffectPanel::BuildSection(wxSizer* parentSizer, const nlohmann::json& group,
                                    const nlohmann::json& allProps) {
    std::string label = group.value("label", "");
    int columns = group.value("columns", 4);

    // Build a lookup of property id -> property json for quick access.
    std::map<std::string, nlohmann::json> propLookup;
    for (const auto& prop : allProps) {
        propLookup[prop.value("id", "")] = prop;
    }

    // If a label is provided wrap the section in a wxStaticBoxSizer so it
    // reads visually as a grouped region, otherwise just use a plain grid.
    wxSizer* contentSizer = nullptr;
    wxWindow* parentWin = this;

    wxFlexGridSizer* innerGrid = new wxFlexGridSizer(0, columns, 0, 0);
    if (columns >= 2) {
        // Column 1 is typically labels; grow column 2 (controls) so rows
        // expand horizontally the same way the flat panel grid does.
        innerGrid->AddGrowableCol(1);
    } else {
        innerGrid->AddGrowableCol(0);
    }

    if (!label.empty()) {
        auto* boxSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxString(label));
        boxSizer->Add(innerGrid, 1, wxALL | wxEXPAND, 2);
        parentSizer->Add(boxSizer, 0, wxALL | wxEXPAND, 5);
    } else {
        parentSizer->Add(innerGrid, 0, wxALL | wxEXPAND, 5);
    }
    contentSizer = innerGrid;

    // Emit each listed property into the section's inner grid, preserving
    // declared order within the section.
    if (group.contains("properties")) {
        for (const auto& pid : group["properties"]) {
            auto it = propLookup.find(pid.get<std::string>());
            if (it == propLookup.end()) continue;
            BuildPropertyRow(parentWin, contentSizer, it->second, columns);
        }
    }
}

void JsonEffectPanel::BuildPropertyRow(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    std::string label = prop.value("label", id);
    std::string type = prop.value("type", "int");
    std::string controlType = prop.value("controlType", "slider");
    bool hasValueCurve = prop.value("valueCurve", false);
    bool isLockable = prop.value("lockable", false);
    int divisor = prop.value("divisor", 1);
    std::string settingPrefix = prop.value("settingPrefix", "");
    std::string tooltip = prop.value("tooltip", "");

    PropertyInfo info;
    info.id = id;
    info.controlType = controlType;
    info.type = type;
    info.divisor = divisor;
    info.suppressIfDefault = prop.value("suppressIfDefault", false);
    info.settingPrefix = settingPrefix;
    info.dynamicOptions = prop.value("dynamicOptions", "");
    if (prop.contains("default")) {
        info.defaultValue = prop["default"];
    }

    if (controlType == "slider") {
        int minVal = prop.value("min", 0);
        int maxVal = prop.value("max", 100);
        int defaultInt;
        if (divisor > 1) {
            double defaultFloat = prop.value("default", 0.0);
            defaultInt = static_cast<int>(defaultFloat * divisor);
        } else {
            defaultInt = prop.value("default", 0);
        }

        // Column 1: Label
        wxWindowID labelId = wxNewId();
        std::string labelName = "ID_STATICTEXT_" + id;
        auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                             wxDefaultSize, 0, wxString(labelName));
        sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

        // Column 2: Slider + VC button in sub-sizer
        auto* sliderSizer = new wxFlexGridSizer(0, 2, 0, 0);
        sliderSizer->AddGrowableCol(0);

        if (divisor > 1) {
            // Float slider: primary control is IDD_SLIDER (buddy), text is ID_TEXTCTRL
            std::string sliderName = "IDD_SLIDER_" + id;
            wxWindowID sliderId = wxNewId();
            wxSlider* slider = nullptr;

            switch (divisor) {
                case 10:
                    slider = new BulkEditSliderF1(parentWin, sliderId, defaultInt, minVal, maxVal,
                                                   wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator,
                                                   wxString(sliderName));
                    break;
                case 100:
                    slider = new BulkEditSliderF2(parentWin, sliderId, defaultInt, minVal, maxVal,
                                                   wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator,
                                                   wxString(sliderName));
                    break;
                case 360:
                    slider = new BulkEditSliderF360(parentWin, sliderId, defaultInt, minVal, maxVal,
                                                     wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator,
                                                     wxString(sliderName));
                    break;
                default:
                    slider = new BulkEditSliderF1(parentWin, sliderId, defaultInt, minVal, maxVal,
                                                   wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator,
                                                   wxString(sliderName));
                    break;
            }
            info.buddySlider = slider;
            sliderSizer->Add(slider, 1, wxALL | wxEXPAND, 2);
        } else if (settingPrefix == "TEXTCTRL") {
            // Integer slider where TEXTCTRL is the primary setting key (e.g. On effect)
            // Slider is buddy (IDD_SLIDER_), text control is primary (ID_TEXTCTRL_)
            std::string sliderName = "IDD_SLIDER_" + id;
            wxWindowID sliderId = wxNewId();
            auto* slider = new BulkEditSlider(parentWin, sliderId, defaultInt, minVal, maxVal,
                                               wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator,
                                               wxString(sliderName));
            info.buddySlider = slider;
            sliderSizer->Add(slider, 1, wxALL | wxEXPAND, 2);
        } else {
            // Integer slider: primary control is ID_SLIDER
            std::string sliderName = "ID_SLIDER_" + id;
            wxWindowID sliderId = wxNewId();
            auto* slider = new BulkEditSlider(parentWin, sliderId, defaultInt, minVal, maxVal,
                                               wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator,
                                               wxString(sliderName));
            info.slider = slider;
            sliderSizer->Add(slider, 1, wxALL | wxEXPAND, 2);
        }

        // Value curve button
        if (hasValueCurve) {
            std::string vcName = "ID_VALUECURVE_" + id;
            wxWindowID vcId = wxNewId();
            auto* vcBtn = new BulkEditValueCurveButton(parentWin, vcId, GetValueCurveNotSelectedBitmap(),
                                                        wxDefaultPosition, wxDefaultSize,
                                                        wxBU_AUTODRAW | wxBORDER_NONE,
                                                        wxDefaultValidator, wxString(vcName));
            info.valueCurveBtn = vcBtn;

            // Set limits and divisor (vcMin/vcMax override slider min/max for value curves)
            float vcMinF = prop.contains("vcMin") ? prop["vcMin"].get<float>() : static_cast<float>(minVal);
            float vcMaxF = prop.contains("vcMax") ? prop["vcMax"].get<float>() : static_cast<float>(maxVal);
            vcBtn->GetValue()->SetLimits(vcMinF, vcMaxF);
            if (divisor > 1) {
                vcBtn->GetValue()->SetDivisor(divisor);
            }

            sliderSizer->Add(vcBtn, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);

            // Connect VC button click
            Connect(vcId, wxEVT_COMMAND_BUTTON_CLICKED,
                    (wxObjectEventFunction)&JsonEffectPanel::OnVCButtonClick);
        }

        sizer->Add(sliderSizer, 1, wxALL | wxEXPAND, 0);

        // Column 3: Text buddy control
        if (divisor > 1) {
            // Float text: ID_TEXTCTRL is the primary setting control
            std::string textName = "ID_TEXTCTRL_" + id;
            wxWindowID textId = wxNewId();
            double defaultFloat = prop.value("default", 0.0);
            wxString defaultStr = wxString::Format("%.1f", defaultFloat);
            BulkEditTextCtrl* textCtrl = nullptr;

            switch (divisor) {
                case 10:
                    textCtrl = new BulkEditTextCtrlF1(parentWin, textId, defaultStr,
                                                       wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                                       0, wxDefaultValidator, wxString(textName));
                    break;
                case 100:
                    textCtrl = new BulkEditTextCtrlF2(parentWin, textId, defaultStr,
                                                       wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                                       0, wxDefaultValidator, wxString(textName));
                    break;
                case 360:
                    textCtrl = new BulkEditTextCtrlF360(parentWin, textId, defaultStr,
                                                         wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                                         0, wxDefaultValidator, wxString(textName));
                    break;
                default:
                    textCtrl = new BulkEditTextCtrlF1(parentWin, textId, defaultStr,
                                                       wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                                       0, wxDefaultValidator, wxString(textName));
                    break;
            }
            textCtrl->SetMaxLength(5);
            info.textCtrl = textCtrl;
            sizer->Add(textCtrl, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
        } else if (settingPrefix == "TEXTCTRL") {
            // Integer slider where text is the primary setting (ID_TEXTCTRL_)
            std::string textName = "ID_TEXTCTRL_" + id;
            wxWindowID textId = wxNewId();
            wxString defaultStr = wxString::Format("%d", defaultInt);
            auto* textCtrl = new BulkEditTextCtrl(parentWin, textId, defaultStr,
                                                    wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                                    0, wxDefaultValidator, wxString(textName));
            textCtrl->SetMaxLength(5);
            info.textCtrl = textCtrl;
            sizer->Add(textCtrl, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
        } else {
            // Integer slider: IDD_TEXTCTRL is a display buddy
            std::string textName = "IDD_TEXTCTRL_" + id;
            wxWindowID textId = wxNewId();
            wxString defaultStr = wxString::Format("%d", defaultInt);
            auto* textCtrl = new BulkEditTextCtrl(parentWin, textId, defaultStr,
                                                    wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                                    0, wxDefaultValidator, wxString(textName));
            textCtrl->SetMaxLength(5);
            info.buddyText = textCtrl;
            sizer->Add(textCtrl, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
        }

        // Column 4: Lock button
        if (isLockable) {
            std::string lockName = "ID_BITMAPBUTTON_SLIDER_" + id;
            wxWindowID lockId = wxNewId();
            auto* lockBtn = new xlLockButton(parentWin, lockId, wxNullBitmap, wxDefaultPosition, wxSize(14, 14),
                                              wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
                                              wxString(lockName));
            lockBtn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
            sizer->Add(lockBtn, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            Connect(lockId, wxEVT_COMMAND_BUTTON_CLICKED,
                    (wxObjectEventFunction)&JsonEffectPanel::OnLockButtonClick);
        } else if (cols >= 4) {
            sizer->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        }

    } else if (controlType == "checkbox") {
        bool defaultVal = prop.value("default", false);
        std::string checkboxLabel = prop.value("checkboxLabel", "");

        std::string ctrlName = "ID_CHECKBOX_" + id;
        wxWindowID ctrlId = wxNewId();
        // If there's a separate checkboxLabel, use it on the control and put the main label in col 1
        wxString cbText = checkboxLabel.empty() ? wxString(label) : wxString(checkboxLabel);
        auto* cb = new BulkEditCheckBox(parentWin, ctrlId, cbText, wxDefaultPosition, wxDefaultSize,
                                         0, wxDefaultValidator, wxString(ctrlName));
        cb->SetValue(defaultVal);
        info.checkBox = cb;

        if (!checkboxLabel.empty()) {
            // Has separate label: col 1 = label, col 2 = checkbox
            wxWindowID labelId = wxNewId();
            std::string labelName = "ID_STATICTEXT_" + id;
            auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                                 wxDefaultSize, 0, wxString(labelName));
            sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
        } else {
            // No separate label: spacer for col 1, checkbox has its own label text
            sizer->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
        }
        sizer->Add(cb, 1, wxALL | wxEXPAND, 2);

        // Spacer for col 3 (buddy text column)
        sizer->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

        // Lock button (col 4)
        if (isLockable) {
            std::string lockName = "ID_BITMAPBUTTON_CHECKBOX_" + id;
            wxWindowID lockId = wxNewId();
            auto* lockBtn = new xlLockButton(parentWin, lockId, wxNullBitmap, wxDefaultPosition, wxSize(14, 14),
                                              wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
                                              wxString(lockName));
            lockBtn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
            sizer->Add(lockBtn, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            Connect(lockId, wxEVT_COMMAND_BUTTON_CLICKED,
                    (wxObjectEventFunction)&JsonEffectPanel::OnLockButtonClick);
        } else if (cols >= 4) {
            sizer->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        }

    } else if (controlType == "choice" || controlType == "combobox") {
        // Column 1: Label
        wxWindowID labelId = wxNewId();
        std::string labelName = "ID_STATICTEXT_" + id;
        auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                             wxDefaultSize, 0, wxString(labelName));
        sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

        // Column 2: Choice control
        std::string ctrlName = "ID_CHOICE_" + id;
        wxWindowID ctrlId = wxNewId();
        auto* choice = new BulkEditChoice(parentWin, ctrlId, wxDefaultPosition, wxDefaultSize,
                                           0, nullptr, 0, wxDefaultValidator, wxString(ctrlName));
        info.choice = choice;

        // Add options
        if (prop.contains("options")) {
            std::string defaultVal = prop.value("default", "");
            for (const auto& opt : prop["options"]) {
                choice->Append(wxString(opt.get<std::string>()));
            }
            if (!defaultVal.empty()) {
                choice->SetStringSelection(wxString(defaultVal));
            } else {
                choice->SetSelection(0);
            }
        }
        sizer->Add(choice, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        // Columns 3+4: spacers (choices don't have buddy text or usually lock)
        if (cols >= 3) {
            sizer->Add(-1, -1, 1, wxALL, 1);
        }
        if (cols >= 4) {
            if (isLockable) {
                std::string lockName = "ID_BITMAPBUTTON_CHOICE_" + id;
                wxWindowID lockId = wxNewId();
                auto* lockBtn = new xlLockButton(parentWin, lockId, wxNullBitmap, wxDefaultPosition, wxSize(14, 14),
                                                  wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
                                                  wxString(lockName));
                lockBtn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
                sizer->Add(lockBtn, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
                Connect(lockId, wxEVT_COMMAND_BUTTON_CLICKED,
                        (wxObjectEventFunction)&JsonEffectPanel::OnLockButtonClick);
            } else {
                sizer->Add(-1, -1, 1, wxALL, 1);
            }
        }

    } else if (controlType == "spin") {
        int minVal = prop.value("min", 0);
        int maxVal = prop.value("max", 100);
        int defaultVal = prop.value("default", 0);

        // Column 1: Label
        wxWindowID labelId = wxNewId();
        std::string labelName = "ID_STATICTEXT_" + id;
        auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                             wxDefaultSize, 0, wxString(labelName));
        sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

        // Column 2: Spin control
        std::string ctrlName = "ID_SPINCTRL_" + id;
        wxWindowID ctrlId = wxNewId();
        auto* spin = new BulkEditSpinCtrl(parentWin, ctrlId, wxString::Format("%d", defaultVal),
                                           wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
                                           minVal, maxVal, defaultVal, wxString(ctrlName));
        info.spinCtrl = spin;
        sizer->Add(spin, 1, wxALL | wxEXPAND, 2);

        // Columns 3+4: spacers
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    } else if (controlType == "text") {
        std::string defaultVal = prop.value("default", "");

        // Column 1: Label
        wxWindowID labelId = wxNewId();
        std::string labelName = "ID_STATICTEXT_" + id;
        auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                             wxDefaultSize, 0, wxString(labelName));
        sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

        // Column 2: Text control
        std::string ctrlName = "ID_TEXTCTRL_" + id;
        wxWindowID ctrlId = wxNewId();
        auto* textCtrl = new BulkEditTextCtrl(parentWin, ctrlId, wxString(defaultVal),
                                               wxDefaultPosition, wxDefaultSize, 0,
                                               wxDefaultValidator, wxString(ctrlName));
        info.textCtrl = textCtrl;
        sizer->Add(textCtrl, 1, wxALL | wxEXPAND, 2);

        // Columns 3+4: spacers
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    } else if (controlType == "filepicker") {
        std::string wildcard = prop.value("fileFilter", "All files (*.*)|*.*");
        std::string message = prop.value("fileMessage", "Select a file");

        // Column 1: Label
        wxWindowID labelId = wxNewId();
        std::string labelName = "ID_STATICTEXT_" + id;
        auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                             wxDefaultSize, 0, wxString(labelName));
        sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        // Column 2: File picker (spans remaining columns)
        std::string ctrlName = "ID_FILEPICKERCTRL_" + id;
        wxWindowID ctrlId = wxNewId();
        auto* picker = new BulkEditFilePickerCtrl(parentWin, ctrlId, wxEmptyString,
                                                    wxString(message), wxString(wildcard),
                                                    wxDefaultPosition, wxDefaultSize,
                                                    wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                    wxDefaultValidator, wxString(ctrlName));
        info.filePicker = picker;
        sizer->Add(picker, 1, wxALL | wxEXPAND, 5);

        // Columns 3+4: spacers
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    } else if (controlType == "fontpicker") {
        // Column 1: Label
        wxWindowID labelId = wxNewId();
        std::string labelName = "ID_STATICTEXT_" + id;
        auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                             wxDefaultSize, 0, wxString(labelName));
        sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

        // Column 2: Font picker
        std::string ctrlName = "ID_FONTPICKER_" + id;
        wxWindowID ctrlId = wxNewId();
        auto* picker = new BulkEditFontPicker(parentWin, ctrlId, wxNullFont, wxDefaultPosition, wxDefaultSize,
                                               wxFNTP_FONTDESC_AS_LABEL, wxDefaultValidator,
                                               wxString(ctrlName));
        info.fontPicker = picker;
        sizer->Add(picker, 1, wxALL | wxEXPAND, 2);

        // Columns 3+4: spacers
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    } else if (controlType == "colourpicker") {
        std::string defaultColor = prop.value("default", "#FFFFFF");

        // Column 1: Label
        wxWindowID labelId = wxNewId();
        std::string labelName = "ID_STATICTEXT_" + id;
        auto* staticText = new wxStaticText(parentWin, labelId, wxString(label), wxDefaultPosition,
                                             wxDefaultSize, 0, wxString(labelName));
        sizer->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

        // Column 2: Colour picker
        std::string ctrlName = "ID_COLOURPICKERCTRL_" + id;
        wxWindowID ctrlId = wxNewId();
        wxColour colour;
        colour.Set(wxString(defaultColor));
        auto* picker = new BulkEditColourPickerCtrl(parentWin, ctrlId, colour, wxDefaultPosition, wxDefaultSize,
                                                      wxCLRP_DEFAULT_STYLE, wxDefaultValidator,
                                                      wxString(ctrlName));
        info.colourPicker = picker;
        sizer->Add(picker, 1, wxALL | wxEXPAND, 2);

        // Columns 3+4: spacers
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    } else if (controlType == "radiobutton") {
        bool defaultVal = prop.value("default", false);

        // Column 1: spacer (radio buttons have their own label)
        sizer->Add(-1, -1, 1, wxALL, 2);

        // Column 2: Radio button
        std::string ctrlName = "ID_RADIOBUTTON_" + id;
        wxWindowID ctrlId = wxNewId();
        auto* rb = new wxRadioButton(parentWin, ctrlId, wxString(label), wxDefaultPosition, wxDefaultSize,
                                      0, wxDefaultValidator, wxString(ctrlName));
        rb->SetValue(defaultVal);
        info.radioButton = rb;
        sizer->Add(rb, 1, wxALL | wxEXPAND, 2);

        // Columns 3+4: spacers
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);
    } else if (controlType == "custom") {
        // Delegate to subclass to create the control
        wxWindow* customCtrl = CreateCustomControl(parentWin, sizer, prop, cols);
        if (customCtrl == nullptr) {
            spdlog::warn("JsonEffectPanel: property '{}' has controlType 'custom' but CreateCustomControl returned null. "
                          "If this is a subclass with a custom override, ensure the subclass passes deferBuild=true to JsonEffectPanel.", id);
        } else if (!tooltip.empty()) {
            customCtrl->SetToolTip(wxString(tooltip));
        }
        // Don't store in properties_ - the subclass manages custom controls
        return;
    }

    properties_[id] = info;

    // Apply tooltip to the primary control for this property
    if (!tooltip.empty()) {
        wxWindow* primary = FindControlForProperty(id);
        if (primary != nullptr) {
            primary->SetToolTip(wxString(tooltip));
        }
    }
}

void JsonEffectPanel::ParseVisibilityRules(const nlohmann::json& metadata) {
    if (!metadata.contains("visibilityRules")) return;

    for (const auto& rule : metadata["visibilityRules"]) {
        VisibilityRule vr;
        if (rule.contains("when")) {
            const auto& when = rule["when"];
            vr.conditionPropertyId = when.value("property", "");
            if (when.contains("equals")) {
                vr.conditionEquals = true;
                if (when["equals"].is_boolean()) {
                    vr.conditionBoolValue = when["equals"].get<bool>();
                } else if (when["equals"].is_string()) {
                    vr.conditionStringEquals = when["equals"].get<std::string>();
                }
            }
            if (when.contains("notEquals")) {
                vr.conditionEquals = false;
                if (when["notEquals"].is_boolean()) {
                    vr.conditionBoolValue = when["notEquals"].get<bool>();
                } else if (when["notEquals"].is_string()) {
                    vr.conditionStringEquals = when["notEquals"].get<std::string>();
                }
            }
            if (when.contains("oneOf")) {
                for (const auto& v : when["oneOf"]) {
                    vr.conditionOneOf.push_back(v.get<std::string>());
                }
            }
            if (when.contains("startsWith")) {
                vr.conditionStartsWith = when["startsWith"].get<std::string>();
            }
            if (when.contains("any")) {
                for (const auto& v : when["any"]) {
                    vr.conditionAnyIds.push_back(v.get<std::string>());
                }
            }
        }

        auto readIds = [](const nlohmann::json& rule, const std::string& key, std::vector<std::string>& out) {
            if (rule.contains(key)) {
                for (const auto& id : rule[key]) {
                    out.push_back(id.get<std::string>());
                }
            }
        };
        readIds(rule, "enable", vr.enableIds);
        readIds(rule, "disable", vr.disableIds);
        readIds(rule, "show", vr.showIds);
        readIds(rule, "hide", vr.hideIds);

        visibilityRules_.push_back(vr);
    }
}

wxWindow* JsonEffectPanel::FindControlForProperty(const std::string& propId) {
    auto it = properties_.find(propId);
    if (it == properties_.end()) return nullptr;
    const auto& info = it->second;
    if (info.slider) return info.slider;
    if (info.textCtrl) return info.textCtrl;
    if (info.checkBox) return info.checkBox;
    if (info.choice) return info.choice;
    if (info.spinCtrl) return info.spinCtrl;
    if (info.filePicker) return info.filePicker;
    if (info.fontPicker) return info.fontPicker;
    if (info.colourPicker) return info.colourPicker;
    if (info.radioButton) return info.radioButton;
    if (info.buddySlider) return info.buddySlider;
    return nullptr;
}

void JsonEffectPanel::ApplyVisibilityRules() {
    for (const auto& rule : visibilityRules_) {
        bool conditionMet = false;

        // "any" condition: OR of multiple checkbox properties
        if (!rule.conditionAnyIds.empty()) {
            for (const auto& anyId : rule.conditionAnyIds) {
                auto it = properties_.find(anyId);
                if (it != properties_.end() && it->second.checkBox) {
                    if (it->second.checkBox->GetValue()) {
                        conditionMet = true;
                        break;
                    }
                }
            }
        } else {
            // Single property condition
            auto it = properties_.find(rule.conditionPropertyId);
            if (it != properties_.end()) {
                const auto& info = it->second;
                if (info.checkBox) {
                    bool val = info.checkBox->GetValue();
                    conditionMet = rule.conditionEquals ? (val == rule.conditionBoolValue)
                                                         : (val != rule.conditionBoolValue);
                } else if (info.choice) {
                    std::string sel = info.choice->GetStringSelection().ToStdString();
                    if (!rule.conditionOneOf.empty()) {
                        for (const auto& v : rule.conditionOneOf) {
                            if (sel == v) { conditionMet = true; break; }
                        }
                    } else if (!rule.conditionStartsWith.empty()) {
                        conditionMet = sel.find(rule.conditionStartsWith) == 0;
                    } else if (!rule.conditionStringEquals.empty()) {
                        conditionMet = rule.conditionEquals ? (sel == rule.conditionStringEquals)
                                                             : (sel != rule.conditionStringEquals);
                    }
                }
            }
        }

        auto setEnabled = [this](const std::vector<std::string>& ids, bool enabled) {
            for (const auto& id : ids) {
                auto it2 = properties_.find(id);
                if (it2 != properties_.end()) {
                    wxWindow* ctrl = FindControlForProperty(id);
                    if (ctrl) ctrl->Enable(enabled);
                    // Also enable/disable buddy controls and VC button
                    if (it2->second.buddySlider) it2->second.buddySlider->Enable(enabled);
                    if (it2->second.buddyText) it2->second.buddyText->Enable(enabled);
                    if (it2->second.textCtrl && ctrl != it2->second.textCtrl)
                        it2->second.textCtrl->Enable(enabled);
                    if (it2->second.valueCurveBtn) it2->second.valueCurveBtn->Enable(enabled);
                }
            }
        };

        // Tracks whether show/hide was actually applied; triggers a single
        // Layout() pass at the end so sizer cells collapse/expand correctly.
        bool visibilityChanged = false;
        auto setVisible = [this, &visibilityChanged](const std::vector<std::string>& ids, bool visible) {
            for (const auto& id : ids) {
                auto it2 = properties_.find(id);
                if (it2 == properties_.end()) continue;

                wxWindow* ctrl = FindControlForProperty(id);
                if (ctrl) { ctrl->Show(visible); visibilityChanged = true; }
                if (it2->second.buddySlider) it2->second.buddySlider->Show(visible);
                if (it2->second.buddyText) it2->second.buddyText->Show(visible);
                if (it2->second.textCtrl && ctrl != it2->second.textCtrl)
                    it2->second.textCtrl->Show(visible);
                if (it2->second.valueCurveBtn) it2->second.valueCurveBtn->Show(visible);

                // Hide the sibling label so the row doesn't leave an orphan cell.
                wxWindow* label = wxWindow::FindWindowByName(wxString::Format("ID_STATICTEXT_%s", id), this);
                if (label) label->Show(visible);

                // Hide the lock button if present. The name encodes the control
                // type (SLIDER/CHECKBOX/CHOICE); the wrong ones just return null.
                for (const char* type : { "SLIDER", "CHECKBOX", "CHOICE" }) {
                    wxWindow* lock = wxWindow::FindWindowByName(
                        wxString::Format("ID_BITMAPBUTTON_%s_%s", type, id), this);
                    if (lock) lock->Show(visible);
                }
            }
        };

        if (conditionMet) {
            setEnabled(rule.enableIds, true);
            setEnabled(rule.disableIds, false);
            setVisible(rule.showIds, true);
            setVisible(rule.hideIds, false);
        } else {
            // Reverse the rule when condition is not met
            setEnabled(rule.enableIds, false);
            setEnabled(rule.disableIds, true);
            setVisible(rule.showIds, false);
            setVisible(rule.hideIds, true);
        }

        // wxFlexGridSizer won't collapse hidden cells without a relayout.
        if (visibilityChanged) {
            Layout();
        }
    }
}

wxString JsonEffectPanel::GetEffectString() {
    // Check if any property uses suppressIfDefault
    bool hasSuppression = false;
    for (const auto& [id, info] : properties_) {
        if (info.suppressIfDefault) { hasSuppression = true; break; }
    }
    if (!hasSuppression) {
        return xlEffectPanel::GetEffectString();
    }

    // Build set of setting keys to suppress (those at default value)
    std::set<std::string> suppressKeys;
    for (const auto& [id, info] : properties_) {
        if (!info.suppressIfDefault || info.defaultValue.is_null()) continue;

        bool isDefault = false;
        if (info.controlType == "slider") {
            if (info.divisor > 1 && info.textCtrl) {
                // Use wxAtof for locale-independent parsing (wxString::ToDouble
                // is locale-aware and breaks for users in locales with ',' decimal).
                double current = wxAtof(info.textCtrl->GetValue());
                isDefault = std::abs(current - info.defaultValue.get<double>()) < 0.001;
            } else if (!info.settingPrefix.empty() && info.textCtrl) {
                long current = 0;
                info.textCtrl->GetValue().ToLong(&current);
                isDefault = current == info.defaultValue.get<int>();
            } else if (info.slider) {
                isDefault = info.slider->GetValue() == info.defaultValue.get<int>();
            } else if (info.buddySlider) {
                isDefault = static_cast<wxSlider*>(info.buddySlider)->GetValue() == info.defaultValue.get<int>();
            }
        } else if (info.controlType == "checkbox" && info.checkBox) {
            isDefault = info.checkBox->GetValue() == info.defaultValue.get<bool>();
        } else if ((info.controlType == "choice" || info.controlType == "combobox") && info.choice) {
            isDefault = info.choice->GetStringSelection().ToStdString() == info.defaultValue.get<std::string>();
        } else if (info.controlType == "spin" && info.spinCtrl) {
            isDefault = info.spinCtrl->GetValue() == info.defaultValue.get<int>();
        } else if (info.controlType == "text" && info.textCtrl) {
            isDefault = info.textCtrl->GetValue().ToStdString() == info.defaultValue.get<std::string>();
        }

        if (isDefault) {
            // Build the setting key prefix for this property
            std::string prefix;
            if (!info.settingPrefix.empty()) {
                prefix = info.settingPrefix;
            } else if (info.controlType == "slider") {
                prefix = info.divisor > 1 ? "TEXTCTRL" : "SLIDER";
            } else if (info.controlType == "checkbox") {
                prefix = "CHECKBOX";
            } else if (info.controlType == "choice" || info.controlType == "combobox") {
                prefix = "CHOICE";
            } else if (info.controlType == "spin") {
                prefix = "SPINCTRL";
            } else if (info.controlType == "text") {
                prefix = "TEXTCTRL";
            }
            suppressKeys.insert("E_" + prefix + "_" + id);
        }
    }

    // Get the full effect string from the base class, then filter
    wxString full = xlEffectPanel::GetEffectString();
    if (suppressKeys.empty() || full.empty()) return full;

    wxString result;
    wxStringTokenizer tkz(full, ",");
    while (tkz.HasMoreTokens()) {
        wxString token = tkz.GetNextToken();
        wxString key = token.BeforeFirst('=');
        if (suppressKeys.find(key.ToStdString()) == suppressKeys.end()) {
            if (!result.empty()) result += ",";
            result += token;
        }
    }
    return result;
}

void JsonEffectPanel::ValidateWindow() {
    ApplyVisibilityRules();
}

void JsonEffectPanel::SetRenderableEffect(RenderableEffect* eff) {
    if (eff == nullptr) return;
    for (auto& [id, info] : properties_) {
        if (info.dynamicOptions == "effect" && info.choice) {
            auto options = eff->GetSettingOptions(id);
            if (!options.empty()) {
                wxString selection = info.choice->GetStringSelection();
                info.choice->Clear();
                for (const auto& opt : options) {
                    info.choice->Append(wxString(opt));
                }
                if (!selection.empty()) {
                    info.choice->SetStringSelection(selection);
                }
                // If still no selection (e.g. brand-new effect), try the JSON
                // default first, then fall back to the first available option.
                if (info.choice->GetSelection() == wxNOT_FOUND &&
                    !info.defaultValue.is_null() && info.defaultValue.is_string()) {
                    std::string def = info.defaultValue.get<std::string>();
                    if (!def.empty()) {
                        info.choice->SetStringSelection(wxString(def));
                    }
                }
                if (info.choice->GetSelection() == wxNOT_FOUND && info.choice->GetCount() > 0) {
                    info.choice->SetSelection(0);
                }
            }
        }
    }
}

void JsonEffectPanel::RepopulateTimingTrackChoices() {
    // Walk all properties; for each one that wants timing tracks, query
    // mSequenceElements directly with the appropriate layer-count filter.
    // Regular ("timingTracks") = layers <= 1, lyric ("lyricTimingTracks") = 3.
    for (auto& [id, info] : properties_) {
        bool wantsRegular = info.dynamicOptions == "timingTracks";
        bool wantsLyric = info.dynamicOptions == "lyricTimingTracks";
        if ((!wantsRegular && !wantsLyric) || info.choice == nullptr) continue;

        wxString selection = info.choice->GetStringSelection();
        info.choice->Clear();
        if (mSequenceElements != nullptr) {
            for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
                Element* e = mSequenceElements->GetElement(i);
                if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) continue;
                int layers = e->GetEffectLayerCount();
                bool match = wantsRegular ? (layers <= 1) : (layers == 3);
                if (match) info.choice->Append(e->GetName());
            }
        }
        if (!selection.empty()) {
            info.choice->SetStringSelection(selection);
        }
        // Fall back to JSON default for brand-new effects.
        if (info.choice->GetSelection() == wxNOT_FOUND &&
            !info.defaultValue.is_null() && info.defaultValue.is_string()) {
            std::string def = info.defaultValue.get<std::string>();
            if (!def.empty()) info.choice->SetStringSelection(wxString(def));
        }
        info.choice->Enable(info.choice->GetCount() > 0);
    }
}

void JsonEffectPanel::SetPanelStatus(Model* cls) {
    RepopulateTimingTrackChoices();

    // Populate model-driven choices (states, faces, modelNodeNames). For
    // ModelGroups, use the first contained model — matches legacy behavior.
    Model* m = cls;
    if (cls != nullptr && cls->GetDisplayAs() == DisplayAsType::ModelGroup) {
        m = static_cast<ModelGroup*>(cls)->GetFirstModel();
    }

    auto populateFromDefinitions = [this, m](const std::string& sourceName, const FaceStateData& defs) {
        for (auto& [id, info] : properties_) {
            if (info.dynamicOptions != sourceName || info.choice == nullptr) continue;
            wxString selection = info.choice->GetStringSelection();
            info.choice->Clear();
            if (m != nullptr) {
                std::set<std::string> seen;
                for (const auto& it : defs) {
                    if (seen.insert(it.first).second) {
                        info.choice->Append(wxString(it.first));
                    }
                }
            }
            if (!selection.empty()) {
                info.choice->SetStringSelection(selection);
            }
            if (info.choice->GetSelection() == wxNOT_FOUND && info.choice->GetCount() > 0) {
                info.choice->SetSelection(0);
            }
        }
    };

    if (m != nullptr) {
        populateFromDefinitions("states", m->GetStateInfo());
        populateFromDefinitions("faces", m->GetFaceInfo());

        // Populate any choice that wants the model's per-channel node names
        // (e.g. Servo's Channel selector). Skips empty / "-"-prefixed names
        // to match legacy ServoPanel behavior.
        int numChannels = m->GetNumChannels();
        for (auto& [id, info] : properties_) {
            if (info.dynamicOptions != "modelNodeNames" || info.choice == nullptr) continue;
            wxString selection = info.choice->GetStringSelection();
            info.choice->Clear();
            for (int i = 0; i <= numChannels; ++i) {
                std::string name = m->GetNodeName(i);
                if (!name.empty() && name[0] != '-') {
                    info.choice->Append(wxString(name));
                }
            }
            if (!selection.empty()) {
                info.choice->SetStringSelection(selection);
            }
            if (info.choice->GetSelection() == wxNOT_FOUND && info.choice->GetCount() > 0) {
                info.choice->SetSelection(0);
            }
        }
    } else {
        // Model is null (brand-new effect with no associated model) — clear any
        // model-driven choices so stale entries from a previous model aren't shown.
        for (auto& [id, info] : properties_) {
            if ((info.dynamicOptions == "states" ||
                 info.dynamicOptions == "faces" ||
                 info.dynamicOptions == "modelNodeNames") && info.choice) {
                info.choice->Clear();
            }
        }
    }
}

void JsonEffectPanel::SetDefaultParameters() {
    // Deactivate all value curves
    for (auto& [id, info] : properties_) {
        if (info.valueCurveBtn) {
            info.valueCurveBtn->SetActive(false);
        }
    }

    // Set controls to their default values from metadata
    for (auto& [id, info] : properties_) {
        if (info.defaultValue.is_null()) continue;

        if (info.controlType == "slider") {
            if (info.divisor > 1) {
                // Float slider - set the buddy IDD_SLIDER
                if (info.buddySlider) {
                    double fval = info.defaultValue.get<double>();
                    int ival = static_cast<int>(fval * info.divisor);
                    SetSliderValue(static_cast<wxSlider*>(info.buddySlider), ival);
                }
            } else {
                if (info.slider) {
                    SetSliderValue(info.slider, info.defaultValue.get<int>());
                }
            }
        } else if (info.controlType == "checkbox") {
            if (info.checkBox) {
                SetCheckBoxValue(info.checkBox, info.defaultValue.get<bool>());
            }
        } else if (info.controlType == "choice" || info.controlType == "combobox") {
            if (info.choice) {
                std::string def = info.defaultValue.get<std::string>();
                // Skip empty defaults: SetChoiceValue would fire a spurious change
                // event with no useful payload (and SetStringSelection("") fails).
                if (!def.empty()) {
                    SetChoiceValue(info.choice, def);
                }
            }
        } else if (info.controlType == "spin") {
            if (info.spinCtrl) {
                SetSpinValue(info.spinCtrl, info.defaultValue.get<int>());
            }
        } else if (info.controlType == "text") {
            if (info.textCtrl) {
                std::string def = info.defaultValue.get<std::string>();
                // Skip empty default for text controls for the same reason.
                if (!def.empty()) {
                    SetTextValue(info.textCtrl, def);
                }
            }
        }
    }

    // Enable canvas mode if the effect requires it
    if (metadata_.value("canvasMode", false)) {
        xLightsFrame* frame = xLightsApp::GetFrame();
        if (frame) {
            TimingPanel* layerBlendingPanel = frame->GetLayerBlendingPanel();
            if (layerBlendingPanel) {
                layerBlendingPanel->CheckBox_Canvas->SetValue(true);
            }
        }
    }
}
