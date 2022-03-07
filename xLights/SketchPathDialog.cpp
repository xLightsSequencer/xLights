#include "SketchPathDialog.h"

//(*InternalHeaders(SketchPathDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SketchPathDialog)
const long SketchPathDialog::ID_STATICBOX1 = wxNewId();
const long SketchPathDialog::ID_STATICTEXT1 = wxNewId();
const long SketchPathDialog::ID_BUTTON1 = wxNewId();
const long SketchPathDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SketchPathDialog,wxDialog)
	//(*EventTable(SketchPathDialog)
	//*)
END_EVENT_TABLE()

SketchPathDialog::SketchPathDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SketchPathDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	StaticBox1 = new wxStaticBox(this, ID_STATICBOX1, _("Current Point"), wxDefaultPosition, wxDefaultSize, wxBORDER_STATIC, _T("ID_STATICBOX1"));
	FlexGridSizer1->Add(StaticBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

SketchPathDialog::~SketchPathDialog()
{
	//(*Destroy(SketchPathDialog)
	//*)
}

