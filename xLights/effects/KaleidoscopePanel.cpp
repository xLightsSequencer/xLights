/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "KaleidoscopePanel.h"
#include "EffectPanelUtils.h"
#include "KaleidoscopeEffect.h"

//(*InternalHeaders(KaleidoscopePanel)
#include <wx/bmpbuttn.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(KaleidoscopePanel)
const long KaleidoscopePanel::ID_TEXTCTRL1 = wxNewId();
const long KaleidoscopePanel::ID_STATICTEXT8 = wxNewId();
const long KaleidoscopePanel::ID_CHOICE_Kaleidoscope_Type = wxNewId();
const long KaleidoscopePanel::ID_STATICTEXT10 = wxNewId();
const long KaleidoscopePanel::ID_SLIDER_Kaleidoscope_X = wxNewId();
const long KaleidoscopePanel::ID_VALUECURVE_Kaleidoscope_X = wxNewId();
const long KaleidoscopePanel::IDD_TEXTCTRL_Kaleidoscope_X = wxNewId();
const long KaleidoscopePanel::ID_STATICTEXT11 = wxNewId();
const long KaleidoscopePanel::ID_SLIDER_Kaleidoscope_Y = wxNewId();
const long KaleidoscopePanel::ID_VALUECURVE_Kaleidoscope_Y = wxNewId();
const long KaleidoscopePanel::IDD_TEXTCTRL_Kaleidoscope_Y = wxNewId();
const long KaleidoscopePanel::ID_STATICTEXT12 = wxNewId();
const long KaleidoscopePanel::ID_SLIDER_Kaleidoscope_Size = wxNewId();
const long KaleidoscopePanel::ID_VALUECURVE_Kaleidoscope_Size = wxNewId();
const long KaleidoscopePanel::IDD_TEXTCTRL_Kaleidoscope_Size = wxNewId();
const long KaleidoscopePanel::ID_STATICTEXT13 = wxNewId();
const long KaleidoscopePanel::ID_SLIDER_Kaleidoscope_Rotation = wxNewId();
const long KaleidoscopePanel::ID_VALUECURVE_Kaleidoscope_Rotation = wxNewId();
const long KaleidoscopePanel::IDD_TEXTCTRL_Kaleidoscope_Rotation = wxNewId();
//*)

KaleidoscopePreview::KaleidoscopePreview( wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name, bool coreProfile)
   : xlGLCanvas( parent, id, pos, size, style, name, coreProfile )
{

}

KaleidoscopePreview::~KaleidoscopePreview()
{

}

void KaleidoscopePreview::InitializeGLCanvas()
{
   SetCurrentGLContext();

   mIsInitialized = true;
}

BEGIN_EVENT_TABLE(KaleidoscopePanel,wxPanel)
	//(*EventTable(KaleidoscopePanel)
	//*)
END_EVENT_TABLE()

KaleidoscopePanel::KaleidoscopePanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(KaleidoscopePanel)
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, _("The Kaleidoscope effect distorts the pixels in the layers below it. The Canvas option in Layer Blending must be enabled for it to work."), wxDefaultPosition, wxDLG_UNIT(this,wxSize(99,32)), wxTE_NO_VSCROLL|wxTE_MULTILINE|wxTE_READONLY|wxNO_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl1->Disable();
	FlexGridSizer3->Add(TextCtrl1, 1, wxEXPAND, 2);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Kaleidoscope Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer4->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Choice_Kaleidoscope_Type = new wxChoice(this, ID_CHOICE_Kaleidoscope_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Kaleidoscope_Type"));
	FlexGridSizer5->Add(Choice_Kaleidoscope_Type, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Center X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer4->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Kaleidoscope_X = new BulkEditSlider(this, ID_SLIDER_Kaleidoscope_X, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Kaleidoscope_X"));
	FlexGridSizer7->Add(Slider_Kaleidoscope_X, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Kaleidoscope_X = new BulkEditValueCurveButton(this, ID_VALUECURVE_Kaleidoscope_X, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Kaleidoscope_X"));
	FlexGridSizer7->Add(BitmapButton_Kaleidoscope_X, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Kaleidoscope_X = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Kaleidoscope_X, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Kaleidoscope_X"));
	FlexGridSizer7->Add(TextCtrl_Kaleidoscope_X, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Center Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer4->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_Kaleidoscope_Y = new BulkEditSlider(this, ID_SLIDER_Kaleidoscope_Y, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Kaleidoscope_Y"));
	FlexGridSizer8->Add(Slider_Kaleidoscope_Y, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Kaleidoscope_Y = new BulkEditValueCurveButton(this, ID_VALUECURVE_Kaleidoscope_Y, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Kaleidoscope_Y"));
	FlexGridSizer8->Add(BitmapButton_Kaleidoscope_Y, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Kaleidoscope_Y = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Kaleidoscope_Y, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Kaleidoscope_Y"));
	FlexGridSizer8->Add(TextCtrl_Kaleidoscope_Y, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer4->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	Slider_Kaleidoscope_Size = new BulkEditSlider(this, ID_SLIDER_Kaleidoscope_Size, 5, 2, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Kaleidoscope_Size"));
	FlexGridSizer9->Add(Slider_Kaleidoscope_Size, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Kaleidoscope_Size = new BulkEditValueCurveButton(this, ID_VALUECURVE_Kaleidoscope_Size, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Kaleidoscope_Size"));
	FlexGridSizer9->Add(BitmapButton_Kaleidoscope_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Kaleidoscope_Size = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Kaleidoscope_Size, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Kaleidoscope_Size"));
	FlexGridSizer9->Add(TextCtrl_Kaleidoscope_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	FlexGridSizer4->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	Slider_Kaleidoscope_Rotation = new BulkEditSlider(this, ID_SLIDER_Kaleidoscope_Rotation, 0, 0, 359, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Kaleidoscope_Rotation"));
	FlexGridSizer10->Add(Slider_Kaleidoscope_Rotation, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Kaleidoscope_Rotation = new BulkEditValueCurveButton(this, ID_VALUECURVE_Kaleidoscope_Rotation, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Kaleidoscope_Rotation"));
	FlexGridSizer10->Add(BitmapButton_Kaleidoscope_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Kaleidoscope_Rotation = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Kaleidoscope_Rotation, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Kaleidoscope_Rotation"));
	FlexGridSizer10->Add(TextCtrl_Kaleidoscope_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE_Kaleidoscope_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&KaleidoscopePanel::OnChoice_Kaleidoscope_TypeSelect);
	Connect(ID_VALUECURVE_Kaleidoscope_X,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KaleidoscopePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Kaleidoscope_Y,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KaleidoscopePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Kaleidoscope_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KaleidoscopePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Kaleidoscope_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KaleidoscopePanel::OnVCButtonClick);
	//*)

	SetName( "ID_PANEL_Kaleidoscope" );

    Choice_Kaleidoscope_Type->Append(_("Triangle"));
    Choice_Kaleidoscope_Type->Append(_("Square"));
    //Choice_Kaleidoscope_Type->Append(_("Hexagon"));
    Choice_Kaleidoscope_Type->SetSelection(0);

	Connect( wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&KaleidoscopePanel::OnVCChanged, 0, this );

	BitmapButton_Kaleidoscope_X->SetLimits( KALEIDOSCOPE_X_MIN, KALEIDOSCOPE_X_MAX);
	BitmapButton_Kaleidoscope_Y->SetLimits(KALEIDOSCOPE_Y_MIN, KALEIDOSCOPE_Y_MAX);
    BitmapButton_Kaleidoscope_Size->SetLimits(KALEIDOSCOPE_SIZE_MIN, KALEIDOSCOPE_SIZE_MAX);
    BitmapButton_Kaleidoscope_Rotation->SetLimits(KALEIDOSCOPE_ROTATION_MIN, KALEIDOSCOPE_ROTATION_MAX);

	ValidateWindow();
}

KaleidoscopePanel::~KaleidoscopePanel()
{
	//(*Destroy(KaleidoscopePanel)
	//*)
}

void KaleidoscopePanel::OnVCButtonClick(wxCommandEvent& event)
{
   EffectPanelUtils::OnVCButtonClick( event );
}

//PANEL_EVENT_HANDLERS(KaleidoscopePanel)

void KaleidoscopePanel::OnVCChanged(wxCommandEvent& event)
{
   EffectPanelUtils::OnVCChanged( event );
}

void KaleidoscopePanel::OnChoice_Kaleidoscope_TypeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void KaleidoscopePanel::ValidateWindow()
{
}