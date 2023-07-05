#include "MovingHeadPanel.h"
#include "MovingHeadEffect.h"

//(*InternalHeaders(MovingHeadPanel)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MovingHeadPanel)
const long MovingHeadPanel::ID_STATICTEXT_Pan = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_Pan = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_Pan = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_Pan = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_Tilt = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_Tilt = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_Tilt = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_Tilt = wxNewId();
//*)

BEGIN_EVENT_TABLE(MovingHeadPanel,wxPanel)
	//(*EventTable(MovingHeadPanel)
	//*)
END_EVENT_TABLE()

MovingHeadPanel::MovingHeadPanel(wxWindow* parent) : xlEffectPanel(parent)
{
    //(*Initialize(MovingHeadPanel)
    BulkEditTextCtrlF1* TextCtrl_Pan;
    BulkEditTextCtrlF1* TextCtrl_Tilt;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer_Main = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Main->AddGrowableCol(0);
    FlexGridSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer1->AddGrowableCol(1);
    Label_Pan = new wxStaticText(this, ID_STATICTEXT_Pan, _("Pan (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pan"));
    FlexGridSizer1->Add(Label_Pan, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Pan = new BulkEditSliderF1(this, IDD_SLIDER_Pan, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Pan"));
    FlexGridSizer1->Add(Slider_Pan, 1, wxALL|wxEXPAND, 2);
    ValueCurve_Pan = new BulkEditValueCurveButton(this, ID_VALUECURVE_Pan, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Pan"));
    FlexGridSizer1->Add(ValueCurve_Pan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Pan = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Pan, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Pan"));
    FlexGridSizer1->Add(TextCtrl_Pan, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer_Main->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    Label_Tilt = new wxStaticText(this, ID_STATICTEXT_Tilt, _("Tilt (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Tilt"));
    FlexGridSizer2->Add(Label_Tilt, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Tilt = new BulkEditSliderF1(this, IDD_SLIDER_Tilt, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tilt"));
    FlexGridSizer2->Add(Slider_Tilt, 1, wxALL|wxEXPAND, 2);
    ValueCurve_Tilt = new BulkEditValueCurveButton(this, ID_VALUECURVE_Tilt, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Tilt"));
    FlexGridSizer2->Add(ValueCurve_Tilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Tilt = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Tilt, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Tilt"));
    FlexGridSizer2->Add(TextCtrl_Tilt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer_Main->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer_Main);
    FlexGridSizer_Main->Fit(this);
    FlexGridSizer_Main->SetSizeHints(this);
    //*)
    
    SetName("ID_PANEL_MOVINGHEAD");

    Connect(ID_VALUECURVE_Pan,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Tilt,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&MovingHeadPanel::OnVCChanged, nullptr, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&MovingHeadPanel::OnValidateWindow, nullptr, this);

    ValueCurve_Pan->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_Pan->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);

    ValueCurve_Tilt->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_Tilt->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);

    ValidateWindow();
}

MovingHeadPanel::~MovingHeadPanel()
{
	//(*Destroy(MovingHeadPanel)
	//*)
}

void MovingHeadPanel::ValidateWindow()
{
}

