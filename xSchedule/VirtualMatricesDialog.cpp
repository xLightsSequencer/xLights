/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VirtualMatricesDialog.h"
#include "VirtualMatrix.h"
#include "VirtualMatrixDialog.h"
#include "../xLights/outputs/OutputManager.h"
#include "ScheduleOptions.h"

//(*InternalHeaders(VirtualMatricesDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(VirtualMatricesDialog)
const long VirtualMatricesDialog::ID_LISTVIEW1 = wxNewId();
const long VirtualMatricesDialog::ID_BUTTON1 = wxNewId();
const long VirtualMatricesDialog::ID_BUTTON2 = wxNewId();
const long VirtualMatricesDialog::ID_BUTTON3 = wxNewId();
const long VirtualMatricesDialog::ID_BUTTON4 = wxNewId();
const long VirtualMatricesDialog::ID_BUTTON5 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VirtualMatricesDialog,wxDialog)
	//(*EventTable(VirtualMatricesDialog)
	//*)
END_EVENT_TABLE()

VirtualMatricesDialog::VirtualMatricesDialog(wxWindow* parent, OutputManager* outputManager, std::list<VirtualMatrix*>* vmatrices, ScheduleOptions* options, wxWindowID id,const wxPoint& pos,const wxSize& size) : _vmatrices(vmatrices)
{
    _outputManager = outputManager;
    _options = options;

	//(*Initialize(VirtualMatricesDialog)
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

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&VirtualMatricesDialog::OnListView1ItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&VirtualMatricesDialog::OnListView1ItemActivated);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&VirtualMatricesDialog::OnListView1KeyDown);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VirtualMatricesDialog::OnButton_AddClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VirtualMatricesDialog::OnButton_EditClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VirtualMatricesDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VirtualMatricesDialog::OnButton_OkClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VirtualMatricesDialog::OnButton_CancelClick);
	//*)

    ListView1->InsertColumn(0, "Name");
    ListView1->InsertColumn(1, "Width");
    ListView1->InsertColumn(2, "Height");
    ListView1->InsertColumn(3, "Topmost");
    ListView1->InsertColumn(4, "Rotation");
    ListView1->InsertColumn(5, "Size");
    ListView1->InsertColumn(6, "Location");
    ListView1->InsertColumn(7, "Start Channel");
    ListView1->InsertColumn(8, "Quality");
    ListView1->InsertColumn(9, "Matrix Multiplier");
    ListView1->InsertColumn(10, "Pixel");

    SetSize(1000, 400);

    PopulateList();

    ValidateWindow();
}

VirtualMatricesDialog::~VirtualMatricesDialog()
{
	//(*Destroy(VirtualMatricesDialog)
	//*)
}

void VirtualMatricesDialog::OnListView1ItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void VirtualMatricesDialog::OnListView1ItemActivated(wxListEvent& event)
{
    DoEdit();
}

void VirtualMatricesDialog::OnListView1KeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        DoDelete();
    }
}

void VirtualMatricesDialog::OnButton_AddClick(wxCommandEvent& event)
{
    DoAdd();
}

void VirtualMatricesDialog::OnButton_EditClick(wxCommandEvent& event)
{
    DoEdit();
}

void VirtualMatricesDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    DoDelete();
}

void VirtualMatricesDialog::OnButton_OkClick(wxCommandEvent& event)
{
    while (_vmatrices->size() > 0)
    {
        auto todelete = _vmatrices->front();
        _vmatrices->remove(todelete);
        delete todelete;
    }

    for (int i = 0; i < ListView1->GetItemCount(); i++)
    {
        std::string name = ListView1->GetItemText(i, 0).ToStdString();
        int width = wxAtoi(ListView1->GetItemText(i, 1));
        int height = wxAtoi(ListView1->GetItemText(i, 2));
        bool topMost = ListView1->GetItemText(i, 3) == "Yes";
        std::string rotation = ListView1->GetItemText(i, 4).ToStdString();
        auto sz = wxSplit(ListView1->GetItemText(i, 5), ',');
        wxSize size(300, 300);
        if (sz.GetCount() == 2)
        {
            size = wxSize(wxAtoi(sz[0]), wxAtoi(sz[1]));
        }
        auto l = wxSplit(ListView1->GetItemText(i, 6), ',');
        wxPoint location(0, 0);
        if (l.GetCount() == 2)
        {
            location = wxPoint(wxAtoi(l[0]), wxAtoi(l[1]));
        }
        std::string startChannel = ListView1->GetItemText(i, 7).ToStdString();
        std::string quality = ListView1->GetItemText(i, 8).ToStdString();
        bool useMatrixSize = false;
        int matrixMultiplier = 1;
        if (ListView1->GetItemText(i, 9) != "")
        {
            matrixMultiplier = wxAtoi(ListView1->GetItemText(i, 9));
            useMatrixSize = true;
        }
        std::string pixelChannels = ListView1->GetItemText(i, 10).ToStdString();

        _vmatrices->push_back(new VirtualMatrix(_outputManager, width, height, topMost, rotation, pixelChannels, quality, startChannel, name, size, location, useMatrixSize, matrixMultiplier));
    }

    EndDialog(wxID_OK);
}

void VirtualMatricesDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void VirtualMatricesDialog::ValidateWindow()
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

void VirtualMatricesDialog::DoAdd()
{
    std::string name = "";
    int width = 32;
    int height = 16;
    bool topMost = true;
    std::string rotation = "";
    std::string pixelChannels = "";
    wxSize size = _options->GetDefaultVideoSize();
    wxPoint location = _options->GetDefaultVideoPos();
    std::string startChannel = "1";
    std::string quality = "";
    bool useMatrixSize = false;
    int matrixMultiplier = 1;

    VirtualMatrixDialog dlg(this, _outputManager, name, rotation, pixelChannels, quality, size, location, width, height, topMost, startChannel, useMatrixSize, matrixMultiplier, _options);

    if (dlg.ShowModal() == wxID_OK)
    {
        int row = ListView1->GetItemCount();
        ListView1->InsertItem(row, name);
        ListView1->SetItem(row, 1, wxString::Format(wxT("%i"), width));
        ListView1->SetItem(row, 2, wxString::Format(wxT("%i"), height));
        ListView1->SetItem(row, 3, topMost ? "Yes" : "No");
        ListView1->SetItem(row, 4, rotation);
        ListView1->SetItem(row, 5, wxString::Format(wxT("%i,%i"), size.GetWidth(), size.GetHeight()));
        ListView1->SetItem(row, 6, wxString::Format(wxT("%i,%i"), location.x, location.y));
        ListView1->SetItem(row, 7, startChannel);
        ListView1->SetItem(row, 8, quality);
        if (useMatrixSize)
        {
            ListView1->SetItem(row, 9, wxString::Format("%d", matrixMultiplier));
        }
        else
        {
            ListView1->SetItem(row, 9, "");
        }
        ListView1->SetItem(row, 10, pixelChannels);
    }

    ValidateWindow();
}

void VirtualMatricesDialog::DoDelete()
{
    if (ListView1->GetSelectedItemCount() != 1) return;

    int item = ListView1->GetFirstSelected();

    ListView1->DeleteItem(item);

    ValidateWindow();
}

void VirtualMatricesDialog::DoEdit()
{
    if (ListView1->GetSelectedItemCount() != 1) return;

    int item = ListView1->GetFirstSelected();

    std::string name = ListView1->GetItemText(item, 0).ToStdString();
    int width = wxAtoi(ListView1->GetItemText(item, 1));
    int height = wxAtoi(ListView1->GetItemText(item, 2));
    bool topMost = ListView1->GetItemText(item, 3) == "Yes";
    std::string rotation = ListView1->GetItemText(item, 4).ToStdString();
    std::string pixelChannels = ListView1->GetItemText(item, 10).ToStdString();
    auto sz = wxSplit(ListView1->GetItemText(item, 5), ',');
    wxSize size(300, 300);
    if (sz.GetCount() == 2)
    {
        size = wxSize(wxAtoi(sz[0]), wxAtoi(sz[1]));
    }
    auto l = wxSplit(ListView1->GetItemText(item, 6), ',');
    wxPoint location(0, 0);
    if (l.GetCount() == 2)
    {
        location = wxPoint(wxAtoi(l[0]), wxAtoi(l[1]));
    }
    std::string startChannel = ListView1->GetItemText(item, 7).ToStdString();
    std::string quality = ListView1->GetItemText(item, 8).ToStdString();
    bool useMatrixSize = false;
    int matrixMultiplier = 1;
    if (ListView1->GetItemText(item, 9) != "")
    {
        matrixMultiplier = wxAtoi(ListView1->GetItemText(item, 9));
        useMatrixSize = true;
    }

    VirtualMatrixDialog dlg(this, _outputManager, name, rotation, pixelChannels, quality, size, location, width, height, topMost, startChannel, useMatrixSize, matrixMultiplier, _options);

    if (dlg.ShowModal() == wxID_OK)
    {
        ListView1->SetItem(item, 0, name);
        ListView1->SetItem(item, 1, wxString::Format(wxT("%i"), width));
        ListView1->SetItem(item, 2, wxString::Format(wxT("%i"), height));
        ListView1->SetItem(item, 3, topMost ? "Yes" : "No");
        ListView1->SetItem(item, 4, rotation);
        ListView1->SetItem(item, 5, wxString::Format(wxT("%i,%i"), size.GetWidth(), size.GetHeight()));
        ListView1->SetItem(item, 6, wxString::Format(wxT("%i,%i"), location.x, location.y));
        ListView1->SetItem(item, 7, startChannel);
        ListView1->SetItem(item, 8, quality);
        if (useMatrixSize)
        {
            ListView1->SetItem(item, 9, wxString::Format("%d", matrixMultiplier));
        }
        else
        {
            ListView1->SetItem(item, 9, "");
        }
        ListView1->SetItem(item, 10, pixelChannels);
    }

    ValidateWindow();
}

void VirtualMatricesDialog::PopulateList()
{
    for (auto it = _vmatrices->begin(); it != _vmatrices->end(); ++it)
    {
        int row = ListView1->GetItemCount();
        ListView1->InsertItem(row, (*it)->GetName());
        ListView1->SetItem(row, 1, wxString::Format(wxT("%ld"), (long)(*it)->GetWidth()));
        ListView1->SetItem(row, 2, wxString::Format(wxT("%ld"), (long)(*it)->GetHeight()));
        ListView1->SetItem(row, 3, (*it)->GetTopMost() ? "Yes" : "No");
        ListView1->SetItem(row, 4, (*it)->GetRotation());
        ListView1->SetItem(row, 5, wxString::Format(wxT("%d,%d"), (*it)->GetSize().GetWidth(), (*it)->GetSize().GetHeight()));
        ListView1->SetItem(row, 6, wxString::Format(wxT("%d,%d"), (*it)->GetLocation().x, (*it)->GetLocation().y));
        ListView1->SetItem(row, 7, (*it)->GetStartChannel());
        ListView1->SetItem(row, 8, (*it)->GetScalingQuality());
        if ((*it)->GetUseMatrixSize())
        {
            ListView1->SetItem(row, 9, wxString::Format("%d", (*it)->GetMatrixMultiplier()));
        }
        else
        {
            ListView1->SetItem(row, 9, "");
        }
        ListView1->SetItem(row, 10, (*it)->GetPixelChannels());
    }
}