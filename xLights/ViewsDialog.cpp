#include "ViewsDialog.h"

#include "djdebug.cpp"

#define WANT_DEBUG_IMPL
#define WANT_DEBUG  100 //unbuffered in case app crashes

#ifndef debug_function //dummy defs if debug cpp not included above
#define debug(level, ...)
#define debug_more(level, ...)
#define debug_function(level)
#endif

//cut down on mem allocs outside debug() when WANT_DEBUG is off:
#ifdef WANT_DEBUG
#define IFDEBUG(stmt)  stmt
#else
#define IFDEBUG(stmt)
#endif // WANT_DEBUG



//(*InternalHeaders(ViewsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ViewsDialog)
const long ViewsDialog::ID_BT_ADD_VIEW = wxNewId();
const long ViewsDialog::ID_BT_DELETE_VIEW = wxNewId();
const long ViewsDialog::ID_STATICTEXT3 = wxNewId();
const long ViewsDialog::ID_LST_VIEWS = wxNewId();
const long ViewsDialog::ID_STATICTEXT4 = wxNewId();
const long ViewsDialog::ID_TEXT_VIEW_NAME = wxNewId();
const long ViewsDialog::ID_STATICTEXT2 = wxNewId();
const long ViewsDialog::ID_LST_ADD_MODEL_VIEWS = wxNewId();
const long ViewsDialog::ID_BT_ADD_MODEL_VIEW = wxNewId();
const long ViewsDialog::ID_BT_REVOVE_MODEL_VIEW = wxNewId();
const long ViewsDialog::ID_STATICTEXT1 = wxNewId();
const long ViewsDialog::ID_LST_MODELS_VIEW = wxNewId();
const long ViewsDialog::ID_BUTTON_UPDATE_VIEW = wxNewId();
const long ViewsDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ViewsDialog,wxDialog)
	//(*EventTable(ViewsDialog)
	//*)
END_EVENT_TABLE()

ViewsDialog::ViewsDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(ViewsDialog)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
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
	FlexGridSizer1->AddGrowableCol(0);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Views:"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(1);
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer9->AddGrowableCol(1);
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	btAddView = new wxButton(this, ID_BT_ADD_VIEW, _("Add View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BT_ADD_VIEW"));
	FlexGridSizer8->Add(btAddView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btRemoveView = new wxButton(this, ID_BT_DELETE_VIEW, _("Remove View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BT_DELETE_VIEW"));
	FlexGridSizer8->Add(btRemoveView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	FlexGridSizer6->AddGrowableRow(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Views:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer6->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	lstViews = new wxListBox(this, ID_LST_VIEWS, wxDefaultPosition, wxSize(-1,100), 0, 0, 0, wxDefaultValidator, _T("ID_LST_VIEWS"));
	FlexGridSizer6->Add(lstViews, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer9->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer10->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("View Details:"));
	FlexGridSizer12 = new wxFlexGridSizer(3, 1, 0, 0);
	FlexGridSizer12->AddGrowableCol(0);
	FlexGridSizer12->AddGrowableRow(1);
	FlexGridSizer13 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer13->AddGrowableCol(1);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer13->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Text_ViewName = new wxTextCtrl(this, ID_TEXT_VIEW_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXT_VIEW_NAME"));
	FlexGridSizer13->Add(Text_ViewName, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	FlexGridSizer12->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableCol(2);
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	FlexGridSizer11->AddGrowableRow(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Models to add:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer11->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	lstAddModelToViews = new wxListBox(this, ID_LST_ADD_MODEL_VIEWS, wxDefaultPosition, wxSize(-1,120), 0, 0, wxLB_SORT|wxVSCROLL, wxDefaultValidator, _T("ID_LST_ADD_MODEL_VIEWS"));
	FlexGridSizer11->Add(lstAddModelToViews, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(2, 1, 0, 0);
	btAddModelToView = new wxButton(this, ID_BT_ADD_MODEL_VIEW, _("->"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BT_ADD_MODEL_VIEW"));
	FlexGridSizer4->Add(btAddModelToView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btRemoveModelFromView = new wxButton(this, ID_BT_REVOVE_MODEL_VIEW, _("<-"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BT_REVOVE_MODEL_VIEW"));
	FlexGridSizer4->Add(btRemoveModelFromView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Models in View:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	lstModelsInView = new wxListBox(this, ID_LST_MODELS_VIEW, wxDefaultPosition, wxSize(-1,120), 0, 0, wxLB_SORT|wxVSCROLL, wxDefaultValidator, _T("ID_LST_MODELS_VIEW"));
	FlexGridSizer5->Add(lstModelsInView, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer12->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	btnUpdateView = new wxButton(this, ID_BUTTON_UPDATE_VIEW, _("Update View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_UPDATE_VIEW"));
	FlexGridSizer7->Add(btnUpdateView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btCloseViews = new wxButton(this, ID_BUTTON1, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer7->Add(btCloseViews, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer10->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BT_ADD_VIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsDialog::OnBtAddViewClick);
	Connect(ID_BT_DELETE_VIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsDialog::OnBtRemoveViewClick);
	Connect(ID_LST_VIEWS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&ViewsDialog::OnlstViewsSelect);
	Connect(ID_TEXT_VIEW_NAME,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&ViewsDialog::OnText_ViewNameTextEnter);
	Connect(ID_BT_ADD_MODEL_VIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsDialog::OnbtAddModelToViewClick);
	Connect(ID_BT_REVOVE_MODEL_VIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsDialog::OnbtRemoveModelFromViewClick);
	Connect(ID_BUTTON_UPDATE_VIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsDialog::OnbtnUpdateViewClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsDialog::OnbtCloseViewsClick);
	//*)
}

void ViewsDialog::SetModelAndViewNodes(wxXmlNode* modelsNode,wxXmlNode* viewsNode)
{
    wxString name;
    wxXmlNode* e;
    models = modelsNode;
    views = viewsNode;
    for(e=views->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "view")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                lstViews->Append(name,e);
            }
        }
    }

}

ViewsDialog::~ViewsDialog()
{
	//(*Destroy(ViewsDialog)
	//*)
}


void ViewsDialog::OnlstViewsSelect(wxCommandEvent& event)
{
    wxString viewModels;
    wxXmlNode* e;
    e=(wxXmlNode*)(lstViews->GetClientData(lstViews->GetSelection()));
    viewModels = e->GetAttribute("models");
    debug(1,"Model=");
    lstModelsInView->Clear();
    wxArrayString model=wxSplit(viewModels,',');
    for(int i=0;i<model.size();i++)
    {
        lstModelsInView->Append(model[i]);
    }
    Text_ViewName->SetValue(lstViews->GetString(lstViews->GetSelection()));
    PopulateUnusedModels(model);
}

void ViewsDialog::PopulateUnusedModels(wxArrayString model)
{
    wxString name;
    wxXmlNode* e;
    lstAddModelToViews->Clear();
    for(e=models->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            if(model.Index(name,false,false)==wxNOT_FOUND)
            {
                lstAddModelToViews->Append(name);
            }
        }
    }
}

void ViewsDialog::OnbtAddModelToViewClick(wxCommandEvent& event)
{
    int selectedIndex = lstAddModelToViews->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND)
    {
        lstModelsInView->Append(lstAddModelToViews->GetString(selectedIndex));
        lstAddModelToViews->Delete(selectedIndex);
    }
    if(selectedIndex<lstAddModelToViews->GetCount())
    {
        lstAddModelToViews->SetSelection(selectedIndex,TRUE);
    }
    else
    {
        lstAddModelToViews->SetSelection(lstAddModelToViews->GetCount()-1,TRUE);
    }
}

void ViewsDialog::OnbtRemoveModelFromViewClick(wxCommandEvent& event)
{
    int selectedIndex = lstModelsInView->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND)
    {
        lstAddModelToViews->Append(lstModelsInView->GetString(selectedIndex));
        lstModelsInView->Delete(selectedIndex);
    }
    if(selectedIndex<lstModelsInView->GetCount())
    {
        lstModelsInView->SetSelection(selectedIndex,TRUE);
    }
    else
    {
        lstModelsInView->SetSelection(lstModelsInView->GetCount()-1,TRUE);
    }
}

void ViewsDialog::OnbtnUpdateViewClick(wxCommandEvent& event)
{
    wxXmlNode* e;
    if(lstViews->GetSelection() != wxNOT_FOUND)
    {
        wxString viewModels="";
        for(size_t i=0;i<lstModelsInView->GetCount();i++)
        {
            if (i<lstModelsInView->GetCount()-1)
            {
                viewModels += lstModelsInView->GetString(i) + ",";
            }
            else
            {
                viewModels += lstModelsInView->GetString(i);
            }
        }
        lstViews->SetString(lstViews->GetSelection(),Text_ViewName->GetValue());
        e=(wxXmlNode*)(lstViews->GetClientData(lstViews->GetSelection()));
        e->DeleteAttribute("name");
        e->AddAttribute("name",Text_ViewName->GetValue());
        e->DeleteAttribute("models");
        e->AddAttribute("models",viewModels);
    }
}

void ViewsDialog::OnText_ViewNameTextEnter(wxCommandEvent& event)
{
}

void ViewsDialog::OnBtAddViewClick(wxCommandEvent& event)
{
    wxArrayString arrModels;
    wxXmlNode* e=new wxXmlNode(wxXML_ELEMENT_NODE, "view");
    e->AddAttribute("name", "New View");
    e->AddAttribute("models", "");
    views->AddChild(e);
    lstViews->Append("New View",e);
    lstViews->SetSelection(lstViews->GetCount()-1);
    Text_ViewName->SetValue("New View");
    PopulateUnusedModels(arrModels);
}

void ViewsDialog::OnBtRemoveViewClick(wxCommandEvent& event)
{
    if(lstViews->GetSelection() != wxNOT_FOUND)
    {
        wxXmlNode* e=(wxXmlNode*)(lstViews->GetClientData(lstViews->GetSelection()));
        views->RemoveChild(e);
        lstViews->Delete(lstViews->GetSelection());
    }
}

void ViewsDialog::OnbtCloseViewsClick(wxCommandEvent& event)
{
    this->EndModal(wxID_OK);
}
