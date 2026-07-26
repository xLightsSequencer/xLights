/***************************************************************
 * This source files comes from the xLights project
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

#include <wx/preferences.h>
#include <wx/statbmp.h>
#include "effectpanels/EffectIconCache.h"
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
	wxStaticText* StaticText1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	MainSizer = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Select Effects for Generate Random"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	MainSizer->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	EffectsGridSizer = new wxFlexGridSizer(0, 4, 0, 0);
	MainSizer->Add(EffectsGridSizer, 1, wxALL|wxEXPAND, 0);
	SetSizer(MainSizer);
	MainSizer->Fit(this);
	MainSizer->SetSizeHints(this);
	//*)

    const wxArrayString &selected = frame->RandomEffectsToUse();
    for (int i = 0; i < (int)frame->GetEffectManager().size(); i++) {
        RenderableEffect* eff = frame->GetEffectManager()[i];
        wxString n = eff->Name();
        bool checked = selected.Index(n) >= 0;
        wxWindowID id = wxNewId();

        wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

        wxCheckBox *cb = new wxCheckBox(this, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, n);
        cb->SetValue(checked);
        rowSizer->Add(cb, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

        wxStaticBitmap* icon = new wxStaticBitmap(this, wxID_ANY, EffectIconCache::GetEffectIcon(eff, 16));
        rowSizer->Add(icon, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

        wxStaticText* label = new wxStaticText(this, wxID_ANY, n);
        rowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 0);

        _effectCheckBoxes.push_back(cb);
        EffectsGridSizer->Add(rowSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
        Connect(id,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&RandomEffectsSettingsPanel::OnEffectCheckBoxClick);
    }
    EffectsGridSizer->Layout();
    MainSizer->Layout();
    MainSizer->Fit(this);
    MainSizer->SetSizeHints(this);
}

RandomEffectsSettingsPanel::~RandomEffectsSettingsPanel()
{
	//(*Destroy(RandomEffectsSettingsPanel)
	//*)
}

bool RandomEffectsSettingsPanel::TransferDataToWindow() {
    return true;
}
bool RandomEffectsSettingsPanel::TransferDataFromWindow() {
    wxArrayString selected;
    for (wxCheckBox* cb : _effectCheckBoxes) {
        if (cb->IsChecked()) {
            selected.push_back(cb->GetName());
        }
    }
    frame->SetRandomEffectsToUse(selected);
    return true;
}
void RandomEffectsSettingsPanel::OnEffectCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
