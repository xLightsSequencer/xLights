#include "StartChannelDialog.h"
#include <vector>

//(*InternalHeaders(StartChannelDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "outputs/OutputManager.h"
#include "outputs/Output.h"

//(*IdInit(StartChannelDialog)
const long StartChannelDialog::ID_SPINCTRL1 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON1 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON2 = wxNewId();
const long StartChannelDialog::ID_CHOICE2 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON5 = wxNewId();
const long StartChannelDialog::ID_CHOICE3 = wxNewId();
const long StartChannelDialog::ID_CHOICE4 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON3 = wxNewId();
const long StartChannelDialog::ID_CHOICE1 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON4 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON6 = wxNewId();
const long StartChannelDialog::ID_CHOICE5 = wxNewId();
//*)

BEGIN_EVENT_TABLE(StartChannelDialog,wxDialog)
	//(*EventTable(StartChannelDialog)
	//*)
END_EVENT_TABLE()

#include "models/ModelManager.h"
#include "models/Model.h"

StartChannelDialog::StartChannelDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = nullptr;

	//(*Initialize(StartChannelDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticText* StaticText1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	StartChannel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 999999, 1, _T("ID_SPINCTRL1"));
	StartChannel->SetValue(_T("1"));
	FlexGridSizer2->Add(StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Offset From"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	NoneButton = new wxRadioButton(this, ID_RADIOBUTTON1, _("None (Absolute)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	NoneButton->SetValue(true);
	FlexGridSizer3->Add(NoneButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	OutputButton = new wxRadioButton(this, ID_RADIOBUTTON2, _("Output Number"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	FlexGridSizer3->Add(OutputButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	OutputChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer3->Add(OutputChoice, 1, wxALL|wxEXPAND, 5);
	UniverseButton = new wxRadioButton(this, ID_RADIOBUTTON5, _("Universe Number"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON5"));
	FlexGridSizer3->Add(UniverseButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	ipChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer4->Add(ipChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	universeChoice = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	FlexGridSizer4->Add(universeChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ModelButton = new wxRadioButton(this, ID_RADIOBUTTON3, _("End of Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	FlexGridSizer3->Add(ModelButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ModelChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ModelChoice->SetMinSize(wxDLG_UNIT(this,wxSize(100,-1)));
	FlexGridSizer3->Add(ModelChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StartModelButton = new wxRadioButton(this, ID_RADIOBUTTON4, _("Start of Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	FlexGridSizer3->Add(StartModelButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton1 = new wxRadioButton(this, ID_RADIOBUTTON6, _("Controller"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON6"));
	FlexGridSizer3->Add(RadioButton1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceController = new wxChoice(this, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
	FlexGridSizer3->Add(ChoiceController, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 3);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	Connect(ID_RADIOBUTTON5,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnipChoiceSelect);
	Connect(ID_RADIOBUTTON3,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	Connect(ID_RADIOBUTTON4,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	//*)
}

StartChannelDialog::~StartChannelDialog()
{
	//(*Destroy(StartChannelDialog)
	//*)
}

void StartChannelDialog::Set(const wxString &s, const ModelManager &models) {
    _outputManager = models.GetOutputManager();
    wxString start = s;

    wxArrayString  list;
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->second->GetDisplayAs() != "ModelGroup") {
            list.push_back(it->first);
        }
    }

    OutputChoice->Freeze();
    OutputChoice->Clear();
    for (int i = 1; i <= models.GetOutputManager()->GetOutputCount(); i++)
    {
        OutputChoice->AppendString(wxString::Format(wxT("%i"), i));
    }
    OutputChoice->Thaw();

    ipChoice->Clear();
    ipChoice->AppendString("ANY");
    auto ips = models.GetOutputManager()->GetIps();
    for (auto it = ips.begin(); it != ips.end(); ++it)
    {
        if (*it != "MULTICAST")
        {
            if (!ipChoice->SetStringSelection(*it))
            {
                ipChoice->AppendString(*it);
            }
        }
    }
    ipChoice->SetStringSelection("ANY");
    SetUniverseOptionsBasedOnIP(ipChoice->GetStringSelection());

    auto outputs = _outputManager->GetOutputs();
    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if ((*it)->IsLookedUpByControllerName())
        {
            ChoiceController->AppendString((*it)->GetDescription());
        }
    }

    ModelChoice->Append(list);
    if (start.Contains(":")) {
        wxString sNet = start.SubString(0, start.Find(":")-1);
        if (sNet[0] == '!')
        {
            ChoiceController->Enable();
            RadioButton1->SetValue(true);
            OutputChoice->Disable();
            ipChoice->Disable();
            universeChoice->Disable();
            ModelChoice->Disable();
            ChoiceController->SetStringSelection(sNet.substr(1));
        }
        else if (sNet[0] == '@') {
            ModelChoice->Enable();
            StartModelButton->SetValue(true);
            OutputChoice->Disable();
            ipChoice->Disable();
            universeChoice->Disable();
            ChoiceController->Disable();
            ModelChoice->SetStringSelection(sNet.SubString(1, sNet.size()));
        }
        else if (sNet[0] == '#')
        {
            ModelChoice->Disable();
            OutputChoice->Disable();
            ipChoice->Enable();
            universeChoice->Enable();
            UniverseButton->SetValue(true);
            ChoiceController->Disable();
            wxArrayString cs = wxSplit(start.SubString(1, start.Length()), ':');
            if (cs.Count() == 3)
            {
                ipChoice->SetStringSelection(cs[0].Trim(false).Trim(true));
                SetUniverseOptionsBasedOnIP(ipChoice->GetStringSelection());
                universeChoice->SetStringSelection(cs[1].Trim(false).Trim(true));
                if (universeChoice->GetStringSelection() == "" && universeChoice->GetCount() > 0)
                {
                    universeChoice->SetSelection(0);
                }
            }
            else if (cs.Count() == 2)
            {
                ipChoice->SetStringSelection("ANY");
                SetUniverseOptionsBasedOnIP(ipChoice->GetStringSelection());
                universeChoice->SetStringSelection(cs[0].Trim(false).Trim(true));
                if (universeChoice->GetStringSelection() == "" && universeChoice->GetCount() > 0)
                {
                    universeChoice->SetSelection(0);
                }
            }
        }
        else if (sNet[0] == '>' || sNet[0] == '<' || models[sNet.ToStdString()] != nullptr) {
            ModelChoice->Enable();
            ModelButton->SetValue(true);
            OutputChoice->Disable();
            ipChoice->Disable();
            universeChoice->Disable();
            ChoiceController->Disable();
            ModelChoice->SetStringSelection(sNet[0] == '<' || sNet[0] == '>'  ? sNet.SubString(1, sNet.size()) : sNet);
        } else {
            OutputChoice->SetStringSelection(sNet);
            if (OutputChoice->GetStringSelection() == "" && OutputChoice->GetCount() > 0)
            {
                OutputChoice->SetSelection(0);
            }
            ModelChoice->Disable();
            OutputChoice->Enable();
            ipChoice->Disable();
            universeChoice->Disable();
            OutputButton->SetValue(true);
            ChoiceController->Disable();
        }
        start = start.SubString(start.Find(':', true) + 1, start.size());
    } else {
        NoneButton->SetValue(true);
        OutputChoice->Disable();
        ModelChoice->Disable();
        ipChoice->Disable();
        universeChoice->Disable();
        ChoiceController->Disable();
    }

    if (ChoiceController->GetCount() == 0)
    {
        if (RadioButton1->GetValue())
        {
            NoneButton->SetValue(true);
            OutputChoice->Disable();
            ModelChoice->Disable();
            ipChoice->Disable();
            universeChoice->Disable();
            ChoiceController->Disable();
        }
        RadioButton1->Enable(false);
        ChoiceController->Enable(false);
    }

    StartChannel->SetValue(start);
}

std::string StartChannelDialog::Get() {
    if (OutputButton->GetValue()) {
        return std::string(OutputChoice->GetStringSelection().c_str()) + ":" + std::to_string(StartChannel->GetValue());
    }
    else if (RadioButton1->GetValue())
    {
        return "!" + ChoiceController->GetStringSelection() + ":" + std::to_string(StartChannel->GetValue());
    }
    else if (UniverseButton->GetValue())
    {
        if (ipChoice->GetStringSelection() == "ANY")
        {
            return "#" + std::string(universeChoice->GetStringSelection().c_str()) + ":" + std::to_string(StartChannel->GetValue());
        }
        else
        {
            return "#" + std::string(ipChoice->GetStringSelection().c_str()) + ":" + std::string(universeChoice->GetStringSelection().c_str()) + ":" + std::to_string(StartChannel->GetValue());
        }
    }
     else if (ModelButton->GetValue()) {
        return ">" + ModelChoice->GetStringSelection().ToStdString() + ":" + std::to_string(StartChannel->GetValue());
    } else if (StartModelButton->GetValue()) {
        return "@" + ModelChoice->GetStringSelection().ToStdString() + ":" + std::to_string(StartChannel->GetValue());
    }
    return std::to_string(StartChannel->GetValue());
}

void StartChannelDialog::OnButtonSelect(wxCommandEvent& event)
{
    if (NoneButton->GetValue()) {
        ModelChoice->Disable();
        OutputChoice->Disable();
        ipChoice->Disable();
        universeChoice->Disable();
        ChoiceController->Disable();
    }
    else if (OutputButton->GetValue()) {
        ModelChoice->Disable();
        OutputChoice->Enable();
        ipChoice->Disable();
        universeChoice->Disable();
        ChoiceController->Disable();
    } else if (RadioButton1->GetValue()){
        ModelChoice->Disable();
        OutputChoice->Disable();
        ipChoice->Disable();
        universeChoice->Disable();
        ChoiceController->Enable();
    } else if (UniverseButton->GetValue()) {
        ModelChoice->Disable();
        OutputChoice->Disable();
        ipChoice->Enable();
        universeChoice->Enable();
        ChoiceController->Disable();
    } else {
        ModelChoice->Enable();
        OutputChoice->Disable();
        ipChoice->Disable();
        universeChoice->Disable();
        ChoiceController->Disable();
    }

    if (ModelButton->GetValue())
    {
        StartChannel->SetValue(1);
    }
}

void StartChannelDialog::SetUniverseOptionsBasedOnIP(wxString ip)
{
    if (_outputManager == nullptr)
        return;

    wxString uu = universeChoice->GetStringSelection();

    universeChoice->Clear();
    if (ip == "ANY")
    {
        auto uus = _outputManager->GetIPUniverses();
        std::list<int> us;
        for (auto it = uus.begin(); it != uus.end(); ++it)
        {
            us.push_back(*it);
        }
        us.sort();
        for (auto it = us.begin(); it != us.end(); ++it)
        {
            wxString u = wxString::Format(wxT("%i"), *it);
            if (!universeChoice->SetStringSelection(u))
            {
                universeChoice->AppendString(u);
            }
        }
    }
    else
    {
        std::list<int> uus = _outputManager->GetIPUniverses(ip.ToStdString());
        std::list<int> us;
        for (auto it = uus.begin(); it != uus.end(); ++it)
        {
            us.push_back(*it);
        }
        us.sort();
        for (auto it = us.begin(); it != us.end(); ++it)
        {
            wxString u = wxString::Format(wxT("%i"), *it);
            if (!universeChoice->SetStringSelection(u))
            {
                universeChoice->AppendString(u);
            }
        }
    }

    universeChoice->SetStringSelection(uu);
    if (universeChoice->GetStringSelection() == "" && universeChoice->GetCount() > 0)
    {
        universeChoice->SetSelection(0);
    }
}

void StartChannelDialog::OnipChoiceSelect(wxCommandEvent& event)
{
    SetUniverseOptionsBasedOnIP(ipChoice->GetStringSelection());
}
