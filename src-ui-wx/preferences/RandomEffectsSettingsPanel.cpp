/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RandomEffectsSettingsPanel.h"

//(*InternalHeaders(RandomEffectsSettingsPanel)
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/preferences.h>

#include "effects/RenderableEffect.h"
#include "xLightsMain.h"

//(*IdInit(RandomEffectsSettingsPanel)
//*)

BEGIN_EVENT_TABLE(RandomEffectsSettingsPanel,wxPanel)
	//(*EventTable(RandomEffectsSettingsPanel)
	//*)
END_EVENT_TABLE()

RandomEffectsSettingsPanel::RandomEffectsSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(RandomEffectsSettingsPanel)
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	//*)

    // Two-list shuttle: effects on the right are used by Generate Random,
    // effects on the left are not. Arrow buttons (or double-click) move the
    // selection between the lists. wxLB_SORT keeps both alphabetical.
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(new wxStaticText(this, wxID_ANY,
        _("Effects on the right are used by Generate Random. Move effects between the lists to include or exclude them.")),
        0, wxALL, 5);

    auto* row = new wxBoxSizer(wxHORIZONTAL);

    auto* leftCol = new wxBoxSizer(wxVERTICAL);
    leftCol->Add(new wxStaticText(this, wxID_ANY, _("Not used")), 0, wxLEFT | wxBOTTOM, 2);
    _availableList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(260, 340), 0, nullptr, wxLB_EXTENDED | wxLB_SORT);
    _availableList->SetMinSize(wxSize(260, 340)); // stop wxEXPAND shrinking it and clipping effect names
    leftCol->Add(_availableList, 1, wxEXPAND);
    row->Add(leftCol, 1, wxEXPAND | wxRIGHT, 6);

    auto* btnCol = new wxBoxSizer(wxVERTICAL);
    auto* btnAdd = new wxButton(this, wxID_ANY, _(">"), wxDefaultPosition, wxSize(44, -1));
    auto* btnRemove = new wxButton(this, wxID_ANY, _("<"), wxDefaultPosition, wxSize(44, -1));
    btnAdd->SetToolTip(_("Use the selected effects"));
    btnRemove->SetToolTip(_("Stop using the selected effects"));
    btnCol->AddStretchSpacer();
    btnCol->Add(btnAdd, 0, wxBOTTOM, 6);
    btnCol->Add(btnRemove, 0);
    btnCol->AddStretchSpacer();
    row->Add(btnCol, 0, wxALIGN_CENTER_VERTICAL);

    auto* rightCol = new wxBoxSizer(wxVERTICAL);
    rightCol->Add(new wxStaticText(this, wxID_ANY, _("Used")), 0, wxLEFT | wxBOTTOM, 2);
    _usedList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(260, 340), 0, nullptr, wxLB_EXTENDED | wxLB_SORT);
    _usedList->SetMinSize(wxSize(260, 340)); // stop wxEXPAND shrinking it and clipping effect names
    rightCol->Add(_usedList, 1, wxEXPAND);
    row->Add(rightCol, 1, wxEXPAND | wxLEFT, 6);

    mainSizer->Add(row, 1, wxEXPAND | wxALL, 5);
    SetSizer(mainSizer);
    mainSizer->SetSizeHints(this);
    // The page lives in a scrolled window that lays the panel out at its own
    // minimum size, so force a width wide enough for both lists to show full
    // effect names rather than clipping them with an ellipsis.
    SetMinSize(wxSize(620, 420));

    const wxArrayString& used = frame->RandomEffectsToUse();
    for (int i = 0; i < (int)frame->GetEffectManager().size(); i++) {
        wxString n = frame->GetEffectManager()[i]->Name();
        if (used.Index(n) >= 0) {
            _usedList->Append(n);
        } else {
            _availableList->Append(n);
        }
    }

    btnAdd->Bind(wxEVT_BUTTON, &RandomEffectsSettingsPanel::OnAdd, this);
    btnRemove->Bind(wxEVT_BUTTON, &RandomEffectsSettingsPanel::OnRemove, this);
    _availableList->Bind(wxEVT_LISTBOX_DCLICK, &RandomEffectsSettingsPanel::OnAvailableDClick, this);
    _usedList->Bind(wxEVT_LISTBOX_DCLICK, &RandomEffectsSettingsPanel::OnUsedDClick, this);
}

RandomEffectsSettingsPanel::~RandomEffectsSettingsPanel()
{
	//(*Destroy(RandomEffectsSettingsPanel)
	//*)
}

void RandomEffectsSettingsPanel::MoveSelected(wxListBox* from, wxListBox* to)
{
    wxArrayInt sel;
    from->GetSelections(sel);
    if (sel.IsEmpty()) return;

    // Capture the strings before deleting (indices shift on delete).
    wxArrayString moving;
    for (size_t i = 0; i < sel.GetCount(); ++i) {
        moving.Add(from->GetString(sel[i]));
    }
    // GetSelections returns ascending indices; delete high-to-low so the
    // remaining indices stay valid.
    for (int i = (int)sel.GetCount() - 1; i >= 0; --i) {
        from->Delete(sel[i]);
    }
    for (const auto& s : moving) {
        to->Append(s); // wxLB_SORT keeps the destination alphabetical
    }
    ApplyIfImmediate();
}

void RandomEffectsSettingsPanel::ApplyIfImmediate()
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void RandomEffectsSettingsPanel::OnAdd(wxCommandEvent& event)
{
    MoveSelected(_availableList, _usedList);
}

void RandomEffectsSettingsPanel::OnRemove(wxCommandEvent& event)
{
    MoveSelected(_usedList, _availableList);
}

void RandomEffectsSettingsPanel::OnAvailableDClick(wxCommandEvent& event)
{
    MoveSelected(_availableList, _usedList);
}

void RandomEffectsSettingsPanel::OnUsedDClick(wxCommandEvent& event)
{
    MoveSelected(_usedList, _availableList);
}

bool RandomEffectsSettingsPanel::TransferDataToWindow() {
    return true;
}

bool RandomEffectsSettingsPanel::TransferDataFromWindow() {
    wxArrayString selected;
    for (unsigned int i = 0; i < _usedList->GetCount(); ++i) {
        selected.push_back(_usedList->GetString(i));
    }
    frame->SetRandomEffectsToUse(selected);
    return true;
}
