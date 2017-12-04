#include "OutputProcessingDialog.h"
#include "OutputProcessDimWhite.h"
#include "OutputProcessColourOrder.h"
#include "OutputProcessReverse.h"
#include "OutputProcessDim.h"
#include "OutputProcessSet.h"
#include "OutputProcessRemap.h"
#include "OutputProcessDeadChannel.h"
#include "DimDialog.h"
#include "DimWhiteDialog.h"
#include "SetDialog.h"
#include "RemapDialog.h"
#include "ColourOrderDialog.h"
#include "AddReverseDialog.h"
#include "GammaDialog.h"
#include "OutputProcessGamma.h"
#include "DeadChannelDialog.h"

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
const long OutputProcessingDialog::ID_BUTTON12 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON7 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON8 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON9 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON10 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON11 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON5 = wxNewId();
const long OutputProcessingDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OutputProcessingDialog,wxDialog)
	//(*EventTable(OutputProcessingDialog)
	//*)
END_EVENT_TABLE()

OutputProcessingDialog::OutputProcessingDialog(wxWindow* parent, OutputManager* outputManager, std::list<OutputProcess*>* op,wxWindowID id,const wxPoint& pos,const wxSize& size) : _op(op)
{
    _outputManager = outputManager;
    _dragging = false;

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
	Button_AddDeadChannel = new wxButton(this, ID_BUTTON12, _("Add Dead Channel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
	BoxSizer2->Add(Button_AddDeadChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Dim = new wxButton(this, ID_BUTTON7, _("Add Dim"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	BoxSizer2->Add(Button_Dim, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_DimWhite = new wxButton(this, ID_BUTTON8, _("Add Dim White"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	BoxSizer2->Add(Button_DimWhite, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ColourOrder = new wxButton(this, ID_BUTTON9, _("Add Color Order"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
	BoxSizer2->Add(Button_ColourOrder, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Reverse = new wxButton(this, ID_BUTTON10, _("Add Reverse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
	BoxSizer2->Add(Button_Reverse, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Gamma = new wxButton(this, ID_BUTTON11, _("Add Gamma"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
	BoxSizer2->Add(Button_Gamma, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON5, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	Button_Ok->SetDefault();
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesBeginDrag);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesItemSelect);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesItemActivated);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesItemRClick);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&OutputProcessingDialog::OnListView_ProcessesKeyDown);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_EditClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddRemapClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddSetClick);
	Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddDeadChannelClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddDimClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_AddDimWhiteClick);
	Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_ColourOrderClick);
	Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_ReverseClick);
	Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_GammaClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_OkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputProcessingDialog::OnButton_CancelClick);
	//*)

    int w, h;
    GetSize(&w, &h);
    SetSize(600, h);

    ListView_Processes->InsertColumn(0, "Type");
    ListView_Processes->InsertColumn(1, "");
    ListView_Processes->InsertColumn(2, "");
    ListView_Processes->InsertColumn(3, "");
    ListView_Processes->InsertColumn(4, "Description");

    int i = 0;
    for (auto it = op->begin(); it != op->end(); ++it)
    {
        ListView_Processes->InsertItem(i, (*it)->GetType());
        ListView_Processes->SetItem(i, 1, (*it)->GetStartChannel());
        ListView_Processes->SetItem(i, 2, wxString::Format(wxT("%ld"), (long)(*it)->GetP1()));
        if ((*it)->GetType() == "Gamma")
        {
            ListView_Processes->SetItem(i, 3, ((OutputProcessGamma*)(*it))->GetGammaSettings());
        }
        else
        {
            ListView_Processes->SetItem(i, 3, wxString::Format(wxT("%ld"), (long)(*it)->GetP2()));
        }
        ListView_Processes->SetItem(i, 4, (*it)->GetDescription());
        if (!(*it)->IsEnabled())
        {
            ListView_Processes->SetItemTextColour(i, *wxLIGHT_GREY);
        }
        i++;
    }

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());

    ValidateWindow();
}

OutputProcessingDialog::~OutputProcessingDialog()
{
	//(*Destroy(OutputProcessingDialog)
	//*)
}


void OutputProcessingDialog::OnListView_ProcessesBeginDrag(wxListEvent& event)
{
    if (ListView_Processes->GetSelectedItemCount() != 1) return;

    ListView_Processes->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(OutputProcessingDialog::OnDragEnd), nullptr, this);
    // trigger when user leaves window to abort drag
    ListView_Processes->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OutputProcessingDialog::OnDragQuit), nullptr, this);
    // trigger when mouse moves
    ListView_Processes->Connect(wxEVT_MOTION, wxMouseEventHandler(OutputProcessingDialog::OnMouseMove), nullptr, this);

    _dragging = true;
    SetCursor(wxCURSOR_HAND);
}

// abort dragging a list item because user has left window
void OutputProcessingDialog::OnDragQuit(wxMouseEvent& event)
{
    // restore cursor and disconnect unconditionally
    SetCursor(wxCURSOR_ARROW);
    ListView_Processes->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(OutputProcessingDialog::OnDragEnd));
    ListView_Processes->Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OutputProcessingDialog::OnDragQuit));
    ListView_Processes->Disconnect(wxEVT_MOTION, wxMouseEventHandler(OutputProcessingDialog::OnMouseMove));
    //HighlightDropItem(nullptr);
}

void OutputProcessingDialog::OnMouseMove(wxMouseEvent& event)
{
    int flags = wxLIST_HITTEST_ONITEM;
    int dropitem = ListView_Processes->HitTest(event.GetPosition(), flags);
    int topitem = ListView_Processes->GetTopItem();
    int bottomitem = topitem + ListView_Processes->GetCountPerPage();

    if (topitem == dropitem && topitem > 0)
    {
        ListView_Processes->EnsureVisible(topitem - 1);
    }
    else if (bottomitem == dropitem && dropitem < ListView_Processes->GetItemCount() - 1)
    {
        ListView_Processes->EnsureVisible(bottomitem + 1);
    }

    if (dropitem == ListView_Processes->GetItemCount() - 1)
    {
        ListView_Processes->ScrollLines(1);
    }

    SetCursor(wxCURSOR_HAND);
}

void OutputProcessingDialog::OnDragEnd(wxMouseEvent& event)
{
    int flags = wxLIST_HITTEST_ONITEM;
    int dropitem = ListView_Processes->HitTest(event.GetPosition(), flags);

    if (dropitem >= 0 && dropitem < ListView_Processes->GetItemCount())
    {
        int dragitem = ListView_Processes->GetFirstSelected();

        if (dragitem >= 0 && dragitem < ListView_Processes->GetItemCount())
        {
            // move drag item below drop item
            if (dragitem != dropitem)
            {
                std::string type = ListView_Processes->GetItemText(dragitem, 0).ToStdString();
                std::string sc = ListView_Processes->GetItemText(dragitem, 1).ToStdString();
                std::string p1 = ListView_Processes->GetItemText(dragitem, 2).ToStdString();
                std::string p2 = ListView_Processes->GetItemText(dragitem, 3).ToStdString();
                std::string d = ListView_Processes->GetItemText(dragitem, 4).ToStdString();
                bool e = ListView_Processes->GetItemTextColour(dragitem) != *wxLIGHT_GREY;

                ListView_Processes->DeleteItem(dragitem);

                if (dropitem > dragitem) dropitem--;

                ListView_Processes->InsertItem(dropitem + 1, type);
                ListView_Processes->SetItem(dropitem + 1, 1, sc);
                ListView_Processes->SetItem(dropitem + 1, 2, p1);
                ListView_Processes->SetItem(dropitem + 1, 3, p1);
                ListView_Processes->SetItem(dropitem + 1, 4, d);
                if (!e) ListView_Processes->SetItemTextColour(dropitem + 1 , *wxLIGHT_GREY);

                ListView_Processes->EnsureVisible(dropitem + 1);

                if (dropitem + 1 == ListView_Processes->GetItemCount() - 1)
                {
                    ListView_Processes->ScrollLines(1);
                }
            }
        }
    }

    _dragging = false;
    SetCursor(wxCURSOR_ARROW);

    // disconnect both functions
    ListView_Processes->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(OutputProcessingDialog::OnDragEnd));
    ListView_Processes->Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OutputProcessingDialog::OnDragQuit));
    ListView_Processes->Disconnect(wxEVT_MOTION, wxMouseEventHandler(OutputProcessingDialog::OnMouseMove));
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
            op = new OutputProcessDim(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)), ListView_Processes->GetItemText(i,4).ToStdString());
        }
        else if (type == "Dim White")
        {
            op = new OutputProcessDimWhite(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)), ListView_Processes->GetItemText(i, 4).ToStdString());
        }
        else if (type == "Set")
        {
            op = new OutputProcessSet(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)), ListView_Processes->GetItemText(i, 4).ToStdString());
        }
        else if (type == "Dead Channel")
        {
            op = new OutputProcessDeadChannel(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), ListView_Processes->GetItemText(i, 4).ToStdString());
        }
        else if (type == "Remap")
        {
            op = new OutputProcessRemap(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)), ListView_Processes->GetItemText(i, 4).ToStdString());
        }
        else if (type == "Color Order")
        {
            op = new OutputProcessColourOrder(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)), ListView_Processes->GetItemText(i, 4).ToStdString());
        }
        else if (type == "Reverse")
        {
            op = new OutputProcessReverse(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), wxAtol(ListView_Processes->GetItemText(i, 3)), ListView_Processes->GetItemText(i, 4).ToStdString());
        }
        else if (type == "Gamma")
        {
            float g = 1.0;
            float gR = 1.0;
            float gG = 1.0;
            float gB = 1.0;

            wxArrayString floats = wxSplit(ListView_Processes->GetItemText(i, 3), ',');
            if (floats.size() == 4)
            {
                g = wxAtof(floats[0]);
                gR = wxAtof(floats[1]);
                gG = wxAtof(floats[2]);
                gB = wxAtof(floats[3]);
            }
            op = new OutputProcessGamma(_outputManager, ListView_Processes->GetItemText(i, 1).ToStdString(), wxAtol(ListView_Processes->GetItemText(i, 2)), g, gR, gG, gB, ListView_Processes->GetItemText(i, 4).ToStdString());
        }

        if (op != nullptr)
        {
            op->Enable(ListView_Processes->GetItemTextColour(i) != *wxLIGHT_GREY);
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
        std::string sc = ListView_Processes->GetItemText(row, 1).ToStdString();
        size_t p1 = wxAtol(ListView_Processes->GetItemText(row, 2));
        std::string p1s = ListView_Processes->GetItemText(row, 2).ToStdString();
        size_t p2 = wxAtol(ListView_Processes->GetItemText(row, 3));
        std::string d = ListView_Processes->GetItemText(row, 4).ToStdString();
        bool e = ListView_Processes->GetItemTextColour(row) != *wxLIGHT_GREY;

        float gamma = 1.0;
        float gammaR = 1.0;
        float gammaG = 1.0;
        float gammaB = 1.0;

        wxArrayString floats = wxSplit(ListView_Processes->GetItemText(row, 3), ',');
        if (floats.size() == 4)
        {
            gamma = wxAtof(floats[0]);
            gammaR = wxAtof(floats[1]);
            gammaG = wxAtof(floats[2]);
            gammaB = wxAtof(floats[3]);
        }

        // this is wasteful ... but whatever
        DimDialog dlgd(this, _outputManager, sc, p1, p2, d, e);
        DimWhiteDialog dlgdw(this, _outputManager, sc, p1, p2, d, e);
        SetDialog dlgs(this, _outputManager, sc, p1, p2, d, e);
        DeadChannelDialog dlgdc(this, _outputManager, sc, p1, d, e);
        RemapDialog dlgr(this, _outputManager, sc, p1s, p2, d, e);
        AddReverseDialog dlgrv(this, _outputManager, sc, p1, p2, d, e);
        GammaDialog dlgg(this, _outputManager, sc, p1, gamma, gammaR, gammaG, gammaB, d, e);
        ColourOrderDialog dlgc(this, _outputManager, sc, p1, p2, d, e);
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
        else if (type == "Color Order")
        {
            res = dlgc.ShowModal();
        }
        else if (type == "Reverse")
        {
            res = dlgrv.ShowModal();
        }
        else if (type == "Gamma")
        {
            res = dlgg.ShowModal();
        }
        else if (type == "Dead Channel")
        {
            res = dlgdc.ShowModal();
            p2 = 0;
        }

        if (res == wxID_OK)
        {
            ListView_Processes->SetItem(row, 1, sc);
            if (type == "Remap")
            {
                ListView_Processes->SetItem(row, 2, p1s);
            }
            else
            {
                ListView_Processes->SetItem(row, 2, wxString::Format(wxT("%ld"), (long)p1));
            }
            if (type == "Gamma")
            {
                ListView_Processes->SetItem(row, 3, wxString::Format(wxT("%.2f,%.2f,%.2f,%.2f"), gamma, gammaR, gammaG, gammaB));
            }
            else
            {
                ListView_Processes->SetItem(row, 3, wxString::Format(wxT("%ld"), (long)p2));
            }
            ListView_Processes->SetItem(row, 4, d);
            if (!e)
            {
                ListView_Processes->SetItemTextColour(row, *wxLIGHT_GREY);
            }
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

void OutputProcessingDialog::OnButton_ColourOrderClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Color Order");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_ReverseClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Reverse");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_GammaClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Gamma");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}

void OutputProcessingDialog::OnButton_AddDeadChannelClick(wxCommandEvent& event)
{
    ListView_Processes->InsertItem(ListView_Processes->GetItemCount(), "Dead Channel");
    ListView_Processes->Select(ListView_Processes->GetItemCount() - 1);
    if (!EditSelectedItem())
    {
        ListView_Processes->DeleteItem(ListView_Processes->GetItemCount() - 1);
    }
    ValidateWindow();
}

void OutputProcessingDialog::OnListView_ProcessesItemRClick(wxListEvent& event)
{
}
