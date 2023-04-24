/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/dnd.h>
#include "TopEffectsPanel.h"
#include <wx/event.h>
#include <wx/artprov.h>
#include "sequencer/EffectDropTarget.h"
#include "../include/Off.xpm"
#include "xLightsMain.h"
#include "xLightsApp.h"
#include "sequencer/MainSequencer.h"

//(*InternalHeaders(TopEffectsPanel)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)

//(*IdInit(TopEffectsPanel)
const long TopEffectsPanel::ID_BITMAPBUTTON_RANDOMIZE = wxNewId();
const long TopEffectsPanel::ID_BUTTON_UpdateEffect = wxNewId();
const long TopEffectsPanel::ID_BITMAPBUTTON_SelectedEffect = wxNewId();
//*)

BEGIN_EVENT_TABLE(TopEffectsPanel,wxPanel)
	//(*EventTable(TopEffectsPanel)
	//*)
END_EVENT_TABLE()

TopEffectsPanel::TopEffectsPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TopEffectsPanel)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	MainSizer = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableCol(2);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	BitmapButtonRandomize = new wxBitmapButton(this, ID_BITMAPBUTTON_RANDOMIZE,  wxArtProvider::GetBitmapBundle("xlART_DICE_ICON", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_RANDOMIZE"));
	BitmapButtonRandomize->SetToolTip(_("Randomize color and effect settings without a Lock"));
	FlexGridSizer5->Add(BitmapButtonRandomize, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonUpdateEffect = new wxButton(this, ID_BUTTON_UpdateEffect, _("Update"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_UpdateEffect"));
	FlexGridSizer5->Add(ButtonUpdateEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButtonSelectedEffect = new DragEffectBitmapButton(this, ID_BITMAPBUTTON_SelectedEffect, Off, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SelectedEffect"));
	BitmapButtonSelectedEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer5->Add(BitmapButtonSelectedEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	EffectSizer = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->Add(EffectSizer, 1, wxALL|wxEXPAND, 0);
	MainSizer->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	SetSizer(MainSizer);

	Connect(ID_BITMAPBUTTON_RANDOMIZE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TopEffectsPanel::OnButtonRandomizeEffectClick);
	Connect(ID_BUTTON_UpdateEffect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TopEffectsPanel::OnButtonUpdateEffectClick);
	Connect(ID_BITMAPBUTTON_SelectedEffect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TopEffectsPanel::OnBitmapButtonSelectedEffectClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&TopEffectsPanel::OnResize);
	//*)

    BitmapButtonSelectedEffect->SetBitmap(wxBitmap(Off));
}

TopEffectsPanel::~TopEffectsPanel()
{
	//(*Destroy(TopEffectsPanel)
	//*)
}


void TopEffectsPanel::SetDragIconBuffer(const wxBitmapBundle &buffer)
{
    BitmapButtonSelectedEffect->SetBitmap(buffer);
}

void TopEffectsPanel::OnResize(wxSizeEvent& event)
{
    wxCommandEvent eventWindowResized(EVT_WINDOW_RESIZED);
    wxPostEvent(GetParent(), eventWindowResized);
    Refresh();
}

void TopEffectsPanel::OnBitmapButtonSelectedEffectClick(wxCommandEvent& event)
{

}

void TopEffectsPanel::OnLeftDown(wxMouseEvent& event)
{
}


void TopEffectsPanel::OnButtonUpdateEffectClick(wxCommandEvent& event)
{
    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
	if (alleffects > 1) {
        if (wxMessageBox("Are you sure you want to change the settings on all selected effects?", "Update all", wxYES_NO | wxCENTRE, this) == wxNO)
            return;
    }

    wxCommandEvent eventEffectUpdated(EVT_EFFECT_UPDATED);
    wxPostEvent(GetParent(), eventEffectUpdated);
    Refresh();

}

void TopEffectsPanel::OnButtonRandomizeEffectClick(wxCommandEvent& event)
{
    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects > 1) {
        if (wxMessageBox("Are you sure you want to randomise the settings on all selected effects?", "Update all", wxYES_NO | wxCENTRE, this) == wxNO)
            return;
    }

    wxCommandEvent eventEffectRandomize(EVT_EFFECT_RANDOMIZE);
	wxPostEvent(GetParent(), eventEffectRandomize);
	Refresh();
}
