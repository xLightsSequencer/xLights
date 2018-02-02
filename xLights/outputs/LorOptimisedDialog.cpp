#include "LorOptimisedDialog.h"

//(*InternalHeaders(LorOptimisedDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "SerialOutput.h"
#include "LOROptimisedOutput.h"
#include "OutputManager.h"
#include "LorControllers.h"
#include "LorController.h"
#include "LorControllerDialog.h"

//(*IdInit(LorOptimisedDialog)
const long LorOptimisedDialog::ID_STATICTEXT_EXPLANATION = wxNewId();
const long LorOptimisedDialog::ID_STATICTEXT_PORT = wxNewId();
const long LorOptimisedDialog::ID_CHOICE_PORT = wxNewId();
const long LorOptimisedDialog::ID_STATICTEXT_RATE = wxNewId();
const long LorOptimisedDialog::ID_CHOICE_BAUD_RATE = wxNewId();
const long LorOptimisedDialog::ID_STATICTEXT2 = wxNewId();
const long LorOptimisedDialog::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long LorOptimisedDialog::ID_LISTVIEW_CONTROLLERS = wxNewId();
const long LorOptimisedDialog::ID_BUTTON_ADD = wxNewId();
const long LorOptimisedDialog::ID_BUTTON_EDIT = wxNewId();
const long LorOptimisedDialog::ID_BUTTON_DELETE = wxNewId();
const long LorOptimisedDialog::ID_BUTTON_OK = wxNewId();
//*)

BEGIN_EVENT_TABLE(LorOptimisedDialog,wxDialog)
	//(*EventTable(LorOptimisedDialog)
	//*)
END_EVENT_TABLE()

LorOptimisedDialog::LorOptimisedDialog(wxWindow* parent, LOROptimisedOutput** serial, LorControllers* lorControllers, OutputManager* outputManager, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _lorControllers = lorControllers;
    _original = *serial;
    _serial = serial;
    _outputManager = outputManager;

	//(*Initialize(LorOptimisedDialog)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, _("LOR Setup"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, wxEmptyString);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticTextExplanation = new wxStaticText(this, ID_STATICTEXT_EXPLANATION, _("LOR output designed to use some of the more advanced commands to\nreduce the amount of bytes sent out the LOR network.\n\nSetup all the controllers that are on this LOR network so that xLights\nwill know which Unit ID\'s to utilize for each controller."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_EXPLANATION"));
	FlexGridSizer3->Add(StaticTextExplanation, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Connection Details"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticTextPort = new wxStaticText(this, ID_STATICTEXT_PORT, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PORT"));
	FlexGridSizer2->Add(StaticTextPort, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ChoicePort = new wxChoice(this, ID_CHOICE_PORT, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PORT"));
	FlexGridSizer2->Add(ChoicePort, 1, wxALL, 5);
	StaticTextRate = new wxStaticText(this, ID_STATICTEXT_RATE, _("Baud Rate"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_RATE"));
	FlexGridSizer2->Add(StaticTextRate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceBaudRate = new wxChoice(this, ID_CHOICE_BAUD_RATE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BAUD_RATE"));
	ChoiceBaudRate->Append(_("9600"));
	ChoiceBaudRate->Append(_("19200"));
	ChoiceBaudRate->Append(_("38400"));
	ChoiceBaudRate->Append(_("57600"));
	ChoiceBaudRate->Append(_("115200"));
	ChoiceBaudRate->Append(_("250000"));
	ChoiceBaudRate->Append(_("512000"));
	FlexGridSizer2->Add(ChoiceBaudRate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxSize(271,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_DESCRIPTION"));
	TextCtrl_Description->SetMaxLength(64);
	FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Controllers"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	ListView_Controllers = new wxListView(this, ID_LISTVIEW_CONTROLLERS, wxDefaultPosition, wxDLG_UNIT(this,wxSize(240,100)), wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, _T("ID_LISTVIEW_CONTROLLERS"));
	FlexGridSizer4->Add(ListView_Controllers, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_Add = new wxButton(this, ID_BUTTON_ADD, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD"));
	FlexGridSizer6->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Edit = new wxButton(this, ID_BUTTON_EDIT, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EDIT"));
	FlexGridSizer6->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Delete = new wxButton(this, ID_BUTTON_DELETE, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE"));
	FlexGridSizer6->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer3->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OK"));
	FlexGridSizer5->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE_PORT,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LorOptimisedDialog::OnChoicePortSelect);
	Connect(ID_CHOICE_BAUD_RATE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LorOptimisedDialog::OnChoiceBaudRateSelect);
	Connect(ID_TEXTCTRL_DESCRIPTION,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&LorOptimisedDialog::OnTextCtrl_DescriptionText);
	Connect(ID_LISTVIEW_CONTROLLERS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&LorOptimisedDialog::OnListView_ControllersItemSelect);
	Connect(ID_LISTVIEW_CONTROLLERS,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&LorOptimisedDialog::OnListView_ControllersItemSelect);
	Connect(ID_BUTTON_ADD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LorOptimisedDialog::OnButton_AddClick);
	Connect(ID_BUTTON_EDIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LorOptimisedDialog::OnButton_EditClick);
	Connect(ID_BUTTON_DELETE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LorOptimisedDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LorOptimisedDialog::OnButton_OkClick);
	//*)

    auto ports = SerialOutput::GetPossibleSerialPorts();
    for (auto it = ports.begin(); it != ports.end(); ++it)
    {
        ChoicePort->Append(*it);
    }

    FlexGridSizer1->Fit(this);

    ChoicePort->SetStringSelection((*_serial)->GetCommPort());
    if ((*_serial)->GetBaudRate() == 0)
    {
        // dont select anything
    }
    else
    {
        ChoiceBaudRate->SetStringSelection(wxString::Format(wxT("%i"), (*_serial)->GetBaudRate()));
    }
    TextCtrl_Description->SetValue((*_serial)->GetDescription());

    Button_Ok->SetDefault();

    LoadList();

    ValidateWindow();
}

LorOptimisedDialog::~LorOptimisedDialog()
{
	//(*Destroy(LorOptimisedDialog)
	//*)
}

void LorOptimisedDialog::OnButton_OkClick(wxCommandEvent& event)
{
    (*_serial)->SetCommPort(ChoicePort->GetStringSelection().ToStdString());
    if (ChoiceBaudRate->IsEnabled())
    {
        (*_serial)->SetBaudRate(wxAtoi(ChoiceBaudRate->GetStringSelection()));
    }
    (*_serial)->SetDescription(TextCtrl_Description->GetValue().ToStdString());

    EndDialog(wxID_OK);
}

void LorOptimisedDialog::OnChoicePortSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void LorOptimisedDialog::OnChoiceBaudRateSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void LorOptimisedDialog::LoadList()
{
    ListView_Controllers->ClearAll();
    ListView_Controllers->AppendColumn("Type");
    ListView_Controllers->AppendColumn("Unit ID");
    ListView_Controllers->AppendColumn("# Channels");
    ListView_Controllers->AppendColumn("Addr Mode");
    ListView_Controllers->AppendColumn("Description");

    for (auto it = _lorControllers->GetControllers()->begin(); it != _lorControllers->GetControllers()->end(); ++it)
    {
        ListView_Controllers->InsertItem(ListView_Controllers->GetItemCount(), (*it)->GetType());
        ListView_Controllers->SetItemData(ListView_Controllers->GetItemCount() - 1, (*it)->GetUnitId());
        ListView_Controllers->SetItem(ListView_Controllers->GetItemCount() - 1, 1, wxString::Format("%d", (*it)->GetUnitId()));
        ListView_Controllers->SetItem(ListView_Controllers->GetItemCount() - 1, 2, wxString::Format("%d", (*it)->GetNumChannels()));
        ListView_Controllers->SetItem(ListView_Controllers->GetItemCount() - 1, 3, (*it)->GetModeString());
        ListView_Controllers->SetItem(ListView_Controllers->GetItemCount() - 1, 4, (*it)->GetDescription());
    }
}

void LorOptimisedDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    int id = ListView_Controllers->GetItemData(ListView_Controllers->GetFirstSelected());

    for (auto it = _lorControllers->GetControllers()->begin(); it != _lorControllers->GetControllers()->end(); ++it)
    {
        if ((*it)->GetUnitId() == id)
        {
            auto todelete = *it;
            _lorControllers->GetControllers()->remove(*it);
            delete todelete;
            _lorControllers->SetDirty();
            LoadList();
            break;
        }
    }

    ValidateWindow();
}

void LorOptimisedDialog::OnButton_EditClick(wxCommandEvent& event)
{
    EditSelected();
}

void LorOptimisedDialog::EditSelected()
{
    int id = ListView_Controllers->GetItemData(ListView_Controllers->GetFirstSelected());
    LorController* e = nullptr;
    for (auto it = _lorControllers->GetControllers()->begin(); it != _lorControllers->GetControllers()->end(); ++it)
    {
        if ((*it)->GetUnitId() == id)
        {
            e = *it;
            break;
        }
    }

    if (e != nullptr)
    {
        LorControllerDialog dlg(this, e);
        if (dlg.ShowModal() == wxID_OK)
        {
            // update should have occurred in the dialog
        }
    }

    LoadList();

    ValidateWindow();
}

void LorOptimisedDialog::OnButton_AddClick(wxCommandEvent& event)
{
    LorControllerDialog dlg(this, nullptr);
    if (dlg.ShowModal() == wxID_OK)
    {
        // update should have occurred in the dialog
        _lorControllers->GetControllers()->push_back(dlg.GetController());
    }

    LoadList();

    ValidateWindow();
}

void LorOptimisedDialog::OnTextCtrl_DescriptionText(wxCommandEvent& event)
{
}

void LorOptimisedDialog::OnListView_ControllersItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void LorOptimisedDialog::ValidateWindow()
{
    if (ListView_Controllers->GetSelectedItemCount() > 0)
    {
        Button_Edit->Enable();
        Button_Delete->Enable();
    }
    else
    {
        Button_Edit->Enable(false);
        Button_Delete->Enable(false);
    }

    ListView_Controllers->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListView_Controllers->GetColumnWidth(0) < 100)
        ListView_Controllers->SetColumnWidth(0, 100);
    ListView_Controllers->SetColumnWidth(1, wxLIST_AUTOSIZE);
    if (ListView_Controllers->GetColumnWidth(1) < 80)
        ListView_Controllers->SetColumnWidth(1, 80);
    ListView_Controllers->SetColumnWidth(2, wxLIST_AUTOSIZE);
    if (ListView_Controllers->GetColumnWidth(2) < 80)
        ListView_Controllers->SetColumnWidth(2, 80);
    ListView_Controllers->SetColumnWidth(3, wxLIST_AUTOSIZE);
    if (ListView_Controllers->GetColumnWidth(3) < 80)
        ListView_Controllers->SetColumnWidth(3, 80);
    if (ListView_Controllers->GetColumnWidth(4) < 100)
        ListView_Controllers->SetColumnWidth(4, 100);

    if (ChoicePort->GetStringSelection().IsEmpty() ||
        (ChoiceBaudRate->IsEnabled() && ChoiceBaudRate->GetStringSelection() == ""))
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}
