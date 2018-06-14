#include "WarpPanel.h"
#include "xlGLCanvas.h"

//(*InternalHeaders(WarpPanel)
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)

//(*IdInit(WarpPanel)
const long WarpPanel::ID_TEXTCTRL1 = wxNewId();
const long WarpPanel::ID_STATICTEXT8 = wxNewId();
const long WarpPanel::ID_CHOICE_Warp_Type = wxNewId();
const long WarpPanel::ID_STATICTEXT9 = wxNewId();
const long WarpPanel::ID_CHOICE_Warp_Treatment = wxNewId();
const long WarpPanel::ID_STATICTEXT10 = wxNewId();
const long WarpPanel::ID_SLIDER_Warp_X = wxNewId();
const long WarpPanel::ID_VALUECURVE_Warp_X = wxNewId();
const long WarpPanel::ID_TEXTCTRL_Warp_X = wxNewId();
const long WarpPanel::ID_STATICTEXT11 = wxNewId();
const long WarpPanel::ID_SLIDER_Warp_Y = wxNewId();
const long WarpPanel::ID_VALUECURVE_Warp_Y = wxNewId();
const long WarpPanel::ID_TEXTCTRL_Warp_Y = wxNewId();
const long WarpPanel::ID_STATICTEXT12 = wxNewId();
const long WarpPanel::ID_SLIDER_Warp_Cycle_Count = wxNewId();
const long WarpPanel::ID_TEXTCTRL_Warp_Cycle_Count = wxNewId();
const long WarpPanel::ID_STATICTEXT13 = wxNewId();
const long WarpPanel::ID_SLIDER_Warp_Speed = wxNewId();
const long WarpPanel::ID_TEXTCTRL_Warp_Speed = wxNewId();
const long WarpPanel::ID_STATICTEXT14 = wxNewId();
const long WarpPanel::ID_SLIDER_Warp_Frequency = wxNewId();
const long WarpPanel::ID_TEXTCTRL_Warp_Frequency = wxNewId();
//*)

WarpPreview::WarpPreview( wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name, bool coreProfile)
   : xlGLCanvas( parent, id, pos, size, style, name, coreProfile )
{

}

WarpPreview::~WarpPreview()
{

}

void WarpPreview::InitializeGLCanvas()
{
   SetCurrentGLContext();

   mIsInitialized = true;
}

BEGIN_EVENT_TABLE(WarpPanel,wxPanel)
	//(*EventTable(WarpPanel)
	//*)
END_EVENT_TABLE()

WarpPanel::WarpPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(WarpPanel)
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, _("The warp effect distorts the pixels in the layers below it. The Canvas option in Layer Blending must be enabled for it to work."), wxDefaultPosition, wxDLG_UNIT(this,wxSize(99,32)), wxTE_NO_VSCROLL|wxTE_MULTILINE|wxTE_READONLY|wxNO_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl1->Disable();
	TextCtrl1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
	FlexGridSizer3->Add(TextCtrl1, 1, wxEXPAND, 2);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Warp Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer4->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Choice_Warp_Type = new wxChoice(this, ID_CHOICE_Warp_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Warp_Type"));
	Choice_Warp_Type->Append(_("water drops"));
	Choice_Warp_Type->Append(_("dissolve"));
	Choice_Warp_Type->Append(_("circle reveal"));
	Choice_Warp_Type->Append(_("banded swirl"));
	Choice_Warp_Type->Append(_("ripple"));
	FlexGridSizer5->Add(Choice_Warp_Type, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Treatment"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer4->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Choice_Warp_Treatment = new wxChoice(this, ID_CHOICE_Warp_Treatment, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Warp_Treatment"));
	Choice_Warp_Treatment->Append(_("constant"));
	Choice_Warp_Treatment->Append(_("in"));
	Choice_Warp_Treatment->Append(_("out"));
	FlexGridSizer6->Add(Choice_Warp_Treatment, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer4->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Warp_X = new BulkEditSlider(this, ID_SLIDER_Warp_X, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Warp_X"));
	FlexGridSizer7->Add(Slider_Warp_X, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Warp_X = new BulkEditValueCurveButton(this, ID_VALUECURVE_Warp_X, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Warp_X"));
	FlexGridSizer7->Add(BitmapButton_Warp_X, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Warp_X = new BulkEditTextCtrl(this, ID_TEXTCTRL_Warp_X, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Warp_X"));
	FlexGridSizer7->Add(TextCtrl_Warp_X, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer4->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_Warp_Y = new BulkEditSlider(this, ID_SLIDER_Warp_Y, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Warp_Y"));
	FlexGridSizer8->Add(Slider_Warp_Y, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Warp_Y = new BulkEditValueCurveButton(this, ID_VALUECURVE_Warp_Y, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Warp_Y"));
	FlexGridSizer8->Add(BitmapButton_Warp_Y, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Warp_Y = new BulkEditTextCtrl(this, ID_TEXTCTRL_Warp_Y, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Warp_Y"));
	FlexGridSizer8->Add(TextCtrl_Warp_Y, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Cycle Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer4->Add(StaticText12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	Slider_Warp_Cycle_Count = new BulkEditSlider(this, ID_SLIDER_Warp_Cycle_Count, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Warp_Cycle_Count"));
	FlexGridSizer9->Add(Slider_Warp_Cycle_Count, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Warp_Cycle_Count = new BulkEditTextCtrl(this, ID_TEXTCTRL_Warp_Cycle_Count, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Warp_Cycle_Count"));
	FlexGridSizer9->Add(TextCtrl_Warp_Cycle_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	FlexGridSizer4->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	Slider_Warp_Speed = new BulkEditSlider(this, ID_SLIDER_Warp_Speed, 20, 0, 40, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Warp_Speed"));
	FlexGridSizer10->Add(Slider_Warp_Speed, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Warp_Speed = new BulkEditTextCtrl(this, ID_TEXTCTRL_Warp_Speed, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Warp_Speed"));
	FlexGridSizer10->Add(TextCtrl_Warp_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("Frequency"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	FlexGridSizer4->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	Slider_Warp_Frequency = new BulkEditSlider(this, ID_SLIDER_Warp_Frequency, 20, 0, 40, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Warp_Frequency"));
	FlexGridSizer11->Add(Slider_Warp_Frequency, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Warp_Frequency = new BulkEditTextCtrl(this, ID_TEXTCTRL_Warp_Frequency, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Warp_Frequency"));
	FlexGridSizer11->Add(TextCtrl_Warp_Frequency, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

	BitmapButton_Warp_X->SetLimits( 0, 100 );
	BitmapButton_Warp_Y->SetLimits( 0, 100 );
}

WarpPanel::~WarpPanel()
{
	//(*Destroy(WarpPanel)
	//*)
}
void WarpPanel::OnChoice_Warp_TypeSelect(wxCommandEvent& event)
{
}

void WarpPanel::OnVCButtonClick(wxCommandEvent& event)
{
   int x = 1;
}
