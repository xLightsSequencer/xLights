#include "OptionsDialog.h"

#include "ScheduleOptions.h"
#include "ButtonDetailsDialog.h"
#include "ProjectorDetailsDialog.h"
#include "UserButton.h"
#include "Projector.h"

//(*InternalHeaders(OptionsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(OptionsDialog)
const long OptionsDialog::ID_CHECKBOX4 = wxNewId();
const long OptionsDialog::ID_CHECKBOX3 = wxNewId();
const long OptionsDialog::ID_CHECKBOX5 = wxNewId();
const long OptionsDialog::ID_CHECKBOX2 = wxNewId();
const long OptionsDialog::ID_STATICTEXT1 = wxNewId();
const long OptionsDialog::ID_LISTVIEW2 = wxNewId();
const long OptionsDialog::ID_BUTTON4 = wxNewId();
const long OptionsDialog::ID_BUTTON8 = wxNewId();
const long OptionsDialog::ID_BUTTON3 = wxNewId();
const long OptionsDialog::ID_STATICTEXT2 = wxNewId();
const long OptionsDialog::ID_LISTVIEW1 = wxNewId();
const long OptionsDialog::ID_BUTTON5 = wxNewId();
const long OptionsDialog::ID_BUTTON6 = wxNewId();
const long OptionsDialog::ID_BUTTON7 = wxNewId();
const long OptionsDialog::ID_STATICTEXT3 = wxNewId();
const long OptionsDialog::ID_SPINCTRL1 = wxNewId();
const long OptionsDialog::ID_STATICTEXT4 = wxNewId();
const long OptionsDialog::ID_TEXTCTRL1 = wxNewId();
const long OptionsDialog::ID_CHECKBOX1 = wxNewId();
const long OptionsDialog::ID_STATICTEXT5 = wxNewId();
const long OptionsDialog::ID_TEXTCTRL2 = wxNewId();
const long OptionsDialog::ID_STATICTEXT6 = wxNewId();
const long OptionsDialog::ID_SPINCTRL2 = wxNewId();
const long OptionsDialog::ID_BUTTON1 = wxNewId();
const long OptionsDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OptionsDialog,wxDialog)
	//(*EventTable(OptionsDialog)
	//*)
END_EVENT_TABLE()

OptionsDialog::OptionsDialog(wxWindow* parent, ScheduleOptions* options, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _options = options;
    _dragging = false;

	//(*Initialize(OptionsDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Options"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	CheckBox_SimpleMode = new wxCheckBox(this, ID_CHECKBOX4, _("Advanced Mode"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_SimpleMode->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SimpleMode, 1, wxALL|wxEXPAND, 5);
	CheckBox_SendOffWhenNotRunning = new wxCheckBox(this, ID_CHECKBOX3, _("Send data when not running sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_SendOffWhenNotRunning->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SendOffWhenNotRunning, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_RunBackground = new wxCheckBox(this, ID_CHECKBOX5, _("Run background playlist when not running sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_RunBackground->SetValue(false);
	FlexGridSizer1->Add(CheckBox_RunBackground, 1, wxALL|wxEXPAND, 5);
	CheckBox_Sync = new wxCheckBox(this, ID_CHECKBOX2, _("Use ArtNet/E1.31 Synchronisation Protocols"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_Sync->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Sync, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableRow(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Projectors:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	ListView_Projectors = new wxListView(this, ID_LISTVIEW2, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_SORT_HEADER|wxVSCROLL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_LISTVIEW2"));
	FlexGridSizer3->Add(ListView_Projectors, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_AddProjector = new wxButton(this, ID_BUTTON4, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer4->Add(Button_AddProjector, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ProjectorEdit = new wxButton(this, ID_BUTTON8, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	FlexGridSizer4->Add(Button_ProjectorEdit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_DeleteProjector = new wxButton(this, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer4->Add(Button_DeleteProjector, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	FlexGridSizer5->AddGrowableRow(0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Buttons:    "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer5->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	ListView_Buttons = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_SORT_HEADER|wxVSCROLL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer5->Add(ListView_Buttons, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_ButtonAdd = new wxButton(this, ID_BUTTON5, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer6->Add(Button_ButtonAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ButtonEdit = new wxButton(this, ID_BUTTON6, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer6->Add(Button_ButtonEdit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ButtonDelete = new wxButton(this, ID_BUTTON7, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	FlexGridSizer6->Add(Button_ButtonDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Web Server Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer8->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_WebServerPort = new wxSpinCtrl(this, ID_SPINCTRL1, _T("80"), wxDefaultPosition, wxDefaultSize, 0, 1, 64000, 80, _T("ID_SPINCTRL1"));
	SpinCtrl_WebServerPort->SetValue(_T("80"));
	FlexGridSizer8->Add(SpinCtrl_WebServerPort, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Web Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer8->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_wwwRoot = new wxTextCtrl(this, ID_TEXTCTRL1, _("xScheduleWeb"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer8->Add(TextCtrl_wwwRoot, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer8->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_APIOnly = new wxCheckBox(this, ID_CHECKBOX1, _("API Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_APIOnly->SetValue(false);
	FlexGridSizer8->Add(CheckBox_APIOnly, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Password:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer8->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Password = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer8->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Timeout (minutes):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer8->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_PasswordTimeout = new wxSpinCtrl(this, ID_SPINCTRL2, _T("30"), wxDefaultPosition, wxDefaultSize, 0, 1, 1440, 30, _T("ID_SPINCTRL2"));
	SpinCtrl_PasswordTimeout->SetValue(_T("30"));
	FlexGridSizer8->Add(SpinCtrl_PasswordTimeout, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW2,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&OptionsDialog::OnListView_ProjectorsItemSelect);
	Connect(ID_LISTVIEW2,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&OptionsDialog::OnListView_ProjectorsItemActivated);
	Connect(ID_LISTVIEW2,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&OptionsDialog::OnListView_ProjectorsKeyDown);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_AddProjectorClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ProjectorEditClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_DeleteProjectorClick);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsBeginDrag);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsItemActivated);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsKeyDown);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonAddClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonEditClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonDeleteClick);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OptionsDialog::OnTextCtrl_wwwRootText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_CancelClick);
	//*)

    ListView_Projectors->AppendColumn("Name");
    ListView_Projectors->AppendColumn("IP");
    ListView_Projectors->AppendColumn("Password");

    ListView_Buttons->AppendColumn("Label");
    ListView_Buttons->AppendColumn("Command");
    ListView_Buttons->AppendColumn("Parameters");
    ListView_Buttons->AppendColumn("Hotkey");
    ListView_Buttons->AppendColumn("Web Color");

    CheckBox_SendOffWhenNotRunning->SetValue(options->IsSendOffWhenNotRunning());
    CheckBox_RunBackground->SetValue(options->IsSendBackgroundWhenNotRunning());
    CheckBox_Sync->SetValue(options->IsSync());
    CheckBox_APIOnly->SetValue(options->GetAPIOnly());
    CheckBox_SimpleMode->SetValue(options->IsAdvancedMode());

    SpinCtrl_WebServerPort->SetValue(options->GetWebServerPort());
    SpinCtrl_PasswordTimeout->SetValue(options->GetPasswordTimeout());

    TextCtrl_wwwRoot->SetValue(options->GetWWWRoot());
    TextCtrl_Password->SetValue(options->GetPassword());

    LoadProjectors();

    LoadButtons();

    SetSize(800, 700);
    Layout();

    int namew = 150;
    int hotkeyw = 70;
    int w, h;
    int colorw = 150;
    ListView_Buttons->GetSize(&w, &h);
    ListView_Buttons->SetColumnWidth(0, namew);
    ListView_Buttons->SetColumnWidth(1, (w - colorw - namew - hotkeyw - 1) / 2);
    ListView_Buttons->SetColumnWidth(2, (w - colorw - namew - hotkeyw - 1) / 2);
    ListView_Buttons->SetColumnWidth(3, hotkeyw);
    ListView_Buttons->SetColumnWidth(4, colorw);
    ListView_Projectors->SetColumnWidth(0, namew);
    ListView_Projectors->SetColumnWidth(1, (w - namew - 1) / 2);
    ListView_Projectors->SetColumnWidth(2, (w - namew - 1) / 2);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());

    ValidateWindow();
}

void OptionsDialog::LoadProjectors()
{
    ListView_Projectors->DeleteAllItems();
    auto ps = _options->GetProjectors();
    int i = 0;
    for (auto it = ps.begin(); it != ps.end(); ++it)
    {
        ListView_Projectors->InsertItem(i, (*it)->GetName());
        ListView_Projectors->SetItem(i, 1, (*it)->GetIP());
        ListView_Projectors->SetItem(i, 2, (*it)->GetPassword());
        i++;
    }
}

void OptionsDialog::LoadButtons()
{
    ListView_Buttons->DeleteAllItems();
    auto ps = _options->GetButtons();
    int i = 0;
    for (auto it = ps.begin(); it != ps.end(); ++it)
    {
        ListView_Buttons->InsertItem(i, (*it)->GetLabel());
        ListView_Buttons->SetItem(i, 1, (*it)->GetCommand());
        ListView_Buttons->SetItem(i, 2, (*it)->GetParameters());
        if ((*it)->GetHotkey() == '~')
        {
            ListView_Buttons->SetItem(i, 3, "");
        }
        else
        {
            ListView_Buttons->SetItem(i, 3, (*it)->GetHotkey());
        }
        ListView_Buttons->SetItem(i, 4, (*it)->GetColorName());
        i++;
    }
}

OptionsDialog::~OptionsDialog()
{
	//(*Destroy(OptionsDialog)
	//*)
}


void OptionsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _options->SetSync(CheckBox_Sync->GetValue());
    _options->SetSendOffWhenNotRunning(CheckBox_SendOffWhenNotRunning->GetValue());
    _options->SetSendBackgroundWhenNotRunning(CheckBox_RunBackground->GetValue());
    _options->SetWebServerPort(SpinCtrl_WebServerPort->GetValue());
    _options->SetWWWRoot(TextCtrl_wwwRoot->GetValue().ToStdString());
    _options->SetAPIOnly(CheckBox_APIOnly->GetValue());
    _options->SetPassword(TextCtrl_Password->GetValue().ToStdString());
    _options->SetPasswordTimeout(SpinCtrl_PasswordTimeout->GetValue());
    _options->SetAdvancedMode(CheckBox_SimpleMode->GetValue());

    _options->ClearProjectors();
    for (int i = 0; i < ListView_Projectors->GetItemCount(); i++)
    {
        _options->AddProjector(ListView_Projectors->GetItemText(i, 0).ToStdString(),
                                ListView_Projectors->GetItemText(i, 1).ToStdString(),
                                ListView_Projectors->GetItemText(i, 2).ToStdString());
    }

    _options->ClearButtons();
    for (int i = 0; i < ListView_Buttons->GetItemCount(); i++)
    {
        char hotkey = '~';
        if (ListView_Buttons->GetItemText(i, 3).Length() > 0)
        {
            hotkey = ListView_Buttons->GetItemText(i, 3)[0];
        }
        _options->AddButton(ListView_Buttons->GetItemText(i, 0).ToStdString(), 
                            ListView_Buttons->GetItemText(i, 1).ToStdString(), 
                            ListView_Buttons->GetItemText(i, 2).ToStdString(), 
                            hotkey,
                            ListView_Buttons->GetItemText(i, 4).ToStdString());
    }

    EndDialog(wxID_OK);
}

void OptionsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void OptionsDialog::OnButton_AddProjectorClick(wxCommandEvent& event)
{
    std::string projector = "";
    std::string ip = "";
    std::string password = "";

    ProjectorDetailsDialog dlg(this, projector, ip, password);

    if (dlg.ShowModal() == wxID_OK)
    {
        int row = ListView_Projectors->GetItemCount();
        ListView_Projectors->InsertItem(row, projector);
        ListView_Projectors->SetItem(row, 1, ip);
        ListView_Projectors->SetItem(row, 2, password);
    }

    ValidateWindow();
}

void OptionsDialog::OnButton_ProjectorEditClick(wxCommandEvent& event)
{
    if (ListView_Projectors->GetSelectedItemCount() != 1) return;

    int row = ListView_Projectors->GetFirstSelected();

    EditProjector(row);
}

void OptionsDialog::EditProjector(int row)
{
    std::string projector = ListView_Projectors->GetItemText(row, 0).ToStdString();
    std::string ip = ListView_Projectors->GetItemText(row, 1).ToStdString();
    std::string password = ListView_Projectors->GetItemText(row, 2).ToStdString();

    ProjectorDetailsDialog dlg(this, projector, ip, password);

    if (dlg.ShowModal() == wxID_OK)
    {
        ListView_Projectors->SetItemText(row, projector);
        ListView_Projectors->SetItem(row, 1, ip);
        ListView_Projectors->SetItem(row, 2, password);
    }

    ValidateWindow();
}

void OptionsDialog::OnButton_DeleteProjectorClick(wxCommandEvent& event)
{
    if (ListView_Projectors->GetSelectedItemCount() != 1) return;

    int row = ListView_Projectors->GetFirstSelected();
    ListView_Projectors->DeleteItem(row);

    ValidateWindow();
}

void OptionsDialog::OnButton_ButtonAddClick(wxCommandEvent& event)
{
    std::string label = "";
    std::string command = "";
    std::string parameter = "";
    std::string color = "default";
    char hotkey = '~';

    ButtonDetailsDialog dlg(this, label, command, color, parameter, hotkey);

    if (dlg.ShowModal() == wxID_OK)
    {
        int row = ListView_Buttons->GetItemCount();
        ListView_Buttons->InsertItem(row, label);
        ListView_Buttons->SetItem(row, 1, command);
        ListView_Buttons->SetItem(row, 2, parameter);
        ListView_Buttons->SetItem(row, 3, hotkey);
        ListView_Buttons->SetItem(row, 4, color);
    }

    ValidateWindow();
}

void OptionsDialog::OnButton_ButtonEditClick(wxCommandEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() != 1) return;

    int row = ListView_Buttons->GetFirstSelected();

    EditButton(row);
}

void OptionsDialog::EditButton(int row)
{
    std::string label = ListView_Buttons->GetItemText(row, 0).ToStdString();
    std::string command = ListView_Buttons->GetItemText(row, 1).ToStdString();
    std::string parameter = ListView_Buttons->GetItemText(row, 2).ToStdString();
    char hotkey = '~';
    if (ListView_Buttons->GetItemText(row, 3).Length() > 0)
    {
        hotkey = ListView_Buttons->GetItemText(row, 3)[0];
    }
    std::string color = ListView_Buttons->GetItemText(row, 4).ToStdString();

    ButtonDetailsDialog dlg(this, label, command, color, parameter, hotkey);

    if (dlg.ShowModal() == wxID_OK)
    {
        ListView_Buttons->SetItemText(row, label);
        ListView_Buttons->SetItem(row, 1, command);
        ListView_Buttons->SetItem(row, 2, parameter);
        ListView_Buttons->SetItem(row, 3, hotkey);
        ListView_Buttons->SetItem(row, 4, color);
    }

    ValidateWindow();
}

void OptionsDialog::OnButton_ButtonDeleteClick(wxCommandEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() != 1) return;

    ListView_Buttons->DeleteItem(ListView_Buttons->GetFirstSelected());

    ValidateWindow();
}

void OptionsDialog::ValidateWindow()
{
    if (ListView_Buttons->GetSelectedItemCount() == 1)
    {
        Button_ButtonDelete->Enable(true);
        Button_ButtonEdit->Enable(true);
    }
    else
    {
        Button_ButtonDelete->Enable(false);
        Button_ButtonEdit->Enable(false);
    }

    if (ListView_Projectors->GetSelectedItemCount() == 1)
    {
        Button_DeleteProjector->Enable(true);
        Button_ProjectorEdit->Enable(true);
    }
    else
    {
        Button_DeleteProjector->Enable(false);
        Button_ProjectorEdit->Enable(false);
    }

    if (TextCtrl_wwwRoot->GetValue() == "")
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable();
    }
}

void OptionsDialog::OnTextCtrl_wwwRootText(wxCommandEvent& event)
{
    ValidateWindow();
}

void OptionsDialog::OnListView_ButtonsBeginDrag(wxListEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() != 1) return;

    ListView_Buttons->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(OptionsDialog::OnButtonsDragEnd), nullptr, this);
    // trigger when user leaves window to abort drag
    ListView_Buttons->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OptionsDialog::OnButtonsDragQuit), nullptr, this);
    // trigger when mouse moves
    ListView_Buttons->Connect(wxEVT_MOTION, wxMouseEventHandler(OptionsDialog::OnButtonsMouseMove), nullptr, this);

    _dragging = true;
    SetCursor(wxCURSOR_HAND);
}

// abort dragging a list item because user has left window
void OptionsDialog::OnButtonsDragQuit(wxMouseEvent& event)
{
    // restore cursor and disconnect unconditionally
    SetCursor(wxCURSOR_ARROW);
    ListView_Buttons->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(OptionsDialog::OnButtonsDragEnd));
    ListView_Buttons->Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OptionsDialog::OnButtonsDragQuit));
    ListView_Buttons->Disconnect(wxEVT_MOTION, wxMouseEventHandler(OptionsDialog::OnButtonsMouseMove));
    //HighlightDropItem(nullptr);
}

void OptionsDialog::OnButtonsDragEnd(wxMouseEvent& event)
{
    int flags = wxLIST_HITTEST_ONITEM;
    int dropitem = ListView_Buttons->HitTest(event.GetPosition(), flags);

    if (dropitem >= 0 && dropitem < ListView_Buttons->GetItemCount())
    {
        int dragitem = ListView_Buttons->GetFirstSelected();

        if (dragitem >= 0 && dragitem < ListView_Buttons->GetItemCount())
        {
            // move drag item below drop item
            if (dragitem != dropitem)
            {
                std::string label = ListView_Buttons->GetItemText(dragitem, 0).ToStdString();
                std::string command = ListView_Buttons->GetItemText(dragitem, 1).ToStdString();
                std::string parameters = ListView_Buttons->GetItemText(dragitem, 2).ToStdString();
                std::string hotkey = ListView_Buttons->GetItemText(dragitem, 3).ToStdString();
                std::string color = ListView_Buttons->GetItemText(dragitem, 4).ToStdString();

                ListView_Buttons->DeleteItem(dragitem);

                if (dropitem > dragitem) dropitem--;

                ListView_Buttons->InsertItem(dropitem + 1, label);
                ListView_Buttons->SetItem(dropitem + 1, 1, command);
                ListView_Buttons->SetItem(dropitem + 1, 2, parameters);
                ListView_Buttons->SetItem(dropitem + 1, 3, hotkey);
                ListView_Buttons->SetItem(dropitem + 1, 4, color);

                ListView_Buttons->EnsureVisible(dropitem + 1);

                if (dropitem + 1 == ListView_Buttons->GetItemCount() - 1)
                {
                    ListView_Buttons->ScrollLines(1);
                }
            }
        }
    }

    _dragging = false;
    SetCursor(wxCURSOR_ARROW);
    //HighlightDropItem(nullptr);

    // disconnect both functions
    ListView_Buttons->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(OptionsDialog::OnButtonsDragEnd));
    ListView_Buttons->Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OptionsDialog::OnButtonsDragQuit));
    ListView_Buttons->Disconnect(wxEVT_MOTION, wxMouseEventHandler(OptionsDialog::OnButtonsMouseMove));
}

void OptionsDialog::OnButtonsMouseMove(wxMouseEvent& event)
{
    int flags = wxLIST_HITTEST_ONITEM;
    int dropitem = ListView_Buttons->HitTest(event.GetPosition(), flags);
    int topitem = ListView_Buttons->GetTopItem();
    int bottomitem = topitem + ListView_Buttons->GetCountPerPage();

    if (topitem == dropitem && topitem > 0)
    {
        ListView_Buttons->EnsureVisible(topitem - 1);
    }
    else if (bottomitem == dropitem && dropitem < ListView_Buttons->GetItemCount() - 1)
    {
        ListView_Buttons->EnsureVisible(bottomitem + 1);
    }

    if (dropitem == ListView_Buttons->GetItemCount() - 1)
    {
        ListView_Buttons->ScrollLines(1);
    }

    SetCursor(wxCURSOR_HAND);
    //HighlightDropItem(&dropitem);
}

void OptionsDialog::OnListView_ButtonsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void OptionsDialog::OnListView_ButtonsItemActivated(wxListEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() >= 0)
    {
        EditButton(ListView_Buttons->GetFirstSelected());
    }
    ValidateWindow();
}

void OptionsDialog::OnListView_ButtonsKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        if (ListView_Buttons->GetSelectedItemCount() >= 0)
        {
            ListView_Buttons->DeleteItem(ListView_Buttons->GetFirstSelected());
        }
    }
    ValidateWindow();
}

void OptionsDialog::OnListView_ProjectorsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void OptionsDialog::OnListView_ProjectorsItemActivated(wxListEvent& event)
{
    if (ListView_Projectors->GetSelectedItemCount() >= 0)
    {
        EditProjector(ListView_Projectors->GetFirstSelected());
    }
    ValidateWindow();
}

void OptionsDialog::OnListView_ProjectorsKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        if (ListView_Projectors->GetSelectedItemCount() >= 0)
        {
            ListView_Projectors->DeleteItem(ListView_Projectors->GetFirstSelected());
        }
    }
    ValidateWindow();
}
