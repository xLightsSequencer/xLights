//(*InternalHeaders(MIDIAssociateDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "MIDIAssociateDialog.h"
#include "MIDIListener.h"

//(*IdInit(MIDIAssociateDialog)
const long MIDIAssociateDialog::ID_STATICTEXT4 = wxNewId();
const long MIDIAssociateDialog::ID_TEXTCTRL1 = wxNewId();
const long MIDIAssociateDialog::ID_STATICTEXT1 = wxNewId();
const long MIDIAssociateDialog::ID_CHOICE1 = wxNewId();
const long MIDIAssociateDialog::ID_STATICTEXT2 = wxNewId();
const long MIDIAssociateDialog::ID_CHOICE2 = wxNewId();
const long MIDIAssociateDialog::ID_STATICTEXT3 = wxNewId();
const long MIDIAssociateDialog::ID_CHOICE3 = wxNewId();
const long MIDIAssociateDialog::ID_BUTTON3 = wxNewId();
const long MIDIAssociateDialog::ID_BUTTON1 = wxNewId();
const long MIDIAssociateDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MIDIAssociateDialog,wxDialog)
	//(*EventTable(MIDIAssociateDialog)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_MIDI, MIDIAssociateDialog::OnMIDIEvent)
END_EVENT_TABLE()

MIDIAssociateDialog::MIDIAssociateDialog(wxWindow* parent, std::string controlName, MIDIListener* midiListener, int status, int channel, int data1, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _midiListener = midiListener;

    //(*Initialize(MIDIAssociateDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;

    Create(parent, id, _("Map MIDI"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxMAXIMIZE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Key Code:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer3->Add(StaticText4, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_KeyCode = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer3->Add(TextCtrl_KeyCode, 1, wxALL | wxEXPAND, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Status:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Status = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer3->Add(Choice_Status, 1, wxALL | wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer3->Add(StaticText2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Channel = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer3->Add(Choice_Channel, 1, wxALL | wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Data1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer3->Add(StaticText3, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Data1 = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    FlexGridSizer3->Add(Choice_Data1, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer3->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Scan = new wxButton(this, ID_BUTTON3, _("Scan"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer3->Add(Button_Scan, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(Button_Ok, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(Button_Cancel, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MIDIAssociateDialog::OnButton_ScanClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MIDIAssociateDialog::OnButton_OkClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MIDIAssociateDialog::OnButton_CancelClick);
    //*)

    TextCtrl_KeyCode->SetValue(controlName);

    Choice_Status->Append("0x8n - Note Off");
    Choice_Status->Append("0x9n - Note On");
    Choice_Status->Append("0xAn - Polyphonic Key Pressure");
    Choice_Status->Append("0xBn - Control Change");
    Choice_Status->Append("0xCn - Program Change");
    Choice_Status->Append("0xDn - Channel Pressure");
    Choice_Status->Append("0xEn - Pitch Bend");
    Choice_Status->SetSelection(1);

    for (int i = 0; i < 16; i++)
    {
        Choice_Channel->Append(wxString::Format("0x0%X", i));
    }
    Choice_Channel->Append("ANY");
    Choice_Channel->SetSelection(0);

    for (int i = 0; i < 256; i++)
    {
        Choice_Data1->Append(wxString::Format("0x%02X", i));
    }
    Choice_Data1->Append("ANY");
    Choice_Data1->SetSelection(0);

    Choice_Status->SetSelection((status >> 4) - 8);
    Choice_Channel->SetSelection(channel);
    Choice_Data1->SetSelection(data1);
}

MIDIAssociateDialog::~MIDIAssociateDialog()
{
	//(*Destroy(MIDIAssociateDialog)
	//*)
}


void MIDIAssociateDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    if (!Button_Scan->IsEnabled())
    {
        _midiListener->ClearTempWindow();
    }
    EndDialog(wxID_CANCEL);
}

void MIDIAssociateDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if (!Button_Scan->IsEnabled())
    {
        _midiListener->ClearTempWindow();
    }
    EndDialog(wxID_OK);
}

void MIDIAssociateDialog::OnButton_ScanClick(wxCommandEvent& event)
{
    _midiListener->SetTempWindow(this);
    Button_Scan->Disable();
}

void MIDIAssociateDialog::OnMIDIEvent(wxCommandEvent& event)
{
    if (Button_Scan->IsEnabled()) return;

    Button_Scan->Enable();
    _midiListener->ClearTempWindow();
    wxByte status = (event.GetInt() >> 24) & 0xFF;
    wxByte channel = (event.GetInt() >> 16) & 0xFF;
    wxByte data1 = (event.GetInt() >> 8) & 0xFF;
    wxByte data2 = event.GetInt() & 0xFF;

    Choice_Status->SetSelection(((status & 0xF0) >> 4) - 8);
    Choice_Channel->SetSelection(channel);
    Choice_Data1->SetSelection(data1);
}
