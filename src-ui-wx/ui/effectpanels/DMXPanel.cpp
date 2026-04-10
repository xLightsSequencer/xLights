/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DMXPanel.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choicdlg.h>
#include <wx/menu.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>

#include <format>

#include "effects/DMXEffect.h"
#include "render/Effect.h"
#include "render/Element.h"
#include "models/ModelGroup.h"
#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/utils/wxUtilities.h"
#include "../../ui/sequencer/MainSequencer.h"
#include "../../ui/setup/RemapDMXChannelsDialog.h"
#include "../../xLightsApp.h"
#include "../../xLightsMain.h"
#include "UtilFunctions.h"

DMXPanel::DMXPanel(wxWindow* parent, const nlohmann::json& metadata) :
    JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    SetName("ID_PANEL_DMX");
    ValidateWindow();
}

wxWindow* DMXPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                         const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "DMX_ChannelsNotebook") {
        return BuildChannelsNotebook(parentWin, sizer);
    }
    if (id == "DMX_ButtonsRow") {
        return BuildButtonsRow(parentWin, sizer);
    }
    return nullptr;
}

wxWindow* DMXPanel::BuildChannelsNotebook(wxWindow* parentWin, wxSizer* sizer) {
    _notebook = new wxNotebook(parentWin, wxNewId(), wxDefaultPosition, wxDefaultSize,
                                0, _T("ID_NOTEBOOK_DMX"));

    for (int page = 0; page < 3; ++page) {
        auto* pagePanel = new wxPanel(_notebook, wxID_ANY);
        BuildChannelPage(pagePanel, page * 16 + 1);
        wxString label = wxString::Format("Channels %d-%d", page * 16 + 1, page * 16 + 16);
        _notebook->AddPage(pagePanel, label, false);
    }

    sizer->Add(_notebook, 1, wxALL | wxEXPAND, 2);
    return _notebook;
}

void DMXPanel::BuildChannelPage(wxWindow* pagePanel, int startChannel) {
    // 5 columns per row: label | slider | VC | text | invert checkbox.
    // Control names match the legacy wxSmith names exactly so the render
    // engine (which reads SLIDER_DMXn / VALUECURVE_DMXn / CHECKBOX_INVDMXn
    // with the E_ prefix stripped at render time) continues to work and
    // existing sequences round-trip unchanged.
    auto* grid = new wxFlexGridSizer(0, 5, 0, 0);
    grid->AddGrowableCol(1);

    for (int ch = startChannel; ch < startChannel + 16; ++ch) {
        std::string chNum = std::format("{}", ch);

        // Label
        auto* label = new wxStaticText(pagePanel, wxNewId(),
                                        wxString::Format("Channel %d:", ch),
                                        wxDefaultPosition, wxDefaultSize, 0,
                                        wxString::Format("ID_STATICTEXT_DMX%d", ch));
        grid->Add(label, 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

        // Slider (primary serialized control)
        auto* slider = new BulkEditSlider(pagePanel, wxNewId(), 0, DMX_MIN, DMX_MAX,
                                           wxDefaultPosition, wxDefaultSize, 0,
                                           wxDefaultValidator,
                                           wxString::Format("ID_SLIDER_DMX%d", ch));
        grid->Add(slider, 1, wxALL | wxEXPAND, 2);

        // Value curve button
        auto* vcb = new BulkEditValueCurveButton(pagePanel, wxNewId(),
                                                  GetValueCurveNotSelectedBitmap(),
                                                  wxDefaultPosition, wxDefaultSize,
                                                  wxBU_AUTODRAW | wxBORDER_NONE,
                                                  wxDefaultValidator,
                                                  wxString::Format("ID_VALUECURVE_DMX%d", ch));
        vcb->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
        grid->Add(vcb, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        vcb->Bind(wxEVT_BUTTON, &DMXPanel::OnVCButtonClick, this);

        // Text buddy (IDD_ prefix so it's not serialized)
        auto* text = new BulkEditTextCtrl(pagePanel, wxNewId(), _("0"),
                                           wxDefaultPosition,
                                           wxDLG_UNIT(pagePanel, wxSize(20, -1)),
                                           wxTE_PROCESS_ENTER, wxDefaultValidator,
                                           wxString::Format("IDD_TEXTCTRL_DMX%d", ch));
        grid->Add(text, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

        // Invert checkbox
        auto* inv = new BulkEditCheckBox(pagePanel, wxNewId(), _("Inv"),
                                          wxDefaultPosition, wxDefaultSize, 0,
                                          wxDefaultValidator,
                                          wxString::Format("ID_CHECKBOX_INVDMX%d", ch));
        grid->Add(inv, 1, wxALL | wxEXPAND, 2);
    }

    pagePanel->SetSizer(grid);
}

wxWindow* DMXPanel::BuildButtonsRow(wxWindow* parentWin, wxSizer* sizer) {
    auto* row = new wxFlexGridSizer(0, 3, 0, 0);

    _buttonRemap = new wxButton(parentWin, wxNewId(), _("Remap Channels"));
    row->Add(_buttonRemap, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    _buttonRemap->Bind(wxEVT_BUTTON, &DMXPanel::OnRemapClick, this);
    _buttonRemap->Bind(wxEVT_CONTEXT_MENU, &DMXPanel::OnRemapRClick, this);

    _buttonSaveAsState = new wxButton(parentWin, wxNewId(), _("Save As State"));
    row->Add(_buttonSaveAsState, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    _buttonSaveAsState->Bind(wxEVT_BUTTON, &DMXPanel::OnSaveAsStateClick, this);

    _buttonLoadState = new wxButton(parentWin, wxNewId(), _("Load From State"));
    row->Add(_buttonLoadState, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    _buttonLoadState->Bind(wxEVT_BUTTON, &DMXPanel::OnLoadFromStateClick, this);

    sizer->Add(row, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    return _buttonRemap;
}

void DMXPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    if (_buttonSaveAsState == nullptr) return;

    bool disableSave = false;
    auto models = GetActiveModels();
    if (models.empty()) disableSave = true;

    for (int i = 0; i < DMX_CHANNELS && !disableSave; ++i) {
        wxString vcName = wxString::Format("ID_VALUECURVE_DMX%d", i + 1);
        auto* curve = dynamic_cast<ValueCurveButton*>(
            wxWindow::FindWindowByName(vcName, this));
        if (curve && curve->GetValue()->IsActive()) {
            disableSave = true;
        }
    }

    _buttonSaveAsState->Enable(!disableSave);
}

void DMXPanel::SetDefaultParameters() {
    JsonEffectPanel::SetDefaultParameters();

    for (int i = 1; i <= DMX_CHANNELS; ++i) {
        if (auto* vc = dynamic_cast<BulkEditValueCurveButton*>(
                wxWindow::FindWindowByName(wxString::Format("ID_VALUECURVE_DMX%d", i), this))) {
            vc->SetActive(false);
        }
        if (auto* slider = dynamic_cast<wxSlider*>(
                wxWindow::FindWindowByName(wxString::Format("ID_SLIDER_DMX%d", i), this))) {
            SetSliderValue(slider, 0);
        }
        if (auto* inv = dynamic_cast<wxCheckBox*>(
                wxWindow::FindWindowByName(wxString::Format("ID_CHECKBOX_INVDMX%d", i), this))) {
            SetCheckBoxValue(inv, false);
        }
    }
}

void DMXPanel::SetPanelStatus(Model* cls) {
    JsonEffectPanel::SetPanelStatus(cls);

    if (cls == nullptr) return;

    Model* m = cls;
    if (cls->GetDisplayAs() == DisplayAsType::ModelGroup) {
        m = dynamic_cast<ModelGroup*>(cls)->GetFirstModel();
        if (m == nullptr) m = cls;
    }

    int const num_channels = m->GetNumChannels();

    for (int i = 1; i <= DMX_CHANNELS; ++i) {
        auto* label = dynamic_cast<wxStaticText*>(
            wxWindow::FindWindowByName(wxString::Format("ID_STATICTEXT_DMX%d", i), this));
        auto* slider = wxWindow::FindWindowByName(wxString::Format("ID_SLIDER_DMX%d", i), this);
        auto* curve = wxWindow::FindWindowByName(wxString::Format("ID_VALUECURVE_DMX%d", i), this);
        auto* text = wxWindow::FindWindowByName(wxString::Format("IDD_TEXTCTRL_DMX%d", i), this);
        auto* inv = wxWindow::FindWindowByName(wxString::Format("ID_CHECKBOX_INVDMX%d", i), this);

        if (label) {
            std::string name = m->GetNodeName(i - 1);
            if (name.empty()) {
                label->SetLabel(wxString::Format("Channel %d:", i));
            } else {
                label->SetLabel(wxString::Format("%s:", name));
            }
        }
        bool enabled = (i <= num_channels);
        if (label) label->Enable(enabled);
        if (slider) slider->Enable(enabled);
        if (curve) curve->Enable(enabled);
        if (text) text->Enable(enabled);
        if (inv) inv->Enable(enabled);
    }

    if (_notebook) _notebook->Layout();
    Refresh();
}

std::list<Model*> DMXPanel::GetActiveModels() {
    std::list<Model*> res;

    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return res;

    auto effect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
    if (effect == nullptr || effect->GetParentEffectLayer() == nullptr) return res;

    Element* element = effect->GetParentEffectLayer()->GetParentElement();
    if (element == nullptr) return res;

    ModelElement* me = dynamic_cast<ModelElement*>(element);
    if (me == nullptr) return res;

    auto model = xLightsApp::GetFrame()->AllModels[me->GetModelName()];
    if (model == nullptr) return res;

    if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
        auto mg = dynamic_cast<ModelGroup*>(model);
        if (mg != nullptr) {
            for (const auto& it : mg->GetFlatModels(true, false)) {
                if (it->GetDisplayAs() != DisplayAsType::ModelGroup &&
                    it->GetDisplayAs() != DisplayAsType::SubModel) {
                    res.push_back(it);
                }
            }
        }
    } else if (model->GetDisplayAs() == DisplayAsType::SubModel) {
        // SubModels are not added.
    } else {
        res.push_back(model);
    }

    return res;
}

void DMXPanel::OnRemapClick(wxCommandEvent& /*event*/) {
    RemapDMXChannelsDialog dlg(this);
    if (dlg.ShowModal() != wxID_OK) return;

    // Snapshot current values so the remap can reference "from" channels
    // while writing "to" channels without trampling itself.
    std::vector<int> sliders(DMX_CHANNELS);
    std::vector<std::string> curves(DMX_CHANNELS);
    std::vector<bool> checks(DMX_CHANNELS);

    for (int i = 0; i < DMX_CHANNELS; ++i) {
        auto* slider = dynamic_cast<wxSlider*>(
            wxWindow::FindWindowByName(wxString::Format("ID_SLIDER_DMX%d", i + 1), this));
        auto* curve = dynamic_cast<ValueCurveButton*>(
            wxWindow::FindWindowByName(wxString::Format("ID_VALUECURVE_DMX%d", i + 1), this));
        auto* check = dynamic_cast<wxCheckBox*>(
            wxWindow::FindWindowByName(wxString::Format("ID_CHECKBOX_INVDMX%d", i + 1), this));
        if (slider) sliders[i] = slider->GetValue();
        if (curve) curves[i] = curve->GetValue()->Serialise();
        if (check) checks[i] = check->GetValue();
    }

    for (int i = 0; i < DMX_CHANNELS; ++i) {
        if (!dlg.DoMapping(i)) continue;

        int from = i;
        int to = dlg.GetToChannel(i);
        auto* slider = dynamic_cast<wxSlider*>(
            wxWindow::FindWindowByName(wxString::Format("ID_SLIDER_DMX%d", to), this));
        auto* curve = dynamic_cast<ValueCurveButton*>(
            wxWindow::FindWindowByName(wxString::Format("ID_VALUECURVE_DMX%d", to), this));
        auto* text = dynamic_cast<wxTextCtrl*>(
            wxWindow::FindWindowByName(wxString::Format("IDD_TEXTCTRL_DMX%d", to), this));
        auto* check = dynamic_cast<wxCheckBox*>(
            wxWindow::FindWindowByName(wxString::Format("ID_CHECKBOX_INVDMX%d", to), this));

        double scale = dlg.GetChanScale(i);
        int offset = dlg.GetChanOffset(i);
        int new_value = ((double)sliders[from] * scale) + offset;

        const std::string& inv = dlg.GetChanInvert(i);
        if (inv == "Check") {
            checks[from] = true;
        } else if (inv == "Uncheck") {
            checks[from] = false;
        }

        if (text) text->SetValue(wxString::Format("%d", new_value));
        if (slider) slider->SetValue(new_value);
        if (check) check->SetValue(checks[from]);
        if (curve) {
            curve->GetValue()->Deserialise(curves[from]);
            curve->GetValue()->ScaleAndOffsetValues(scale, offset);
            wxCommandEvent vcevent;
            vcevent.SetEventObject(curve);
            OnVCChanged(vcevent);
            curve->UpdateState();
        }
    }
    FireChangeEvent();
}

void DMXPanel::OnRemapRClick(wxContextMenuEvent& /*event*/) {
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return;

    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects < 1) return;

    wxMenu mnu;
    mnu.Append(wxID_ANY, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&DMXPanel::OnRemapBulkEdit, nullptr, this);
    PopupMenu(&mnu);
}

void DMXPanel::OnRemapBulkEdit(wxCommandEvent& /*event*/) {
    RemapDMXChannelsDialog dlg(this);
    if (dlg.ShowModal() != wxID_OK) return;

    std::vector<std::tuple<int, int, float, int, std::string>> dmxmappings;
    for (int i = 0; i < DMX_CHANNELS; ++i) {
        if (dlg.DoMapping(i)) {
            dmxmappings.emplace_back(i + 1, dlg.GetToChannel(i),
                                     dlg.GetChanScale(i), dlg.GetChanOffset(i),
                                     dlg.GetChanInvert(i));
        }
    }

    if (dmxmappings.empty()) return;

    xLightsApp::GetFrame()->GetMainSequencer()->RemapSelectedDMXEffectValues(dmxmappings);
    auto effect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
    if (effect != nullptr) {
        xLightsApp::GetFrame()->GetMainSequencer()->PanelEffectGrid->RaiseSelectedEffectChanged(effect, true, true);
    }
}

void DMXPanel::OnSaveAsStateClick(wxCommandEvent& /*event*/) {
    uint32_t maxChannels = 0;

    auto models = GetActiveModels();
    std::string stateName;
    while (stateName.empty()) {
        wxTextEntryDialog dlg(this, "Enter name for the state", "State Name");
        if (dlg.ShowModal() != wxID_OK) return;

        stateName = dlg.GetValue().ToStdString();
        stateName = ::Lower(stateName);
        stateName = StripAllBut(stateName, "abcdefghijklmnopqrstuvwxyz0123456789-_/\\|#");

        for (const auto& it : models) {
            if (it->GetChanCount() > maxChannels) maxChannels = it->GetChanCount();
            if (it->HasState(stateName)) {
                if (wxMessageBox("A State With Same Name Already Exists\nOverride it?",
                                 "Override State", wxYES_NO | wxICON_QUESTION, this) == wxNO) {
                    stateName = "";
                }
                break;
            }
        }
    }

    std::map<std::string, std::string> attributes;
    attributes["CustomColors"] = "1";
    attributes["Name"] = stateName;
    attributes["Type"] = "SingleNode";

    for (uint32_t i = 0; i < DMX_CHANNELS; ++i) {
        if (i < maxChannels) {
            attributes[std::format("s{}-Name", i + 1)] = stateName;

            auto* label = dynamic_cast<wxStaticText*>(
                wxWindow::FindWindowByName(wxString::Format("ID_STATICTEXT_DMX%d", i + 1), this));
            std::string l;
            if (label) {
                l = label->GetLabelText().ToStdString();
                if (!l.empty() && l.back() == ':') l.pop_back();
            }
            if (StartsWith(l, "Channel")) {
                l = std::format("Node {}", i + 1);
            }
            attributes[std::format("s{}", i + 1)] = l;

            auto* slider = dynamic_cast<wxSlider*>(
                wxWindow::FindWindowByName(wxString::Format("ID_SLIDER_DMX%d", i + 1), this));
            int v = slider ? slider->GetValue() : 0;
            attributes[std::format("s{}-Color", i + 1)] = std::format("#{:02x}{:02x}{:02x}", v, v, v);
        } else {
            attributes[std::format("s{}-Name", i + 1)] = "";
            attributes[std::format("s{}", i + 1)] = "";
            attributes[std::format("s{}-Color", i + 1)] = "";
        }
    }

    for (auto& it : models) {
        it->AddState(attributes);
    }
    wxPostEvent(xLightsApp::GetFrame(), wxCommandEvent(EVT_RGBEFFECTS_CHANGED));
}

void DMXPanel::OnLoadFromStateClick(wxCommandEvent& /*event*/) {
    uint32_t maxChannels = DMX_CHANNELS;

    auto models = GetActiveModels();
    if (models.empty()) return;

    auto m = models.front();
    if (m == nullptr) return;

    maxChannels = std::min(maxChannels, m->GetChanCount());
    wxArrayString choices;
    for (const auto& key : m->GetStateInfo()) {
        choices.push_back(key.first);
    }
    wxSingleChoiceDialog dlg(this, "Select State", "Select State", choices);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string stateName = dlg.GetStringSelection().ToStdString();
    auto states = m->GetStateInfo().at(stateName);

    if (states["CustomColors"] != "1" || states["Type"] != "SingleNode") {
        DisplayError("State does not have Force Custom Colors or Single Node Type");
        return;
    }

    for (size_t i = 0; i < maxChannels; ++i) {
        auto attr = std::format("s{}-Name", (int)i + 1);
        if (states.count(attr) == 0) continue;

        auto colattr = std::format("s{}-Color", (int)i + 1);
        xlColor dmxValue(states[colattr]);

        if (auto* slider = dynamic_cast<wxSlider*>(
                wxWindow::FindWindowByName(wxString::Format("ID_SLIDER_DMX%d", (int)i + 1), this))) {
            slider->SetValue(dmxValue.red);
        }
        if (auto* text = dynamic_cast<wxTextCtrl*>(
                wxWindow::FindWindowByName(wxString::Format("IDD_TEXTCTRL_DMX%d", (int)i + 1), this))) {
            text->SetValue(wxString::Format("%d", dmxValue.red));
        }
    }
    FireChangeEvent();
}
