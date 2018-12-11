#include "ControllerConnectionDialog.h"
#include "models/Model.h"

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"
#include <wx/xml/xml.h>

//(*InternalHeaders(ControllerConnectionDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ControllerConnectionDialog)
const long ControllerConnectionDialog::ID_STATICTEXT1 = wxNewId();
const long ControllerConnectionDialog::ID_CHOICE1 = wxNewId();
const long ControllerConnectionDialog::ID_STATICTEXT2 = wxNewId();
const long ControllerConnectionDialog::ID_SPINCTRL1 = wxNewId();
const long ControllerConnectionDialog::ID_DMX_channel = wxNewId();
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
const long ControllerConnectionDialog::ID_BUTTON1 = wxNewId();
const long ControllerConnectionDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ControllerConnectionDialog,wxDialog)
	//(*EventTable(ControllerConnectionDialog)
	//*)
END_EVENT_TABLE()

ControllerConnectionDialog::ControllerConnectionDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ControllerConnectionDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxGridBagSizer* GridBagSizer1;
	wxSpinCtrl* GammaPlaceHolder;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Protocol"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Protocol = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Protocol->SetFocus();
	GridBagSizer1->Add(Choice_Protocol, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Port = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Port->SetValue(_T("1"));
	GridBagSizer1->Add(SpinCtrl_Port, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	DMXChannel = new wxSpinCtrl(this, ID_DMX_channel, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 512, 1, _T("ID_DMX_channel"));
	DMXChannel->SetValue(_T("1"));
	DMXChannel->Disable();
	GridBagSizer1->Add(DMXChannel, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	CheckBox_PixelDirection = new wxCheckBox(this, ID_CHECKBOX1, _("Set Direction"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_PixelDirection->SetValue(false);
	GridBagSizer1->Add(CheckBox_PixelDirection, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PixelDirection = new wxChoice(this, ID_PIXEL_reverse, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_PIXEL_reverse"));
	PixelDirection->SetSelection( PixelDirection->Append(_("Forward")) );
	PixelDirection->Append(_("Reverse"));
	PixelDirection->Disable();
	GridBagSizer1->Add(PixelDirection, wxGBPosition(4, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	CheckBox_ColorOrder = new wxCheckBox(this, ID_CHECKBOX5, _("Set Color Order"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_ColorOrder->SetValue(false);
	GridBagSizer1->Add(CheckBox_ColorOrder, wxGBPosition(5, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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
	GridBagSizer1->Add(ColorOrder, wxGBPosition(5, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	CheckBox_NullNodes = new wxCheckBox(this, ID_CHECKBOX2, _("Set Null Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_NullNodes->SetValue(false);
	GridBagSizer1->Add(CheckBox_NullNodes, wxGBPosition(6, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NullNodes = new wxSpinCtrl(this, ID_PIXEL_nullNodes, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 25, 0, _T("ID_PIXEL_nullNodes"));
	NullNodes->SetValue(_T("0"));
	NullNodes->Disable();
	GridBagSizer1->Add(NullNodes, wxGBPosition(6, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	CheckBox_Brightness = new wxCheckBox(this, ID_CHECKBOX3, _("Set Brightness"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_Brightness->SetValue(false);
	GridBagSizer1->Add(CheckBox_Brightness, wxGBPosition(7, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Brightness = new wxSpinCtrl(this, ID_PIXEL_brightness, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 100, _T("ID_PIXEL_brightness"));
	Brightness->SetValue(_T("100"));
	Brightness->Disable();
	GridBagSizer1->Add(Brightness, wxGBPosition(7, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	CheckBox_Gamma = new wxCheckBox(this, ID_CHECKBOX4, _("Set Gamma"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_Gamma->SetValue(false);
	GridBagSizer1->Add(CheckBox_Gamma, wxGBPosition(8, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	GammaPlaceHolder = new wxSpinCtrl(this, ID_PIXEL_gamma, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 10, 0, _T("ID_PIXEL_gamma"));
	GridBagSizer1->Add(GammaPlaceHolder, wxGBPosition(8, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
	CheckBox_DMXChannel = new wxCheckBox(this, ID_CHECKBOX6, _("DMX Channel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_DMXChannel->SetValue(false);
	GridBagSizer1->Add(CheckBox_DMXChannel, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(GridBagSizer1, 1, wxALL|wxEXPAND, 2);
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
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnCheckBox_DMXChannelClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnButton_CancelClick);
	//*)

    Gamma = new wxSpinCtrlDouble(this, ID_PIXEL_gamma, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0.1, 5.0, 1.0, 0.1, _T("ID_PIXEL_gamma"));
    Gamma->Disable();
    GridBagSizer1->Replace(GammaPlaceHolder, Gamma);
    GammaPlaceHolder->Destroy();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);


    auto protocols = Model::GetProtocols();
    for (auto it = protocols.begin(); it != protocols.end(); ++it) {
        Choice_Protocol->AppendString(wxString(it->c_str()));
    }
    Choice_Protocol->SetSelection(0);

    SetEscapeId(Button_Cancel->GetId());
    Button_Ok->SetDefault();
    Choice_Protocol->SetFocus();
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

void ControllerConnectionDialog::Set(wxXmlNode *node) {
    if (node != nullptr) {
    
        wxString protocol = node->GetAttribute("Protocol", "WS2811");
        protocol.UpperCase();
        Choice_Protocol->SetStringSelection(protocol);
        SpinCtrl_Port->SetValue(wxAtoi(node->GetAttribute("Port", "1")));
        
        if (protocol == "DMX") {
            if (node->HasAttribute("channel")) {
                int c = wxAtoi(node->GetAttribute("channel"));
                if (c > 0 && c < 513) {
                    CheckBox_DMXChannel->SetValue(true);
                    DMXChannel->Enable(true);
                    DMXChannel->SetValue(c);
                }
            }
        } else if (Model::IsPixelProtocol(protocol)) {
            if (node->HasAttribute("brightness")) {
                CheckBox_Brightness->SetValue(true);
                Brightness->Enable(true);
                Brightness->SetValue(wxAtoi(node->GetAttribute("brightness")));
            }
            if (node->HasAttribute("gamma")) {
                CheckBox_Gamma->SetValue(true);
                Gamma->Enable(true);
                Gamma->SetValue(wxAtof(node->GetAttribute("gamma")));
            }
            if (node->HasAttribute("nullNodes")) {
                CheckBox_NullNodes->SetValue(true);
                NullNodes->Enable(true);
                NullNodes->SetValue(wxAtoi(node->GetAttribute("nullNodes")));
            }
            if (node->HasAttribute("colorOrder")) {
                CheckBox_ColorOrder->SetValue(true);
                ColorOrder->Enable(true);
                ColorOrder->SetStringSelection(node->GetAttribute("colorOrder"));
            }
            if (node->HasAttribute("reverse")) {
                CheckBox_PixelDirection->SetValue(true);
                PixelDirection->Enable(true);
                PixelDirection->SetSelection(wxAtoi(node->GetAttribute("reverse")));
            }
        }
    }
    ProtocolSelected(Choice_Protocol->GetStringSelection());
    Choice_Protocol->SetFocus();
    Choice_Protocol->SetFocusFromKbd();
    SpinCtrl_Port->SetFocus();
    SpinCtrl_Port->SetFocusFromKbd();
}

void ControllerConnectionDialog::Get(wxXmlNode *node) {
    std::string protocol = Choice_Protocol->GetStringSelection().ToStdString();
    node->DeleteAttribute("Protocol");
    node->AddAttribute("Protocol", protocol);
    node->DeleteAttribute("Port");
    node->AddAttribute("Port", wxString::Format("%d", SpinCtrl_Port->GetValue()));

    node->DeleteAttribute("channel");
    node->DeleteAttribute("brightness");
    node->DeleteAttribute("gamma");
    node->DeleteAttribute("nullNodes");
    node->DeleteAttribute("colorOrder");
    node->DeleteAttribute("reverse");
    if (protocol == "DMX" || protocol == "dmx") {
        if (CheckBox_DMXChannel->IsChecked()) {
            node->AddAttribute("Port", wxString::Format("%d", DMXChannel->GetValue()));
        }
    } else if (Model::IsPixelProtocol(protocol)) {
        wxJSONValue value;
        if (CheckBox_Brightness->IsChecked()) node->AddAttribute("brightness", wxString::Format("%d", Brightness->GetValue()));
        if (CheckBox_Gamma->IsChecked()) node->AddAttribute("gamma", wxString::Format("%f", Gamma->GetValue()));
        if (CheckBox_NullNodes->IsChecked()) node->AddAttribute("nullNodes", wxString::Format("%d", NullNodes->GetValue()));
        if (CheckBox_ColorOrder->IsChecked()) node->AddAttribute("colorOrder", ColorOrder->GetStringSelection());
        if (CheckBox_PixelDirection->IsChecked()) node->AddAttribute("reverse", wxString::Format("%d",  PixelDirection->GetSelection()));
    }
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
    std::string protocol = Choice_Protocol->GetStringSelection().ToStdString();
    ProtocolSelected(protocol);
}

void ControllerConnectionDialog::OnCheckBox_DMXChannelClick(wxCommandEvent& event)
{
    DMXChannel->Enable(CheckBox_DMXChannel->IsChecked());
}

void ControllerConnectionDialog::ProtocolSelected(const wxString &protocol)
{
    bool isPixel = Model::IsPixelProtocol(protocol.ToStdString());
    bool isDMX = protocol == "DMX" || protocol == "dmx";

    CheckBox_DMXChannel->Show(isDMX);
    DMXChannel->Show(isDMX);
    ColorOrder->Show(isPixel);
    CheckBox_ColorOrder->Show(isPixel);
    PixelDirection->Show(isPixel);
    CheckBox_PixelDirection->Show(isPixel);
    NullNodes->Show(isPixel);
    CheckBox_NullNodes->Show(isPixel);
    Gamma->Show(isPixel);
    CheckBox_Gamma->Show(isPixel);
    Brightness->Show(isPixel);
    CheckBox_Brightness->Show(isPixel);
}

