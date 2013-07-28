#include "CustomModelDialog.h"

//(*InternalHeaders(CustomModel)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(CustomModel)
const long CustomModelDialog::ID_GRID1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(CustomModelDialog,wxDialog)
	//(*EventTable(CustomModel)
	//*)
END_EVENT_TABLE()

CustomModelDialog::CustomModelDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(CustomModel)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	gdModelChans = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxSize(407,430), wxVSCROLL|wxHSCROLL|wxFULL_REPAINT_ON_RESIZE, _T("ID_GRID1"));
	gdModelChans->CreateGrid(1,1);
	gdModelChans->EnableEditing(true);
	gdModelChans->EnableGridLines(true);
	gdModelChans->SetColLabelSize(20);
	gdModelChans->SetRowLabelSize(30);
	gdModelChans->SetDefaultColSize(30, true);
	gdModelChans->SetDefaultCellFont( gdModelChans->GetFont() );
	gdModelChans->SetDefaultCellTextColour( gdModelChans->GetForegroundColour() );
	FlexGridSizer1->Add(gdModelChans, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

CustomModelDialog::~CustomModelDialog()
{
	//(*Destroy(CustomModel)
	//*)
}

