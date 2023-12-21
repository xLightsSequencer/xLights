#include "EditAliasesDialog.h"
#include "models/Model.h"

//(*InternalHeaders(EditAliasesDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EditAliasesDialog)
const long EditAliasesDialog::ID_LISTBOX1 = wxNewId();
const long EditAliasesDialog::ID_BUTTON1 = wxNewId();
const long EditAliasesDialog::ID_BUTTON2 = wxNewId();
const long EditAliasesDialog::ID_BUTTON3 = wxNewId();
const long EditAliasesDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EditAliasesDialog,wxDialog)
	//(*EventTable(EditAliasesDialog)
	//*)
END_EVENT_TABLE()

EditAliasesDialog::EditAliasesDialog(wxWindow* parent, Model* m, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    _m(m)
{
	//(*Initialize(EditAliasesDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, _("Model Aliases"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListBoxAliases = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
	FlexGridSizer1->Add(ListBoxAliases, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAdd = new wxButton(this, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(ButtonAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDelete = new wxButton(this, ID_BUTTON2, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(ButtonDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTON3, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	ButtonOk->SetDefault();
	FlexGridSizer3->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer3->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&EditAliasesDialog::OnListBoxAliasesSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditAliasesDialog::OnButtonAddClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditAliasesDialog::OnButtonDeleteClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditAliasesDialog::OnButtonOkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditAliasesDialog::OnButtonCancelClick);
	//*)

	for (const auto& it : _m->GetAliases())
	{
        ListBoxAliases->Append(it);
	}

	SetEscapeId(ID_BUTTON4);

	ValidateWindow();
}

EditAliasesDialog::~EditAliasesDialog()
{
	//(*Destroy(EditAliasesDialog)
	//*)
}

void EditAliasesDialog::ValidateWindow()
{
    if (ListBoxAliases->GetSelection()>= 0)
	{
        ButtonDelete->Enable();
	}
	else
	{
        ButtonDelete->Disable();
	}
}

void EditAliasesDialog::OnButtonAddClick(wxCommandEvent& event)
{
    wxTextEntryDialog te(this, "Alias to add", "Add an alias");

    if (te.ShowModal() == wxID_OK) {
        auto add = te.GetValue().Lower();

        bool found = false;
        for (int i = 0; !found && i < ListBoxAliases->GetCount(); ++i) {
            if (ListBoxAliases->GetString(i) == add)
                found = true;
        }

        if (!found) {
            ListBoxAliases->Append(add);
        }
    }
    ValidateWindow();
}

void EditAliasesDialog::OnButtonDeleteClick(wxCommandEvent& event)
{
    if (ListBoxAliases->GetSelection() >= 0) {
        ListBoxAliases->Delete(ListBoxAliases->GetSelection());
    }
    ValidateWindow();
}

void EditAliasesDialog::OnButtonOkClick(wxCommandEvent& event)
{
    std::list<std::string> aliases;
    for (int i = 0; i < ListBoxAliases->GetCount(); ++i)
	{
        aliases.push_back(ListBoxAliases->GetString(i));
	}
    _m->SetAliases(aliases);
    EndDialog(wxID_OK);
}

void EditAliasesDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void EditAliasesDialog::OnListBoxAliasesSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
