/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ToolbarsSettingsPanel.h"

#include <wx/button.h>
#include <wx/dataview.h>
#include <wx/preferences.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "effectpanels/EffectIconCache.h"
#include "effects/RenderableEffect.h"
#include "xLightsMain.h"

namespace {
    constexpr unsigned int COL_VISIBLE = 0;
    constexpr unsigned int COL_NAME = 1;
    constexpr int EFFECT_ICON_SIZE = 16;

    wxVariant MakeIconTextValue(xLightsFrame* frame, const std::string& name)
    {
        wxDataViewIconText iconText(name);
        if (frame != nullptr) {
            RenderableEffect* eff = frame->GetEffectManager().GetEffect(name);
            if (eff != nullptr) {
                iconText.SetBitmapBundle(EffectIconCache::GetEffectIcon(eff, EFFECT_ICON_SIZE));
            }
        }
        wxVariant value;
        value << iconText;
        return value;
    }
}

ToolbarsSettingsPanel::ToolbarsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL), frame(f)
{
    _mainSizer = new wxBoxSizer(wxVERTICAL);

    _mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Effects Toolbar")), 0, wxLEFT | wxTOP | wxRIGHT, 6);
    _mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Choose which effects appear in the toolbar, and their order:")),
                     0, wxLEFT | wxRIGHT | wxBOTTOM, 6);

    _effectsToolbarList = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER | wxDV_SINGLE);
    _effectsToolbarList->AppendToggleColumn(_("Visible"), wxDATAVIEW_CELL_ACTIVATABLE, FromDIP(30), wxALIGN_CENTER);
    _effectsToolbarList->AppendIconTextColumn(_("Effect"));
    _effectsToolbarList->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, &ToolbarsSettingsPanel::OnToggleChanged, this);
    _effectsToolbarList->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &ToolbarsSettingsPanel::OnSelectionChanged, this);
    // Tall enough to show at least 8 rows without scrolling immediately.
    _effectsToolbarList->SetMinSize(wxSize(-1, FromDIP(220)));

    wxBoxSizer* listRow = new wxBoxSizer(wxHORIZONTAL);
    listRow->Add(_effectsToolbarList, 1, wxEXPAND);

    wxBoxSizer* buttonColumn = new wxBoxSizer(wxVERTICAL);
    _upButton = new wxButton(this, wxID_ANY, _("Move Up"));
    _downButton = new wxButton(this, wxID_ANY, _("Move Down"));
    _upButton->Bind(wxEVT_BUTTON, &ToolbarsSettingsPanel::OnMoveUp, this);
    _downButton->Bind(wxEVT_BUTTON, &ToolbarsSettingsPanel::OnMoveDown, this);
    buttonColumn->AddStretchSpacer(1);
    buttonColumn->Add(_upButton, 0, wxEXPAND | wxBOTTOM, 6);
    buttonColumn->Add(_downButton, 0, wxEXPAND, 0);
    buttonColumn->AddStretchSpacer(1);
    wxButton* resetButton = new wxButton(this, wxID_ANY, _("Reset to Defaults"));
    resetButton->Bind(wxEVT_BUTTON, &ToolbarsSettingsPanel::OnResetToDefaults, this);
    buttonColumn->Add(resetButton, 0, wxEXPAND);
    listRow->Add(buttonColumn, 0, wxEXPAND | wxLEFT, 6);

    _mainSizer->Add(listRow, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 6);

    SetSizer(_mainSizer);

    static const std::vector<std::pair<std::string, bool>> emptyLayout;
    const auto& layout = frame != nullptr ? frame->GetEffectsToolbarLayout() : emptyLayout;
    PopulateRows(layout);
}

void ToolbarsSettingsPanel::PopulateRows(const std::vector<std::pair<std::string, bool>>& layout)
{
    _suppressChangeEvents = true;

    _effectsToolbarList->DeleteAllItems();
    for (const auto& entry : layout) {
        wxVector<wxVariant> row;
        row.push_back(entry.second);
        row.push_back(MakeIconTextValue(frame, entry.first));
        _effectsToolbarList->AppendItem(row);
    }

    if (_effectsToolbarList->GetItemCount() > 0) {
        _effectsToolbarList->SelectRow(0);
    }

    _suppressChangeEvents = false;
    UpdateMoveButtonState();
}

std::string ToolbarsSettingsPanel::GetRowName(unsigned int row) const
{
    wxVariant v;
    _effectsToolbarList->GetValue(v, row, COL_NAME);
    wxDataViewIconText iconText;
    iconText << v;
    return iconText.GetText().ToStdString();
}

std::vector<std::pair<std::string, bool>> ToolbarsSettingsPanel::DefaultEffectsToolbarLayout() const
{
    std::vector<std::pair<std::string, bool>> layout;
    if (frame == nullptr) return layout;

    EffectManager& em = frame->GetEffectManager();
    layout.reserve(em.size());
    for (size_t i = 0; i < em.size(); ++i) {
        layout.emplace_back(em.GetEffectName(static_cast<int>(i)), true);
    }
    return layout;
}

void ToolbarsSettingsPanel::OnResetToDefaults(wxCommandEvent& event)
{
    PopulateRows(DefaultEffectsToolbarLayout());
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ToolbarsSettingsPanel::OnToggleChanged(wxDataViewEvent& event)
{
    event.Skip();
    if (_suppressChangeEvents) {
        return;
    }
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ToolbarsSettingsPanel::OnSelectionChanged(wxDataViewEvent& event)
{
    event.Skip();
    UpdateMoveButtonState();
}

void ToolbarsSettingsPanel::OnMoveUp(wxCommandEvent& event)
{
    MoveSelectedRow(-1);
}

void ToolbarsSettingsPanel::OnMoveDown(wxCommandEvent& event)
{
    MoveSelectedRow(1);
}

void ToolbarsSettingsPanel::MoveSelectedRow(int direction)
{
    int row = _effectsToolbarList->GetSelectedRow();
    if (row == wxNOT_FOUND) return;

    int count = _effectsToolbarList->GetItemCount();
    int target = row + direction;
    if (target < 0 || target >= count) return;

    std::vector<std::pair<std::string, bool>> layout;
    layout.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        layout.emplace_back(GetRowName(static_cast<unsigned int>(i)),
                             _effectsToolbarList->GetToggleValue(static_cast<unsigned int>(i), COL_VISIBLE));
    }
    std::swap(layout[row], layout[target]);

    PopulateRows(layout);
    _effectsToolbarList->SelectRow(static_cast<unsigned int>(target));
    UpdateMoveButtonState();

    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ToolbarsSettingsPanel::UpdateMoveButtonState()
{
    int sel = _effectsToolbarList->GetSelectedRow();
    int count = _effectsToolbarList->GetItemCount();
    _upButton->Enable(sel > 0);
    _downButton->Enable(sel >= 0 && sel < count - 1);
}

bool ToolbarsSettingsPanel::TransferDataFromWindow()
{
    if (frame == nullptr || _effectsToolbarList == nullptr) {
        return true;
    }

    std::vector<std::pair<std::string, bool>> layout;
    int count = _effectsToolbarList->GetItemCount();
    layout.reserve(static_cast<size_t>(count));
    for (int row = 0; row < count; ++row) {
        bool visible = _effectsToolbarList->GetToggleValue(static_cast<unsigned int>(row), COL_VISIBLE);
        layout.emplace_back(GetRowName(static_cast<unsigned int>(row)), visible);
    }
    frame->SetEffectsToolbarLayout(std::move(layout));
    return true;
}
