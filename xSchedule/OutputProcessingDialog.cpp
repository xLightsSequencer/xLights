#include "OutputProcessingDialog.h"
#include "OutputProcessDimWhite.h"
#include "OutputProcessDim.h"
#include "OutputProcessSet.h"
#include "OutputProcessRemap.h"
#include "DimDialog.h"
#include "DimWhiteDialog.h"
#include "SetDialog.h"
#include "RemapDialog.h"

//(*InternalHeaders(OutputProcessingDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(OutputProcessingDialog)
const long OutputProcessingDialog::ID_LISTVIEW1 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON1 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON2 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON3 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON6 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON7 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON8 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON5 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OutputProcessingDialog,wxDialog)
	//(*EventTable(OutputProcessingDialog)
	//*)
END_EVENT_TABLE()

OutputProcessingDialog::OutputProcessingDialog(wxWindow* parent, std::list<OutputProcess*>* op,wxWindowID id,const wxPoint& pos,const wxSize& size) : _op(op)
{

	//(*Initialize(OutputProcessingDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Output Processing"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	ListView_Processes = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxVSCROLL, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer2->Add(ListView_Processes, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_Edit = new wxButton(this, ID_BUTTON1, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Delete = new wxButton(this, ID_BUTTON2, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	Button_AddRemap = new wxButton(this, ID_BUTTON3, _("Add Remap"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	BoxSizer2->Add(Button_AddRemap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_AddSet = new wxButton(this, ID_BUTTON6, _("Add Set"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	BoxSizer2->Add(Button_AddSet, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Dim = new wxButton(this, ID_BUTTON7, _("Add Dim"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	BoxSizer2->Add(Button_Dim, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_DimWhite = new wxButton(this, ID_BUTTON8, _("Add Dim White"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	BoxSizer2->Add(Button_DimWhite, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON5, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesBeginDrag);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesItemActivated);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesKeyDown);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_EditClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddRemapClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddSetClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddDimClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddDimWhiteClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_OkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_CancelClick);
	//*)

    ListView_Processes->InsertColumn(0, "Type");
    ListView_Processes->InsertColumn(1, "");
    ListView_Processes->InsertColumn(2, "");
    ListView_Processes->InsertColumn(3, "");

    int i = 0;
    for (auto it = op->begin(); it != op->end(); ++it)
    {
        ListView_Processes->InsertItem(i, (*it)->GetType());
        ListView_Processes->SetItem(i, 1, wxString::Format(wxT("%i"), (*it)->GetStartChannel()));
        ListView_Processes->SetItem(i, 2, wxString::Format(wxT("%i"), (*it)->GetP1()));
        ListView_Processes->SetItem(i, 3, wxString::Format(wxT("%i"), (*it)->GetP2()));
        i++;
    }

    ValidateWindow();
}

OutputProcessingDialog::~OutputProcessingDialog()
{
	//(*Destroy(OutputProcessingDialog)
	//*)
}


void OutputProcessingDialog::OnListView_ProcessesBeginDrag(wxListEvent& event)
{
}

void OutputProcessingDialog::OnListView_ProcessesItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void OutputProcessingDialog::OnListView_ProcessesItemActivated(wxListEvent& event)
{
    EditSelectedItem();
    ValidateWindow();
}

void OutputProcessingDialog::OnListView_ProcessesKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        DeleteSelectedItem();
    }

    ValidateWindow();
}

void OutputProcessingDialog::OnButton_EditClick(wxCommandEvent& event)
{
    EditSelectedItem();
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    DeleteSelectedItem();
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_AddRemapClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Remap");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_AddSetClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Set");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_OkClick(wxCommandEvent& event)
{
    while (_op->size() > 0)
    {
        auto todelete = _op->front();
        _op->remove(todelete);
        delete todelete;
    }

    for (int i = 0; i < ListView_Processes->GetItemCount(); i++)
    {
        std::string type = ListView_Processes->GetItemText(i, 0).ToStdString();

        OutputProcess* op = nullptr;
        if (type == "Dim")
        {
            op = new OutputProcessDim(wxAtol(ListView_Processes->GetItemText(i, 1)), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)));
        }
        else if (type == "Dim White")
        {
            op = new OutputProcessDimWhite(wxAtol(ListView_Processes->GetItemText(i, 1)), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)));
        }
        else if (type == "Set")
        {
            op = new OutputProcessSet(wxAtol(ListView_Processes->GetItemText(i, 1)), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)));
        }
        else if (type == "Remap")
        {
            op = new OutputProcessRemap(wxAtol(ListView_Processes->GetItemText(i, 1)), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)));
        }

        if (op != nullptr)
        {
            _op->push_back(op);
        }
    }

    EndDialog(wxID_OK);
}

void OutputProcessingDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void OutputProcessingDialog::OnButton_DimWhiteClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void OutputProcessingDialog::ValidateWindow()
{
    if (ListView_Processes->GetSelectedItemCount() > 0)
    {
        Button_Delete->Enable();
        Button_Edit->Enable();
    }
    else
    {
        Button_Delete->Enable(false);
        Button_Edit->Enable(false);
    }
}

void OutputProcessingDialog::DeleteSelectedItem()
{
    if (ListView_Processes->GetSelectedItemCount() >= 0)
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetFirstSelected());
    }
}

bool OutputProcessingDialog::EditSelectedItem()
{
    bool result = false;

    if (ListView_Processes->GetSelectedItemCount() >= 0)
    {
        int row = ListView_Processes->GetFirstSelected();

        std::string type = ListView_Processes->GetItemText(row, 0).ToStdString();
        size_t sc = wxAtol(ListView_Processes->GetItemText(row, 1).ToStdString());
        size_t p1 = wxAtol(ListView_Processes->GetItemText(row, 2).ToStdString());
        size_t p2 = wxAtol(ListView_Processes->GetItemText(row, 3).ToStdString());

        // this is wasteful ... but whatever
        DimDialog dlgd(this, sc, p1, p2);
        DimWhiteDialog dlgdw(this, sc, p1, p2);
        SetDialog dlgs(this, sc, p1, p2);
        RemapDialog dlgr(this, sc, p1, p2);
        int res = wxID_CANCEL;

        if (type == "Dim")
        {
            res = dlgd.ShowModal();
        }
        else if (type == "Dim White")
        {
            res = dlgdw.ShowModal();
        }
        else if (type == "Set")
        {
            res = dlgs.ShowModal();
        }
        else if (type == "Remap")
        {
            res = dlgr.ShowModal();
        }

        if (res == wxID_OK)
        {
            ListView_Processes->SetItem(row, 1, wxString::Format(wxT("%i"), sc));
            ListView_Processes->SetItem(row, 2, wxString::Format(wxT("%i"), p1));
            ListView_Processes->SetItem(row, 3, wxString::Format(wxT("%i"), p2));
            result = true;
        }
    }

    ValidateWindow();

    return result;
}

void OutputProcessingDialog::OnButton_AddDimClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Dim");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_AddDimWhiteClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Dim White");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}
