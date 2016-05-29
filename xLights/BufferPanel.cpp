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
#include "../include/valuecurvenotselected.xpm"
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
const long BufferPanel::ID_STATICTEXT1 = wxNewId();
const long BufferPanel::ID_SLIDER_Rotation = wxNewId();
const long BufferPanel::ID_VALUECURVE_Rotation = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Rotation = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Rotation = wxNewId();
const long BufferPanel::ID_STATICTEXT3 = wxNewId();
const long BufferPanel::ID_SLIDER_Rotations = wxNewId();
const long BufferPanel::ID_VALUECURVE_Rotations = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Rotations = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Rotations = wxNewId();
const long BufferPanel::ID_STATICTEXT6 = wxNewId();
const long BufferPanel::ID_SLIDER_PivotPointX = wxNewId();
const long BufferPanel::ID_VALUECURVE_PivotPointX = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_PivotPointX = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_PivotPointX = wxNewId();
const long BufferPanel::ID_STATICTEXT7 = wxNewId();
const long BufferPanel::ID_SLIDER_PivotPointY = wxNewId();
const long BufferPanel::ID_VALUECURVE_PivotPointY = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_PivotPointY = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_PivotPointY = wxNewId();
const long BufferPanel::ID_STATICTEXT4 = wxNewId();
const long BufferPanel::ID_SLIDER_Zoom = wxNewId();
const long BufferPanel::ID_VALUECURVE_Zoom = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Zoom = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Zoom = wxNewId();
const long BufferPanel::ID_STATICTEXT5 = wxNewId();
const long BufferPanel::ID_SLIDER_ZoomQuality = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_ZoomQuality = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_ZoomQuality = wxNewId();
const long BufferPanel::ID_CHECKBOX_OverlayBkg = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_OverlayBkg = wxNewId();
const long BufferPanel::ID_SCROLLED_ColorScroll = wxNewId();
const long BufferPanel::ID_PANEL1 = wxNewId();
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
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxBitmapButton* BitmapButtonBufferStyle;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;
	wxStaticText* StaticText4;
	wxBitmapButton* BitmapButton_BufferTransform;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	BufferScrollWindow = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_ColorScroll, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_ColorScroll"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	Sizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Sizer2->AddGrowableCol(1);
	StaticText4 = new wxStaticText(BufferScrollWindow, wxID_ANY, _("Render Style"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	Sizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferStyleChoice = new wxChoice(BufferScrollWindow, ID_CHOICE_BufferStyle, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferStyle"));
	BufferStyleChoice->SetSelection( BufferStyleChoice->Append(_("Default")) );
	BufferStyleChoice->Append(_("Per Preview"));
	Sizer2->Add(BufferStyleChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButtonBufferStyle = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_CHOICE_BufferStyle, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferStyle"));
	BitmapButtonBufferStyle->SetDefault();
	BitmapButtonBufferStyle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButtonBufferStyle->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButtonBufferStyle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(BufferScrollWindow, wxID_ANY, _("Transformation"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	Sizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferTransform = new wxChoice(BufferScrollWindow, ID_CHOICE_BufferTransform, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferTransform"));
	BufferTransform->SetSelection( BufferTransform->Append(_("None")) );
	BufferTransform->Append(_("Rotate CC 90"));
	BufferTransform->Append(_("Rotate CW 90"));
	BufferTransform->Append(_("Rotate 180"));
	BufferTransform->Append(_("Flip Vertical"));
	BufferTransform->Append(_("Flip Horizontal"));
	Sizer2->Add(BufferTransform, 1, wxALL, 2);
	BitmapButton_BufferTransform = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_CHOICE_BufferTransform, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferTransform"));
	BitmapButton_BufferTransform->SetDefault();
	BitmapButton_BufferTransform->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_BufferTransform->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_BufferTransform, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(BufferScrollWindow, ID_STATICTEXT2, _("Blur"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	Sizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_EffectBlur = new wxSlider(BufferScrollWindow, ID_SLIDER_EffectBlur, 1, 1, 15, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_EffectBlur"));
	FlexGridSizer6->Add(Slider_EffectBlur, 1, wxALL|wxEXPAND, 1);
	BitmapButton_Blur = new ValueCurveButton(BufferScrollWindow, ID_VALUECURVE_Blur, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Blur"));
	FlexGridSizer6->Add(BitmapButton_Blur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_EffectBlur = new wxTextCtrl(BufferScrollWindow, IDD_TEXTCTRL_EffectBlur, _("1"), wxDefaultPosition, wxDLG_UNIT(BufferScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_EffectBlur"));
	TextCtrl_EffectBlur->SetMaxLength(2);
	FlexGridSizer6->Add(TextCtrl_EffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 2);
	BitmapButton_EffectBlur = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_SLIDER_EffectBlur, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_EffectBlur"));
	BitmapButton_EffectBlur->SetDefault();
	BitmapButton_EffectBlur->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_EffectBlur->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_EffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(BufferScrollWindow, ID_STATICTEXT1, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	Sizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Rotation = new wxSlider(BufferScrollWindow, ID_SLIDER_Rotation, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Rotation"));
	FlexGridSizer2->Add(Slider_Rotation, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCRotation = new ValueCurveButton(BufferScrollWindow, ID_VALUECURVE_Rotation, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Rotation"));
	FlexGridSizer2->Add(BitmapButton_VCRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Rotation = new wxTextCtrl(BufferScrollWindow, IDD_TEXTCTRL_Rotation, _("0"), wxDefaultPosition, wxDLG_UNIT(BufferScrollWindow,wxSize(20,-1)), 0, _rotation, _T("IDD_TEXTCTRL_Rotation"));
	TextCtrl_Rotation->SetMaxLength(3);
	FlexGridSizer2->Add(TextCtrl_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Rotation = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_Rotation, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Rotation"));
	BitmapButton_Rotation->SetDefault();
	BitmapButton_Rotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Rotation->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(BufferScrollWindow, ID_STATICTEXT3, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	Sizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Rotations = new wxSlider(BufferScrollWindow, ID_SLIDER_Rotations, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Rotations"));
	FlexGridSizer5->Add(Slider_Rotations, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCRotations = new ValueCurveButton(BufferScrollWindow, ID_VALUECURVE_Rotations, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Rotations"));
	FlexGridSizer5->Add(BitmapButton_VCRotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Rotations = new wxTextCtrl(BufferScrollWindow, IDD_TEXTCTRL_Rotations, _("0.0"), wxDefaultPosition, wxDLG_UNIT(BufferScrollWindow,wxSize(20,-1)), 0, _rotations, _T("IDD_TEXTCTRL_Rotations"));
	FlexGridSizer5->Add(TextCtrl_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Rotations = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_Rotations, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Rotations"));
	BitmapButton_Rotations->SetDefault();
	BitmapButton_Rotations->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Rotations->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText8 = new wxStaticText(BufferScrollWindow, ID_STATICTEXT6, _("Pivot Point X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	Sizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	Slider_PivotPointX = new wxSlider(BufferScrollWindow, ID_SLIDER_PivotPointX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PivotPointX"));
	FlexGridSizer9->Add(Slider_PivotPointX, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCPivotPointX = new ValueCurveButton(BufferScrollWindow, ID_VALUECURVE_PivotPointX, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_PivotPointX"));
	FlexGridSizer9->Add(BitmapButton_VCPivotPointX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_PivotPointX = new wxTextCtrl(BufferScrollWindow, IDD_TEXTCTRL_PivotPointX, _("50"), wxDefaultPosition, wxDLG_UNIT(BufferScrollWindow,wxSize(20,-1)), 0, _pivotpointx, _T("IDD_TEXTCTRL_PivotPointX"));
	FlexGridSizer9->Add(TextCtrl_PivotPointX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 2);
	BitmapButton_PivotPointX = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_PivotPointX, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_PivotPointX"));
	BitmapButton_PivotPointX->SetDefault();
	BitmapButton_PivotPointX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_PivotPointX->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_PivotPointX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(BufferScrollWindow, ID_STATICTEXT7, _("Pivot Point Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	Sizer2->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	Slider_PivotPointY = new wxSlider(BufferScrollWindow, ID_SLIDER_PivotPointY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PivotPointY"));
	FlexGridSizer10->Add(Slider_PivotPointY, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCPivotPointY = new ValueCurveButton(BufferScrollWindow, ID_VALUECURVE_PivotPointY, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_PivotPointY"));
	FlexGridSizer10->Add(BitmapButton_VCPivotPointY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_PivotPointY = new wxTextCtrl(BufferScrollWindow, IDD_TEXTCTRL_PivotPointY, _("50"), wxDefaultPosition, wxDLG_UNIT(BufferScrollWindow,wxSize(20,-1)), 0, _pivotpointy, _T("IDD_TEXTCTRL_PivotPointY"));
	FlexGridSizer10->Add(TextCtrl_PivotPointY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 2);
	BitmapButton_PivotPointY = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_PivotPointY, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_PivotPointY"));
	BitmapButton_PivotPointY->SetDefault();
	BitmapButton_PivotPointY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_PivotPointY->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_PivotPointY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(BufferScrollWindow, ID_STATICTEXT4, _("Zoom"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	Sizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Zoom = new wxSlider(BufferScrollWindow, ID_SLIDER_Zoom, 0, 0, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Zoom"));
	FlexGridSizer7->Add(Slider_Zoom, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCZoom = new ValueCurveButton(BufferScrollWindow, ID_VALUECURVE_Zoom, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Zoom"));
	FlexGridSizer7->Add(BitmapButton_VCZoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Zoom = new wxTextCtrl(BufferScrollWindow, IDD_TEXTCTRL_Zoom, _("0.0"), wxDefaultPosition, wxDLG_UNIT(BufferScrollWindow,wxSize(20,-1)), 0, _zoom, _T("IDD_TEXTCTRL_Zoom"));
	FlexGridSizer7->Add(TextCtrl_Zoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Zoom = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_Zoom, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Zoom"));
	BitmapButton_Zoom->SetDefault();
	BitmapButton_Zoom->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Zoom->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_Zoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(BufferScrollWindow, ID_STATICTEXT5, _("Zoom Quality"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	Sizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_ZoomQuality = new wxSlider(BufferScrollWindow, ID_SLIDER_ZoomQuality, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ZoomQuality"));
	FlexGridSizer8->Add(Slider_ZoomQuality, 1, wxALL|wxEXPAND, 2);
	TextCtrl_ZoomQuality = new wxTextCtrl(BufferScrollWindow, IDD_TEXTCTRL_ZoomQuality, _("1"), wxDefaultPosition, wxDLG_UNIT(BufferScrollWindow,wxSize(20,-1)), 0, _zoomquality, _T("IDD_TEXTCTRL_ZoomQuality"));
	FlexGridSizer8->Add(TextCtrl_ZoomQuality, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 2);
	BitmapButton_ZoomQuality = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_ZoomQuality, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_ZoomQuality"));
	BitmapButton_ZoomQuality->SetDefault();
	BitmapButton_ZoomQuality->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_ZoomQuality->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_ZoomQuality, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_OverlayBkg = new wxCheckBox(BufferScrollWindow, ID_CHECKBOX_OverlayBkg, _("Persistent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OverlayBkg"));
	CheckBox_OverlayBkg->SetValue(false);
	Sizer2->Add(CheckBox_OverlayBkg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Sizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_OverlayBkg = new wxBitmapButton(BufferScrollWindow, ID_BITMAPBUTTON_OverlayBkg, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_OverlayBkg"));
	BitmapButton_OverlayBkg->SetDefault();
	BitmapButton_OverlayBkg->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_OverlayBkg->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	Sizer2->Add(BitmapButton_OverlayBkg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(Sizer2, 0, wxEXPAND, 10);
	SubBufferPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	SubBufferPanelSizer->AddGrowableCol(0);
	SubBufferPanelSizer->AddGrowableRow(0);
	FlexGridSizer4->Add(SubBufferPanelSizer, 1, wxALL|wxEXPAND, 0);
	BufferScrollWindow->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(BufferScrollWindow);
	FlexGridSizer4->SetSizeHints(BufferScrollWindow);
	FlexGridSizer3->Add(BufferScrollWindow, 0, wxALIGN_LEFT, 0);
	Panel_Sizer->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_Sizer);
	FlexGridSizer3->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Add(Panel_Sizer, 0, wxALIGN_LEFT, 0);
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
	Connect(ID_BITMAPBUTTON_OverlayBkg,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Panel_Sizer->Connect(wxEVT_SIZE,(wxObjectEventFunction)&BufferPanel::OnResize,0,this);
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

    subBufferPanel = new SubBufferPanel(BufferScrollWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    SubBufferPanelSizer->Add(subBufferPanel, 1, wxALL|wxEXPAND, 2);
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
    wxSize s = GetSize();
    Panel_Sizer->SetSize(s);
    Panel_Sizer->SetMinSize(s);
    Panel_Sizer->SetMaxSize(s);
    Panel_Sizer->Refresh();

    BufferScrollWindow->SetSize(s);
    BufferScrollWindow->SetMinSize(s);
    BufferScrollWindow->SetMaxSize(s);

    BufferScrollWindow->FitInside();
    BufferScrollWindow->SetScrollRate(5, 5);
    BufferScrollWindow->Refresh();
}

void BufferPanel::OnBitmapButton_BlurClick(wxCommandEvent& event)
{
    BitmapButton_Blur->ToggleActive();
    ValidateWindow();
    if (BitmapButton_Blur->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(this, BitmapButton_Blur->GetValue());
        vcd.ShowModal();
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
        BitmapButton_Blur->GetValue()->SetParameter1(Slider_EffectBlur->GetValue());
    }
}

void BufferPanel::OnSlider_RotationCmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    if (BitmapButton_VCRotation->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCRotation->GetValue()->SetParameter1(Slider_Rotation->GetValue());
    }
}

void BufferPanel::OnSlider_PivotPointXCmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    if (BitmapButton_VCPivotPointX->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCPivotPointX->GetValue()->SetParameter1(Slider_PivotPointX->GetValue());
    }
}

void BufferPanel::OnSlider_PivotPointYCmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    if (BitmapButton_VCPivotPointY->GetValue()->GetType() == "Flat")
    {
        BitmapButton_VCPivotPointY->GetValue()->SetParameter1(Slider_PivotPointY->GetValue());
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
        BitmapButton_VCRotations->GetValue()->SetParameter1(i);
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
        BitmapButton_VCZoom->GetValue()->SetParameter1(i);
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




