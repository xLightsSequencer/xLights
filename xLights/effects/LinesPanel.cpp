//(*InternalHeaders(LinesPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include "LinesPanel.h"
#include "EffectPanelUtils.h"
#include "LinesEffect.h"
#include "UtilFunctions.h"

//(*IdInit(LinesPanel)
const long LinesPanel::ID_STATICTEXT_Lines_Bars = wxNewId();
const long LinesPanel::ID_SLIDER_Lines_Objects = wxNewId();
const long LinesPanel::IDD_TEXTCTRL_Lines_Objects = wxNewId();
const long LinesPanel::ID_BITMAPBUTTON_SLIDER_Lines_Objects = wxNewId();
const long LinesPanel::ID_STATICTEXT_Lines_Segments = wxNewId();
const long LinesPanel::ID_SLIDER_Lines_Segments = wxNewId();
const long LinesPanel::IDD_TEXTCTRL_Lines_Segments = wxNewId();
const long LinesPanel::ID_BITMAPBUTTON_SLIDER_Lines_Segments = wxNewId();
const long LinesPanel::ID_STATICTEXT2 = wxNewId();
const long LinesPanel::ID_SLIDER_Lines_Thickness = wxNewId();
const long LinesPanel::ID_VALUECURVE_Lines_Thickness = wxNewId();
const long LinesPanel::ID_TEXTCTRL_Lines_Thickness = wxNewId();
const long LinesPanel::ID_STATICTEXT1 = wxNewId();
const long LinesPanel::ID_SLIDER_Lines_Speed = wxNewId();
const long LinesPanel::ID_VALUECURVE_Lines_Speed = wxNewId();
const long LinesPanel::ID_TEXTCTRL_Lines_Speed = wxNewId();
const long LinesPanel::ID_STATICTEXT_Lines_Trails = wxNewId();
const long LinesPanel::ID_SLIDER_Lines_Trails = wxNewId();
const long LinesPanel::IDD_TEXTCTRL_Lines_Trails = wxNewId();
const long LinesPanel::ID_BITMAPBUTTON_SLIDER_Lines_Trails = wxNewId();
const long LinesPanel::ID_CHECKBOX_Lines_FadeTrails = wxNewId();
//*)

BEGIN_EVENT_TABLE(LinesPanel,wxPanel)
	//(*EventTable(LinesPanel)
	//*)
END_EVENT_TABLE()

LinesPanel::LinesPanel(wxWindow* parent)
{
	//(*Initialize(LinesPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer31;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer31 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Lines_Bars, _("Lines"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lines_Bars"));
	FlexGridSizer31->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Lines_Objects = new BulkEditSlider(this, ID_SLIDER_Lines_Objects, 2, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lines_Objects"));
	FlexGridSizer4->Add(Slider_Lines_Objects, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Lines_Objects = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Lines_Objects, _("2"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Lines_Objects"));
	FlexGridSizer31->Add(TextCtrl_Lines_Objects, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Lines_Objects = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Lines_Objects, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Lines_Objects"));
	BitmapButton_Lines_Objects->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Lines_Objects, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT_Lines_Segments, _("Points"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lines_Segments"));
	FlexGridSizer31->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(2, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Lines_Segments = new BulkEditSlider(this, ID_SLIDER_Lines_Segments, 3, 2, 6, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lines_Segments"));
	FlexGridSizer3->Add(Slider_Lines_Segments, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Lines_Segments = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Lines_Segments, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Lines_Segments"));
	FlexGridSizer31->Add(TextCtrl_Lines_Segments, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Lines_Segments = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Lines_Segments, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Lines_Segments"));
	BitmapButton_Lines_Segments->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Lines_Segments, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Lines_Thickness = new BulkEditSlider(this, ID_SLIDER_Lines_Thickness, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lines_Thickness"));
	FlexGridSizer1->Add(Slider_Lines_Thickness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Lines_Thickness = new BulkEditValueCurveButton(this, ID_VALUECURVE_Lines_Thickness, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Lines_Thickness"));
	FlexGridSizer1->Add(BitmapButton_Lines_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Lines_Thickness = new BulkEditTextCtrl(this, ID_TEXTCTRL_Lines_Thickness, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Lines_Thickness"));
	FlexGridSizer31->Add(TextCtrl_Lines_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText15 = new wxStaticText(this, ID_STATICTEXT1, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer31->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Lines_Speed = new BulkEditSlider(this, ID_SLIDER_Lines_Speed, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lines_Speed"));
	FlexGridSizer5->Add(Slider_Lines_Speed, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Lines_Speed = new BulkEditValueCurveButton(this, ID_VALUECURVE_Lines_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Lines_Speed"));
	FlexGridSizer5->Add(BitmapButton_Lines_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Lines_Speed = new BulkEditTextCtrl(this, ID_TEXTCTRL_Lines_Speed, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Lines_Speed"));
	FlexGridSizer31->Add(TextCtrl_Lines_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT_Lines_Trails, _("Trails"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lines_Trails"));
	FlexGridSizer31->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Lines_Trails = new BulkEditSlider(this, ID_SLIDER_Lines_Trails, 0, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lines_Trails"));
	FlexGridSizer2->Add(Slider_Lines_Trails, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Lines_Trails = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Lines_Trails, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Lines_Trails"));
	FlexGridSizer31->Add(TextCtrl_Lines_Trails, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Lines_Trails = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Lines_Trails, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Lines_Trails"));
	BitmapButton_Lines_Trails->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Lines_Trails, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FadeTrails = new BulkEditCheckBox(this, ID_CHECKBOX_Lines_FadeTrails, _("Fade Trails"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Lines_FadeTrails"));
	CheckBox_FadeTrails->SetValue(true);
	FlexGridSizer31->Add(CheckBox_FadeTrails, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Lines_Objects,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Lines_Segments,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinesPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Lines_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinesPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Lines_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinesPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Lines_Trails,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinesPanel::OnLockButtonClick);
	//*)
    
    SetName("ID_PANEL_Lines");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&LinesPanel::OnVCChanged, 0, this);

    BitmapButton_Lines_Speed->GetValue()->SetLimits(LINES_SPEED_MIN, LINES_SPEED_MAX);
    BitmapButton_Lines_Thickness->GetValue()->SetLimits(LINES_THICKNESS_MIN, LINES_THICKNESS_MAX);

	ValidateWindow();
}

LinesPanel::~LinesPanel()
{
	//(*Destroy(LinesPanel)
	//*)
}

PANEL_EVENT_HANDLERS(LinesPanel)

void LinesPanel::ValidateWindow()
{
}