/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "MagicWordsDialog.h"
#include "MagicWordDialog.h"
#include "SMSDaemonoptions.h"
#include "MagicWord.h"

//(*InternalHeaders(MagicWordsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MagicWordsDialog)
const long MagicWordsDialog::ID_LISTVIEW1 = wxNewId();
const long MagicWordsDialog::ID_BUTTON1 = wxNewId();
const long MagicWordsDialog::ID_BUTTON2 = wxNewId();
const long MagicWordsDialog::ID_BUTTON3 = wxNewId();
const long MagicWordsDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MagicWordsDialog,wxDialog)
	//(*EventTable(MagicWordsDialog)
	//*)
END_EVENT_TABLE()

MagicWordsDialog::MagicWordsDialog(wxWindow* parent, SMSDaemonOptions* options, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _options = options;

	//(*Initialize(MagicWordsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Magic Words"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListView_Words = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer1->Add(ListView_Words, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_Add = new wxButton(this, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Edit = new wxButton(this, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Delete = new wxButton(this, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON4, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer1->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&MagicWordsDialog::OnListView_EventsItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&MagicWordsDialog::OnListView_EventsItemActivated);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MagicWordsDialog::OnButton_AddClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MagicWordsDialog::OnButton_EditClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MagicWordsDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MagicWordsDialog::OnButton_CloseClick);
	//*)

    LoadList();

    ValidateWindow();
}

void MagicWordsDialog::LoadList()
{
    ListView_Words->ClearAll();
    ListView_Words->AppendColumn("Magic words");

    for (const auto& it : _options->GetMagicWords())
    {
        ListView_Words->InsertItem(ListView_Words->GetItemCount(), it->GetMagicWord());
        ListView_Words->SetItemData(ListView_Words->GetItemCount() - 1, it->GetId());
    }
}

MagicWordsDialog::~MagicWordsDialog()
{
	//(*Destroy(MagicWordsDialog)
	//*)
}


void MagicWordsDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void MagicWordsDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    int id = ListView_Words->GetItemData(ListView_Words->GetFirstSelected());

    for (auto it = _options->GetMagicWords().begin(); it != _options->GetMagicWords().end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            auto todelete = *it;
            _options->GetMagicWords().remove(*it);
            delete todelete;
            _options->SetDirty();
            LoadList();
            break;
        }
    }

    ValidateWindow();
}

void MagicWordsDialog::OnButton_EditClick(wxCommandEvent& event)
{
    EditSelected();
}

void MagicWordsDialog::EditSelected()
{
    int id = ListView_Words->GetItemData(ListView_Words->GetFirstSelected());
    MagicWord* e = nullptr;
    for (const auto& it : _options->GetMagicWords())
    {
        if (it->GetId() == id)
        {
            e = it;
            break;
        }
    }

    if (e != nullptr)
    {
        MagicWordDialog dlg(this, e);
        if (dlg.ShowModal() == wxID_OK)
        {
            // update should have occurred in the dialog
        }
    }

    LoadList();

    ValidateWindow();
}

void MagicWordsDialog::OnButton_AddClick(wxCommandEvent& event)
{
    MagicWordDialog dlg(this, nullptr);
    if (dlg.ShowModal() == wxID_OK)
    {
        // update should have occurred in the dialog
        _options->GetMagicWords().push_back(dlg.GetMagicWord());
    }

    LoadList();

    ValidateWindow();
}

void MagicWordsDialog::ValidateWindow()
{
    if (ListView_Words->GetSelectedItemCount() > 0)
    {
        Button_Edit->Enable();
        Button_Delete->Enable();
    }
    else
    {
        Button_Edit->Enable(false);
        Button_Delete->Enable(false);
    }

    ListView_Words->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListView_Words->GetColumnWidth(0) < 80)
        ListView_Words->SetColumnWidth(0, 80);
}

void MagicWordsDialog::OnListView_EventsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void MagicWordsDialog::OnListView_EventsItemActivated(wxListEvent& event)
{
    EditSelected();
}
