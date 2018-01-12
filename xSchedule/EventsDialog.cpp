#include "EventsDialog.h"
#include "ScheduleOptions.h"
#include "events/EventBase.h"
#include "EventDialog.h"

//(*InternalHeaders(EventsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventsDialog)
const long EventsDialog::ID_LISTVIEW1 = wxNewId();
const long EventsDialog::ID_BUTTON1 = wxNewId();
const long EventsDialog::ID_BUTTON2 = wxNewId();
const long EventsDialog::ID_BUTTON3 = wxNewId();
const long EventsDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventsDialog,wxDialog)
	//(*EventTable(EventsDialog)
	//*)
END_EVENT_TABLE()

EventsDialog::EventsDialog(wxWindow* parent, ScheduleOptions* scheduleOptions,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _scheduleOptions = scheduleOptions;

	//(*Initialize(EventsDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListView_Events = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer1->Add(ListView_Events, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&EventsDialog::OnListView_EventsItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&EventsDialog::OnListView_EventsItemActivated);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EventsDialog::OnButton_AddClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EventsDialog::OnButton_EditClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EventsDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EventsDialog::OnButton_CloseClick);
	//*)

    LoadList();

    ValidateWindow();
}

void EventsDialog::LoadList()
{
    ListView_Events->ClearAll();
    ListView_Events->AppendColumn("Type");
    ListView_Events->AppendColumn("Name");

    for (auto it = _scheduleOptions->GetEvents()->begin(); it != _scheduleOptions->GetEvents()->end(); ++it)
    {
        ListView_Events->InsertItem(ListView_Events->GetItemCount(), (*it)->GetType());
        ListView_Events->SetItem(ListView_Events->GetItemCount() - 1, 1, (*it)->GetName());
        ListView_Events->SetItemData(ListView_Events->GetItemCount() - 1, (*it)->GetId());
    }
}

EventsDialog::~EventsDialog()
{
	//(*Destroy(EventsDialog)
	//*)
}


void EventsDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void EventsDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    int id = ListView_Events->GetItemData(ListView_Events->GetFirstSelected());

    for (auto it = _scheduleOptions->GetEvents()->begin(); it != _scheduleOptions->GetEvents()->end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            auto todelete = *it;
            _scheduleOptions->GetEvents()->remove(*it);
            delete todelete;
            _scheduleOptions->SetDirty();
            LoadList();
            break;
        }
    }

    ValidateWindow();
}

void EventsDialog::OnButton_EditClick(wxCommandEvent& event)
{
    EditSelected();
}

void EventsDialog::EditSelected()
{
    int id = ListView_Events->GetItemData(ListView_Events->GetFirstSelected());
    EventBase* e = nullptr;
    for (auto it = _scheduleOptions->GetEvents()->begin(); it != _scheduleOptions->GetEvents()->end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            e = *it;
            break;
        }
    }

    if (e != nullptr)
    {
        EventDialog dlg(this, e);
        if (dlg.ShowModal() == wxID_OK)
        {
            // update should have occurred in the dialog
        }
    }

    LoadList();

    ValidateWindow();
}

void EventsDialog::OnButton_AddClick(wxCommandEvent& event)
{
    EventDialog dlg(this, nullptr);
    if (dlg.ShowModal() == wxID_OK)
    {
        // update should have occurred in the dialog
        _scheduleOptions->GetEvents()->push_back(dlg.GetEvent());
    }

    LoadList();

    ValidateWindow();
}

void EventsDialog::ValidateWindow()
{
    if (ListView_Events->GetSelectedItemCount() > 0)
    {
        Button_Edit->Enable();
        Button_Delete->Enable();
    }
    else
    {
        Button_Edit->Enable(false);
        Button_Delete->Enable(false);
    }

    ListView_Events->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListView_Events->GetColumnWidth(0) < 80)
        ListView_Events->SetColumnWidth(0, 80);
    ListView_Events->SetColumnWidth(1, wxLIST_AUTOSIZE);
    if (ListView_Events->GetColumnWidth(1) < 80)
        ListView_Events->SetColumnWidth(1, 80);
}
void EventsDialog::OnListView_EventsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void EventsDialog::OnListView_EventsItemActivated(wxListEvent& event)
{
    EditSelected();
}
