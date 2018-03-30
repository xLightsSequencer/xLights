#include "OptionChooser.h"

//(*InternalHeaders(OptionChooser)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(OptionChooser)
const long OptionChooser::ID_STATICTEXT_Option_Select = wxNewId();
const long OptionChooser::ID_LISTBOX_Options = wxNewId();
//*)

BEGIN_EVENT_TABLE(OptionChooser,wxDialog)
	//(*EventTable(OptionChooser)
	//*)
END_EVENT_TABLE()

OptionChooser::OptionChooser(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(OptionChooser)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText_Option_Select = new wxStaticText(this, ID_STATICTEXT_Option_Select, _("Select Options:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Option_Select"));
	FlexGridSizer1->Add(StaticText_Option_Select, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListBox_Options = new wxListBox(this, ID_LISTBOX_Options, wxDefaultPosition, wxDLG_UNIT(this,wxSize(120,80)), 0, 0, wxLB_MULTIPLE, wxDefaultValidator, _T("ID_LISTBOX_Options"));
	FlexGridSizer1->Add(ListBox_Options, 1, wxALL|wxEXPAND, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
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

OptionChooser::~OptionChooser()
{
	//(*Destroy(OptionChooser)
	//*)
}

void OptionChooser::SetInstructionText(const wxString& text)
{
    StaticText_Option_Select->SetLabel(text);
    Fit();
}

void OptionChooser::SetOptions(const wxArrayString& options)
{
    ListBox_Options->Append(options);
}

void OptionChooser::GetSelectedOptions(wxArrayString& options)
{
    wxArrayInt selections;
    ListBox_Options->GetSelections(selections);
    for( int i = 0; i < selections.GetCount(); i++ )
    {
        options.push_back(ListBox_Options->GetString(selections[i]));
    }
}
