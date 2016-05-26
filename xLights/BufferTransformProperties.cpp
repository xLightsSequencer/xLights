#include "BufferTransformProperties.h"

//(*InternalHeaders(BufferTransformProperties)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/valnum.h>

//(*IdInit(BufferTransformProperties)
const long BufferTransformProperties::ID_STATICTEXT1 = wxNewId();
const long BufferTransformProperties::IDD_SLIDER_Rotations = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL1 = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT2 = wxNewId();
const long BufferTransformProperties::IDD_SLIDER_Zooms = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL_Zooms = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT4 = wxNewId();
const long BufferTransformProperties::IDD_SLIDER_ZoomMaximum = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL_ZoomMaximum = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT5 = wxNewId();
const long BufferTransformProperties::ID_SLIDER5 = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL_XCenter = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT6 = wxNewId();
const long BufferTransformProperties::ID_SLIDER4 = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL_YCenter = wxNewId();
const long BufferTransformProperties::ID_BUTTON1 = wxNewId();
const long BufferTransformProperties::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BufferTransformProperties,wxDialog)
	//(*EventTable(BufferTransformProperties)
	//*)
END_EVENT_TABLE()

BufferTransformProperties::BufferTransformProperties(wxWindow* parent,RotoZoomParms* parms, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    __rotations = parms->GetRotations();
    __zooms = parms->GetZooms();
    __zoommaximum = parms->GetZoomMaximum();
    __xcenter = parms->GetXCenter();
    __ycenter = parms->GetYCenter();
    _parms = parms;
    wxIntegerValidator<int> _rotations(&__rotations, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _rotations.SetMin(0);
    _rotations.SetMax(20);
    wxIntegerValidator<int> _zooms(&__zooms, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _zooms.SetMin(0);
    _zooms.SetMax(20);
    wxIntegerValidator<int> _zoommaximum(&__zoommaximum, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _zoommaximum.SetMin(-50);
    _zoommaximum.SetMax(50);
    wxIntegerValidator<int> _xcenter(&__xcenter, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _xcenter.SetMin(-50);
    _xcenter.SetMax(50);
    wxIntegerValidator<int> _ycenter(&__ycenter, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _ycenter.SetMin(-50);
    _ycenter.SetMax(50);

	//(*Initialize(BufferTransformProperties)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Buffer Transform Properties"), wxDefaultPosition, wxDefaultSize, wxCAPTION, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Rotations = new wxSlider(this, IDD_SLIDER_Rotations, 1, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Rotations"));
	FlexGridSizer2->Add(Slider_Rotations, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Rotations = new wxTextCtrl(this, ID_TEXTCTRL1, _("1"), wxDefaultPosition, wxSize(36,24), 0, _rotations, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Zooms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Zooms = new wxSlider(this, IDD_SLIDER_Zooms, 1, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Zooms"));
	FlexGridSizer2->Add(Slider_Zooms, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Zooms = new wxTextCtrl(this, ID_TEXTCTRL_Zooms, _("1"), wxDefaultPosition, wxSize(36,24), 0, _zooms, _T("ID_TEXTCTRL_Zooms"));
	FlexGridSizer2->Add(TextCtrl_Zooms, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Zoom Maximum"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_ZoomMaximum = new wxSlider(this, IDD_SLIDER_ZoomMaximum, 10, -50, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_ZoomMaximum"));
	FlexGridSizer2->Add(Slider_ZoomMaximum, 1, wxALL|wxEXPAND, 5);
	TextCtrl_ZoomMaximum = new wxTextCtrl(this, ID_TEXTCTRL_ZoomMaximum, _("10"), wxDefaultPosition, wxSize(36,24), 0, _zoommaximum, _T("ID_TEXTCTRL_ZoomMaximum"));
	FlexGridSizer2->Add(TextCtrl_ZoomMaximum, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("X Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_XCenter = new wxSlider(this, ID_SLIDER5, 0, -50, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER5"));
	FlexGridSizer2->Add(Slider_XCenter, 1, wxALL|wxEXPAND, 5);
	TextCtrl_XCenter = new wxTextCtrl(this, ID_TEXTCTRL_XCenter, _("0"), wxDefaultPosition, wxSize(36,24), 0, _xcenter, _T("ID_TEXTCTRL_XCenter"));
	FlexGridSizer2->Add(TextCtrl_XCenter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Y Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_YCenter = new wxSlider(this, ID_SLIDER4, 0, -50, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER4"));
	FlexGridSizer2->Add(Slider_YCenter, 1, wxALL|wxEXPAND, 5);
	TextCtrl_YCenter = new wxTextCtrl(this, ID_TEXTCTRL_YCenter, _("0"), wxDefaultPosition, wxSize(36,24), 0, _ycenter, _T("ID_TEXTCTRL_YCenter"));
	FlexGridSizer2->Add(TextCtrl_YCenter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(IDD_SLIDER_Rotations,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_RotationsCmdSliderUpdated);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_RotationsText);
	Connect(IDD_SLIDER_Zooms,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_ZoomsCmdSliderUpdated);
	Connect(ID_TEXTCTRL_Zooms,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_ZoomsText);
	Connect(IDD_SLIDER_ZoomMaximum,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_ZoomMaximumCmdSliderUpdated);
	Connect(ID_TEXTCTRL_ZoomMaximum,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_ZoomMaximumText);
	Connect(ID_SLIDER5,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_XCenterCmdSliderUpdated);
	Connect(ID_TEXTCTRL_XCenter,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_XCenterText);
	Connect(ID_SLIDER4,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_YCenterCmdSliderUpdated);
	Connect(ID_TEXTCTRL_YCenter,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_YCenterText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferTransformProperties::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferTransformProperties::OnButton_CancelClick);
	//*)
}

BufferTransformProperties::~BufferTransformProperties()
{
	//(*Destroy(BufferTransformProperties)
	//*)
}


void BufferTransformProperties::OnButton_OkClick(wxCommandEvent& event)
{
    _parms->ApplySettings(Slider_Rotations->GetValue(), Slider_Zooms->GetValue(), Slider_ZoomMaximum->GetValue(), Slider_XCenter->GetValue(), Slider_YCenter->GetValue());
    EndDialog(wxOK);
}

void BufferTransformProperties::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxCANCEL);
}

void BufferTransformProperties::OnSlider_RotationsCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Rotations->SetValue(wxString::Format("%d", Slider_Rotations->GetValue()));
}

void BufferTransformProperties::OnSlider_ZoomsCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Zooms->SetValue(wxString::Format("%d", Slider_Zooms->GetValue()));
}

void BufferTransformProperties::OnSlider_ZoomMaximumCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_ZoomMaximum->SetValue(wxString::Format("%d", Slider_ZoomMaximum->GetValue()));
}

void BufferTransformProperties::OnSlider_XCenterCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_XCenter->SetValue(wxString::Format("%d", Slider_XCenter->GetValue()));
}

void BufferTransformProperties::OnSlider_YCenterCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_YCenter->SetValue(wxString::Format("%d", Slider_YCenter->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_RotationsText(wxCommandEvent& event)
{
    Slider_Rotations->SetValue(wxAtoi(TextCtrl_Rotations->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_ZoomsText(wxCommandEvent& event)
{
    Slider_Zooms->SetValue(wxAtoi(TextCtrl_Zooms->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_ZoomMaximumText(wxCommandEvent& event)
{
    Slider_ZoomMaximum->SetValue(wxAtoi(TextCtrl_ZoomMaximum->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_XCenterText(wxCommandEvent& event)
{
    Slider_XCenter->SetValue(wxAtoi(TextCtrl_XCenter->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_YCenterText(wxCommandEvent& event)
{
    Slider_YCenter->SetValue(wxAtoi(TextCtrl_YCenter->GetValue()));
}
