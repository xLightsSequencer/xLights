#include "ModelViewSelector.h"

//(*InternalHeaders(ModelViewSelector)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ModelViewSelector)
const long ModelViewSelector::ID_STATICTEXT_TYPE = wxNewId();
const long ModelViewSelector::ID_LISTBOX_ELEMENTS = wxNewId();
const long ModelViewSelector::ID_STATICTEXT1 = wxNewId();
const long ModelViewSelector::ID_BUTTON_ADD = wxNewId();
const long ModelViewSelector::ID_BUTTON_CLOSE = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelViewSelector,wxDialog)
	//(*EventTable(ModelViewSelector)
	//*)
END_EVENT_TABLE()

ModelViewSelector::ModelViewSelector(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelViewSelector)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Add Models"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	SetMinSize(wxSize(200,250));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	StaticTextType = new wxStaticText(this, ID_STATICTEXT_TYPE, _("Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_TYPE"));
	FlexGridSizer2->Add(StaticTextType, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBoxElements = new wxListBox(this, ID_LISTBOX_ELEMENTS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_MULTIPLE|wxLB_SORT|wxVSCROLL, wxDefaultValidator, _T("ID_LISTBOX_ELEMENTS"));
	ListBoxElements->SetMinSize(wxDLG_UNIT(this,wxSize(150,150)));
	FlexGridSizer2->Add(ListBoxElements, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Multiple elements can be selected"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT1"));
	FlexGridSizer4->Add(StaticText1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableCol(1);
	ButtonAdd = new wxButton(this, ID_BUTTON_ADD, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD"));
	FlexGridSizer3->Add(ButtonAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonClose = new wxButton(this, ID_BUTTON_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLOSE"));
	FlexGridSizer3->Add(ButtonClose, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_ADD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelViewSelector::OnButtonAddClick);
	Connect(ID_BUTTON_CLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelViewSelector::OnButtonCloseClick);
	//*)
}

ModelViewSelector::~ModelViewSelector()
{
	//(*Destroy(ModelViewSelector)
	//*)
}

void ModelViewSelector::Initialize(wxString type)
{
    if(type == "view")
    {
        SetLabel("Add Views");
        StaticTextType->SetLabel("Views:");
        PopulateViewsToAdd();
    }
    else
    {
        SetLabel("Add Models");
        StaticTextType->SetLabel("Models:");
        PopulateModelsToAdd();
    }
}

void ModelViewSelector::PopulateViewsToAdd()
{
    ListBoxElements->Clear();
    for(wxXmlNode* e=mViews->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "view")
        {
            wxString name=e->GetAttribute("name");
            if (!mSequenceElements->ElementExists(name))
            {
                ListBoxElements->Append(name,e);
            }
        }
    }
}

void ModelViewSelector::PopulateModelsToAdd()
{
    ListBoxElements->Clear();
    for(wxXmlNode* e=mModels->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            wxString name=e->GetAttribute("name");
            if (!mSequenceElements->ElementExists(name))
            {
                ListBoxElements->Append(name,e);
            }
        }
    }
    for(wxXmlNode* e=mModelGroups->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "modelGroup")
        {
            wxString name=e->GetAttribute("name");
            if (!mSequenceElements->ElementExists(name))
            {
                ListBoxElements->Append(name,e);
            }
        }
    }
}



void ModelViewSelector::SetSequenceElementsModelsViews(SequenceElements* elements,wxXmlNode* models,wxXmlNode* modelGroups, wxXmlNode* views)
{
    mSequenceElements = elements;
    mModelGroups = modelGroups;
    mModels = models;
    mViews = views;
}



void ModelViewSelector::OnButtonAddClick(wxCommandEvent& eevent)
{
    for(int i=0;i<ListBoxElements->GetCount();i++)
    {
        if (ListBoxElements->IsSelected(i))
        {
            ElementsToAdd.push_back(ListBoxElements->GetString(i));
        }
    }
    this->EndModal(wxID_OK);
}

void ModelViewSelector::OnButtonCloseClick(wxCommandEvent& event)
{
    this->EndModal(wxID_CANCEL);
}
