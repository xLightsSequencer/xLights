#include <wx/dnd.h>
#include "TopEffectsPanel.h"
#include <wx/event.h>
#include "xLightsMain.h"
#include "EffectDropTarget.h"

//(*InternalHeaders(TopEffectsPanel)
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(TopEffectsPanel)
const long TopEffectsPanel::ID_BUTTON1 = wxNewId();
const long TopEffectsPanel::ID_BITMAPBUTTON_SelectedEffect = wxNewId();
const long TopEffectsPanel::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TopEffectsPanel,wxPanel)
	//(*EventTable(TopEffectsPanel)
	//*)
END_EVENT_TABLE()

TopEffectsPanel::TopEffectsPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TopEffectsPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableCol(2);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	Button1 = new wxButton(this, ID_BUTTON1, _("Update (F5)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButtonSelectedEffect = new DragEffectBitmapButton(this, ID_BITMAPBUTTON_SelectedEffect, butterfly, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_SelectedEffect"));
	BitmapButtonSelectedEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer5->Add(BitmapButtonSelectedEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	Panel_EffectContainer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer4->Add(Panel_EffectContainer, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SelectedEffect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TopEffectsPanel::OnBitmapButtonSelectedEffectClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&TopEffectsPanel::OnResize);
	//*)

    SetDropTarget(new EffectDropTarget(false));
}

TopEffectsPanel::~TopEffectsPanel()
{
	//(*Destroy(TopEffectsPanel)
	//*)
}


void TopEffectsPanel::SetDragIconBuffer(const char** buffer)
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

