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

PreviewModels::PreviewModels(wxWindow* parent,wxXmlNode* ModelGroups, wxXmlNode* Models, wxWindowID id,const wxPoint& pos,const wxSize& size)
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
	ListBoxModelGroups = new wxListBox(this, ID_LISTBOX_MODEL_GROUPS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_MODEL_GROUPS"));
	ListBoxModelGroups->SetMinSize(wxSize(120,100));
	FlexGridSizer4->Add(ListBoxModelGroups, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Group Details:"));
	FlexGridSizer13 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Group Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer6->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextModelGroupName = new wxTextCtrl(this, ID_TEXTCTRL_MODEL_GROUP_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_MODEL_GROUP_NAME"));
	FlexGridSizer6->Add(TextModelGroupName, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(FlexGridSizer6, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13->Add(FlexGridSizer7, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Add to Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer10->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBoxAddToModelGroup = new wxListBox(this, ID_LISTBOX_ADD_TO_MODEL_GROUP, wxDefaultPosition, wxSize(100,63), 0, 0, wxLB_SORT|wxVSCROLL, wxDefaultValidator, _T("ID_LISTBOX_ADD_TO_MODEL_GROUP"));
	ListBoxAddToModelGroup->SetMinSize(wxSize(120,100));
	FlexGridSizer10->Add(ListBoxAddToModelGroup, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	ListBoxModelsInGroup = new wxListBox(this, ID_LISTBOX_MODELS_IN_GROUP, wxDefaultPosition, wxSize(100,0), 0, 0, wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_MODELS_IN_GROUP"));
	ListBoxModelsInGroup->SetMinSize(wxSize(120,100));
	FlexGridSizer9->Add(ListBoxModelsInGroup, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	FlexGridSizer1->Add(FlexGridSizer3, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BT_ADD_MODEL_GROUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonAddModelGroupClick);
	Connect(ID_BUTTON_REMOVE_MODEL_GROUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonRemoveModelGroupClick);
	Connect(ID_LISTBOX_MODEL_GROUPS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&PreviewModels::OnListBoxModelGroupsSelect);
	Connect(ID_BUTTON_ADD_TO_MODEL_GROUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonAddToModelGroupClick);
	Connect(ID_BUTTON_REMOVE_FROM_MODEL_GROUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonRemoveFromModelGroupClick);
	Connect(ID_BUTTON_UPDATE_GROUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonUpdateGroupClick);
	Connect(ID_BUTTON_CLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonCloseClick);
	//*)
	mModelGroups = ModelGroups;
	mModels = Models;
	PopulateModelGroups();
}

PreviewModels::~PreviewModels()
{
	//(*Destroy(PreviewModels)
	//*)
}

void PreviewModels::PopulateModelGroups()
{
    wxString name;
    wxXmlNode* e;
	for(e=mModelGroups->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "modelGroup")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                ListBoxModelGroups->Append(name,e);
            }
        }
    }
}

void PreviewModels::UpdateSelectedModel()
{
    wxString groupModels;
    wxXmlNode* e;
    e=(wxXmlNode*)(ListBoxModelGroups->GetClientData(ListBoxModelGroups->GetSelection()));
    groupModels = e->GetAttribute("models");
    ListBoxModelsInGroup->Clear();
    wxArrayString ModelsInGroup=wxSplit(groupModels,',');
    for(int i=0;i<ModelsInGroup.size();i++)
    {
        ListBoxModelsInGroup->Append(ModelsInGroup[i]);
    }
    TextModelGroupName->SetValue(ListBoxModelGroups->GetString(ListBoxModelGroups->GetSelection()));
    PopulateUnusedModels(ModelsInGroup);
}


void PreviewModels::OnListBoxModelGroupsSelect(wxCommandEvent& event)
{
    UpdateSelectedModel();
}

void PreviewModels::PopulateUnusedModels(wxArrayString ModelsInGroup)
{
    wxString name;
    wxXmlNode* e;
    ListBoxAddToModelGroup->Clear();
    for(e=mModels->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            if(ModelsInGroup.Index(name,false,false)==wxNOT_FOUND)
            {
                ListBoxAddToModelGroup->Append(name);
            }
        }
    }
}

void PreviewModels::OnButtonAddToModelGroupClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxAddToModelGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND)
    {
        ListBoxModelsInGroup->Append(ListBoxAddToModelGroup->GetString(selectedIndex));
        ListBoxAddToModelGroup->Delete(selectedIndex);
    }
    if(selectedIndex<ListBoxAddToModelGroup->GetCount())
    {
        ListBoxAddToModelGroup->SetSelection(selectedIndex,TRUE);
    }
    else
    {
        ListBoxAddToModelGroup->SetSelection(ListBoxAddToModelGroup->GetCount()-1,TRUE);
    }
}

void PreviewModels::OnButtonRemoveFromModelGroupClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxModelsInGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND)
    {
        ListBoxAddToModelGroup->Append(ListBoxModelsInGroup->GetString(selectedIndex));
        ListBoxModelsInGroup->Delete(selectedIndex);
    }
    if(selectedIndex<ListBoxModelsInGroup->GetCount())
    {
        ListBoxModelsInGroup->SetSelection(selectedIndex,TRUE);
    }
    else
    {
        ListBoxModelsInGroup->SetSelection(ListBoxModelsInGroup->GetCount()-1,TRUE);
    }
}

void PreviewModels::OnButtonUpdateGroupClick(wxCommandEvent& event)
{
    wxXmlNode* e;
    if(ListBoxModelGroups->GetSelection() != wxNOT_FOUND)
    {
        wxString ModelsInGroup="";
        for(int i=0;i<ListBoxModelsInGroup->GetCount();i++)
        {
            if (i<ListBoxModelsInGroup->GetCount()-1)
            {
                ModelsInGroup += ListBoxModelsInGroup->GetString(i) + ",";
            }
            else
            {
                ModelsInGroup += ListBoxModelsInGroup->GetString(i);
            }
        }
        ListBoxModelGroups->SetString(ListBoxModelGroups->GetSelection(),TextModelGroupName->GetValue());
        e=(wxXmlNode*)(ListBoxModelGroups->GetClientData(ListBoxModelGroups->GetSelection()));
        e->DeleteAttribute("name");
        e->AddAttribute("name",TextModelGroupName->GetValue());
        e->DeleteAttribute("models");
        e->AddAttribute("models",ModelsInGroup);
    }
}

void PreviewModels::OnButtonRemoveModelGroupClick(wxCommandEvent& event)
{
    int selected_index = ListBoxModelGroups->GetSelection();
    if(selected_index != wxNOT_FOUND)
    {
        wxXmlNode* e=(wxXmlNode*)(ListBoxModelGroups->GetClientData(selected_index));
        mModelGroups->RemoveChild(e);
        ListBoxModelGroups->Delete(selected_index);
        int model_count = ListBoxModelGroups->GetCount();
        if( model_count > 0 )
        {
            if( selected_index >= model_count )
            {
                selected_index = model_count - 1;
            }
            ListBoxModelGroups->SetSelection(selected_index);
            UpdateSelectedModel();
        }
    }
}

void PreviewModels::OnButtonAddModelGroupClick(wxCommandEvent& event)
{
    wxArrayString arrModelsInGroup;
    wxXmlNode* e=new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
    e->AddAttribute("name", "New Model Group");
    e->AddAttribute("models", "");
    mModelGroups->AddChild(e);
    int item_index = ListBoxModelGroups->Append("New Model Group",e);
    ListBoxModelGroups->SetSelection(item_index);
    TextModelGroupName->SetValue("New Model Group");
    PopulateUnusedModels(arrModelsInGroup);
    ListBoxModelsInGroup->Clear();
}

void PreviewModels::OnButtonCloseClick(wxCommandEvent& event)
{
    this->EndModal(wxID_OK);
}
