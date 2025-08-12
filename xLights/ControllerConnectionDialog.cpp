/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerConnectionDialog.h"
#include "models/Model.h"

#include <wx/xml/xml.h>
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "Pixels.h"

//(*InternalHeaders(ControllerConnectionDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ControllerConnectionDialog)
const long ControllerConnectionDialog::ID_STATICTEXT1 = wxNewId();
const long ControllerConnectionDialog::ID_CHOICE1 = wxNewId();
const long ControllerConnectionDialog::ID_STATICTEXT2 = wxNewId();
const long ControllerConnectionDialog::ID_SPINCTRL1 = wxNewId();
const long ControllerConnectionDialog::ID_STATICTEXT3 = wxNewId();
const long ControllerConnectionDialog::ID_CHOICE2 = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX1 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_reverse = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX5 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_colorOrder = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX2 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_startNullNodes = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX7 = wxNewId();
const long ControllerConnectionDialog::ID_SPINCTRL_EndNullNodes = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX3 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_brightness = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX4 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_gamma = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX6 = wxNewId();
const long ControllerConnectionDialog::ID_SPINCTRL2 = wxNewId();
const long ControllerConnectionDialog::ID_BUTTON1 = wxNewId();
const long ControllerConnectionDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ControllerConnectionDialog,wxDialog)
	//(*EventTable(ControllerConnectionDialog)
	//*)
END_EVENT_TABLE()

ControllerConnectionDialog::ControllerConnectionDialog(wxWindow* parent, controller_connection_bulkedit type, wxWindowID id, const wxPoint& pos, const wxSize& size) {
    _type = type;

	//(*Initialize(ControllerConnectionDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxSpinCtrl* GammaPlaceHolder;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Protocol"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Protocol = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Protocol->SetFocus();
	FlexGridSizer2->Add(Choice_Protocol, 1, wxALL|wxEXPAND, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Port = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Port->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_Port, 1, wxALL|wxEXPAND, 2);
	SmartRemote = new wxStaticText(this, ID_STATICTEXT3, _("Smart Remote"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(SmartRemote, 1, wxALL|wxEXPAND, 5);
	Choice_SmartRemote = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer2->Add(Choice_SmartRemote, 1, wxALL|wxEXPAND, 5);
	CheckBox_PixelDirection = new wxCheckBox(this, ID_CHECKBOX1, _("Set Direction"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_PixelDirection->SetValue(false);
	FlexGridSizer2->Add(CheckBox_PixelDirection, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PixelDirection = new wxChoice(this, ID_PIXEL_reverse, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_PIXEL_reverse"));
	PixelDirection->SetSelection( PixelDirection->Append(_("Forward")) );
	PixelDirection->Append(_("Reverse"));
	PixelDirection->Disable();
	FlexGridSizer2->Add(PixelDirection, 1, wxALL|wxEXPAND, 2);
	CheckBox_ColorOrder = new wxCheckBox(this, ID_CHECKBOX5, _("Set Color Order"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_ColorOrder->SetValue(false);
	FlexGridSizer2->Add(CheckBox_ColorOrder, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ColorOrder = new wxChoice(this, ID_PIXEL_colorOrder, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_PIXEL_colorOrder"));
	ColorOrder->SetSelection( ColorOrder->Append(_("RGB")) );
	ColorOrder->Append(_("RBG"));
	ColorOrder->Append(_("GBR"));
	ColorOrder->Append(_("GRB"));
	ColorOrder->Append(_("BGR"));
	ColorOrder->Append(_("BRG"));
	ColorOrder->Append(_("RGBW"));
	ColorOrder->Append(_("RBGW"));
	ColorOrder->Append(_("GBRW"));
	ColorOrder->Append(_("GRBW"));
	ColorOrder->Append(_("BGRW"));
	ColorOrder->Append(_("BRGW"));
	ColorOrder->Append(_("WRGB"));
	ColorOrder->Append(_("WRBG"));
	ColorOrder->Append(_("WGBR"));
	ColorOrder->Append(_("WGRB"));
	ColorOrder->Append(_("WBGR"));
	ColorOrder->Append(_("WBRG"));
	ColorOrder->Disable();
	FlexGridSizer2->Add(ColorOrder, 1, wxALL|wxEXPAND, 2);
	CheckBox_StartNullNodes = new wxCheckBox(this, ID_CHECKBOX2, _("Set Start Null Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_StartNullNodes->SetValue(false);
	FlexGridSizer2->Add(CheckBox_StartNullNodes, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StartNullNodes = new wxSpinCtrl(this, ID_PIXEL_startNullNodes, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 25, 0, _T("ID_PIXEL_startNullNodes"));
	StartNullNodes->SetValue(_T("0"));
	StartNullNodes->Disable();
	FlexGridSizer2->Add(StartNullNodes, 1, wxALL|wxEXPAND, 2);
	CheckBox_EndNullNodes = new wxCheckBox(this, ID_CHECKBOX7, _("Set End Null Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
	CheckBox_EndNullNodes->SetValue(false);
	FlexGridSizer2->Add(CheckBox_EndNullNodes, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	EndNullNodes = new wxSpinCtrl(this, ID_SPINCTRL_EndNullNodes, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 25, 0, _T("ID_SPINCTRL_EndNullNodes"));
	EndNullNodes->SetValue(_T("0"));
	EndNullNodes->Disable();
	FlexGridSizer2->Add(EndNullNodes, 1, wxALL|wxEXPAND, 2);
	CheckBox_Brightness = new wxCheckBox(this, ID_CHECKBOX3, _("Set Brightness"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_Brightness->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Brightness, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Brightness = new wxSpinCtrl(this, ID_PIXEL_brightness, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 100, _T("ID_PIXEL_brightness"));
	Brightness->SetValue(_T("100"));
	Brightness->Disable();
	FlexGridSizer2->Add(Brightness, 1, wxALL|wxEXPAND, 2);
	CheckBox_Gamma = new wxCheckBox(this, ID_CHECKBOX4, _("Set Gamma"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_Gamma->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Gamma, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	GammaPlaceHolder = new wxSpinCtrl(this, ID_PIXEL_gamma, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 10, 0, _T("ID_PIXEL_gamma"));
	GammaPlaceHolder->Disable();
	FlexGridSizer2->Add(GammaPlaceHolder, 1, wxALL|wxEXPAND, 2);
	CheckBox_GroupCount = new wxCheckBox(this, ID_CHECKBOX6, _("Group Count"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_GroupCount->SetValue(false);
	FlexGridSizer2->Add(CheckBox_GroupCount, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	GroupCount = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL2"));
	GroupCount->SetValue(_T("1"));
	GroupCount->Disable();
	FlexGridSizer2->Add(GroupCount, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ControllerConnectionDialog::OnChoice_ProtocolSelect);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnPixelDirectionClick);
	Connect(ID_CHECKBOX5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnCheckBox_ColorOrderClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnCheckBox_NullNodesClick);
	Connect(ID_CHECKBOX7,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnCheckBox_EndNullNodesClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnCheckBox_BrightnessClick);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnCheckBox_GammaClick);
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnCheckBox_GroupCountClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnButton_CancelClick);
	//*)

    Gamma = new wxSpinCtrlDouble(this, ID_PIXEL_gamma, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0.1, 5.0, 1.0, 0.1, _T("ID_PIXEL_gamma"));
    Gamma->Disable();
    FlexGridSizer2->Replace(GammaPlaceHolder, Gamma);
    GammaPlaceHolder->Destroy();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Choice_Protocol->AppendString("");
    auto protocols = GetAllPixelTypes(true, true);
    for (auto it = protocols.begin(); it != protocols.end(); ++it) {
        Choice_Protocol->AppendString(wxString(it->c_str()));
    }
    Choice_Protocol->SetSelection(1);

    SetEscapeId(Button_Cancel->GetId());
    Button_Ok->SetDefault();
    Choice_Protocol->SetFocus();

    switch(type)
    {
    case controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION:
    case controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT:
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERBRIGHTNESS:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERSTARTNULLNODES:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERENDNULLNODES:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        break;
    case controller_connection_bulkedit::CEBE_SMARTREMOTE:
    case controller_connection_bulkedit::CEBE_SMARTREMOTETYPE:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERGAMMA:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERCOLOURORDER:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERGROUPCOUNT:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERDIRECTION:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERPROTOCOL:
        StaticText2->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        StartNullNodes->Hide();
        CheckBox_StartNullNodes->Hide();
        EndNullNodes->Hide();
        CheckBox_EndNullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        SmartRemote->Hide();
        Choice_SmartRemote->Hide();
        break;
    }


    Layout();
    Fit();
}

ControllerConnectionDialog::~ControllerConnectionDialog()
{
	//(*Destroy(ControllerConnectionDialog)
	//*)
}


void ControllerConnectionDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void ControllerConnectionDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ControllerConnectionDialog::Set(Model* m) {
    if (m != nullptr) {
        auto node = m->GetControllerConnection();
        if (node != nullptr) {
            _protocol = node->GetAttribute("Protocol", "ws2811");
            if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION ||
                _type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT ||
                _type == controller_connection_bulkedit::CEBE_CONTROLLERPROTOCOL) {
                _protocol.UpperCase();
                Choice_Protocol->SetStringSelection(_protocol);
            }

            if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION ||
                _type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT) {
                SpinCtrl_Port->SetValue(wxAtoi(node->GetAttribute("Port", "1")));
            }

            if (IsPixelProtocol(_protocol)) {
                if (_type == controller_connection_bulkedit::CEBE_CONTROLLERBRIGHTNESS) {
                    if (node->HasAttribute("brightness")) {
                        CheckBox_Brightness->SetValue(true);
                        Brightness->Enable(true);
                        Brightness->SetValue(wxAtoi(node->GetAttribute("brightness")));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_CONTROLLERGAMMA) {
                    if (node->HasAttribute("gamma")) {
                        CheckBox_Gamma->SetValue(true);
                        Gamma->Enable(true);
                        Gamma->SetValue(wxAtof(node->GetAttribute("gamma")));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_CONTROLLERSTARTNULLNODES) {
                    if (node->HasAttribute("nullNodes")) {
                        CheckBox_StartNullNodes->SetValue(true);
                        StartNullNodes->Enable(true);
                        StartNullNodes->SetValue(wxAtoi(node->GetAttribute("nullNodes")));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_CONTROLLERENDNULLNODES) {
                    if (node->HasAttribute("endNullNodes")) {
                        CheckBox_EndNullNodes->SetValue(true);
                        EndNullNodes->Enable(true);
                        EndNullNodes->SetValue(wxAtoi(node->GetAttribute("endNullNodes")));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCOLOURORDER) {
                    if (node->HasAttribute("colorOrder")) {
                        CheckBox_ColorOrder->SetValue(true);
                        ColorOrder->Enable(true);
                        ColorOrder->SetStringSelection(node->GetAttribute("colorOrder"));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_CONTROLLERDIRECTION) {
                    if (node->HasAttribute("reverse")) {
                        CheckBox_PixelDirection->SetValue(true);
                        PixelDirection->Enable(true);
                        PixelDirection->SetSelection(wxAtoi(node->GetAttribute("reverse")));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_CONTROLLERGROUPCOUNT) {
                    if (node->HasAttribute("groupCount")) {
                        CheckBox_GroupCount->SetValue(true);
                        GroupCount->Enable(true);
                        GroupCount->SetValue(wxAtoi(node->GetAttribute("groupCount")));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_SMARTREMOTE) {

                    Choice_SmartRemote->AppendString("None");
                    int smartRemoteCount = m->GetSmartRemoteCount();
                    for (int i = 0; i < smartRemoteCount; i++) {
                        Choice_SmartRemote->AppendString(wxString(char(65 + i)));
                    }
                    Choice_SmartRemote->SetSelection(0);

                    if (node->HasAttribute("SmartRemote")) {
                        Choice_SmartRemote->SetSelection(wxAtoi(node->GetAttribute("SmartRemote")));
                    }
                }
                if (_type == controller_connection_bulkedit::CEBE_SMARTREMOTETYPE) {

                    SmartRemote->SetLabelText("SmartRemote Type");
                    for (auto const& type: m->GetSmartRemoteTypes()) {
                        Choice_SmartRemote->AppendString(type);
                    }
                    Choice_SmartRemote->SetSelection(0);

                    if (node->HasAttribute("SmartRemoteType")) {
                        Choice_SmartRemote->SetStringSelection(node->GetAttribute("SmartRemoteType"));
                    }
                }
            }
            if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION ||
                _type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT) {
                Choice_Protocol->SetFocus();
                Choice_Protocol->SetFocusFromKbd();
                SpinCtrl_Port->SetFocus();
                SpinCtrl_Port->SetFocusFromKbd();
            }
            if (_type == controller_connection_bulkedit::CEBE_CONTROLLERPROTOCOL) {
                Choice_Protocol->SetFocus();
                Choice_Protocol->SetFocusFromKbd();
            }
        }
    }
}

void ControllerConnectionDialog::Get(wxXmlNode* node, int strings) {
    if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION ||
        _type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT) {
        _protocol = Choice_Protocol->GetStringSelection().ToStdString();
        node->DeleteAttribute("Protocol");
        node->AddAttribute("Protocol", _protocol);
        node->DeleteAttribute("Port");
        node->AddAttribute("Port", wxString::Format("%d", SpinCtrl_Port->GetValue()));
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT) {
            SpinCtrl_Port->SetValue(SpinCtrl_Port->GetValue() + strings);
        }
    }

    if (_type == controller_connection_bulkedit::CEBE_CONTROLLERPROTOCOL) {
        _protocol = Choice_Protocol->GetStringSelection().ToStdString();
        node->DeleteAttribute("Protocol");
        node->AddAttribute("Protocol", _protocol);
    }

    if (IsPixelProtocol(_protocol)) {
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERBRIGHTNESS) {
            node->DeleteAttribute("brightness");
            if (CheckBox_Brightness->IsChecked()) node->AddAttribute("brightness", wxString::Format("%d", Brightness->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERGAMMA) {
            node->DeleteAttribute("gamma");
            if (CheckBox_Gamma->IsChecked()) node->AddAttribute("gamma", wxString::Format("%f", Gamma->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERSTARTNULLNODES) {
            node->DeleteAttribute("nullNodes");
            if (CheckBox_StartNullNodes->IsChecked()) node->AddAttribute("nullNodes", wxString::Format("%d", StartNullNodes->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERENDNULLNODES) {
            node->DeleteAttribute("endNullNodes");
            if (CheckBox_EndNullNodes->IsChecked()) node->AddAttribute("endNullNodes", wxString::Format("%d", EndNullNodes->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCOLOURORDER) {
            node->DeleteAttribute("colorOrder");
            if (CheckBox_ColorOrder->IsChecked()) node->AddAttribute("colorOrder", ColorOrder->GetStringSelection());
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERGROUPCOUNT) {
            node->DeleteAttribute("groupCount");
            if (CheckBox_GroupCount->IsChecked()) node->AddAttribute("groupCount", wxString::Format("%d", GroupCount->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_SMARTREMOTE) {
            node->DeleteAttribute("SmartRemote");
            node->AddAttribute("SmartRemote", wxString::Format("%d", Choice_SmartRemote->GetSelection()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERDIRECTION) {
            node->DeleteAttribute("reverse");
            if (CheckBox_PixelDirection->IsChecked()) node->AddAttribute("reverse", wxString::Format("%d", PixelDirection->GetSelection()));
        }

        if (_type == controller_connection_bulkedit::CEBE_SMARTREMOTETYPE) {
            node->DeleteAttribute("SmartRemoteType");
            node->AddAttribute("SmartRemoteType", Choice_SmartRemote->GetStringSelection());
        }
    }
//    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnectionDialog::Get");
//    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnectionDialog::Get");
//    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ControllerConnectionDialog::Get");
//    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnectionDialog::Get");
}

void ControllerConnectionDialog::OnPixelDirectionClick(wxCommandEvent& event)
{
    PixelDirection->Enable(CheckBox_PixelDirection->IsChecked());
}

void ControllerConnectionDialog::OnCheckBox_NullNodesClick(wxCommandEvent& event)
{
    StartNullNodes->Enable(CheckBox_StartNullNodes->IsChecked());
}

void ControllerConnectionDialog::OnCheckBox_GammaClick(wxCommandEvent& event)
{
    Gamma->Enable(CheckBox_Gamma->IsChecked());
}

void ControllerConnectionDialog::OnCheckBox_BrightnessClick(wxCommandEvent& event)
{
    Brightness->Enable(CheckBox_Brightness->IsChecked());
}

void ControllerConnectionDialog::OnCheckBox_ColorOrderClick(wxCommandEvent& event)
{
    ColorOrder->Enable(CheckBox_ColorOrder->IsChecked());
}

void ControllerConnectionDialog::OnChoice_ProtocolSelect(wxCommandEvent& event)
{
    _protocol = Choice_Protocol->GetStringSelection().ToStdString();
}

void ControllerConnectionDialog::OnCheckBox_GroupCountClick(wxCommandEvent& event)
{
    GroupCount->Enable(CheckBox_GroupCount->IsChecked());
}

void ControllerConnectionDialog::OnCheckBox_EndNullNodesClick(wxCommandEvent& event)
{
    EndNullNodes->Enable(CheckBox_EndNullNodes->IsChecked());
}
