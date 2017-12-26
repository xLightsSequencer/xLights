#include "ShapePanel.h"
#include "EffectPanelUtils.h"
#include "ShapeEffect.h"

//(*InternalHeaders(ShapePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(ShapePanel)
const long ShapePanel::ID_STATICTEXT_Shape_ObjectToDraw = wxNewId();
const long ShapePanel::ID_CHOICE_Shape_ObjectToDraw = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Thickness = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Thickness = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Thickness = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Thickness = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Count = wxNewId();
const long ShapePanel::IDD_SLIDER_Shape_Count = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Count = wxNewId();
const long ShapePanel::ID_TEXTCTRL_Shape_Count = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_StartSize = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_StartSize = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_StartSize = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_StartSize = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Lifetime = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Lifetime = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Lifetime = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Lifetime = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Growth = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Growth = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Growth = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Growth = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_CentreX = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_CentreX = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_CentreX = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_CentreX = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_CentreY = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_CentreY = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_CentreY = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_CentreY = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Points = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Points = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Points = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_RandomLocation = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_FadeAway = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_UseMusic = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Sensitivity = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Sensitivity = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Sensitivity = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_FireTiming = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_FireTimingTrack = wxNewId();
const long ShapePanel::ID_CHOICE_Shape_FireTimingTrack = wxNewId();
//*)

BEGIN_EVENT_TABLE(ShapePanel,wxPanel)
	//(*EventTable(ShapePanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, ShapePanel::SetTimingTracks)
END_EVENT_TABLE()

ShapePanel::ShapePanel(wxWindow* parent)
{
	//(*Initialize(ShapePanel)
	BulkEditTextCtrl* TextCtrl34;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	BulkEditTextCtrl* TextCtrl_Shape_Growth;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer57;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;
	BulkEditTextCtrl* TextCtrl1;
	BulkEditTextCtrl* TextCtrl_Shape_StartSize;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer57 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer57->AddGrowableCol(1);
	StaticText69 = new wxStaticText(this, ID_STATICTEXT_Shape_ObjectToDraw, _("Object to Draw"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_ObjectToDraw"));
	FlexGridSizer57->Add(StaticText69, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Shape_ObjectToDraw = new BulkEditChoice(this, ID_CHOICE_Shape_ObjectToDraw, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Shape_ObjectToDraw"));
	Choice_Shape_ObjectToDraw->SetSelection( Choice_Shape_ObjectToDraw->Append(_("Circle")) );
	Choice_Shape_ObjectToDraw->Append(_("Triangle"));
	Choice_Shape_ObjectToDraw->Append(_("Square"));
	Choice_Shape_ObjectToDraw->Append(_("Pentagon"));
	Choice_Shape_ObjectToDraw->Append(_("Hexagon"));
	Choice_Shape_ObjectToDraw->Append(_("Octagon"));
	Choice_Shape_ObjectToDraw->Append(_("Star"));
	Choice_Shape_ObjectToDraw->Append(_("Heart"));
	Choice_Shape_ObjectToDraw->Append(_("Tree"));
	Choice_Shape_ObjectToDraw->Append(_("Snowflake"));
	Choice_Shape_ObjectToDraw->Append(_("Candy Cane"));
	Choice_Shape_ObjectToDraw->Append(_("Random"));
	Choice_Shape_ObjectToDraw->Append(_("Crucifix"));
	Choice_Shape_ObjectToDraw->Append(_("Present"));
	FlexGridSizer57->Add(Choice_Shape_ObjectToDraw, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText72 = new wxStaticText(this, ID_STATICTEXT_Shape_Thickness, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Thickness"));
	FlexGridSizer57->Add(StaticText72, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Shape_Thickness = new BulkEditSlider(this, ID_SLIDER_Shape_Thickness, 3, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Thickness"));
	FlexGridSizer1->Add(Slider_Shape_Thickness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Shape_ThicknessVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Thickness, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Thickness"));
	FlexGridSizer1->Add(BitmapButton_Shape_ThicknessVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Shape_Thickness = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Thickness, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Thickness"));
	TextCtrl_Shape_Thickness->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Shape_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText176 = new wxStaticText(this, ID_STATICTEXT_Shape_Count, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Count"));
	FlexGridSizer57->Add(StaticText176, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Shape_Count = new BulkEditSlider(this, IDD_SLIDER_Shape_Count, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Shape_Count"));
	FlexGridSizer2->Add(Slider_Shape_Count, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Shape_CountVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Count, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Count"));
	FlexGridSizer2->Add(BitmapButton_Shape_CountVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	TextCtrl34 = new BulkEditTextCtrl(this, ID_TEXTCTRL_Shape_Count, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Shape_Count"));
	TextCtrl34->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl34, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT_Shape_StartSize, _("Start Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_StartSize"));
	FlexGridSizer57->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Shape_StartSize = new BulkEditSlider(this, ID_SLIDER_Shape_StartSize, 5, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_StartSize"));
	FlexGridSizer7->Add(Slider_Shape_StartSize, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Shape_StartSizeVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_StartSize, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shape_StartSize"));
	FlexGridSizer7->Add(BitmapButton_Shape_StartSizeVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Shape_StartSize = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_StartSize, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_StartSize"));
	TextCtrl_Shape_StartSize->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Shape_StartSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_Shape_Lifetime, _("Lifetime"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Lifetime"));
	FlexGridSizer57->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Shape_Lifetime = new BulkEditSlider(this, ID_SLIDER_Shape_Lifetime, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Lifetime"));
	FlexGridSizer3->Add(Slider_Shape_Lifetime, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Shape_LifetimeVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Lifetime, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Lifetime"));
	FlexGridSizer3->Add(BitmapButton_Shape_LifetimeVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl1 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Lifetime, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Lifetime"));
	TextCtrl1->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT_Shape_Growth, _("Growth"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Growth"));
	FlexGridSizer57->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Shape_Growth = new BulkEditSlider(this, ID_SLIDER_Shape_Growth, 10, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Growth"));
	FlexGridSizer4->Add(Slider_Shape_Growth, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Shape_GrowthVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Growth, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Growth"));
	FlexGridSizer4->Add(BitmapButton_Shape_GrowthVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Shape_Growth = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Growth, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Growth"));
	TextCtrl_Shape_Growth->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Shape_Growth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT_Shape_CentreX, _("X Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_CentreX"));
	FlexGridSizer57->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Shape_CentreX = new BulkEditSlider(this, ID_SLIDER_Shape_CentreX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_CentreX"));
	FlexGridSizer5->Add(Slider_Shape_CentreX, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Shape_CentreXVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_CentreX, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shape_CentreX"));
	FlexGridSizer5->Add(BitmapButton_Shape_CentreXVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Shape_CentreX = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_CentreX, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_CentreX"));
	TextCtrl_Shape_CentreX->SetMaxLength(4);
	FlexGridSizer57->Add(TextCtrl_Shape_CentreX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT_Shape_CentreY, _("Y Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_CentreY"));
	FlexGridSizer57->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_Shape_CentreY = new BulkEditSlider(this, ID_SLIDER_Shape_CentreY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_CentreY"));
	FlexGridSizer6->Add(Slider_Shape_CentreY, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Shape_CentreYVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_CentreY, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shape_CentreY"));
	FlexGridSizer6->Add(BitmapButton_Shape_CentreYVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Shape_CentreY = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_CentreY, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_CentreY"));
	TextCtrl_Shape_CentreY->SetMaxLength(4);
	FlexGridSizer57->Add(TextCtrl_Shape_CentreY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Shape_Points, _("Points"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Points"));
	FlexGridSizer57->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shape_Points = new BulkEditSlider(this, ID_SLIDER_Shape_Points, 5, 4, 7, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Points"));
	FlexGridSizer57->Add(Slider_Shape_Points, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Shape_Points = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Points, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Points"));
	TextCtrl_Shape_Points->SetMaxLength(1);
	FlexGridSizer57->Add(TextCtrl_Shape_Points, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Shape_RandomLocation = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_RandomLocation, _("Random Location"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_RandomLocation"));
	CheckBox_Shape_RandomLocation->SetValue(false);
	FlexGridSizer57->Add(CheckBox_Shape_RandomLocation, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Shape_FadeAway = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_FadeAway, _("Fade Away"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_FadeAway"));
	CheckBox_Shape_FadeAway->SetValue(false);
	FlexGridSizer57->Add(CheckBox_Shape_FadeAway, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Shape_UseMusic = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_UseMusic, _("Fire with music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_UseMusic"));
	CheckBox_Shape_UseMusic->SetValue(false);
	FlexGridSizer57->Add(CheckBox_Shape_UseMusic, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT_Shape_Sensitivity, _("Trigger level"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Sensitivity"));
	FlexGridSizer57->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shape_Sensitivity = new BulkEditSlider(this, ID_SLIDER_Shape_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Sensitivity"));
	FlexGridSizer57->Add(Slider_Shape_Sensitivity, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Shape_Sensitivity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Sensitivity, _("50"), wxDefaultPosition, wxSize(35,-1), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Sensitivity"));
	TextCtrl_Shape_Sensitivity->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Shape_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Shape_FireTiming = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_FireTiming, _("Fire with timing track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_FireTiming"));
	CheckBox_Shape_FireTiming->SetValue(false);
	FlexGridSizer57->Add(CheckBox_Shape_FireTiming, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT_Shape_FireTimingTrack, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_FireTimingTrack"));
	FlexGridSizer57->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Shape_TimingTrack = new BulkEditChoice(this, ID_CHOICE_Shape_FireTimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Shape_FireTimingTrack"));
	FlexGridSizer57->Add(Choice_Shape_TimingTrack, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer57);
	FlexGridSizer57->Fit(this);
	FlexGridSizer57->SetSizeHints(this);

	Connect(ID_CHOICE_Shape_ObjectToDraw,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ShapePanel::OnChoice_Shape_ObjectToDrawSelect);
	Connect(ID_VALUECURVE_Shape_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Shape_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Shape_StartSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Shape_Lifetime,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Shape_Growth,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Shape_CentreX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Shape_CentreY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
	Connect(ID_CHECKBOX_Shape_RandomLocation,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ShapePanel::OnCheckBox_Shape_RandomLocationClick);
	Connect(ID_CHECKBOX_Shape_UseMusic,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ShapePanel::OnCheckBox_Shape_UseMusicClick);
	Connect(ID_CHECKBOX_Shape_FireTiming,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ShapePanel::OnCheckBox_Shape_FireTimingClick);
	Connect(ID_CHOICE_Shape_FireTimingTrack,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ShapePanel::OnChoice_Shape_TimingTrackSelect);
	//*)
    SetName("ID_PANEL_SHAPE");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ShapePanel::OnVCChanged, 0, this);

    BitmapButton_Shape_ThicknessVC->GetValue()->SetLimits(SHAPE_THICKNESS_MIN, SHAPE_THICKNESS_MAX);;
    BitmapButton_Shape_GrowthVC->GetValue()->SetLimits(SHAPE_GROWTH_MIN, SHAPE_GROWTH_MAX);;
    BitmapButton_Shape_CentreYVC->GetValue()->SetLimits(SHAPE_CENTREY_MIN, SHAPE_CENTREY_MAX);;
    BitmapButton_Shape_CentreXVC->GetValue()->SetLimits(SHAPE_CENTREX_MIN, SHAPE_CENTREX_MAX);;
    BitmapButton_Shape_LifetimeVC->GetValue()->SetLimits(SHAPE_LIFETIME_MIN, SHAPE_LIFETIME_MAX);;
    BitmapButton_Shape_CountVC->GetValue()->SetLimits(SHAPE_COUNT_MIN, SHAPE_COUNT_MAX);;
    BitmapButton_Shape_StartSizeVC->GetValue()->SetLimits(SHAPE_STARTSIZE_MIN, SHAPE_STARTSIZE_MAX);;

    ValidateWindow();
}

ShapePanel::~ShapePanel()
{
	//(*Destroy(ShapePanel)
	//*)
}

PANEL_EVENT_HANDLERS(ShapePanel)

void ShapePanel::ValidateWindow()
{
    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "Star")
    {
        Slider_Shape_Points->Enable();
        TextCtrl_Shape_Points->Enable();
    }
    else
    {
        Slider_Shape_Points->Disable();
        TextCtrl_Shape_Points->Disable();
    }

    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "Snowflake")
    {
        Slider_Shape_Thickness->Disable();
        TextCtrl_Shape_Thickness->Disable();
    }
    else
    {
        Slider_Shape_Thickness->Enable();
        TextCtrl_Shape_Thickness->Enable();
    }

    if (CheckBox_Shape_RandomLocation->IsChecked())
    {
        Slider_Shape_CentreX->Disable();
        Slider_Shape_CentreY->Disable();
        TextCtrl_Shape_CentreX->Disable();
        TextCtrl_Shape_CentreY->Disable();
        BitmapButton_Shape_CentreXVC->Disable();
        BitmapButton_Shape_CentreYVC->Disable();
    }
    else
    {
        Slider_Shape_CentreX->Enable();
        Slider_Shape_CentreY->Enable();
        TextCtrl_Shape_CentreX->Enable();
        TextCtrl_Shape_CentreY->Enable();
        BitmapButton_Shape_CentreXVC->Enable();
        BitmapButton_Shape_CentreYVC->Enable();
    }

    if (CheckBox_Shape_UseMusic->IsChecked())
    {
        Slider_Shape_Sensitivity->Enable();
        TextCtrl_Shape_Sensitivity->Enable();
    }
    else
    {
        Slider_Shape_Sensitivity->Disable();
        TextCtrl_Shape_Sensitivity->Disable();
    }

    if (CheckBox_Shape_FireTiming->IsChecked())
    {
        Choice_Shape_TimingTrack->Enable();
    }
    else
    {
        Choice_Shape_TimingTrack->Disable();
    }
}

void ShapePanel::OnChoice_Shape_ObjectToDrawSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnCheckBox_Shape_RandomLocationClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnCheckBox_Shape_UseMusicClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnCheckBox_Shape_FireTimingClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnChoice_Shape_TimingTrackSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::SetTimingTracks(wxCommandEvent& event)
{
    auto timingtracks = wxSplit(event.GetString(), '|');

    wxString selection = Choice_Shape_TimingTrack->GetStringSelection();

    // check if anything has been removed ... if it has clear the list and we will have to rebuild it as you cant delete items from a combo box
    bool removed = false;
    for (size_t i = 0; i < Choice_Shape_TimingTrack->GetCount(); i++)
    {
        bool found = false;
        for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
        {
            if (*it == Choice_Shape_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_Shape_TimingTrack->Clear();
            removed = true;
            break;
        }
    }

    // add any new timing tracks
    for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
    {
        bool found = false;
        for (size_t i = 0; i < Choice_Shape_TimingTrack->GetCount(); i++)
        {
            if (*it == Choice_Shape_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_Shape_TimingTrack->Append(*it);
        }
    }

    if (removed && Choice_Shape_TimingTrack->GetCount() > 0)
    {
        // go through the list and see if our selected item is there
        bool found = false;
        for (size_t i = 0; i < Choice_Shape_TimingTrack->GetCount(); i++)
        {
            if (selection == Choice_Shape_TimingTrack->GetString(i))
            {
                found = true;
                Choice_Shape_TimingTrack->SetSelection(i);
                break;
            }
        }
        if (!found)
        {
            Choice_Shape_TimingTrack->SetSelection(0);
        }
    }
    ValidateWindow();
}
