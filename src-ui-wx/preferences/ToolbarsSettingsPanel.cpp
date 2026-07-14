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

#include <wx/rearrangectrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "xLightsMain.h"

ToolbarsSettingsPanel::ToolbarsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL), frame(f)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Effects Toolbar"), 0, wxLEFT | wxTOP | wxRIGHT, 6);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Choose which effects appear in the toolbar, and their order:"),
                    0, wxLEFT | wxRIGHT | wxBOTTOM, 6);

    static const std::vector<std::pair<std::string, bool>> emptyLayout;
    const auto& layout = frame != nullptr ? frame->GetEffectsToolbarLayout() : emptyLayout;

    wxArrayString items;
    wxArrayInt order;
    for (size_t i = 0; i < layout.size(); ++i) {
        _effectsToolbarItemNames.push_back(layout[i].first);
        items.Add(layout[i].first);
        order.Add(layout[i].second ? static_cast<int>(i) : ~static_cast<int>(i));
    }

    _effectsToolbarRearrange = new wxRearrangeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, order, items);
    mainSizer->Add(_effectsToolbarRearrange, 1, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);
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
