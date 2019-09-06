#include "CheckboxSelectDialog.h"

//(*InternalHeaders(CheckboxSelectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/menu.h>

//(*IdInit(CheckboxSelectDialog)
const long CheckboxSelectDialog::ID_CHECKLISTBOXITEMS = wxNewId();
const long CheckboxSelectDialog::ID_BUTTONOK = wxNewId();
const long CheckboxSelectDialog::ID_BUTTONCANCEL = wxNewId();
//*)

const long CheckboxSelectDialog::ID_MCU_SELECTALL = wxNewId();
const long CheckboxSelectDialog::ID_MCU_SELECTNONE = wxNewId();

BEGIN_EVENT_TABLE(CheckboxSelectDialog,wxDialog)
	//(*EventTable(CheckboxSelectDialog)
	//*)
END_EVENT_TABLE()

CheckboxSelectDialog::CheckboxSelectDialog(wxWindow* parent, const wxString &title, const wxArrayString& items, const wxArrayString& itemsSelected, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(CheckboxSelectDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Choose Items..."), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	CheckListBox_Items = new wxCheckListBox(this, ID_CHECKLISTBOXITEMS, wxDefaultPosition, wxSize(-1,300), 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOXITEMS"));
	FlexGridSizer1->Add(CheckListBox_Items, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTONOK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONOK"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTONCANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONCANCEL"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKLISTBOXITEMS,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&CheckboxSelectDialog::OnCheckListBox_ItemsToggled);
	Connect(ID_BUTTONOK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CheckboxSelectDialog::OnButton_OkClick);
	Connect(ID_BUTTONCANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CheckboxSelectDialog::OnButton_CancelClick);
	//*)

	Connect(ID_CHECKLISTBOXITEMS, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& CheckboxSelectDialog::OnListRClick);

    for (const auto & item : items)
    {
        CheckListBox_Items->Append(item);
        if (std::find(itemsSelected.begin(), itemsSelected.end(), item) != itemsSelected.end()) 
        {
            CheckListBox_Items->Check(CheckListBox_Items->GetCount() - 1);
        }
    }

	SetTitle(title);

    SetEscapeId(Button_Cancel->GetId());

    ValidateWindow();
}

CheckboxSelectDialog::~CheckboxSelectDialog()
{
	//(*Destroy(CheckboxSelectDialog)
	//*)
}

wxArrayString CheckboxSelectDialog::GetSelectedItems() const
{
    wxArrayString res = wxArrayString();
    wxArrayInt items;
    CheckListBox_Items->GetCheckedItems(items);
    for (const auto& it : items)
    {
        res.Add(CheckListBox_Items->GetString(it));
    }

    return res;
}

void CheckboxSelectDialog::SelectAllLayers()
{
    for (int i = 0; i < CheckListBox_Items->GetCount(); i++)
    {
        CheckListBox_Items->Check(i);
    }
}


void CheckboxSelectDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void CheckboxSelectDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void CheckboxSelectDialog::OnCheckListBox_ItemsToggled(wxCommandEvent& event)
{
    ValidateWindow();
}

void CheckboxSelectDialog::ValidateWindow()
{
    wxArrayInt items;
    CheckListBox_Items->GetCheckedItems(items);
    if (items.Count() == 0)
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}

void CheckboxSelectDialog::OnListRClick(wxContextMenuEvent& event)
{
	wxMenu mnu;
	mnu.Append(ID_MCU_SELECTALL, "Select All");
	mnu.Append(ID_MCU_SELECTNONE, "Deselect All");

	mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&CheckboxSelectDialog::OnPopup, nullptr, this);
	PopupMenu(&mnu);
}

void CheckboxSelectDialog::OnPopup(wxCommandEvent& event)
{
	if (event.GetId() == ID_MCU_SELECTALL)
	{
		SelectAllLayers();
	}
	else if (event.GetId() == ID_MCU_SELECTNONE)
	{
		DeselectAllLayers();
	}
}

void CheckboxSelectDialog::DeselectAllLayers()
{
	for (size_t i = 0; i < CheckListBox_Items->GetCount(); i++)
	{
		CheckListBox_Items->Check(i, false);
	}
}
