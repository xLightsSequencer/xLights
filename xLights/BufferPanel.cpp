#include "BufferPanel.h"

//(*InternalHeaders(BufferPanel)
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <wx/valnum.h>
#include <vector>
#include "models/Model.h"
#include "effects/EffectPanelUtils.h"
#include "../include/padlock16x16-blue.xpm" //-DJ
#include "ValueCurveDialog.h"
#include "SubBufferPanel.h"
//#include "BufferTransformProperties.h"

//(*IdInit(BufferPanel)
const long BufferPanel::ID_CHOICE_BufferStyle = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_CHOICE_BufferStyle = wxNewId();
const long BufferPanel::ID_CHOICE_BufferTransform = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_CHOICE_BufferTransform = wxNewId();
const long BufferPanel::ID_STATICTEXT2 = wxNewId();
const long BufferPanel::ID_SLIDER_EffectBlur = wxNewId();
const long BufferPanel::ID_VALUECURVE_Blur = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_EffectBlur = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_SLIDER_EffectBlur = wxNewId();
const long BufferPanel::ID_CHECKBOX_OverlayBkg = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_OverlayBkg = wxNewId();
const long BufferPanel::ID_SCROLLEDWINDOW1 = wxNewId();
const long BufferPanel::ID_PANEL1 = wxNewId();
const long BufferPanel::ID_CHOICE_Preset = wxNewId();
const long BufferPanel::ID_SLIDER_Rotation = wxNewId();
const long BufferPanel::ID_VALUECURVE_Rotation = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Rotation = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Rotation = wxNewId();
const long BufferPanel::ID_SLIDER_Rotations = wxNewId();
const long BufferPanel::ID_VALUECURVE_Rotations = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Rotations = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Rotations = wxNewId();
const long BufferPanel::ID_SLIDER_PivotPointX = wxNewId();
const long BufferPanel::ID_VALUECURVE_PivotPointX = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_PivotPointX = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_PivotPointX = wxNewId();
const long BufferPanel::ID_SLIDER_PivotPointY = wxNewId();
const long BufferPanel::ID_VALUECURVE_PivotPointY = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_PivotPointY = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_PivotPointY = wxNewId();
const long BufferPanel::ID_SLIDER_Zoom = wxNewId();
const long BufferPanel::ID_VALUECURVE_Zoom = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Zoom = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Zoom = wxNewId();
const long BufferPanel::ID_SLIDER_ZoomQuality = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_ZoomQuality = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_ZoomQuality = wxNewId();
const long BufferPanel::ID_SCROLLEDWINDOW2 = wxNewId();
const long BufferPanel::ID_PANEL4 = wxNewId();
const long BufferPanel::ID_NOTEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BufferPanel,wxPanel)
	//(*EventTable(BufferPanel)
	//*)
END_EVENT_TABLE()

BufferPanel::BufferPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    __rotation = 0;
    __rotations = 0;
    __zoom = 1;
    __zoomquality = 1;
    __blur = 1;
    wxIntegerValidator<int> _rotation(&__rotation, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _rotation.SetMin(0);
    _rotation.SetMax(100);
    wxFloatingPointValidator<float> _rotations(&__rotations, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _rotations.SetPrecision(1);
    _rotations.SetMin(0);
    _rotations.SetMax(10);
    wxFloatingPointValidator<float> _zoom(&__zoom);
    _zoom.SetPrecision(1);
    _zoom.SetMin(0);
    _zoom.SetMax(3);
    wxIntegerValidator<int> _zoomquality(&__zoomquality, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _zoomquality.SetMin(1);
    _zoomquality.SetMax(10);
    wxIntegerValidator<int> _pivotpointx(&__pivotpointx, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _pivotpointx.SetMin(0);
    _pivotpointx.SetMax(100);
    wxIntegerValidator<int> _pivotpointy(&__pivotpointy, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _pivotpointy.SetMin(0);
    _pivotpointy.SetMax(100);
    wxIntegerValidator<int> _blur(&__blur, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _blur.SetMin(1);
    _blur.SetMax(15);

	//(*Initialize(BufferPanel)
	wxStaticText* StaticText9;
	wxStaticText* StaticText2;
	wxPanel* Panel4;
	wxFlexGridSizer* FlexGridSizer10;
	wxStaticText* StaticText6;
	wxFlexGridSizer* FlexGridSizer5;
	wxStaticText* StaticText8;
	wxStaticText* StaticText11;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxFlexGridSizer* FlexGridSizer7;
	wxBitmapButton* BitmapButtonBufferStyle;
	wxPanel* Panel3;
	wxFlexGridSizer* FlexGridSizer15;
	wxStaticText* StaticText7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer11;
	wxStaticText* StaticText4;
	wxBitmapButton* BitmapButton_BufferTransform;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Panel3 = new wxPanel(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	FlexGridSizer11->AddGrowableRow(0);
	ScrolledWindow1 = new wxScrolledWindow(Panel3, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FullBufferSizer = new wxFlexGridSizer(2, 1, 0, 0);
	FullBufferSizer->AddGrowableCol(0);
	FullBufferSizer->AddGrowableRow(1);
	BufferSizer = new wxFlexGridSizer(0, 3, 0, 0);
	BufferSizer->AddGrowableCol(1);
	StaticText4 = new wxStaticText(ScrolledWindow1, wxID_ANY, _("Render Style"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BufferSizer->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferStyleChoice = new wxChoice(ScrolledWindow1, ID_CHOICE_BufferStyle, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferStyle"));
	BufferStyleChoice->SetSelection( BufferStyleChoice->Append(_("Default")) );
	BufferStyleChoice->Append(_("Per Preview"));
	BufferSizer->Add(BufferStyleChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButtonBufferStyle = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_CHOICE_BufferStyle, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferStyle"));
	BitmapButtonBufferStyle->SetDefault();
	BitmapButtonBufferStyle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButtonBufferStyle->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButtonBufferStyle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(ScrolledWindow1, wxID_ANY, _("Transformation"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BufferSizer->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferTransform = new wxChoice(ScrolledWindow1, ID_CHOICE_BufferTransform, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferTransform"));
	BufferTransform->SetSelection( BufferTransform->Append(_("None")) );
	BufferTransform->Append(_("Rotate CC 90"));
	BufferTransform->Append(_("Rotate CW 90"));
	BufferTransform->Append(_("Rotate 180"));
	BufferTransform->Append(_("Flip Vertical"));
	BufferTransform->Append(_("Flip Horizontal"));
	BufferSizer->Add(BufferTransform, 1, wxALL, 2);
	BitmapButton_BufferTransform = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_CHOICE_BufferTransform, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferTransform"));
	BitmapButton_BufferTransform->SetDefault();
	BitmapButton_BufferTransform->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_BufferTransform->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButton_BufferTransform, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT2, _("Blur"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BufferSizer->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_EffectBlur = new wxSlider(ScrolledWindow1, ID_SLIDER_EffectBlur, 1, 1, 15, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_EffectBlur"));
	FlexGridSizer6->Add(Slider_EffectBlur, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Blur = new ValueCurveButton(ScrolledWindow1, ID_VALUECURVE_Blur, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Blur"));
	FlexGridSizer6->Add(BitmapButton_Blur, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_EffectBlur = new wxTextCtrl(ScrolledWindow1, IDD_TEXTCTRL_EffectBlur, _("1"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_EffectBlur"));
	TextCtrl_EffectBlur->SetMaxLength(2);
	FlexGridSizer6->Add(TextCtrl_EffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BufferSizer->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 2);
	BitmapButton_EffectBlur = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_SLIDER_EffectBlur, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_EffectBlur"));
	BitmapButton_EffectBlur->SetDefault();
	BitmapButton_EffectBlur->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_EffectBlur->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButton_EffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OverlayBkg = new wxCheckBox(ScrolledWindow1, ID_CHECKBOX_OverlayBkg, _("Persistent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OverlayBkg"));
	CheckBox_OverlayBkg->SetValue(false);
	BufferSizer->Add(CheckBox_OverlayBkg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_OverlayBkg = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_OverlayBkg, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_OverlayBkg"));
	BitmapButton_OverlayBkg->SetDefault();
	BitmapButton_OverlayBkg->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_OverlayBkg->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButton_OverlayBkg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FullBufferSizer->Add(BufferSizer, 1, wxALL|wxEXPAND, 0);
	SubBufferPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	SubBufferPanelSizer->AddGrowableCol(0);
	SubBufferPanelSizer->AddGrowableRow(0);
	FullBufferSizer->Add(SubBufferPanelSizer, 1, wxALL|wxEXPAND, 0);
	ScrolledWindow1->SetSizer(FullBufferSizer);
	FullBufferSizer->Fit(ScrolledWindow1);
	FullBufferSizer->SetSizeHints(ScrolledWindow1);
	FlexGridSizer11->Add(ScrolledWindow1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 0);
	Panel3->SetSizer(FlexGridSizer11);
	FlexGridSizer11->Fit(Panel3);
	FlexGridSizer11->SetSizeHints(Panel3);
	Panel4 = new wxPanel(Notebook1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer15 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer15->AddGrowableCol(0);
	FlexGridSizer15->AddGrowableRow(0);
	ScrolledWindow2 = new wxScrolledWindow(Panel4, ID_SCROLLEDWINDOW2, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW2"));
	RotoZoomSizer = new wxFlexGridSizer(0, 3, 0, 0);
	RotoZoomSizer->AddGrowableCol(1);
	StaticText11 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Preset"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Preset = new wxChoice(ScrolledWindow2, ID_CHOICE_Preset, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Preset"));
	Choice_Preset->SetSelection( Choice_Preset->Append(wxEmptyString) );
	Choice_Preset->Append(_("None - Reset"));
	Choice_Preset->Append(_("1 Rev CW"));
	Choice_Preset->Append(_("1 Rev CCW"));
	Choice_Preset->Append(_("Explode"));
	Choice_Preset->Append(_("Collapse"));
	Choice_Preset->Append(_("Explode + Spin CW"));
	Choice_Preset->Append(_("Spin CW Accelerate"));
	Choice_Preset->Append(_("Shake"));
	RotoZoomSizer->Add(Choice_Preset, 1, wxALL, 2);
	RotoZoomSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Rotation = new wxSlider(ScrolledWindow2, ID_SLIDER_Rotation, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Rotation"));
	FlexGridSizer2->Add(Slider_Rotation, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCRotation = new ValueCurveButton(ScrolledWindow2, ID_VALUECURVE_Rotation, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Rotation"));
	FlexGridSizer2->Add(BitmapButton_VCRotation, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Rotation = new wxTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_Rotation, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, _rotation, _T("IDD_TEXTCTRL_Rotation"));
	TextCtrl_Rotation->SetMaxLength(3);
	FlexGridSizer2->Add(TextCtrl_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Rotation = new wxBitmapButton(ScrolledWindow2, ID_BITMAPBUTTON_Rotation, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Rotation"));
	BitmapButton_Rotation->SetDefault();
	BitmapButton_Rotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Rotation->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Rotations = new wxSlider(ScrolledWindow2, ID_SLIDER_Rotations, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Rotations"));
	FlexGridSizer5->Add(Slider_Rotations, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCRotations = new ValueCurveButton(ScrolledWindow2, ID_VALUECURVE_Rotations, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Rotations"));
	FlexGridSizer5->Add(BitmapButton_VCRotations, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Rotations = new wxTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_Rotations, _("0.0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, _rotations, _T("IDD_TEXTCTRL_Rotations"));
	FlexGridSizer5->Add(TextCtrl_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Rotations = new wxBitmapButton(ScrolledWindow2, ID_BITMAPBUTTON_Rotations, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Rotations"));
	BitmapButton_Rotations->SetDefault();
	BitmapButton_Rotations->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Rotations->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText8 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Pivot Point X"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	Slider_PivotPointX = new wxSlider(ScrolledWindow2, ID_SLIDER_PivotPointX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PivotPointX"));
	FlexGridSizer9->Add(Slider_PivotPointX, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCPivotPointX = new ValueCurveButton(ScrolledWindow2, ID_VALUECURVE_PivotPointX, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_PivotPointX"));
	FlexGridSizer9->Add(BitmapButton_VCPivotPointX, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_PivotPointX = new wxTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_PivotPointX, _("50"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, _pivotpointx, _T("IDD_TEXTCTRL_PivotPointX"));
	FlexGridSizer9->Add(TextCtrl_PivotPointX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
	BitmapButton_PivotPointX = new wxBitmapButton(ScrolledWindow2, ID_BITMAPBUTTON_PivotPointX, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_PivotPointX"));
	BitmapButton_PivotPointX->SetDefault();
	BitmapButton_PivotPointX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_PivotPointX->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_PivotPointX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Pivot Point Y"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	Slider_PivotPointY = new wxSlider(ScrolledWindow2, ID_SLIDER_PivotPointY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PivotPointY"));
	FlexGridSizer10->Add(Slider_PivotPointY, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCPivotPointY = new ValueCurveButton(ScrolledWindow2, ID_VALUECURVE_PivotPointY, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_PivotPointY"));
	FlexGridSizer10->Add(BitmapButton_VCPivotPointY, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_PivotPointY = new wxTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_PivotPointY, _("50"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, _pivotpointy, _T("IDD_TEXTCTRL_PivotPointY"));
	FlexGridSizer10->Add(TextCtrl_PivotPointY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	BitmapButton_PivotPointY = new wxBitmapButton(ScrolledWindow2, ID_BITMAPBUTTON_PivotPointY, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_PivotPointY"));
	BitmapButton_PivotPointY->SetDefault();
	BitmapButton_PivotPointY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_PivotPointY->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_PivotPointY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Zoom"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Zoom = new wxSlider(ScrolledWindow2, ID_SLIDER_Zoom, 0, 0, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Zoom"));
	FlexGridSizer7->Add(Slider_Zoom, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCZoom = new ValueCurveButton(ScrolledWindow2, ID_VALUECURVE_Zoom, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Zoom"));
	FlexGridSizer7->Add(BitmapButton_VCZoom, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Zoom = new wxTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_Zoom, _("0.0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, _zoom, _T("IDD_TEXTCTRL_Zoom"));
	FlexGridSizer7->Add(TextCtrl_Zoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Zoom = new wxBitmapButton(ScrolledWindow2, ID_BITMAPBUTTON_Zoom, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Zoom"));
	BitmapButton_Zoom->SetDefault();
	BitmapButton_Zoom->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Zoom->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_Zoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Zoom Quality"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_ZoomQuality = new wxSlider(ScrolledWindow2, ID_SLIDER_ZoomQuality, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ZoomQuality"));
	FlexGridSizer8->Add(Slider_ZoomQuality, 1, wxALL|wxEXPAND, 2);
	TextCtrl_ZoomQuality = new wxTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_ZoomQuality, _("1"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, _zoomquality, _T("IDD_TEXTCTRL_ZoomQuality"));
	FlexGridSizer8->Add(TextCtrl_ZoomQuality, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
	BitmapButton_ZoomQuality = new wxBitmapButton(ScrolledWindow2, ID_BITMAPBUTTON_ZoomQuality, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_ZoomQuality"));
	BitmapButton_ZoomQuality->SetDefault();
	BitmapButton_ZoomQuality->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_ZoomQuality->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_ZoomQuality, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ScrolledWindow2->SetSizer(RotoZoomSizer);
	RotoZoomSizer->Fit(ScrolledWindow2);
	RotoZoomSizer->SetSizeHints(ScrolledWindow2);
	FlexGridSizer15->Add(ScrolledWindow2, 1, wxALL|wxEXPAND, 2);
	Panel4->SetSizer(FlexGridSizer15);
	FlexGridSizer15->Fit(Panel4);
	FlexGridSizer15->SetSizeHints(Panel4);
	Notebook1->AddPage(Panel3, _("Buffer"), false);
	Notebook1->AddPage(Panel4, _("Roto-Zoom"), false);
	FlexGridSizer1->Add(Notebook1, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_BufferStyle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_CHOICE_BufferTransform,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BufferPanel::OnBufferTransformSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_BufferTransform,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_EffectBlur,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::OnSlider_EffectBlurCmdSliderUpdated);
	Connect(ID_VALUECURVE_Blur,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnBitmapButton_BlurClick);
	Connect(IDD_TEXTCTRL_EffectBlur,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_EffectBlur,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_OverlayBkg,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_CHOICE_Preset,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BufferPanel::OnChoice_PresetSelect);
	Connect(ID_SLIDER_Rotation,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::OnSlider_RotationCmdSliderUpdated);
	Connect(ID_VALUECURVE_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnBitmapButton_RotationClick);
	Connect(IDD_TEXTCTRL_Rotation,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Rotations,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::OnSlider_RotationsCmdSliderUpdated);
	Connect(ID_VALUECURVE_Rotations,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnBitmapButton_VCRotationsClick);
	Connect(IDD_TEXTCTRL_Rotations,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::OnTextCtrl_RotationsText);
	Connect(ID_BITMAPBUTTON_Rotations,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_PivotPointX,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::OnSlider_PivotPointXCmdSliderUpdated);
	Connect(ID_VALUECURVE_PivotPointX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnBitmapButton_VCPivotPointXClick);
	Connect(IDD_TEXTCTRL_PivotPointX,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_PivotPointX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_PivotPointY,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::OnSlider_PivotPointYCmdSliderUpdated);
	Connect(ID_VALUECURVE_PivotPointY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnBitmapButton_VCPivotPointYClick);
	Connect(IDD_TEXTCTRL_PivotPointY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_PivotPointY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Zoom,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::OnSlider_ZoomCmdSliderUpdated);
	Connect(ID_VALUECURVE_Zoom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnBitmapButton_ZoomClick);
	Connect(IDD_TEXTCTRL_Zoom,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::OnTextCtrl_ZoomText);
	Connect(ID_BITMAPBUTTON_Zoom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_ZoomQuality,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_ZoomQuality,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_ZoomQuality,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&BufferPanel::OnResize);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&BufferPanel::OnVCChanged, 0, this);

    Slider_Rotation->SetValue(__rotation);
    TextCtrl_Rotation->SetValue(wxString::Format("%d", __rotation));
    Slider_Rotations->SetValue(__rotations * 10.0f);
    TextCtrl_Rotations->SetValue(wxString::Format("%.1f", __rotations));
    Slider_Zoom->SetValue(__zoom * 10.0f);
    TextCtrl_Zoom->SetValue(wxString::Format("%.1f", __zoom));
    Slider_ZoomQuality->SetValue(__zoomquality);
    TextCtrl_ZoomQuality->SetValue(wxString::Format("%d", __zoomquality));
    Slider_PivotPointX->SetValue(__pivotpointx);
    TextCtrl_PivotPointX->SetValue(wxString::Format("%d", __pivotpointx));
    Slider_PivotPointY->SetValue(__pivotpointy);
    TextCtrl_PivotPointY->SetValue(wxString::Format("%d", __pivotpointy));

    BitmapButton_Blur->GetValue()->SetLimits(1, 15);
    BitmapButton_VCRotation->GetValue()->SetLimits(0, 100);
    BitmapButton_VCZoom->GetValue()->SetLimits(0, 3);
    BitmapButton_VCRotations->GetValue()->SetLimits(0, 10);
    BitmapButton_VCPivotPointX->GetValue()->SetLimits(0, 100);
    BitmapButton_VCPivotPointY->GetValue()->SetLimits(0, 100);

    subBufferPanel = new SubBufferPanel(ScrolledWindow1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    wxSize s(10,100);
    subBufferPanel->SetMinSize(s);
    SubBufferPanelSizer->Insert(0, subBufferPanel,1, wxALL|wxEXPAND, 2);
    ValidateWindow();
}

void BufferPanel::OnVCChanged(wxCommandEvent& event)
{
    ValidateWindow();
}

BufferPanel::~BufferPanel()
{
	//(*Destroy(BufferPanel)
	//*)
}

PANEL_EVENT_HANDLERS(BufferPanel)

wxString BufferPanel::GetBufferString() {
    wxString s;
    // Persistent
    if (CheckBox_OverlayBkg->GetValue()) {
        s += "B_CHECKBOX_OverlayBkg=1,";
    }
    if (BufferStyleChoice->GetSelection() != 0) {
        s += "B_CHOICE_BufferStyle=";
        s += BufferStyleChoice->GetStringSelection();
        s += ",";
    }
    if (BufferTransform->GetSelection() != 0) {
        s += "B_CHOICE_BufferTransform=";
        s += BufferTransform->GetStringSelection();
        s += ",";
    }

    wxString subB = subBufferPanel->GetValue();
    if (subB.size() > 0) {
        s += "B_CUSTOM_SubBuffer=";
        s += subB;
        s += ",";
    }

    if (BitmapButton_Blur->GetValue()->IsActive())
    {
        wxString blurVC = wxString(BitmapButton_Blur->GetValue()->Serialise().c_str());
        if (blurVC.size() > 0)
        {
            s += "B_VALUECURVE_Blur=";
            s += blurVC;
            s += ",";
        }
    }
    else
    {
        // Blur
        if (Slider_EffectBlur->GetValue() > 1) {
            s += wxString::Format("B_SLIDER_EffectBlur=%d,", Slider_EffectBlur->GetValue());
        }
    }
    if (BitmapButton_VCRotation->GetValue()->IsActive())
    {
        wxString rotationVC = wxString(BitmapButton_VCRotation->GetValue()->Serialise().c_str());
        if (rotationVC.size() > 0)
        {
            s += "B_VALUECURVE_Rotation=";
            s += rotationVC;
            s += ",";
        }
    }
    else
    {
        if (Slider_Rotation->GetValue() > 0) {
            s += wxString::Format("B_SLIDER_Rotation=%d,", Slider_Rotation->GetValue());
        }
    }
    if (BitmapButton_VCZoom->GetValue()->IsActive())
    {
        wxString zoomVC = wxString(BitmapButton_VCZoom->GetValue()->Serialise().c_str());
        if (zoomVC.size() > 0)
        {
            s += "B_VALUECURVE_Zoom=";
            s += zoomVC;
            s += ",";
        }
    }
    else
    {
        if (Slider_Zoom->GetValue() != 10) {
            s += wxString::Format("B_SLIDER_Zoom=%d,", Slider_Zoom->GetValue());
        }
    }
    if (BitmapButton_VCRotations->GetValue()->IsActive())
    {
        wxString rotationsVC = wxString(BitmapButton_VCRotations->GetValue()->Serialise().c_str());
        if (rotationsVC.size() > 0)
        {
            s += "B_VALUECURVE_Rotations=";
            s += rotationsVC;
            s += ",";
        }
    }
    else
    {
        if (Slider_Rotations->GetValue() != 0) {
            s += wxString::Format("B_SLIDER_Rotations=%d,", Slider_Rotations->GetValue());
        }
    }
    if (Slider_ZoomQuality->GetValue() != 1) {
        s += wxString::Format("B_SLIDER_ZoomQuality=%d,", Slider_ZoomQuality->GetValue());
    }
    if (BitmapButton_VCPivotPointX->GetValue()->IsActive())
    {
        wxString pivotpointxVC = wxString(BitmapButton_VCPivotPointX->GetValue()->Serialise().c_str());
        if (pivotpointxVC.size() > 0)
        {
            s += "B_VALUECURVE_PivotPointX=";
            s += pivotpointxVC;
            s += ",";
        }
    }
    else
    {
        if (Slider_PivotPointX->GetValue() != 50) {
            s += wxString::Format("B_SLIDER_PivotPointX=%d,", Slider_PivotPointX->GetValue());
        }
    }
    if (BitmapButton_VCPivotPointY->GetValue()->IsActive())
    {
        wxString pivotpointyVC = wxString(BitmapButton_VCPivotPointY->GetValue()->Serialise().c_str());
        if (pivotpointyVC.size() > 0)
        {
            s += "B_VALUECURVE_PivotPointY=";
            s += pivotpointyVC;
            s += ",";
        }
    }
    else
    {
        if (Slider_PivotPointY->GetValue() != 50) {
            s += wxString::Format("B_SLIDER_PivotPointY=%d,", Slider_PivotPointY->GetValue());
        }
    }
    return s;
}

void BufferPanel::SetDefaultControls(const Model *model) {
    Slider_EffectBlur->SetValue(1);
    CheckBox_OverlayBkg->SetValue(false);
    BufferStyleChoice->Clear();
    if (model != nullptr) {
        const std::vector<std::string> &types = model->GetBufferStyles();
        for (auto it = types.begin(); it != types.end(); it++) {
            BufferStyleChoice->Append(*it);
        }
    }
    if (BufferStyleChoice->IsEmpty()) {
        BufferStyleChoice->Append("Default");
    }
    subBufferPanel->SetDefaults();

    __blur = 1;
    Slider_EffectBlur->SetValue(1);
    TextCtrl_EffectBlur->SetValue("1");
    BufferStyleChoice->SetSelection(0);
    BufferTransform->SetSelection(0);
    BitmapButton_Blur->GetValue()->SetDefault(1.0f, 15.0f);
    BitmapButton_Blur->UpdateState();
    __rotation = 0;
    Slider_Rotation->SetValue(0);
    TextCtrl_Rotation->SetValue("0");
    BitmapButton_VCRotation->GetValue()->SetDefault(0.0f, 100.0f);
    BitmapButton_VCRotation->UpdateState();
    __zoom = 1;
    Slider_Zoom->SetValue(1);
    TextCtrl_Zoom->SetValue("1");
    BitmapButton_VCZoom->GetValue()->SetDefault(0.0f, 3.0f);
    BitmapButton_VCZoom->UpdateState();
    __rotations = 0;
    Slider_Rotations->SetValue(0);
    TextCtrl_Rotations->SetValue("0.0");
    BitmapButton_VCRotations->GetValue()->SetDefault(0.0f, 10.0f);
    BitmapButton_VCRotations->UpdateState();
    __zoomquality = 1;
    Slider_ZoomQuality->SetValue(1);
    TextCtrl_ZoomQuality->SetValue("1");
    __pivotpointx = 50;
    Slider_PivotPointX->SetValue(50);
    TextCtrl_PivotPointX->SetValue("50");
    BitmapButton_VCPivotPointX->GetValue()->SetDefault(0.0f, 100.0f);
    BitmapButton_VCPivotPointX->UpdateState();
    __pivotpointy = 50;
    Slider_PivotPointY->SetValue(50);
    TextCtrl_PivotPointY->SetValue("50");
    BitmapButton_VCPivotPointY->GetValue()->SetDefault(0.0f, 100.0f);
    BitmapButton_VCPivotPointY->UpdateState();
    ValidateWindow();
    wxSizeEvent evt;
    OnResize(evt);
}


void BufferPanel::OnResize(wxSizeEvent& event)
{
    wxSize s = GetClientSize();

    Notebook1->SetSize(s);
    Notebook1->SetMinSize(s);
    Notebook1->SetMaxSize(s);
    Notebook1->Refresh();

    FullBufferSizer->Layout();
    RotoZoomSizer->Layout();

    ScrolledWindow1->SetSizer(FullBufferSizer);
    ScrolledWindow1->FitInside();
    ScrolledWindow1->SetScrollRate(5, 5);

    ScrolledWindow2->SetSizer(RotoZoomSizer);
    ScrolledWindow2->FitInside();
    ScrolledWindow2->SetScrollRate(5, 5);
}

void BufferPanel::OnBitmapButton_BlurClick(wxCommandEvent& event)
{
    BitmapButton_Blur->ToggleActive();
    ValidateWindow();
    if (BitmapButton_Blur->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(this, BitmapButton_Blur->GetValue());
        vcd.ShowModal();
        BitmapButton_Blur->UpdateState();
    }
}

void BufferPanel::OnBitmapButton_RotationClick(wxCommandEvent& event)
{
    BitmapButton_VCRotation->ToggleActive();
    ValidateWindow();
    if (BitmapButton_VCRotation->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(this, BitmapButton_VCRotation->GetValue());
        vcd.ShowModal();
        BitmapButton_VCRotation->UpdateState();
    }
}

void BufferPanel::OnBitmapButton_ZoomClick(wxCommandEvent& event)
{
    BitmapButton_VCZoom->ToggleActive();
    ValidateWindow();
    if (BitmapButton_VCZoom->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(this, BitmapButton_VCZoom->GetValue());
        vcd.ShowModal();
        BitmapButton_VCZoom->UpdateState();
    }
}

void BufferPanel::OnBitmapButton_VCRotationsClick(wxCommandEvent& event)
{
    BitmapButton_VCRotations->ToggleActive();
    ValidateWindow();
    if (BitmapButton_VCRotations->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(this, BitmapButton_VCRotations->GetValue());
        vcd.ShowModal();
        BitmapButton_VCRotations->UpdateState();
    }
}

void BufferPanel::OnBitmapButton_VCPivotPointXClick(wxCommandEvent& event)
{
    BitmapButton_VCPivotPointX->ToggleActive();
    ValidateWindow();
    if (BitmapButton_VCPivotPointX->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(this, BitmapButton_VCPivotPointX->GetValue());
        vcd.ShowModal();
        BitmapButton_VCPivotPointX->UpdateState();
    }
}

void BufferPanel::OnBitmapButton_VCPivotPointYClick(wxCommandEvent& event)
{
    BitmapButton_VCPivotPointY->ToggleActive();
    ValidateWindow();
    if (BitmapButton_VCPivotPointY->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(this, BitmapButton_VCPivotPointY->GetValue());
        vcd.ShowModal();
        BitmapButton_VCPivotPointY->UpdateState();
    }
}

void BufferPanel::ValidateWindow()
{
    if (BitmapButton_Blur->GetValue()->IsActive())
    {
        Slider_EffectBlur->Disable();
        TextCtrl_EffectBlur->Disable();
    }
    else
    {
        Slider_EffectBlur->Enable();
        TextCtrl_EffectBlur->Enable();
    }

    if (BitmapButton_VCRotation->GetValue()->IsActive())
    {
        Slider_Rotation->Disable();
        TextCtrl_Rotation->Disable();
    }
    else
    {
        Slider_Rotation->Enable();
        TextCtrl_Rotation->Enable();
    }

    if (BitmapButton_VCZoom->GetValue()->IsActive())
    {
        Slider_Zoom->Disable();
        TextCtrl_Zoom->Disable();
    }
    else
    {
        Slider_Zoom->Enable();
        TextCtrl_Zoom->Enable();
    }

    if (BitmapButton_VCRotations->GetValue()->IsActive())
    {
        Slider_Rotations->Disable();
        TextCtrl_Rotations->Disable();
    }
    else
    {
        Slider_Rotations->Enable();
        TextCtrl_Rotations->Enable();
    }

    if (BitmapButton_VCPivotPointX->GetValue()->IsActive())
    {
        Slider_PivotPointX->Disable();
        TextCtrl_PivotPointX->Disable();
    }
    else
    {
        Slider_PivotPointX->Enable();
        TextCtrl_PivotPointX->Enable();
    }

    if (BitmapButton_VCPivotPointY->GetValue()->IsActive())
    {
        Slider_PivotPointY->Disable();
        TextCtrl_PivotPointY->Disable();
    }
    else
    {
        Slider_PivotPointY->Enable();
        TextCtrl_PivotPointY->Enable();
    }
}

void BufferPanel::OnBufferTransformSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

#pragma region Sliders and Text Controls
void BufferPanel::OnSlider_EffectBlurCmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    if (BitmapButton_Blur->GetValue()->GetType() == "Flat")
    {
        BitmapButton_Blur->GetValue()->SetUnscaledParameter1(Slider_EffectBlur->GetValue());
    }
}

void BufferPanel::OnSlider_RotationCmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    if (BitmapButton_VCRotation->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCRotation->GetValue()->SetUnscaledParameter1(Slider_Rotation->GetValue());
    }
}

void BufferPanel::OnSlider_PivotPointXCmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    if (BitmapButton_VCPivotPointX->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCPivotPointX->GetValue()->SetUnscaledParameter1(Slider_PivotPointX->GetValue());
    }
}

void BufferPanel::OnSlider_PivotPointYCmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    if (BitmapButton_VCPivotPointY->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCPivotPointY->GetValue()->SetUnscaledParameter1(Slider_PivotPointY->GetValue());
    }
}

void BufferPanel::OnSlider_RotationsCmdSliderUpdated(wxScrollEvent& event)
{
    int i = Slider_Rotations->GetValue();
    __rotations = (float)i / 10.0f;
    wxString txt = wxString::Format("%.1f", __rotations);
    TextCtrl_Rotations->SetValue(txt);
    if (BitmapButton_VCRotations->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCRotations->GetValue()->SetUnscaledParameter1(i);
    }
}

void BufferPanel::OnTextCtrl_RotationsText(wxCommandEvent& event)
{
    float f = wxAtof(TextCtrl_Rotations->GetValue());
    int i = f * 10.0f;
    __rotations = f;
    if (Slider_Rotations->GetValue() != i)
    {
        Slider_Rotations->SetValue(i);
    }
}

void BufferPanel::OnSlider_ZoomCmdSliderUpdated(wxScrollEvent& event)
{
    int i = Slider_Zoom->GetValue();
    __zoom = (float)i / 10.0f;
    wxString txt = wxString::Format("%.1f", __zoom);
    TextCtrl_Zoom->SetValue(txt);
    if (BitmapButton_VCZoom->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCZoom->GetValue()->SetUnscaledParameter1(i);
    }
}

void BufferPanel::OnTextCtrl_ZoomText(wxCommandEvent& event)
{
    float f = wxAtof(TextCtrl_Zoom->GetValue());
    int i = f * 10.0f;
    __zoom = f;
    if (Slider_Zoom->GetValue() != i)
    {
        Slider_Zoom->SetValue(i);
    }
}
#pragma endregion Sliders and Text Controls

void BufferPanel::OnChoice_PresetSelect(wxCommandEvent& event)
{
    wxString preset = Choice_Preset->GetStringSelection();

    Slider_Zoom->SetValue(10);
    TextCtrl_Zoom->SetValue("1.0");
    Slider_PivotPointX->SetValue(50);
    TextCtrl_PivotPointX->SetValue("50");
    Slider_PivotPointY->SetValue(50);
    TextCtrl_PivotPointY->SetValue("50");
    Slider_Rotation->SetValue(0);
    TextCtrl_Rotation->SetValue("0");
    Slider_Rotations->SetValue(0);
    TextCtrl_Rotations->SetValue("0.0");
    Slider_ZoomQuality->SetValue(1);
    TextCtrl_ZoomQuality->SetValue("1");
    BitmapButton_VCPivotPointX->GetValue()->SetDefault();
    BitmapButton_VCPivotPointY->GetValue()->SetDefault();
    BitmapButton_VCZoom->GetValue()->SetDefault();
    BitmapButton_VCRotation->GetValue()->SetDefault();
    BitmapButton_VCRotations->GetValue()->SetDefault();
    BitmapButton_VCPivotPointX->SetActive(false);
    BitmapButton_VCPivotPointY->SetActive(false);
    BitmapButton_VCZoom->SetActive(false);
    BitmapButton_VCRotation->SetActive(false);
    BitmapButton_VCRotations->SetActive(false);

    if (preset == "None - Reset")
    {
        // dont do anything
    }
    else if (preset == "1 Rev CW")
    {
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(100);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
    }
    else if (preset == "1 Rev CCW")
    {
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(100);
        BitmapButton_VCRotation->GetValue()->SetParameter2(0);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
    }
    else if (preset == "Explode")
    {
        BitmapButton_VCZoom->GetValue()->SetType("Ramp");
        BitmapButton_VCZoom->GetValue()->SetParameter1(0);
        BitmapButton_VCZoom->GetValue()->SetParameter2(33);
        BitmapButton_VCZoom->SetActive(true);
    }
    else if (preset == "Collapse")
    {
        BitmapButton_VCZoom->GetValue()->SetType("Ramp");
        BitmapButton_VCZoom->GetValue()->SetParameter1(33);
        BitmapButton_VCZoom->GetValue()->SetParameter2(0);
        BitmapButton_VCZoom->SetActive(true);
    }
    else if (preset == "Explode + Spin CW")
    {
        BitmapButton_VCZoom->GetValue()->SetType("Ramp");
        BitmapButton_VCZoom->GetValue()->SetParameter1(0);
        BitmapButton_VCZoom->GetValue()->SetParameter2(33);
        BitmapButton_VCZoom->SetActive(true);
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(100);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
        TextCtrl_ZoomQuality->SetValue("2");
    }
    else if (preset == "Shake")
    {
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Sine");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(10);
        BitmapButton_VCRotation->GetValue()->SetParameter3(50);
        BitmapButton_VCRotation->GetValue()->SetParameter4(25);
        BitmapButton_VCRotation->GetValue()->SetWrap(true);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
        TextCtrl_ZoomQuality->SetValue("2");
    }
    else if (preset == "Spin CW Accelerate")
    {
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(100);
        BitmapButton_VCRotation->SetActive(true);
        BitmapButton_VCRotations->GetValue()->SetType("Ramp");
        BitmapButton_VCRotations->GetValue()->SetParameter1(0);
        BitmapButton_VCRotations->GetValue()->SetParameter2(50);
        BitmapButton_VCRotations->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
        TextCtrl_ZoomQuality->SetValue("2");
    }
    Choice_Preset->SetStringSelection("");
    ValidateWindow();
}

void BufferPanel::OnButton_ResetClick(wxCommandEvent& event)
{
    subBufferPanel->SetDefaults();
    subBufferPanel->Refresh();
    Refresh();
}
