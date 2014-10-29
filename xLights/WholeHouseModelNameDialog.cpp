#include "WholeHouseModelNameDialog.h"

//(*InternalHeaders(WholeHouseModelNameDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(WholeHouseModelNameDialog)
const long WholeHouseModelNameDialog::ID_STATICTEXT1 = wxNewId();
const long WholeHouseModelNameDialog::ID_TEXT_WHOLE_HOUSE_MODEL_NAME = wxNewId();
//*)

BEGIN_EVENT_TABLE(WholeHouseModelNameDialog,wxDialog)
	//(*EventTable(WholeHouseModelNameDialog)
	//*)
END_EVENT_TABLE()

WholeHouseModelNameDialog::WholeHouseModelNameDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(WholeHouseModelNameDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Model Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Text_WholehouseModelName = new wxTextCtrl(this, ID_TEXT_WHOLE_HOUSE_MODEL_NAME, _("New Wholehouse Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXT_WHOLE_HOUSE_MODEL_NAME"));
	Text_WholehouseModelName->SetMinSize(wxSize(150,20));
	FlexGridSizer3->Add(Text_WholehouseModelName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer2->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(wxID_ANY,wxEVT_INIT_DIALOG,(wxObjectEventFunction)&WholeHouseModelNameDialog::OnInit);
	//*)
}

WholeHouseModelNameDialog::~WholeHouseModelNameDialog()
{
	//(*Destroy(WholeHouseModelNameDialog)
	//*)
}


void WholeHouseModelNameDialog::OnInit(wxInitDialogEvent& event)
{
    Text_WholehouseModelName->SelectAll();
}
