#include "BarsPanel.h"
#include "EffectPanelUtils.h"
#include "BarsEffect.h"

//(*InternalHeaders(BarsPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
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

//(*IdInit(BarsPanel)
const long BarsPanel::ID_STATICTEXT_Bars_BarCount = wxNewId();
const long BarsPanel::ID_SLIDER_Bars_BarCount = wxNewId();
const long BarsPanel::ID_VALUECURVE_Bars_BarCount = wxNewId();
const long BarsPanel::IDD_TEXTCTRL_Bars_BarCount = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_SLIDER_Bars_BarCount = wxNewId();
const long BarsPanel::ID_STATICTEXT_Bars_Cycles = wxNewId();
const long BarsPanel::IDD_SLIDER_Bars_Cycles = wxNewId();
const long BarsPanel::ID_VALUECURVE_Bars_Cycles = wxNewId();
const long BarsPanel::ID_TEXTCTRL_Bars_Cycles = wxNewId();
const long BarsPanel::ID_STATICTEXT_Bars_Direction = wxNewId();
const long BarsPanel::ID_CHOICE_Bars_Direction = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHOICE_Bars_Direction = wxNewId();
const long BarsPanel::ID_STATICTEXT_Bars_Center = wxNewId();
const long BarsPanel::IDD_SLIDER_Bars_Center = wxNewId();
const long BarsPanel::ID_VALUECURVE_Bars_Center = wxNewId();
const long BarsPanel::ID_TEXTCTRL_Bars_Center = wxNewId();
const long BarsPanel::ID_CHECKBOX_Bars_Highlight = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight = wxNewId();
const long BarsPanel::ID_CHECKBOX_Bars_3D = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_3D = wxNewId();
const long BarsPanel::ID_CHECKBOX_Bars_Gradient = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient = wxNewId();
//*)

BEGIN_EVENT_TABLE(BarsPanel,wxPanel)
	//(*EventTable(BarsPanel)
	//*)
END_EVENT_TABLE()

BarsPanel::BarsPanel(wxWindow* parent)
{
	//(*Initialize(BarsPanel)
	BulkEditTextCtrl* TextCtrl15;
	BulkEditTextCtrlF1* TextCtrl35;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer123;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer35;
	wxFlexGridSizer* FlexGridSizer70;
	xlLockButton* BitmapButton_3D;
	xlLockButton* BitmapButton_Direction;
	xlLockButton* BitmapButton_Gradient;
	xlLockButton* BitmapButton_Highlight;
	xlLockButton* BitmapButton_PaletteRep;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer35 = new wxFlexGridSizer(0, 3, wxDLG_UNIT(this,wxSize(0,0)).GetWidth(), 0);
	FlexGridSizer35->AddGrowableCol(1);
	StaticText23 = new wxStaticText(this, ID_STATICTEXT_Bars_BarCount, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Bars_BarCount"));
	FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer123 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer123->AddGrowableCol(0);
	Slider_Bars_BarCount = new BulkEditSlider(this, ID_SLIDER_Bars_BarCount, 1, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Bars_BarCount"));
	FlexGridSizer123->Add(Slider_Bars_BarCount, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Bars_BarCount = new BulkEditValueCurveButton(this, ID_VALUECURVE_Bars_BarCount, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Bars_BarCount"));
	FlexGridSizer123->Add(BitmapButton_Bars_BarCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl15 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Bars_BarCount, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Bars_BarCount"));
	TextCtrl15->SetMaxLength(2);
	FlexGridSizer123->Add(TextCtrl15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer123, 1, wxALL|wxEXPAND, 0);
	BitmapButton_PaletteRep = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Bars_BarCount, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Bars_BarCount"));
	BitmapButton_PaletteRep->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer35->Add(BitmapButton_PaletteRep, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText177 = new wxStaticText(this, ID_STATICTEXT_Bars_Cycles, _("Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Bars_Cycles"));
	FlexGridSizer35->Add(StaticText177, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer70 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer70->AddGrowableCol(0);
	Slider_Bars_Cycles = new BulkEditSliderF1(this, IDD_SLIDER_Bars_Cycles, 10, 0, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Bars_Cycles"));
	FlexGridSizer70->Add(Slider_Bars_Cycles, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Bars_Cycles = new BulkEditValueCurveButton(this, ID_VALUECURVE_Bars_Cycles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Bars_Cycles"));
	FlexGridSizer70->Add(BitmapButton_Bars_Cycles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl35 = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Bars_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Bars_Cycles"));
	TextCtrl35->SetMaxLength(4);
	FlexGridSizer70->Add(TextCtrl35, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer70, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText24 = new wxStaticText(this, ID_STATICTEXT_Bars_Direction, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Bars_Direction"));
	FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Bars_Direction = new BulkEditChoice(this, ID_CHOICE_Bars_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Bars_Direction"));
	Choice_Bars_Direction->SetSelection( Choice_Bars_Direction->Append(_("up")) );
	Choice_Bars_Direction->Append(_("down"));
	Choice_Bars_Direction->Append(_("expand"));
	Choice_Bars_Direction->Append(_("compress"));
	Choice_Bars_Direction->Append(_("Left"));
	Choice_Bars_Direction->Append(_("Right"));
	Choice_Bars_Direction->Append(_("H-expand"));
	Choice_Bars_Direction->Append(_("H-compress"));
	Choice_Bars_Direction->Append(_("Alternate Up"));
	Choice_Bars_Direction->Append(_("Alternate Down"));
	Choice_Bars_Direction->Append(_("Alternate Left"));
	Choice_Bars_Direction->Append(_("Alternate Right"));
	Choice_Bars_Direction->Append(_("Custom Horz"));
	Choice_Bars_Direction->Append(_("Custom Vert"));
	FlexGridSizer35->Add(Choice_Bars_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Direction = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Bars_Direction, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Bars_Direction"));
	BitmapButton_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer35->Add(BitmapButton_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Bars_Center, _("Center Point"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Bars_Center"));
	FlexGridSizer35->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Bars_Center = new BulkEditSlider(this, IDD_SLIDER_Bars_Center, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Bars_Center"));
	FlexGridSizer1->Add(Slider_Bars_Center, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Bars_Center = new BulkEditValueCurveButton(this, ID_VALUECURVE_Bars_Center, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Bars_Center"));
	FlexGridSizer1->Add(BitmapButton_Bars_Center, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Bars_Center = new BulkEditTextCtrl(this, ID_TEXTCTRL_Bars_Center, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Bars_Center"));
	FlexGridSizer1->Add(TextCtrl_Bars_Center, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
	CheckBox_Bars_Highlight = new BulkEditCheckBox(this, ID_CHECKBOX_Bars_Highlight, _("Highlight"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_Highlight"));
	CheckBox_Bars_Highlight->SetValue(false);
	FlexGridSizer10->Add(CheckBox_Bars_Highlight, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Highlight = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight"));
	BitmapButton_Highlight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer10->Add(BitmapButton_Highlight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Bars_3D = new BulkEditCheckBox(this, ID_CHECKBOX_Bars_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_3D"));
	CheckBox_Bars_3D->SetValue(false);
	FlexGridSizer10->Add(CheckBox_Bars_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_3D = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Bars_3D, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_3D"));
	BitmapButton_3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer10->Add(BitmapButton_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Bars_Gradient = new BulkEditCheckBox(this, ID_CHECKBOX_Bars_Gradient, _("Gradient"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_Gradient"));
	CheckBox_Bars_Gradient->SetValue(false);
	FlexGridSizer10->Add(CheckBox_Bars_Gradient, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Gradient = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient"));
	BitmapButton_Gradient->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer10->Add(BitmapButton_Gradient, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer35->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer35);
	FlexGridSizer35->Fit(this);
	FlexGridSizer35->SetSizeHints(this);

	Connect(ID_VALUECURVE_Bars_BarCount,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Bars_BarCount,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Bars_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnVCButtonClick);
	Connect(ID_CHOICE_Bars_Direction,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BarsPanel::OnChoice_Bars_DirectionSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_Bars_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Bars_Center,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	//*)

    SetName("ID_PANEL_BARS");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&BarsPanel::OnVCChanged, 0, this);

    Slider_Bars_BarCount->SetRange(BARCOUNT_MIN, BARCOUNT_MAX);
    Slider_Bars_Cycles->SetRange(BARCYCLES_MIN, BARCYCLES_MAX);
    Slider_Bars_Center->SetRange(BARCENTER_MIN, BARCENTER_MAX);

    BitmapButton_Bars_BarCount->GetValue()->SetLimits(BARCOUNT_MIN, BARCOUNT_MAX);
    BitmapButton_Bars_Cycles->GetValue()->SetLimits(BARCYCLES_MIN, BARCYCLES_MAX);
    BitmapButton_Bars_Cycles->GetValue()->SetDivisor(10);
    BitmapButton_Bars_Center->GetValue()->SetLimits(BARCENTER_MIN, BARCENTER_MAX);

    ValidateWindow();
}

BarsPanel::~BarsPanel()
{
	//(*Destroy(BarsPanel)
	//*)
}

PANEL_EVENT_HANDLERS(BarsPanel)

void BarsPanel::OnChoice_Bars_DirectionSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void BarsPanel::ValidateWindow()
{
    wxString type = Choice_Bars_Direction->GetStringSelection();
    if (type == "expand" || type == "compress" || type == "H-expand" || type == "H-compress" || type == "Custom Horz" || type == "Custom Vert")
    {
        Slider_Bars_Center->Enable();
        TextCtrl_Bars_Center->Enable();
        BitmapButton_Bars_Center->Enable();
    }
    else
    {
        Slider_Bars_Center->Disable();
        TextCtrl_Bars_Center->Disable();
        BitmapButton_Bars_Center->Disable();
    }
}
