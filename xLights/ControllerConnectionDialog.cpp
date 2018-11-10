#include "ControllerConnectionDialog.h"
#include "models/Model.h"

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"
#include <wx/xml/xml.h>
#include "xLightsApp.h"
#include "xLightsMain.h"

//(*InternalHeaders(ControllerConnectionDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ControllerConnectionDialog)
const long ControllerConnectionDialog::ID_STATICTEXT1 = wxNewId();
const long ControllerConnectionDialog::ID_CHOICE1 = wxNewId();
const long ControllerConnectionDialog::ID_STATICTEXT2 = wxNewId();
const long ControllerConnectionDialog::ID_SPINCTRL1 = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX1 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_reverse = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX5 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_colorOrder = wxNewId();
const long ControllerConnectionDialog::ID_CHECKBOX2 = wxNewId();
const long ControllerConnectionDialog::ID_PIXEL_nullNodes = wxNewId();
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

ControllerConnectionDialog::ControllerConnectionDialog(wxWindow* parent, controller_connection_bulkedit type, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
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
	CheckBox_NullNodes = new wxCheckBox(this, ID_CHECKBOX2, _("Set Null Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_NullNodes->SetValue(false);
	FlexGridSizer2->Add(CheckBox_NullNodes, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NullNodes = new wxSpinCtrl(this, ID_PIXEL_nullNodes, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 25, 0, _T("ID_PIXEL_nullNodes"));
	NullNodes->SetValue(_T("0"));
	NullNodes->Disable();
	FlexGridSizer2->Add(NullNodes, 1, wxALL|wxEXPAND, 2);
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
    auto protocols = Model::GetProtocols();
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
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        NullNodes->Hide();
        CheckBox_NullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERBRIGHTNESS:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        PixelDirection->Hide();
        CheckBox_PixelDirection->Hide();
        NullNodes->Hide();
        CheckBox_NullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERNULLNODES:
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
        NullNodes->Hide();
        CheckBox_NullNodes->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
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
        NullNodes->Hide();
        CheckBox_NullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
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
        NullNodes->Hide();
        CheckBox_NullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        break;
    case controller_connection_bulkedit::CEBE_CONTROLLERDIRECTION:
        StaticText1->Hide();
        StaticText2->Hide();
        Choice_Protocol->Hide();
        SpinCtrl_Port->Hide();
        Brightness->Hide();
        CheckBox_Brightness->Hide();
        NullNodes->Hide();
        CheckBox_NullNodes->Hide();
        Gamma->Hide();
        CheckBox_Gamma->Hide();
        ColorOrder->Hide();
        CheckBox_ColorOrder->Hide();
        GroupCount->Hide();
        CheckBox_GroupCount->Hide();
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

void ControllerConnectionDialog::Set(wxXmlNode* node) {
    if (node != nullptr) {

        _protocol = node->GetAttribute("Protocol", "WS2811");
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION) {
            _protocol.UpperCase();
            Choice_Protocol->SetStringSelection(_protocol);
            SpinCtrl_Port->SetValue(wxAtoi(node->GetAttribute("Port", "1")));
        }

        if (Model::IsPixelProtocol(_protocol)) {
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
            if (_type == controller_connection_bulkedit::CEBE_CONTROLLERNULLNODES) {
                if (node->HasAttribute("nullNodes")) {
                    CheckBox_NullNodes->SetValue(true);
                    NullNodes->Enable(true);
                    NullNodes->SetValue(wxAtoi(node->GetAttribute("nullNodes")));
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
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION) {
            Choice_Protocol->SetFocus();
            Choice_Protocol->SetFocusFromKbd();
            SpinCtrl_Port->SetFocus();
            SpinCtrl_Port->SetFocusFromKbd();
        }
    }
}

void ControllerConnectionDialog::Get(wxXmlNode* node) {
    if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION) {
        _protocol = Choice_Protocol->GetStringSelection().ToStdString();
        node->DeleteAttribute("Protocol");
        node->AddAttribute("Protocol", _protocol);
        node->DeleteAttribute("Port");
        node->AddAttribute("Port", wxString::Format("%d", SpinCtrl_Port->GetValue()));
    }

    if (Model::IsPixelProtocol(_protocol)) {
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERBRIGHTNESS) {
            node->DeleteAttribute("brightness");
            if (CheckBox_Brightness->IsChecked()) node->AddAttribute("brightness", wxString::Format("%d", Brightness->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERGAMMA) {
            node->DeleteAttribute("gamma");
            if (CheckBox_Gamma->IsChecked()) node->AddAttribute("gamma", wxString::Format("%f", Gamma->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERNULLNODES) {
            node->DeleteAttribute("nullNodes");
            if (CheckBox_NullNodes->IsChecked()) node->AddAttribute("nullNodes", wxString::Format("%d", NullNodes->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERCOLOURORDER) {
            node->DeleteAttribute("colorOrder");
            if (CheckBox_ColorOrder->IsChecked()) node->AddAttribute("colorOrder", ColorOrder->GetStringSelection());
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERGROUPCOUNT) {
            node->DeleteAttribute("groupCount");
            if (CheckBox_GroupCount->IsChecked()) node->AddAttribute("groupCount", wxString::Format("%d", GroupCount->GetValue()));
        }
        if (_type == controller_connection_bulkedit::CEBE_CONTROLLERDIRECTION) {
            node->DeleteAttribute("reverse");
            if (CheckBox_PixelDirection->IsChecked()) node->AddAttribute("reverse", wxString::Format("%d", PixelDirection->GetSelection()));
        }
    }
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnectionDialog::Get");
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnectionDialog::Get");
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ControllerConnectionDialog::Get");
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnectionDialog::Get");
}

void ControllerConnectionDialog::OnPixelDirectionClick(wxCommandEvent& event)
{
    PixelDirection->Enable(CheckBox_PixelDirection->IsChecked());
}

void ControllerConnectionDialog::OnCheckBox_NullNodesClick(wxCommandEvent& event)
{
    NullNodes->Enable(CheckBox_NullNodes->IsChecked());
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
