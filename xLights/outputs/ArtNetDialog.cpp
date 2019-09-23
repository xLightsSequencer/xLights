#include "ArtNetDialog.h"

//(*InternalHeaders(ArtNetDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "ArtNetOutput.h"
#include "OutputManager.h"
#include <wx/regex.h>

//(*IdInit(ArtNetDialog)
const long ArtNetDialog::ID_TEXTCTRL1 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL1 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL2 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL3 = wxNewId();
const long ArtNetDialog::ID_PANEL1 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL5 = wxNewId();
const long ArtNetDialog::ID_PANEL2 = wxNewId();
const long ArtNetDialog::ID_NOTEBOOK1 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL6 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL4 = wxNewId();
const long ArtNetDialog::ID_TEXTCTRL2 = wxNewId();
const long ArtNetDialog::ID_CHECKBOX1 = wxNewId();
const long ArtNetDialog::ID_BUTTON1 = wxNewId();
const long ArtNetDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ArtNetDialog,wxDialog)
	//(*EventTable(ArtNetDialog)
	//*)
END_EVENT_TABLE()

ArtNetDialog::ArtNetDialog(wxWindow* parent, ArtNetOutput* artnet, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _artnet = artnet;
    _outputManager = outputManager;

	//(*Initialize(ArtNetDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxPanel* Panel2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;
	wxStaticText* StaticText9;

	Create(parent, id, _("ArtNET Setup"), wxDefaultPosition, wxDefaultSize, wxCAPTION, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Sets up an ArtNET connection over ethernet.\n\nThe universe numbers entered here\nshould match the universe numbers \ndefined on your ArtNET device."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlIPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrlIPAddress->SetMaxLength(15);
	FlexGridSizer2->Add(TextCtrlIPAddress, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Panel1 = new wxPanel(Notebook1, ID_PANEL1, wxPoint(159,57), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText4 = new wxStaticText(Panel1, wxID_ANY, _("Network"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlNet = new wxSpinCtrl(Panel1, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 0, _T("ID_SPINCTRL1"));
	SpinCtrlNet->SetValue(_T("0"));
	FlexGridSizer6->Add(SpinCtrlNet, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(Panel1, wxID_ANY, _("Subnet"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlSubnet = new wxSpinCtrl(Panel1, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 15, 0, _T("ID_SPINCTRL2"));
	SpinCtrlSubnet->SetValue(_T("0"));
	FlexGridSizer6->Add(SpinCtrlSubnet, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(Panel1, wxID_ANY, _("Universe"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlUniverse = new wxSpinCtrl(Panel1, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 15, 1, _T("ID_SPINCTRL3"));
	SpinCtrlUniverse->SetValue(_T("1"));
	FlexGridSizer6->Add(SpinCtrlUniverse, 1, wxALL|wxEXPAND, 5);
	Panel1->SetSizer(FlexGridSizer6);
	FlexGridSizer6->Fit(Panel1);
	FlexGridSizer6->SetSizeHints(Panel1);
	Panel2 = new wxPanel(Notebook1, ID_PANEL2, wxPoint(77,6), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticText8 = new wxStaticText(Panel2, wxID_ANY, _("Universe"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer7->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlUniverseOnly = new wxSpinCtrl(Panel2, ID_SPINCTRL5, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 32768, 1, _T("ID_SPINCTRL5"));
	SpinCtrlUniverseOnly->SetValue(_T("1"));
	FlexGridSizer7->Add(SpinCtrlUniverseOnly, 1, wxALL|wxEXPAND, 5);
	Panel2->SetSizer(FlexGridSizer7);
	FlexGridSizer7->Fit(Panel2);
	FlexGridSizer7->SetSizeHints(Panel2);
	Notebook1->AddPage(Panel1, _("Net/Subnet/Universe"), false);
	Notebook1->AddPage(Panel2, _("Universe Only"), false);
	FlexGridSizer5->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText9 = new wxStaticText(this, wxID_ANY, _("# of Universes"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NumUniv = new wxSpinCtrl(this, ID_SPINCTRL6, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 32000, 1, _T("ID_SPINCTRL6"));
	SpinCtrl_NumUniv->SetValue(_T("1"));
	FlexGridSizer4->Add(SpinCtrl_NumUniv, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, wxID_ANY, _("Universe Size"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlChannels = new wxSpinCtrl(this, ID_SPINCTRL4, _T("512"), wxDefaultPosition, wxDefaultSize, 0, 1, 512, 512, _T("ID_SPINCTRL4"));
	SpinCtrlChannels->SetValue(_T("512"));
	FlexGridSizer4->Add(SpinCtrlChannels, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlDescription = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer4->Add(TextCtrlDescription, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SuppressDuplicates = new wxCheckBox(this, ID_CHECKBOX1, _("Suppress duplicate frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_SuppressDuplicates->SetValue(false);
	FlexGridSizer4->Add(CheckBox_SuppressDuplicates, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ArtNetDialog::OnTextCtrlIPAddressText);
	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtNetDialog::OnUniverseChange);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtNetDialog::OnUniverseChange);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtNetDialog::OnUniverseChange);
	Connect(ID_SPINCTRL5,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtNetDialog::OnUniverseChange);
	Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING,(wxObjectEventFunction)&ArtNetDialog::OnNotebook1PageChanging);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ArtNetDialog::OnButtonOkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ArtNetDialog::OnButtonCancelClick);
	//*)

    wxASSERT(artnet != nullptr);

    CheckBox_SuppressDuplicates->SetValue(artnet->IsSuppressDuplicateFrames());
    SpinCtrlNet->SetValue(artnet->GetArtNetNet());
    SpinCtrlSubnet->SetValue(artnet->GetArtNetSubnet());
    SpinCtrlUniverse->SetValue(artnet->GetArtNetUniverse());
    SpinCtrlChannels->SetValue(artnet->GetChannels());
    TextCtrlDescription->SetValue(artnet->GetDescription());
    TextCtrlIPAddress->SetValue(artnet->GetIP());
    SpinCtrlUniverseOnly->SetValue(artnet->GetUniverse());
    SpinCtrl_NumUniv->SetValue(1);
    if (artnet->GetIP() == "")
    {
        SpinCtrl_NumUniv->Enable(true);
    }
    else
    {
        SpinCtrl_NumUniv->Enable(false);
    }

    SetEscapeId(ButtonCancel->GetId());
    ButtonOk->SetDefault();

    ValidateWindow();
}

ArtNetDialog::~ArtNetDialog()
{
	//(*Destroy(ArtNetDialog)
	//*)
}


void ArtNetDialog::OnTextCtrlIPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ArtNetDialog::OnButtonOkClick(wxCommandEvent& event)
{
    UniverseChange(); // make sure universe is consistent

    _artnet->SetIP(TextCtrlIPAddress->GetValue().ToStdString());
    _artnet->SetArtNetUniverse(SpinCtrlNet->GetValue(), SpinCtrlSubnet->GetValue(), SpinCtrlUniverse->GetValue());
    _artnet->SetChannels(SpinCtrlChannels->GetValue());
    _artnet->SetDescription(TextCtrlDescription->GetValue().ToStdString());
    _artnet->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());

    if (SpinCtrl_NumUniv->GetValue() > 1)
    {
        Output* last = _artnet;
        for (int i = 1; i < SpinCtrl_NumUniv->GetValue(); i++)
        {
            ArtNetOutput* a = new ArtNetOutput();
            a->SetIP(TextCtrlIPAddress->GetValue().ToStdString());
            a->SetUniverse(SpinCtrlUniverseOnly->GetValue() + i);
            a->SetDescription(TextCtrlDescription->GetValue().ToStdString());
            a->SetChannels(SpinCtrlChannels->GetValue());
            a->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());
            _outputManager->AddOutput(a, last);
            last = a;
        }
    }

    EndDialog(wxID_OK);
}

void ArtNetDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ArtNetDialog::ValidateWindow()
{
    wxString ips = TextCtrlIPAddress->GetValue().Trim(false).Trim(true);
    if (ips == "")
    {
        ButtonOk->Disable();
    }
    else if (SpinCtrlUniverseOnly->GetValue() + SpinCtrl_NumUniv->GetValue() >= 32000)
    {
        ButtonOk->Disable();
    }
    else
    {
        wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");

        if (regxIPAddr.Matches(ips))
        {
            ButtonOk->Enable();
        }
        else
        {
            ButtonOk->Disable();
        }
    }
}

void ArtNetDialog::OnUniverseChange(wxSpinEvent& event)
{
    UniverseChange();
}

void ArtNetDialog::UniverseChange()
{
    if (Notebook1->GetSelection() == 0)
    {
        // net/subnet/universe
        SpinCtrlUniverseOnly->SetValue(ArtNetOutput::GetArtNetCombinedUniverse(SpinCtrlNet->GetValue(), SpinCtrlSubnet->GetValue(), SpinCtrlUniverse->GetValue()));
    }
    else
    {
        int u = SpinCtrlUniverseOnly->GetValue();
        SpinCtrlNet->SetValue(ArtNetOutput::GetArtNetNet(u));
        SpinCtrlSubnet->SetValue(ArtNetOutput::GetArtNetSubnet(u));
        SpinCtrlUniverse->SetValue(ArtNetOutput::GetArtNetUniverse(u));

    }
}

void ArtNetDialog::OnNotebook1PageChanging(wxNotebookEvent& event)
{
    // Make sure universe is consistent
    UniverseChange();
}
