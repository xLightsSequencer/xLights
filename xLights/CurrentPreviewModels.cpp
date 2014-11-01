#include "CurrentPreviewModels.h"
#include <wx/xml/xml.h>

//(*InternalHeaders(CurrentPreviewModels)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(CurrentPreviewModels)
const long CurrentPreviewModels::ID_STATICTEXT1 = wxNewId();
const long CurrentPreviewModels::ID_CHECKLISTBOX_CURRENT_GROUPS = wxNewId();
const long CurrentPreviewModels::ID_BUTTON_EDIT_GROUPS = wxNewId();
//*)

BEGIN_EVENT_TABLE(CurrentPreviewModels,wxDialog)
	//(*EventTable(CurrentPreviewModels)
	//*)
END_EVENT_TABLE()

CurrentPreviewModels::CurrentPreviewModels(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(CurrentPreviewModels)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Model Groups:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckListBoxCurrentGroups = new wxCheckListBox(this, ID_CHECKLISTBOX_CURRENT_GROUPS, wxDefaultPosition, wxSize(150,140), 0, 0, wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX_CURRENT_GROUPS"));
	FlexGridSizer2->Add(CheckListBoxCurrentGroups, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer4->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonEditGroups = new wxButton(this, ID_BUTTON_EDIT_GROUPS, _("Edit Groups"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EDIT_GROUPS"));
	FlexGridSizer5->Add(ButtonEditGroups, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKLISTBOX_CURRENT_GROUPS,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&CurrentPreviewModels::OnCheckListBoxCurrentGroupsToggled);
	//*)
}

CurrentPreviewModels::~CurrentPreviewModels()
{
	//(*Destroy(CurrentPreviewModels)
	//*)
}


void CurrentPreviewModels::OnCheckListBoxCurrentGroupsToggled(wxCommandEvent& event)
{
    wxString sChecked;
    for(int i=0;i<CheckListBoxCurrentGroups->GetCount();i++)
    {
        wxXmlNode* e = (wxXmlNode*)CheckListBoxCurrentGroups->GetClientData(i);
        e->DeleteAttribute("selected");
        sChecked = CheckListBoxCurrentGroups->IsChecked(i)?"1":"0";
        e->AddAttribute("selected",sChecked);
    }
}
