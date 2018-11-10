#include "ViewpointDialog.h"

//(*InternalHeaders(ViewpointDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ViewpointDialog)
const long ViewpointDialog::ID_CHECKLISTBOX_VIEWPOINTS = wxNewId();
const long ViewpointDialog::ID_BUTTON_APPLY = wxNewId();
const long ViewpointDialog::ID_BUTTON_RENAME = wxNewId();
const long ViewpointDialog::ID_BUTTON_EDIT = wxNewId();
const long ViewpointDialog::ID_BUTTON_REMOVE = wxNewId();
//*)

BEGIN_EVENT_TABLE(ViewpointDialog,wxDialog)
	//(*EventTable(ViewpointDialog)
	//*)
END_EVENT_TABLE()

ViewpointDialog::ViewpointDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ViewpointDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer5;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxSize(246,529));
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	CheckListBoxViewpoints = new wxCheckListBox(this, ID_CHECKLISTBOX_VIEWPOINTS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SORT|wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX_VIEWPOINTS"));
	CheckListBoxViewpoints->SetMinSize(wxSize(-1,400));
	FlexGridSizer2->Add(CheckListBoxViewpoints, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 4, 0, 0);
	ApplyButton = new wxButton(this, ID_BUTTON_APPLY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_APPLY"));
	FlexGridSizer5->Add(ApplyButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RenameButton = new wxButton(this, ID_BUTTON_RENAME, _("Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RENAME"));
	FlexGridSizer5->Add(RenameButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	EditButton = new wxButton(this, ID_BUTTON_EDIT, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EDIT"));
	FlexGridSizer5->Add(EditButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RemoveButton = new wxButton(this, ID_BUTTON_REMOVE, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REMOVE"));
	FlexGridSizer5->Add(RemoveButton, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_BUTTON_APPLY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewpointDialog::OnApplyButtonClick);
	Connect(ID_BUTTON_RENAME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewpointDialog::OnRenameButtonClick);
	Connect(ID_BUTTON_EDIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewpointDialog::OnEditButtonClick);
	Connect(ID_BUTTON_REMOVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewpointDialog::OnRemoveButtonClick);
	//*)
}

ViewpointDialog::~ViewpointDialog()
{
	//(*Destroy(ViewpointDialog)
	//*)
}


void ViewpointDialog::OnApplyButtonClick(wxCommandEvent& event)
{
}

void ViewpointDialog::OnRenameButtonClick(wxCommandEvent& event)
{
}

void ViewpointDialog::OnEditButtonClick(wxCommandEvent& event)
{
}

void ViewpointDialog::OnRemoveButtonClick(wxCommandEvent& event)
{
}
