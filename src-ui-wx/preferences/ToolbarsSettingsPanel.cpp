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
#include <wx/rearrangectrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "xLightsMain.h"

ToolbarsSettingsPanel::ToolbarsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL), frame(f)
{
    _mainSizer = new wxBoxSizer(wxVERTICAL);

    _mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Effects Toolbar")), 0, wxLEFT | wxTOP | wxRIGHT, 6);
    _mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Choose which effects appear in the toolbar, and their order:")),
                     0, wxLEFT | wxRIGHT | wxBOTTOM, 6);

    static const std::vector<std::pair<std::string, bool>> emptyLayout;
    const auto& layout = frame != nullptr ? frame->GetEffectsToolbarLayout() : emptyLayout;
    BuildRearrangeCtrl(layout);

    wxButton* resetButton = new wxButton(this, wxID_ANY, _("Reset to Defaults"));
    resetButton->Bind(wxEVT_BUTTON, &ToolbarsSettingsPanel::OnResetToDefaults, this);
    _mainSizer->Add(resetButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_LEFT, 6);

    SetSizer(_mainSizer);
}

void ToolbarsSettingsPanel::BuildRearrangeCtrl(const std::vector<std::pair<std::string, bool>>& layout)
{
    if (_effectsToolbarRearrange != nullptr) {
        _mainSizer->Detach(_effectsToolbarRearrange);
        _effectsToolbarRearrange->Destroy();
        _effectsToolbarRearrange = nullptr;
    }
    _effectsToolbarItemNames.clear();

    wxArrayString items;
    wxArrayInt order;
    for (size_t i = 0; i < layout.size(); ++i) {
        _effectsToolbarItemNames.push_back(layout[i].first);
        items.Add(wxString(layout[i].first));
        order.Add(layout[i].second ? static_cast<int>(i) : ~static_cast<int>(i));
    }

    _effectsToolbarRearrange = new wxRearrangeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, order, items);
    _mainSizer->Insert(2, _effectsToolbarRearrange, 1, wxEXPAND | wxALL, 6);
    _mainSizer->Layout();
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
    BuildRearrangeCtrl(DefaultEffectsToolbarLayout());
}

bool ToolbarsSettingsPanel::TransferDataFromWindow()
{
    if (frame == nullptr || _effectsToolbarRearrange == nullptr) {
        return true;
    }

    const wxArrayInt& resultOrder = _effectsToolbarRearrange->GetList()->GetCurrentOrder();
    std::vector<std::pair<std::string, bool>> layout;
    layout.reserve(resultOrder.size());
    for (size_t pos = 0; pos < resultOrder.size(); ++pos) {
        int encoded = resultOrder[pos];
        bool visible = encoded >= 0;
        int originalIndex = visible ? encoded : ~encoded;
        if (originalIndex >= 0 && static_cast<size_t>(originalIndex) < _effectsToolbarItemNames.size()) {
            layout.emplace_back(_effectsToolbarItemNames[static_cast<size_t>(originalIndex)], visible);
        }
    }
    frame->SetEffectsToolbarLayout(std::move(layout));
    return true;
}
