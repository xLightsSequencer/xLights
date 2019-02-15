//(*InternalHeaders(SettingsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/file.h>
#include <wx/filename.h>

#include "SettingsDialog.h"
#include "Settings.h"
#include "UniverseEntryDialog.h"
#include "../xLights/IPEntryDialog.h"
#include "../xSchedule/wxMIDI/src/wxMidi.h"
#include "FadeExcludeDialog.h"

//(*IdInit(SettingsDialog)
const long SettingsDialog::ID_STATICTEXT5 = wxNewId();
const long SettingsDialog::ID_CHECKBOX_E131 = wxNewId();
const long SettingsDialog::ID_CHECKBOX_ARTNET = wxNewId();
const long SettingsDialog::ID_STATICTEXT7 = wxNewId();
const long SettingsDialog::ID_STATICTEXT8 = wxNewId();
const long SettingsDialog::ID_BUTTON9 = wxNewId();
const long SettingsDialog::ID_STATICTEXT2 = wxNewId();
const long SettingsDialog::ID_STATICTEXT3 = wxNewId();
const long SettingsDialog::ID_BUTTON12 = wxNewId();
const long SettingsDialog::ID_STATICTEXT9 = wxNewId();
const long SettingsDialog::ID_CHOICE1 = wxNewId();
const long SettingsDialog::ID_STATICTEXT1 = wxNewId();
const long SettingsDialog::ID_CHOICE2 = wxNewId();
const long SettingsDialog::ID_STATICTEXT4 = wxNewId();
const long SettingsDialog::ID_LISTVIEW_UNIVERSES = wxNewId();
const long SettingsDialog::ID_BUTTON3 = wxNewId();
const long SettingsDialog::ID_BUTTON4 = wxNewId();
const long SettingsDialog::ID_BUTTON5 = wxNewId();
const long SettingsDialog::ID_STATICTEXT6 = wxNewId();
const long SettingsDialog::ID_LISTVIEW1 = wxNewId();
const long SettingsDialog::ID_BUTTON8 = wxNewId();
const long SettingsDialog::ID_BUTTON10 = wxNewId();
const long SettingsDialog::ID_BUTTON11 = wxNewId();
const long SettingsDialog::ID_BUTTON6 = wxNewId();
const long SettingsDialog::ID_BUTTON7 = wxNewId();
const long SettingsDialog::ID_BUTTON1 = wxNewId();
const long SettingsDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsDialog,wxDialog)
	//(*EventTable(SettingsDialog)
	//*)
END_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent, Settings* settings, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _settings = settings;

    //(*Initialize(SettingsDialog)
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;

    Create(parent, wxID_ANY, _("xFade Settings"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(2);
    FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer6->AddGrowableCol(1);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Protocols:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    CheckBox_E131 = new wxCheckBox(this, ID_CHECKBOX_E131, _("E131"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_E131"));
    CheckBox_E131->SetValue(true);
    FlexGridSizer7->Add(CheckBox_E131, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ArtNET = new wxCheckBox(this, ID_CHECKBOX_ARTNET, _("ArtNET"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ARTNET"));
    CheckBox_ArtNET->SetValue(false);
    FlexGridSizer7->Add(CheckBox_ArtNET, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Input Interface:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer6->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_InputIP = new wxStaticText(this, ID_STATICTEXT8, _("UNKNOWN"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer6->Add(StaticText_InputIP, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ForceInput = new wxButton(this, ID_BUTTON9, _("Force"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer6->Add(Button_ForceInput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Output Interface:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_OutputIP = new wxStaticText(this, ID_STATICTEXT3, _("UNKNOWN"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer6->Add(StaticText_OutputIP, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ForceOutput = new wxButton(this, ID_BUTTON12, _("Force"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizer6->Add(Button_ForceOutput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT9, _("Frame Timing:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_FrameTiming = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_FrameTiming->Append(_("25ms"));
    Choice_FrameTiming->Append(_("30ms"));
    Choice_FrameTiming->SetSelection( Choice_FrameTiming->Append(_("50ms")) );
    Choice_FrameTiming->Append(_("100ms"));
    FlexGridSizer6->Add(Choice_FrameTiming, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("MIDI Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice1 = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer3->Add(Choice1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(1);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT4, _("Universes:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ListView_Universes = new wxListView(this, ID_LISTVIEW_UNIVERSES, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTVIEW_UNIVERSES"));
    FlexGridSizer4->Add(ListView_Universes, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_Add = new wxButton(this, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer5->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Edit = new wxButton(this, ID_BUTTON4, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer5->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Delete = new wxButton(this, ID_BUTTON5, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer5->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer9->AddGrowableCol(0);
    FlexGridSizer9->AddGrowableRow(1);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT6, _("Channel Fade Exclude:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer9->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ListViewFadeExclude = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTVIEW1"));
    FlexGridSizer9->Add(ListViewFadeExclude, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer10->AddGrowableCol(0);
    ButtonAddFE = new wxButton(this, ID_BUTTON8, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizer10->Add(ButtonAddFE, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_EditFE = new wxButton(this, ID_BUTTON10, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    FlexGridSizer10->Add(Button_EditFE, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_DeleteFE = new wxButton(this, ID_BUTTON11, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    FlexGridSizer10->Add(Button_DeleteFE, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_ExportSettings = new wxButton(this, ID_BUTTON6, _("Apply and Export Settings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer8->Add(Button_ExportSettings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ImportSettings = new wxButton(this, ID_BUTTON7, _("Import Settings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizer8->Add(Button_ImportSettings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHECKBOX_E131,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnCheckBox_E131Click);
    Connect(ID_CHECKBOX_ARTNET,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnCheckBox_ArtNETClick);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_ForceInputClick);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_ForceOutputClick);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SettingsDialog::OnChoice_FrameTimingSelect);
    Connect(ID_LISTVIEW_UNIVERSES,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&SettingsDialog::OnListView_UniversesItemSelect);
    Connect(ID_LISTVIEW_UNIVERSES,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&SettingsDialog::OnListView_UniversesItemActivated);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_AddClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_EditClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_DeleteClick);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&SettingsDialog::OnListViewFadeExcludeItemSelect);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&SettingsDialog::OnListViewFadeExcludeItemActivated);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButtonAddFEClick);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_EditFEClick);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_DeleteFEClick);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_ExportSettingsClick);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_ImportSettingsClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsDialog::OnButton_CancelClick);
    //*)

    ListView_Universes->AppendColumn("Start");
    ListView_Universes->AppendColumn("End");
    ListView_Universes->AppendColumn("IP Address");
    ListView_Universes->AppendColumn("Description");
    ListView_Universes->AppendColumn("Protocol");

    ListViewFadeExclude->AppendColumn("Channel");

    PopulateFields();

    int w, h;
    GetSize(&w, &h);
    if (w < 600) SetSize(600, h);
    Layout();
}

std::list<std::string> SettingsDialog::GetMIDIDevices()
{
    std::list<std::string> res;

    wxMidiSystem* midiSystem = wxMidiSystem::GetInstance();
    int devices = midiSystem->CountDevices();
    for (int i = 0; i < devices; i++)
    {
        wxMidiInDevice* midiDev = new wxMidiInDevice(i);
        if (midiDev->IsInputPort())
        {
            res.push_back(wxString::Format("%s [%s] %d", midiDev->DeviceName(), midiDev->InterfaceUsed(), i).ToStdString());
        }
        delete midiDev;
    }

    return res;
}

SettingsDialog::~SettingsDialog()
{
	//(*Destroy(SettingsDialog)
	//*)
}

void SettingsDialog::ValidateWindow()
{
    if (ListView_Universes->GetSelectedItemCount() == 0)
    {
        Button_Edit->Enable(false);
        Button_Delete->Enable(false);
    }
    else
    {
        Button_Edit->Enable(true);
        Button_Delete->Enable(true);
    }

    if (ListViewFadeExclude->GetSelectedItemCount() == 0)
    {
        Button_EditFE->Enable(false);
        Button_DeleteFE->Enable(false);
    }
    else
    {
        Button_EditFE->Enable(true);
        Button_DeleteFE->Enable(true);
    }

    if (!CheckBox_ArtNET->GetValue() && !CheckBox_E131->GetValue())
    {
        Button_Ok->Disable();
    }
    else
    {
        Button_Ok->Enable();
    }
}

void SettingsDialog::AddUniverseRange(int low, int high, std::string ipAddress, std::string desc, std::string protocol)
{
    for (int i = low; i <= high; i++)
    {
        _targetIPCopy[i] = ipAddress;
        _targetDescCopy[i] = desc;
        _targetProtocolCopy[i] = protocol;
    }

    LoadUniverses();
    ValidateWindow();
}

void SettingsDialog::LoadUniverses()
{
    int sel = ListView_Universes->GetFirstSelected();

    ListView_Universes->Freeze();

    ListView_Universes->DeleteAllItems();

    int startu = 0;
    int lastu = 0;
    std::string lastip = "";
    std::string lastdesc = "";
    std::string lastprotocol = "";

    auto itd = _targetDescCopy.begin();
    auto itp = _targetProtocolCopy.begin();
    for (auto it = _targetIPCopy.begin(); it != _targetIPCopy.end(); ++it)
    {
        if (lastu != it->first - 1 || lastip != it->second)
        {
            if (lastu != 0)
            {
                ListView_Universes->InsertItem(ListView_Universes->GetItemCount(), wxString::Format("%d", startu));
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 1, wxString::Format("%d", lastu));
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 2, lastip);
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 3, lastdesc);
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 4, lastprotocol);

                startu = it->first;
                lastu = it->first;
                lastip = it->second;
                lastdesc = itd->second;
                lastprotocol = itp->second;
            }
            else
            {
                startu = it->first;
                lastu = it->first;
                lastip = it->second;
                lastdesc = itd->second;
                lastprotocol = itp->second;
            }
        }
        else
        {
            lastu = it->first;
        }

        ++itd;
        ++itp;
    }

    if (startu != 0)
    {
        ListView_Universes->InsertItem(ListView_Universes->GetItemCount(), wxString::Format("%d", startu));
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 1, wxString::Format("%d", lastu));
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 2, lastip);
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 3, lastdesc);
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 4, lastprotocol);
    }

    ListView_Universes->Select(sel);

    ListView_Universes->Thaw();
}

void SettingsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void SettingsDialog::Apply()
{
    _settings->_targetIP = _targetIPCopy;
    _settings->_targetDesc = _targetDescCopy;
    _settings->_targetProtocol = _targetProtocolCopy;

    wxString frm = Choice_FrameTiming->GetStringSelection();
    if (frm == "25ms")
    {
        _settings->_frameMS = 25;
    }
    else if (frm == "30ms")
    {
        _settings->_frameMS = 30;
    }
    else if (frm == "100ms")
    {
        _settings->_frameMS = 100;
    }
    else
    {
        _settings->_frameMS = 50;
    }

    _settings->_localOutputIP = _localOutputIPCopy;
    _settings->_localInputIP = _localInputIPCopy;
    _settings->_E131 = CheckBox_E131->GetValue();
    _settings->_ArtNET = CheckBox_ArtNET->GetValue();
    _settings->_midiDevice = Choice1->GetStringSelection();
}

void SettingsDialog::PopulateFields()
{
    StaticText_InputIP->SetLabel(_settings->_localInputIP);
    StaticText_OutputIP->SetLabel(_settings->_localOutputIP);

    _targetIPCopy = _settings->_targetIP;
    _targetDescCopy = _settings->_targetDesc;
    _targetProtocolCopy = _settings->_targetProtocol;
    _localOutputIPCopy = _settings->_localOutputIP;
    _localInputIPCopy = _settings->_localInputIP;

    LoadUniverses();
    LoadFadeExclude();

    for (auto device : GetMIDIDevices())
    {
        Choice1->Append(device);
    }
    Choice1->SetSelection(0);
    if (_settings->_midiDevice != "")
    {
        Choice1->SetStringSelection(_settings->_midiDevice);
    }
}

void SettingsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    Apply();
    EndDialog(wxID_OK);
}

void SettingsDialog::OnListView_UniversesItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void SettingsDialog::OnListView_UniversesItemActivated(wxListEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));
        UniverseEntryDialog dlg(this, start, end, ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 2), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 3), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 4));
        if (dlg.ShowModal() == wxID_OK)
        {
            for (int i = start; i <= end; i++)
            {
                _targetIPCopy.erase(i);
                _targetDescCopy.erase(i);
                _targetProtocolCopy.erase(i);
            }
            AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue(), dlg.TextCtrl_IPAddress->GetValue().ToStdString(), dlg.TextCtrl_Description->GetValue().ToStdString(), dlg.Choice_Protocol->GetStringSelection().ToStdString());
            LoadUniverses();
        }
    }
    ValidateWindow();
}

void SettingsDialog::OnButton_ForceInputClick(wxCommandEvent& event)
{
    IPEntryDialog dlg(this);
    dlg.TextCtrl_IPAddress->SetValue(_localInputIPCopy);

    if (dlg.ShowModal() == wxID_OK)
    {
        if (dlg.TextCtrl_IPAddress->GetValue() == "")
        {
            _localInputIPCopy = _settings->_defaultIP;
        }
        else
        {
            _localInputIPCopy = dlg.TextCtrl_IPAddress->GetValue();
        }
        StaticText_InputIP->SetLabel(_localInputIPCopy);
    }
}

void SettingsDialog::OnButton_ForceOutputClick(wxCommandEvent& event)
{
    IPEntryDialog dlg(this);
    dlg.TextCtrl_IPAddress->SetValue(_localOutputIPCopy);

    if (dlg.ShowModal() == wxID_OK)
    {
        if (dlg.TextCtrl_IPAddress->GetValue() == "")
        {
            _localOutputIPCopy = _settings->_defaultIP;
        }
        else
        {
            _localOutputIPCopy = dlg.TextCtrl_IPAddress->GetValue();
        }
        StaticText_OutputIP->SetLabel(_localOutputIPCopy);
    }
}

void SettingsDialog::OnChoice_FrameTimingSelect(wxCommandEvent& event)
{
}

void SettingsDialog::OnCheckBox_E131Click(wxCommandEvent& event)
{
    ValidateWindow();
}

void SettingsDialog::OnCheckBox_ArtNETClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void SettingsDialog::OnButton_AddClick(wxCommandEvent& event)
{
    UniverseEntryDialog dlg(this, -1, -1, "", "", "As per input");
    if (dlg.ShowModal() == wxID_OK)
    {
        AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue(), dlg.TextCtrl_IPAddress->GetValue().ToStdString(), dlg.TextCtrl_Description->GetValue().ToStdString(), dlg.Choice_Protocol->GetStringSelection().ToStdString());
    }
}

void SettingsDialog::OnButton_EditClick(wxCommandEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));
        UniverseEntryDialog dlg(this, start, end, ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 2), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 3), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 4));
        if (dlg.ShowModal() == wxID_OK)
        {
            for (int i = start; i <= end; i++)
            {
                _targetIPCopy.erase(i);
                _targetDescCopy.erase(i);
                _targetProtocolCopy.erase(i);
            }
            AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue(), dlg.TextCtrl_IPAddress->GetValue().ToStdString(), dlg.TextCtrl_Description->GetValue().ToStdString(), dlg.Choice_Protocol->GetStringSelection().ToStdString());
            LoadUniverses();
        }
    }
}

void SettingsDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));

        for (int i = start; i <= end; i++)
        {
            _targetIPCopy.erase(i);
            _targetDescCopy.erase(i);
            _targetProtocolCopy.erase(i);
        }
        LoadUniverses();
    }
}

void SettingsDialog::OnButton_ImportSettingsClick(wxCommandEvent& event)
{
    wxFileDialog* openDialog = new wxFileDialog(this, "Choose file to Import", wxEmptyString, wxEmptyString,
        "xFade Settings (*.xfade)|*.xfade",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST, wxDefaultPosition);
    if (openDialog->ShowModal() == wxID_OK)
    {
        wxString settings;
        wxFile f;
        wxString filename = openDialog->GetDirectory() + wxFileName::GetPathSeparator() + openDialog->GetFilename();
        if (f.Open(filename))
        {
            f.ReadAll(&settings);
            f.Close();
            _settings->Load(settings);
            PopulateFields();
        }
    }
    delete openDialog;
}

void SettingsDialog::OnButton_ExportSettingsClick(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written

    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString,
        "xFadeSettings.xfade", wxEmptyString,
        "xFade Settings (*.xfade)|*.xfade", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;

    wxFile f;
    if (!f.Create(filename, true) || !f.IsOpened())
    {
        wxMessageBox(wxString::Format("Unable to create file %s.", filename), _("ERROR"));
        ValidateWindow();
        return;
    }

    Apply();
    f.Write(_settings->Serialise());
    f.Close();
}

void SettingsDialog::OnButtonAddFEClick(wxCommandEvent& event)
{
    FadeExcludeDialog dlg(this, _settings, "");
    if (dlg.ShowModal() == wxID_OK)
    {
        _settings->AddFadeExclude(dlg.GetChannel());
        LoadFadeExclude();
        ValidateWindow();
    }
}

void SettingsDialog::OnButton_EditFEClick(wxCommandEvent& event)
{
    if (ListViewFadeExclude->GetSelectedItemCount() > 0)
    {
        std::string old = ListViewFadeExclude->GetItemText(ListViewFadeExclude->GetFirstSelected()).ToStdString();
        FadeExcludeDialog dlg(this, _settings, old);
        if (dlg.ShowModal() == wxID_OK)
        {
            _settings->DeleteFadeExclude(old);
            _settings->AddFadeExclude(dlg.GetChannel());
            LoadFadeExclude();
            ValidateWindow();
        }
    }
}

void SettingsDialog::OnButton_DeleteFEClick(wxCommandEvent& event)
{
    if (ListViewFadeExclude->GetSelectedItemCount() > 0)
    {
        _settings->DeleteFadeExclude(ListViewFadeExclude->GetItemText(ListViewFadeExclude->GetFirstSelected()).ToStdString());
        LoadFadeExclude();
        ValidateWindow();
    }
}

void SettingsDialog::OnListViewFadeExcludeItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void SettingsDialog::OnListViewFadeExcludeItemActivated(wxListEvent& event)
{
    if (ListViewFadeExclude->GetSelectedItemCount() > 0)
    {
        std::string old = ListViewFadeExclude->GetItemText(ListViewFadeExclude->GetFirstSelected()).ToStdString();
        FadeExcludeDialog dlg(this, _settings, old);
        if (dlg.ShowModal() == wxID_OK)
        {
            _settings->DeleteFadeExclude(old);
            _settings->AddFadeExclude(dlg.GetChannel());
            LoadFadeExclude();
        }
    }
    ValidateWindow();
}

void SettingsDialog::LoadFadeExclude()
{
    int sel = ListViewFadeExclude->GetFirstSelected();

    ListViewFadeExclude->Freeze();

    ListViewFadeExclude->DeleteAllItems();

    for (auto it = _settings->GetFadeExclude().begin(); it != _settings->GetFadeExclude().end(); ++it)
    {
        ListViewFadeExclude->InsertItem(ListViewFadeExclude->GetItemCount(), *it);
    }

    ListViewFadeExclude->Select(sel);

    ListViewFadeExclude->Thaw();
}
