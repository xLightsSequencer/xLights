#include "PreviewModels.h"

//(*InternalHeaders(PreviewModels)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PreviewModels)
const long PreviewModels::ID_BT_ADD_MODEL_GROUP = wxNewId();
const long PreviewModels::ID_BUTTON_REMOVE_MODEL_GROUP = wxNewId();
const long PreviewModels::ID_LISTBOX_MODEL_GROUPS = wxNewId();
const long PreviewModels::ID_STATICTEXT1 = wxNewId();
const long PreviewModels::ID_TEXTCTRL_MODEL_GROUP_NAME = wxNewId();
const long PreviewModels::ID_STATICTEXT3 = wxNewId();
const long PreviewModels::ID_LISTBOX_ADD_TO_MODEL_GROUP = wxNewId();
const long PreviewModels::ID_BUTTON_ADD_TO_MODEL_GROUP = wxNewId();
const long PreviewModels::ID_BUTTON_REMOVE_FROM_MODEL_GROUP = wxNewId();
const long PreviewModels::ID_STATICTEXT2 = wxNewId();
const long PreviewModels::ID_LISTBOX_MODELS_IN_GROUP = wxNewId();
const long PreviewModels::ID_BUTTON_UPDATE_GROUP = wxNewId();
const long PreviewModels::ID_BUTTON_CLOSE = wxNewId();
//*)

BEGIN_EVENT_TABLE(PreviewModels,wxDialog)
	//(*EventTable(PreviewModels)
	//*)
END_EVENT_TABLE()

PreviewModels::PreviewModels(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(PreviewModels)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer11;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(525,450));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Model Groups:"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonAddModelGroup = new wxButton(this, ID_BT_ADD_MODEL_GROUP, _("Add Group"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BT_ADD_MODEL_GROUP"));
	FlexGridSizer8->Add(ButtonAddModelGroup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonRemoveModelGroup = new wxButton(this, ID_BUTTON_REMOVE_MODEL_GROUP, _("Remove Group"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REMOVE_MODEL_GROUP"));
	FlexGridSizer8->Add(ButtonRemoveModelGroup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	ListBoxModelGroups = new wxListBox(this, ID_LISTBOX_MODEL_GROUPS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_MODEL_GROUPS"));
	ListBoxModelGroups->SetMinSize(wxSize(120,100));
	FlexGridSizer4->Add(ListBoxModelGroups, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Group Details:"));
	FlexGridSizer13 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Group Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer6->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextModelGroupName = new wxTextCtrl(this, ID_TEXTCTRL_MODEL_GROUP_NAME, wxEmptyString, wxDefaultPosition, wxSize(121,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MODEL_GROUP_NAME"));
	FlexGridSizer6->Add(TextModelGroupName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13->Add(FlexGridSizer7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Add to Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer10->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBoxAddToModelGroup = new wxListBox(this, ID_LISTBOX_ADD_TO_MODEL_GROUP, wxDefaultPosition, wxSize(100,63), 0, 0, wxVSCROLL, wxDefaultValidator, _T("ID_LISTBOX_ADD_TO_MODEL_GROUP"));
	ListBoxAddToModelGroup->SetMinSize(wxSize(120,100));
	FlexGridSizer10->Add(ListBoxAddToModelGroup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddToModelGroup = new wxButton(this, ID_BUTTON_ADD_TO_MODEL_GROUP, _("->"), wxDefaultPosition, wxSize(44,23), 0, wxDefaultValidator, _T("ID_BUTTON_ADD_TO_MODEL_GROUP"));
	FlexGridSizer11->Add(ButtonAddToModelGroup, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonRemoveFromModelGroup = new wxButton(this, ID_BUTTON_REMOVE_FROM_MODEL_GROUP, _("<-"), wxDefaultPosition, wxSize(36,23), 0, wxDefaultValidator, _T("ID_BUTTON_REMOVE_FROM_MODEL_GROUP"));
	FlexGridSizer11->Add(ButtonRemoveFromModelGroup, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Models in Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer9->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBoxModelsInGroup = new wxListBox(this, ID_LISTBOX_MODELS_IN_GROUP, wxDefaultPosition, wxSize(100,0), 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_MODELS_IN_GROUP"));
	ListBoxModelsInGroup->SetMinSize(wxSize(120,100));
	FlexGridSizer9->Add(ListBoxModelsInGroup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonUpdateGroup = new wxButton(this, ID_BUTTON_UPDATE_GROUP, _("Update Group"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_UPDATE_GROUP"));
	FlexGridSizer2->Add(ButtonUpdateGroup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonClose = new wxButton(this, ID_BUTTON_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLOSE"));
	FlexGridSizer2->Add(ButtonClose, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();
	//*)
}

PreviewModels::~PreviewModels()
{
	//(*Destroy(PreviewModels)
	//*)
}

