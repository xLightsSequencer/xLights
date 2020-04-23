/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ButterflyPanel.h"
#include "EffectPanelUtils.h"
#include "ButterflyEffect.h"

//(*InternalHeaders(ButterflyPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(ButterflyPanel)
const long ButterflyPanel::ID_STATICTEXT_Butterfly_Colors = wxNewId();
const long ButterflyPanel::ID_CHOICE_Butterfly_Colors = wxNewId();
const long ButterflyPanel::ID_BITMAPBUTTON_CHOICE_Butterfly_Colors = wxNewId();
const long ButterflyPanel::ID_STATICTEXT_Butterfly_Style = wxNewId();
const long ButterflyPanel::ID_SLIDER_Butterfly_Style = wxNewId();
const long ButterflyPanel::IDD_TEXTCTRL_Butterfly_Style = wxNewId();
const long ButterflyPanel::ID_BITMAPBUTTON_SLIDER_Butterfly_Style = wxNewId();
const long ButterflyPanel::ID_STATICTEXT_Butterfly_Chunks = wxNewId();
const long ButterflyPanel::ID_SLIDER_Butterfly_Chunks = wxNewId();
const long ButterflyPanel::ID_VALUECURVE_Butterfly_Chunks = wxNewId();
const long ButterflyPanel::IDD_TEXTCTRL_Butterfly_Chunks = wxNewId();
const long ButterflyPanel::ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks = wxNewId();
const long ButterflyPanel::ID_STATICTEXT_Butterfly_Skip = wxNewId();
const long ButterflyPanel::ID_SLIDER_Butterfly_Skip = wxNewId();
const long ButterflyPanel::ID_VALUECURVE_Butterfly_Skip = wxNewId();
const long ButterflyPanel::IDD_TEXTCTRL_Butterfly_Skip = wxNewId();
const long ButterflyPanel::ID_BITMAPBUTTON_SLIDER_Butterfly_Skip = wxNewId();
const long ButterflyPanel::ID_STATICTEXT_Butterfly_Speed = wxNewId();
const long ButterflyPanel::ID_SLIDER_Butterfly_Speed = wxNewId();
const long ButterflyPanel::ID_VALUECURVE_Butterfly_Speed = wxNewId();
const long ButterflyPanel::IDD_TEXTCTRL_Butterfly_Speed = wxNewId();
const long ButterflyPanel::ID_BITMAPBUTTON_SLIDER_Butterfly_Speed = wxNewId();
const long ButterflyPanel::ID_STATICTEXT_Butterfly_Direction = wxNewId();
const long ButterflyPanel::ID_CHOICE_Butterfly_Direction = wxNewId();
const long ButterflyPanel::ID_BITMAPBUTTON_CHOICE_Butterfly_Direction = wxNewId();
//*)

BEGIN_EVENT_TABLE(ButterflyPanel,wxPanel)
	//(*EventTable(ButterflyPanel)
	//*)
END_EVENT_TABLE()

ButterflyPanel::ButterflyPanel(wxWindow* parent)
{
	//(*Initialize(ButterflyPanel)
	BulkEditTextCtrl* TextCtrl16;
	BulkEditTextCtrl* TextCtrl17;
	BulkEditTextCtrl* TextCtrl18;
	BulkEditTextCtrl* TextCtrl25;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer36;
	wxFlexGridSizer* FlexGridSizer3;
	xlLockButton* BitmapButton1;
	xlLockButton* BitmapButton_ButterflyChunks;
	xlLockButton* BitmapButton_ButterflyColors;
	xlLockButton* BitmapButton_ButterflyDirection;
	xlLockButton* BitmapButton_ButterflySkip;
	xlLockButton* BitmapButton_ButterflyStyle;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer36 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer36->AddGrowableCol(1);
	StaticText25 = new wxStaticText(this, ID_STATICTEXT_Butterfly_Colors, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Butterfly_Colors"));
	FlexGridSizer36->Add(StaticText25, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Butterfly_Colors = new BulkEditChoice(this, ID_CHOICE_Butterfly_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Butterfly_Colors"));
	Choice_Butterfly_Colors->SetSelection( Choice_Butterfly_Colors->Append(_("Rainbow")) );
	Choice_Butterfly_Colors->Append(_("Palette"));
	FlexGridSizer36->Add(Choice_Butterfly_Colors, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer36->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ButterflyColors = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Butterfly_Colors, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Butterfly_Colors"));
	BitmapButton_ButterflyColors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer36->Add(BitmapButton_ButterflyColors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText26 = new wxStaticText(this, ID_STATICTEXT_Butterfly_Style, _("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Butterfly_Style"));
	FlexGridSizer36->Add(StaticText26, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Butterfly_Style = new BulkEditSlider(this, ID_SLIDER_Butterfly_Style, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Style"));
	FlexGridSizer36->Add(Slider_Butterfly_Style, 1, wxALL|wxEXPAND, 2);
	TextCtrl16 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Butterfly_Style, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Butterfly_Style"));
	TextCtrl16->SetMaxLength(1);
	FlexGridSizer36->Add(TextCtrl16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ButterflyStyle = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Butterfly_Style, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Butterfly_Style"));
	BitmapButton_ButterflyStyle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer36->Add(BitmapButton_ButterflyStyle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText27 = new wxStaticText(this, ID_STATICTEXT_Butterfly_Chunks, _("Bkgrd Chunks"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Butterfly_Chunks"));
	FlexGridSizer36->Add(StaticText27, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Butterfly_Chunks = new BulkEditSlider(this, ID_SLIDER_Butterfly_Chunks, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Chunks"));
	FlexGridSizer1->Add(Slider_Butterfly_Chunks, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Butterfly_Chunks = new BulkEditValueCurveButton(this, ID_VALUECURVE_Butterfly_Chunks, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Butterfly_Chunks"));
	FlexGridSizer1->Add(BitmapButton_Butterfly_Chunks, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer36->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl17 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Butterfly_Chunks, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Butterfly_Chunks"));
	TextCtrl17->SetMaxLength(2);
	FlexGridSizer36->Add(TextCtrl17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ButterflyChunks = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks"));
	BitmapButton_ButterflyChunks->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer36->Add(BitmapButton_ButterflyChunks, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText28 = new wxStaticText(this, ID_STATICTEXT_Butterfly_Skip, _("Bkgrd Skip"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Butterfly_Skip"));
	FlexGridSizer36->Add(StaticText28, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Butterfly_Skip = new BulkEditSlider(this, ID_SLIDER_Butterfly_Skip, 2, 2, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Skip"));
	FlexGridSizer2->Add(Slider_Butterfly_Skip, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Butterfly_Skip = new BulkEditValueCurveButton(this, ID_VALUECURVE_Butterfly_Skip, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Butterfly_Skip"));
	FlexGridSizer2->Add(BitmapButton_Butterfly_Skip, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer36->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl18 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Butterfly_Skip, _("2"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Butterfly_Skip"));
	TextCtrl18->SetMaxLength(2);
	FlexGridSizer36->Add(TextCtrl18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton1 = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Butterfly_Skip, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Butterfly_Skip"));
	BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer36->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText175 = new wxStaticText(this, ID_STATICTEXT_Butterfly_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Butterfly_Speed"));
	FlexGridSizer36->Add(StaticText175, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Butterfly_Speed = new BulkEditSlider(this, ID_SLIDER_Butterfly_Speed, 10, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Speed"));
	FlexGridSizer3->Add(Slider_Butterfly_Speed, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Butterfly_Speed = new BulkEditValueCurveButton(this, ID_VALUECURVE_Butterfly_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Butterfly_Speed"));
	FlexGridSizer3->Add(BitmapButton_Butterfly_Speed, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer36->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl25 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Butterfly_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Butterfly_Speed"));
	TextCtrl25->SetMaxLength(3);
	FlexGridSizer36->Add(TextCtrl25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ButterflySkip = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Butterfly_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Butterfly_Speed"));
	BitmapButton_ButterflySkip->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer36->Add(BitmapButton_ButterflySkip, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText58 = new wxStaticText(this, ID_STATICTEXT_Butterfly_Direction, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Butterfly_Direction"));
	FlexGridSizer36->Add(StaticText58, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Butterfly_Direction = new BulkEditChoice(this, ID_CHOICE_Butterfly_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Butterfly_Direction"));
	Choice_Butterfly_Direction->SetSelection( Choice_Butterfly_Direction->Append(_("Normal")) );
	Choice_Butterfly_Direction->Append(_("Reverse"));
	FlexGridSizer36->Add(Choice_Butterfly_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer36->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ButterflyDirection = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Butterfly_Direction, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Butterfly_Direction"));
	BitmapButton_ButterflyDirection->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer36->Add(BitmapButton_ButterflyDirection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer36);
	FlexGridSizer36->Fit(this);
	FlexGridSizer36->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_Butterfly_Colors,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Butterfly_Style,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Butterfly_Chunks,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Butterfly_Skip,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Butterfly_Skip,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Butterfly_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Butterfly_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButterflyPanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ButterflyPanel::OnVCChanged, 0, this);

    BitmapButton_Butterfly_Speed->GetValue()->SetLimits(BUTTERFLY_SKIP_MIN, BUTTERFLY_SPEED_MAX);
    BitmapButton_Butterfly_Skip->GetValue()->SetLimits(BUTTERFLY_SKIP_MIN, BUTTERFLY_SKIP_MAX);
    BitmapButton_Butterfly_Chunks->GetValue()->SetLimits(BUTTERFLY_CHUNKS_MIN, BUTTERFLY_CHUNKS_MAX);

    SetName("ID_PANEL_BUTTERFLY");
}

ButterflyPanel::~ButterflyPanel()
{
	//(*Destroy(ButterflyPanel)
	//*)
}

PANEL_EVENT_HANDLERS(ButterflyPanel)
