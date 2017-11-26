#include "MatricesDialog.h"
#include "MatrixMapper.h"
#include "MatrixDialog.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(MatricesDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MatricesDialog)
const long MatricesDialog::ID_LISTVIEW1 = wxNewId();
const long MatricesDialog::ID_BUTTON1 = wxNewId();
const long MatricesDialog::ID_BUTTON2 = wxNewId();
const long MatricesDialog::ID_BUTTON3 = wxNewId();
const long MatricesDialog::ID_BUTTON4 = wxNewId();
const long MatricesDialog::ID_BUTTON5 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MatricesDialog,wxDialog)
	//(*EventTable(MatricesDialog)
	//*)
END_EVENT_TABLE()

MatricesDialog::MatricesDialog(wxWindow* parent, OutputManager* outputManager, std::list<MatrixMapper*>* matrices,wxWindowID id,const wxPoint& pos,const wxSize& size) : _matrices(matrices)
{
    _outputManager = outputManager;

	//(*Initialize(MatricesDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Matrices"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListView1 = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer1->Add(ListView1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_Add = new wxButton(this, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Edit = new wxButton(this, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Delete = new wxButton(this, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON4, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	Button_Ok->SetDefault();
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON5, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&MatricesDialog::OnListView1ItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&MatricesDialog::OnListView1ItemActivated);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&MatricesDialog::OnListView1KeyDown);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatricesDialog::OnButton_AddClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatricesDialog::OnButton_EditClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatricesDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatricesDialog::OnButton_OkClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatricesDialog::OnButton_CancelClick);
	//*)

    ListView1->InsertColumn(0, "Name");
    ListView1->InsertColumn(1, "Orientation");
    ListView1->InsertColumn(2, "Strings");
    ListView1->InsertColumn(3, "String Lengths");
    ListView1->InsertColumn(4, "Strands Per String");
    ListView1->InsertColumn(5, "Start Location");
    ListView1->InsertColumn(6, "Start Channel");

    SetSize(800, 400);

    PopulateList();

    ValidateWindow();
}

MatricesDialog::~MatricesDialog()
{
	//(*Destroy(MatricesDialog)
	//*)
}


void MatricesDialog::OnListView1ItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void MatricesDialog::OnListView1ItemActivated(wxListEvent& event)
{
    DoEdit();
}

void MatricesDialog::OnListView1KeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        DoDelete();
    }
}

void MatricesDialog::OnButton_AddClick(wxCommandEvent& event)
{
    DoAdd();
}

void MatricesDialog::OnButton_EditClick(wxCommandEvent& event)
{
    DoEdit();
}

void MatricesDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    DoDelete();
}

void MatricesDialog::OnButton_OkClick(wxCommandEvent& event)
{
    while (_matrices->size() > 0)
    {
        auto todelete = _matrices->front();
        _matrices->remove(todelete);
        delete todelete;
    }

    for (int i = 0; i < ListView1->GetItemCount(); i++)
    {
        std::string name = ListView1->GetItemText(i, 0).ToStdString();
        std::string orientation = ListView1->GetItemText(i, 1).ToStdString();
        int strings = wxAtoi(ListView1->GetItemText(i, 2));
        int stringLength = wxAtoi(ListView1->GetItemText(i, 3));
        int strandsPerString = wxAtoi(ListView1->GetItemText(i, 4));
        std::string startLocation = ListView1->GetItemText(i, 5).ToStdString();
        std::string startChannel = ListView1->GetItemText(i, 6);

        _matrices->push_back(new MatrixMapper(_outputManager, strings, strandsPerString, stringLength, orientation, startLocation, startChannel, name));
    }

    EndDialog(wxID_OK);
}

void MatricesDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void MatricesDialog::ValidateWindow()
{
    if (ListView1->GetSelectedItemCount() == 1)
    {
        Button_Edit->Enable();
        Button_Delete->Enable();
    }
    else
    {
        Button_Edit->Enable(false);
        Button_Delete->Enable(false);
    }
}

void MatricesDialog::DoAdd()
{
    std::string name = "";
    std::string orientation = "";
    int stringLength = 50;
    int strings = 1;
    int strandsPerString = 1;
    std::string startLocation = "";
    std::string startChannel = "1";

    MatrixDialog dlg(this, _outputManager, name, orientation, startLocation, stringLength, strings, strandsPerString, startChannel);

    if (dlg.ShowModal() == wxID_OK)
    {
        int row = ListView1->GetItemCount();
        ListView1->InsertItem(row, name);
        ListView1->SetItem(row, 1, orientation);
        ListView1->SetItem(row, 2, wxString::Format(wxT("%i"), strings));
        ListView1->SetItem(row, 3, wxString::Format(wxT("%i"), stringLength));
        ListView1->SetItem(row, 4, wxString::Format(wxT("%i"), strandsPerString));
        ListView1->SetItem(row, 5, startLocation);
        ListView1->SetItem(row, 6, startChannel);
    }

    ValidateWindow();
}

void MatricesDialog::DoDelete()
{
    if (ListView1->GetSelectedItemCount() != 1) return;

    int item = ListView1->GetFirstSelected();

    ListView1->DeleteItem(item);

    ValidateWindow();
}

void MatricesDialog::DoEdit()
{
    if (ListView1->GetSelectedItemCount() != 1) return;

    int item = ListView1->GetFirstSelected();

    std::string name = ListView1->GetItemText(item, 0).ToStdString();
    std::string orientation = ListView1->GetItemText(item, 1).ToStdString();
    int strings = wxAtoi(ListView1->GetItemText(item, 2));
    int stringLength = wxAtoi(ListView1->GetItemText(item, 3));
    int strandsPerString = wxAtoi(ListView1->GetItemText(item, 4));
    std::string startLocation = ListView1->GetItemText(item, 5).ToStdString();
    std::string startChannel = ListView1->GetItemText(item, 6);

    MatrixDialog dlg(this, _outputManager, name, orientation, startLocation, stringLength, strings, strandsPerString, startChannel);

    if (dlg.ShowModal() == wxID_OK)
    {
        ListView1->SetItem(item, 0, name);
        ListView1->SetItem(item, 1, orientation);
        ListView1->SetItem(item, 2, wxString::Format(wxT("%i"), strings));
        ListView1->SetItem(item, 3, wxString::Format(wxT("%i"), stringLength));
        ListView1->SetItem(item, 4, wxString::Format(wxT("%i"), strandsPerString));
        ListView1->SetItem(item, 5, startLocation);
        ListView1->SetItem(item, 6, startChannel);
    }

    ValidateWindow();
}

void MatricesDialog::PopulateList()
{
    for (auto it = _matrices->begin(); it != _matrices->end(); ++it)
    {
        int row = ListView1->GetItemCount();
        ListView1->InsertItem(row, (*it)->GetName());
        ListView1->SetItem(row, 1, (*it)->GetOrientation());
        ListView1->SetItem(row, 2, wxString::Format(wxT("%i"), (*it)->GetStrings()));
        ListView1->SetItem(row, 3, wxString::Format(wxT("%i"), (*it)->GetStringLength()));
        ListView1->SetItem(row, 4, wxString::Format(wxT("%i"), (*it)->GetStrandsPerString()));
        ListView1->SetItem(row, 5, (*it)->GetStartLocation());
        ListView1->SetItem(row, 6, (*it)->GetStartChannel());
    }
}