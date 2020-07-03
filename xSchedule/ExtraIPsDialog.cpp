/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ExtraIPsDialog.h"

//(*InternalHeaders(ExtraIPsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "ExtraIPDialog.h"

//(*IdInit(ExtraIPsDialog)
const long ExtraIPsDialog::ID_LISTVIEW1 = wxNewId();
const long ExtraIPsDialog::ID_BUTTON2 = wxNewId();
const long ExtraIPsDialog::ID_BUTTON3 = wxNewId();
const long ExtraIPsDialog::ID_BUTTON4 = wxNewId();
const long ExtraIPsDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ExtraIPsDialog,wxDialog)
	//(*EventTable(ExtraIPsDialog)
	//*)
END_EVENT_TABLE()

ExtraIPsDialog::ExtraIPsDialog(wxWindow* parent, std::list<ExtraIP*>* extraIPs, wxWindowID id,const wxPoint& pos,const wxSize& size) : _extraIPs(extraIPs)
{
	//(*Initialize(ExtraIPsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("IPs to Monitor"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListView1 = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer1->Add(ListView1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_Add = new wxButton(this, ID_BUTTON2, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Edit = new wxButton(this, ID_BUTTON3, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Delete = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer2->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON1, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&ExtraIPsDialog::OnListView1ItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&ExtraIPsDialog::OnListView1ItemActivated);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExtraIPsDialog::OnButton_AddClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExtraIPsDialog::OnButton_EditClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExtraIPsDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExtraIPsDialog::OnButton_CloseClick);
	//*)

    ListView1->AppendColumn("IP");
    ListView1->AppendColumn("Description");

    for (auto it: *_extraIPs)
    {
        ListView1->InsertItem(ListView1->GetItemCount(), it->GetIP());
        ListView1->SetItem(ListView1->GetItemCount() - 1, 1, it->GetDescription());
    }
    ValidateWindow();
}

ExtraIPsDialog::~ExtraIPsDialog()
{
	//(*Destroy(ExtraIPsDialog)
	//*)
}


void ExtraIPsDialog::OnButton_AddClick(wxCommandEvent& event)
{
    ExtraIPDialog dlg(this, _extraIPs, nullptr);
    if (dlg.ShowModal() == wxID_OK)
    {
        _extraIPs->push_back(new ExtraIP(dlg.TextCtrl_Ip->GetValue().ToStdString(), dlg.TextCtrl_Description->GetValue().ToStdString()));
        ListView1->InsertItem(ListView1->GetItemCount(), dlg.TextCtrl_Ip->GetValue());
        ListView1->SetItem(ListView1->GetItemCount() - 1, 1, dlg.TextCtrl_Description->GetValue());
        ValidateWindow();
    }
}

void ExtraIPsDialog::EditSelected()
{
    auto ip = ListView1->GetItemText(ListView1->GetFirstSelected(), 0);

    ExtraIP* sel = nullptr;
    for (auto it : *_extraIPs)
    {
        if (*it == ip)
        {
            sel = it;
            break;
        }
    }
    wxASSERT(sel != nullptr);

    ExtraIPDialog dlg(this, _extraIPs, sel);

    if (dlg.ShowModal() == wxID_OK)
    {
        sel->SetIP(dlg.TextCtrl_Ip->GetValue().ToStdString());
        sel->SetDescription(dlg.TextCtrl_Description->GetValue().ToStdString());
        ListView1->SetItem(ListView1->GetFirstSelected(), 0, sel->GetIP());
        ListView1->SetItem(ListView1->GetFirstSelected(), 1, sel->GetDescription());
    }
    ValidateWindow();
}

void ExtraIPsDialog::OnButton_EditClick(wxCommandEvent& event)
{
    EditSelected();
}

void ExtraIPsDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    auto ip = ListView1->GetItemText(ListView1->GetFirstSelected(), 0);
    for (auto it = _extraIPs->begin(); it != _extraIPs->end(); ++it)
    {
        if ((**it) == ip)
        {
            _extraIPs->erase(it);
            break;
        }
    }
    ListView1->DeleteItem(ListView1->GetFirstSelected());
    ValidateWindow();
}

void ExtraIPsDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void ExtraIPsDialog::OnListView1ItemActivated(wxListEvent& event)
{
    EditSelected();
}

void ExtraIPsDialog::OnListView1ItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void ExtraIPsDialog::ValidateWindow()
{
    int sel = ListView1->GetSelectedItemCount();

    if (sel == 0)
    {
        Button_Delete->Disable();
        Button_Edit->Disable();
    }
    else
    {
        Button_Delete->Enable();
        Button_Edit->Enable();
    }
}
