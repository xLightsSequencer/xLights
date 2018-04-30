#include "FPPRemotesDialog.h"

//(*InternalHeaders(FPPRemotesDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/generic/textdlgg.h>
#include "../xLights/outputs/IPOutput.h"
#include "../xLights/UtilFunctions.h"

//(*IdInit(FPPRemotesDialog)
const long FPPRemotesDialog::ID_LISTBOX1 = wxNewId();
const long FPPRemotesDialog::ID_BUTTON1 = wxNewId();
const long FPPRemotesDialog::ID_BUTTON2 = wxNewId();
const long FPPRemotesDialog::ID_BUTTON3 = wxNewId();
const long FPPRemotesDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(FPPRemotesDialog,wxDialog)
	//(*EventTable(FPPRemotesDialog)
	//*)
END_EVENT_TABLE()

FPPRemotesDialog::FPPRemotesDialog(wxWindow* parent, std::list<std::string> remotes, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(FPPRemotesDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("FPP Remote IP Addresses"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListBox_Remotes = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
	FlexGridSizer1->Add(ListBox_Remotes, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_Add = new wxButton(this, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Edit = new wxButton(this, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Delete = new wxButton(this, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON4, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer1->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&FPPRemotesDialog::OnListBox_RemotesSelect);
	Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&FPPRemotesDialog::OnListBox_RemotesDClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPRemotesDialog::OnButton_AddClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPRemotesDialog::OnButton_EditClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPRemotesDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPRemotesDialog::OnButton_CloseClick);
	//*)

    for (auto it = remotes.begin(); it != remotes.end(); ++it)
    {
        ListBox_Remotes->Append(*it);
    }

    ValidateWindow();
}

FPPRemotesDialog::~FPPRemotesDialog()
{
	//(*Destroy(FPPRemotesDialog)
	//*)
}


void FPPRemotesDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void FPPRemotesDialog::OnButton_AddClick(wxCommandEvent& event)
{
    EditItem(-1);
    ValidateWindow();
}

void FPPRemotesDialog::OnButton_EditClick(wxCommandEvent& event)
{
    if (ListBox_Remotes->GetSelection() != wxNOT_FOUND)
    {
        EditItem(ListBox_Remotes->GetSelection());
    }
    ValidateWindow();
}

void FPPRemotesDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    if (ListBox_Remotes->GetSelection() != wxNOT_FOUND)
    {
        ListBox_Remotes->Delete(ListBox_Remotes->GetSelection());
    }
    ValidateWindow();
}

void FPPRemotesDialog::OnListBox_RemotesDClick(wxCommandEvent& event)
{
    if (ListBox_Remotes->GetSelection() != wxNOT_FOUND)
    {
        EditItem(ListBox_Remotes->GetSelection());
    }
    ValidateWindow();
}

void FPPRemotesDialog::OnListBox_RemotesSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void FPPRemotesDialog::EditItem(int item)
{
    wxString ip = "";
    if (item != -1)
    {
        ip = ListBox_Remotes->GetString(item);
    }

    wxTextEntryDialog dlg(this, "Remote IP Address", "IP Address", ip);
    if (dlg.ShowModal() == wxID_OK)
    {
        if (!IsIPValidOrHostname(dlg.GetValue().ToStdString()))
        {
            wxMessageBox("Illegal ip address " + dlg.GetValue());
        }
        else if (dlg.GetValue().Trim(true).Trim(false) == "255.255.255.255")
        {
            wxMessageBox("Illegal use of broadcast ip address " + dlg.GetValue());
        }
        else
        {
            bool duplicate = false;
            for (int i = 0; i < ListBox_Remotes->GetCount(); ++i)
            {
                if (ListBox_Remotes->GetString(i) == dlg.GetValue() && item != i)
                {
                    wxMessageBox("Duplicate ip address " + dlg.GetValue());
                    duplicate = true;
                }
            }

            if (!duplicate)
            {
                if (item == -1)
                {
                    ListBox_Remotes->AppendAndEnsureVisible(dlg.GetValue());
                }
                else
                {
                    ListBox_Remotes->SetString(item, dlg.GetValue());
                }
            }
        }
    }
}

std::list<std::string> FPPRemotesDialog::GetRemotes()
{
    std::list<std::string> res;

    for (int i = 0; i < ListBox_Remotes->GetCount(); ++i)
    {
        res.push_back(ListBox_Remotes->GetString(i).ToStdString());
    }

    return res;
}

void FPPRemotesDialog::ValidateWindow()
{
    if (ListBox_Remotes->GetSelection() != wxNOT_FOUND)
    {
        Button_Delete->Enable(true);
        Button_Edit->Enable(true);
    }
    else
    {
        Button_Delete->Enable(false);
        Button_Edit->Enable(false);
    }
}
