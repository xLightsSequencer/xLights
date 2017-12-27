#include "ServoPanel.h"
#include "ServoEffect.h"
#include "EffectPanelUtils.h"

//(*InternalHeaders(ServoPanel)
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

//(*IdInit(ServoPanel)
const long ServoPanel::ID_STATICTEXT_Channel = wxNewId();
const long ServoPanel::ID_CHOICE_Channel = wxNewId();
const long ServoPanel::ID_CHECKBOX_16bit = wxNewId();
const long ServoPanel::ID_STATICTEXT_Servo = wxNewId();
const long ServoPanel::IDD_SLIDER_Servo = wxNewId();
const long ServoPanel::ID_VALUECURVE_Servo = wxNewId();
const long ServoPanel::ID_TEXTCTRL_Servo = wxNewId();
//*)

BEGIN_EVENT_TABLE(ServoPanel,wxPanel)
	//(*EventTable(ServoPanel)
	//*)
END_EVENT_TABLE()

ServoPanel::ServoPanel(wxWindow* parent)
{
	//(*Initialize(ServoPanel)
	wxFlexGridSizer* FlexGridSizer2;
	BulkEditTextCtrl* TextCtrl_Servo;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer_Main = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer_Main->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Channel, _("Base Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Channel"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Channel = new BulkEditChoice(this, ID_CHOICE_Channel, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Channel"));
	FlexGridSizer2->Add(Choice_Channel, 1, wxALL, 5);
	CheckBox_16bit = new BulkEditCheckBox(this, ID_CHECKBOX_16bit, _("16 bit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_16bit"));
	CheckBox_16bit->SetValue(true);
	FlexGridSizer2->Add(CheckBox_16bit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer_Main->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	Label_DMX1 = new wxStaticText(this, ID_STATICTEXT_Servo, _("Value:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Servo"));
	FlexGridSizer1->Add(Label_DMX1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Servo = new BulkEditSlider(this, IDD_SLIDER_Servo, 0, 0, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Servo"));
	FlexGridSizer1->Add(Slider_Servo, 1, wxALL|wxEXPAND, 2);
	ValueCurve_Servo = new BulkEditValueCurveButton(this, ID_VALUECURVE_Servo, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Servo"));
	FlexGridSizer1->Add(ValueCurve_Servo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Servo = new BulkEditTextCtrl(this, ID_TEXTCTRL_Servo, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Servo"));
	FlexGridSizer1->Add(TextCtrl_Servo, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer_Main->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer_Main);
	FlexGridSizer_Main->Fit(this);
	FlexGridSizer_Main->SetSizeHints(this);

	Connect(ID_VALUECURVE_Servo,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ServoPanel::OnVCButtonClick);
	//*)

    SetName("ID_PANEL_SERVO");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ServoPanel::OnVCChanged, nullptr, this);
    
    ValueCurve_Servo->GetValue()->SetLimits(SERVO_MIN, SERVO_MAX);

}

ServoPanel::~ServoPanel()
{
	//(*Destroy(ServoPanel)
	//*)
}

PANEL_EVENT_HANDLERS(ServoPanel)
