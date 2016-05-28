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
const long BufferTransformProperties::ID_STATICTEXT8 = wxNewId();
const long BufferTransformProperties::ID_SLIDER3 = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL4 = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT7 = wxNewId();
const long BufferTransformProperties::ID_SLIDER2 = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL3 = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT4 = wxNewId();
const long BufferTransformProperties::IDD_SLIDER_ZoomMaximum = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL_ZoomMaximum = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT5 = wxNewId();
const long BufferTransformProperties::ID_SLIDER5 = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL_XCenter = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT6 = wxNewId();
const long BufferTransformProperties::ID_SLIDER4 = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL_YCenter = wxNewId();
const long BufferTransformProperties::ID_STATICTEXT3 = wxNewId();
const long BufferTransformProperties::ID_SLIDER1 = wxNewId();
const long BufferTransformProperties::ID_TEXTCTRL2 = wxNewId();
const long BufferTransformProperties::ID_BUTTON1 = wxNewId();
const long BufferTransformProperties::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BufferTransformProperties,wxDialog)
	//(*EventTable(BufferTransformProperties)
	//*)
END_EVENT_TABLE()

BufferTransformProperties::BufferTransformProperties(wxWindow* parent,RotoZoomParms* parms, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _backup = *parms;
    __rotations = parms->GetRotations();
    __zooms = parms->GetZooms();
    __start = parms->GetStart();
    __zoomminimum = parms->GetZoomMinimum();
    __zoommaximum = parms->GetZoomMaximum();
    __quality = parms->GetQuality();
    __xcenter = parms->GetXCenter();
    __ycenter = parms->GetYCenter();
    _parms = parms;
    wxIntegerValidator<int> _rotations(&__rotations, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _rotations.SetMin(0);
    _rotations.SetMax(200);
    wxIntegerValidator<int> _zooms(&__zooms, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _zooms.SetMin(0);
    _zooms.SetMax(100);
    wxIntegerValidator<int> _start(&__start, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _start.SetMin(0);
    _start.SetMax(100);
    wxIntegerValidator<int> _zoomminimum(&__zoomminimum, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _zoomminimum.SetMin(0);
    _zoomminimum.SetMax(10);
    wxIntegerValidator<int> _zoommaximum(&__zoommaximum, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _zoommaximum.SetMin(10);
    _zoommaximum.SetMax(30);
    wxIntegerValidator<int> _quality(&__quality, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _quality.SetMin(0);
    _quality.SetMax(100);
    wxIntegerValidator<int> _xcenter(&__xcenter, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _xcenter.SetMin(0);
    _xcenter.SetMax(100);
    wxIntegerValidator<int> _ycenter(&__ycenter, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _ycenter.SetMin(0);
    _ycenter.SetMax(100);

	//(*Initialize(BufferTransformProperties)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Buffer Transform Properties"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Rotations = new wxSlider(this, IDD_SLIDER_Rotations, 10, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Rotations"));
	FlexGridSizer2->Add(Slider_Rotations, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Rotations = new wxTextCtrl(this, ID_TEXTCTRL1, _("10"), wxDefaultPosition, wxSize(36,24), 0, _rotations, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Zooms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Zooms = new wxSlider(this, IDD_SLIDER_Zooms, 10, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Zooms"));
	FlexGridSizer2->Add(Slider_Zooms, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Zooms = new wxTextCtrl(this, ID_TEXTCTRL_Zooms, _("10"), wxDefaultPosition, wxSize(36,24), 0, _zooms, _T("ID_TEXTCTRL_Zooms"));
	FlexGridSizer2->Add(TextCtrl_Zooms, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Start = new wxSlider(this, ID_SLIDER3, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER3"));
	FlexGridSizer2->Add(Slider_Start, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Start = new wxTextCtrl(this, ID_TEXTCTRL4, _("0"), wxDefaultPosition, wxSize(36,24), 0, _start, _T("ID_TEXTCTRL4"));
	FlexGridSizer2->Add(TextCtrl_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Zoom Minimum"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_ZoomMinimum = new wxSlider(this, ID_SLIDER2, 1, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER2"));
	FlexGridSizer2->Add(Slider_ZoomMinimum, 1, wxALL|wxEXPAND, 2);
	TextCtrl_ZoomMinimum = new wxTextCtrl(this, ID_TEXTCTRL3, _("10"), wxDefaultPosition, wxSize(36,24), 0, _zoomminimum, _T("ID_TEXTCTRL3"));
	FlexGridSizer2->Add(TextCtrl_ZoomMinimum, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Zoom Maximum"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_ZoomMaximum = new wxSlider(this, IDD_SLIDER_ZoomMaximum, 20, 10, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_ZoomMaximum"));
	FlexGridSizer2->Add(Slider_ZoomMaximum, 1, wxALL|wxEXPAND, 2);
	TextCtrl_ZoomMaximum = new wxTextCtrl(this, ID_TEXTCTRL_ZoomMaximum, _("20"), wxDefaultPosition, wxSize(36,24), 0, _zoommaximum, _T("ID_TEXTCTRL_ZoomMaximum"));
	FlexGridSizer2->Add(TextCtrl_ZoomMaximum, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("X Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_XCenter = new wxSlider(this, ID_SLIDER5, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER5"));
	FlexGridSizer2->Add(Slider_XCenter, 1, wxALL|wxEXPAND, 2);
	TextCtrl_XCenter = new wxTextCtrl(this, ID_TEXTCTRL_XCenter, _("50"), wxDefaultPosition, wxSize(36,24), 0, _xcenter, _T("ID_TEXTCTRL_XCenter"));
	FlexGridSizer2->Add(TextCtrl_XCenter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Y Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_YCenter = new wxSlider(this, ID_SLIDER4, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER4"));
	FlexGridSizer2->Add(Slider_YCenter, 1, wxALL|wxEXPAND, 2);
	TextCtrl_YCenter = new wxTextCtrl(this, ID_TEXTCTRL_YCenter, _("50"), wxDefaultPosition, wxSize(36,24), 0, _ycenter, _T("ID_TEXTCTRL_YCenter"));
	FlexGridSizer2->Add(TextCtrl_YCenter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Quality"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Quality = new wxSlider(this, ID_SLIDER1, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
	Slider_Quality->SetToolTip(_("Keep qiality as low as possible as this can signigicantly impact render time."));
	FlexGridSizer2->Add(Slider_Quality, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Quality = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxSize(36,24), 0, _quality, _T("ID_TEXTCTRL2"));
	TextCtrl_Quality->SetToolTip(_("Keep qiality as low as possible as this can signigicantly impact render time."));
	FlexGridSizer2->Add(TextCtrl_Quality, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(IDD_SLIDER_Rotations,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_RotationsCmdSliderUpdated);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_RotationsText);
	Connect(IDD_SLIDER_Zooms,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_ZoomsCmdSliderUpdated);
	Connect(ID_TEXTCTRL_Zooms,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_ZoomsText);
	Connect(ID_SLIDER3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_StartCmdSliderUpdated);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_StartText);
	Connect(ID_SLIDER2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_ZoomMinimumCmdSliderUpdated);
	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_ZoomMinimumText);
	Connect(IDD_SLIDER_ZoomMaximum,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_ZoomMaximumCmdSliderUpdated);
	Connect(ID_TEXTCTRL_ZoomMaximum,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_ZoomMaximumText);
	Connect(ID_SLIDER5,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_XCenterCmdSliderUpdated);
	Connect(ID_TEXTCTRL_XCenter,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_XCenterText);
	Connect(ID_SLIDER4,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_YCenterCmdSliderUpdated);
	Connect(ID_TEXTCTRL_YCenter,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_YCenterText);
	Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnSlider_QualityCmdSliderUpdated);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferTransformProperties::OnTextCtrl_QualityText);
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
    //_parms->ApplySettings(Slider_Rotations->GetValue(), Slider_Zooms->GetValue(), Slider_ZoomMinimum->GetValue(), Slider_ZoomMaximum->GetValue(), Slider_Quality->GetValue(), Slider_XCenter->GetValue(), Slider_YCenter->GetValue());
    EndDialog(wxOK);
}

void BufferTransformProperties::OnButton_CancelClick(wxCommandEvent& event)
{
    *_parms = _backup;
    EndDialog(wxCANCEL);
}

void BufferTransformProperties::OnSlider_RotationsCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Rotations->SetValue(wxString::Format("%d", Slider_Rotations->GetValue()));
    _parms->SetRotations(Slider_Rotations->GetValue());
}

void BufferTransformProperties::OnSlider_ZoomsCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Zooms->SetValue(wxString::Format("%d", Slider_Zooms->GetValue()));
    _parms->SetZooms(Slider_Zooms->GetValue());
}

void BufferTransformProperties::OnSlider_ZoomMinimumCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_ZoomMinimum->SetValue(wxString::Format("%d", Slider_ZoomMinimum->GetValue()));
    _parms->SetZoomMinimum(Slider_ZoomMinimum->GetValue());
}

void BufferTransformProperties::OnSlider_ZoomMaximumCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_ZoomMaximum->SetValue(wxString::Format("%d", Slider_ZoomMaximum->GetValue()));
    _parms->SetZoomMaximum(Slider_ZoomMaximum->GetValue());
}

void BufferTransformProperties::OnSlider_QualityCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Quality->SetValue(wxString::Format("%d", Slider_Quality->GetValue()));
    _parms->SetQuality(Slider_Quality->GetValue());
}

void BufferTransformProperties::OnSlider_XCenterCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_XCenter->SetValue(wxString::Format("%d", Slider_XCenter->GetValue()));
    _parms->SetXCenter(Slider_XCenter->GetValue());
}

void BufferTransformProperties::OnSlider_YCenterCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_YCenter->SetValue(wxString::Format("%d", Slider_YCenter->GetValue()));
    _parms->SetYCenter(Slider_YCenter->GetValue());
}

void BufferTransformProperties::OnSlider_StartCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Start->SetValue(wxString::Format("%d", Slider_Start->GetValue()));
    _parms->SetStart(Slider_Start->GetValue());
}

void BufferTransformProperties::OnTextCtrl_RotationsText(wxCommandEvent& event)
{
    Slider_Rotations->SetValue(wxAtoi(TextCtrl_Rotations->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_ZoomsText(wxCommandEvent& event)
{
    Slider_Zooms->SetValue(wxAtoi(TextCtrl_Zooms->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_ZoomMinimumText(wxCommandEvent& event)
{
    Slider_ZoomMinimum->SetValue(wxAtoi(TextCtrl_ZoomMinimum->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_ZoomMaximumText(wxCommandEvent& event)
{
    Slider_ZoomMaximum->SetValue(wxAtoi(TextCtrl_ZoomMaximum->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_QualityText(wxCommandEvent& event)
{
    Slider_Quality->SetValue(wxAtoi(TextCtrl_Quality->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_XCenterText(wxCommandEvent& event)
{
    Slider_XCenter->SetValue(wxAtoi(TextCtrl_XCenter->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_YCenterText(wxCommandEvent& event)
{
    Slider_YCenter->SetValue(wxAtoi(TextCtrl_YCenter->GetValue()));
}

void BufferTransformProperties::OnTextCtrl_StartText(wxCommandEvent& event)
{
    Slider_Start->SetValue(wxAtoi(TextCtrl_Start->GetValue()));
}
