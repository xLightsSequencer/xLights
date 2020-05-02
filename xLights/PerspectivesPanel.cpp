/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PerspectivesPanel.h"
#include <wx/wx.h>

//(*InternalHeaders(PerspectivesPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PerspectivesPanel)
const long PerspectivesPanel::ID_BUTTON_ADD_PERSPECTIVE = wxNewId();
const long PerspectivesPanel::D_BUTTON_DELETE_PERSPECTIVE = wxNewId();
const long PerspectivesPanel::ID_BUTTON_RENAME_PERSPECTIVE = wxNewId();
const long PerspectivesPanel::ID_BUTTON_SAVE_PERSPECTIVE = wxNewId();
const long PerspectivesPanel::ID_LISTBOX_PERSPECTIVES = wxNewId();
const long PerspectivesPanel::ID_STATICTEXT1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PerspectivesPanel,wxPanel)
	//(*EventTable(PerspectivesPanel)
	//*)
END_EVENT_TABLE()

PerspectivesPanel::PerspectivesPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(PerspectivesPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 6, 0, 0);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	ButtonAddPerspective = new wxButton(this, ID_BUTTON_ADD_PERSPECTIVE, _("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BUTTON_ADD_PERSPECTIVE"));
	ButtonAddPerspective->SetToolTip(_("Add Perspective"));
	FlexGridSizer3->Add(ButtonAddPerspective, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ButtonDeletePerspective = new wxButton(this, D_BUTTON_DELETE_PERSPECTIVE, _("-"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("D_BUTTON_DELETE_PERSPECTIVE"));
	ButtonDeletePerspective->SetToolTip(_("Delete Perspective"));
	FlexGridSizer3->Add(ButtonDeletePerspective, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ButtonRenamePerspective = new wxButton(this, ID_BUTTON_RENAME_PERSPECTIVE, _("Rename"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BUTTON_RENAME_PERSPECTIVE"));
	ButtonRenamePerspective->SetToolTip(_("Rename Perspective"));
	FlexGridSizer3->Add(ButtonRenamePerspective, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	ButtonSavePerspective = new wxButton(this, ID_BUTTON_SAVE_PERSPECTIVE, _("Save"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BUTTON_SAVE_PERSPECTIVE"));
	ButtonSavePerspective->SetToolTip(_("Save Perspective"));
	FlexGridSizer3->Add(ButtonSavePerspective, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	ListBoxPerspectives = new wxListBox(this, ID_LISTBOX_PERSPECTIVES, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_PERSPECTIVES"));
	FlexGridSizer2->Add(ListBoxPerspectives, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Dbl-Clk to load"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_ADD_PERSPECTIVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnButtonAddPerspectiveClick);
	Connect(D_BUTTON_DELETE_PERSPECTIVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnButtonDeletePerspectiveClick);
	Connect(ID_BUTTON_RENAME_PERSPECTIVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnButtonRenamePerspectiveClick);
	Connect(ID_BUTTON_SAVE_PERSPECTIVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnButtonSavePerspectiveClick);
	Connect(ID_LISTBOX_PERSPECTIVES,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnListBoxPerspectivesDClick);
	Connect(wxEVT_PAINT,(wxObjectEventFunction)&PerspectivesPanel::OnPaint);
	//*)
}

PerspectivesPanel::~PerspectivesPanel()
{
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
	//(*Destroy(PerspectivesPanel)
	//*)
}


void PerspectivesPanel::OnButtonAddPerspectiveClick(wxCommandEvent& event)
{
    wxString name = wxGetTextFromUser("Enter name of perspective","Perspective Name");
    if(name.size()>0 && !CheckForDuplicates(name))
    {
        wxXmlNode* p=new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        p->AddAttribute("name", name);
        p->AddAttribute("settings","");
        mPerspectivesNode->AddChild(p);
        ListBoxPerspectives->Append(name,p);
        wxCommandEvent eventPerspectivesChanged(EVT_PERSPECTIVES_CHANGED);
        wxPostEvent(GetParent(), eventPerspectivesChanged);
    }
}

void PerspectivesPanel::OnPaint(wxPaintEvent& event)
{
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}

void PerspectivesPanel::SetPerspectives(wxXmlNode* perspectivesNode)
{
    mPerspectivesNode = perspectivesNode;
    ListBoxPerspectives->Clear();

    for(wxXmlNode* p=mPerspectivesNode->GetChildren(); p!=NULL; p=p->GetNext() )
    {
        if (p->GetName() == "perspective")
        {
            wxString name=p->GetAttribute("name");
            if (!name.IsEmpty())
            {
                ListBoxPerspectives->Append(name,p);
            }
        }
    }
}

void PerspectivesPanel::OnListBoxPerspectivesDClick(wxCommandEvent& event)
{
    int selection_index = ListBoxPerspectives->GetSelection();
    wxXmlNode* p = (wxXmlNode*)(ListBoxPerspectives->GetClientData(selection_index));
    if( p != nullptr )
    {
        wxCommandEvent eventLoadPerspective(EVT_LOAD_PERSPECTIVE);
        eventLoadPerspective.SetClientData(p);
        wxPostEvent(GetParent(), eventLoadPerspective);
    }
}

void PerspectivesPanel::OnButtonRenamePerspectiveClick(wxCommandEvent& event)
{
    int selection_index = ListBoxPerspectives->GetSelection();
    if( selection_index >= 0 )
    {
        wxXmlNode* p = (wxXmlNode*)(ListBoxPerspectives->GetClientData(selection_index));
        if( p != nullptr )
        {
            wxString name = wxGetTextFromUser("Enter new name for perspective","Rename Perspective ");
            if(name.size()>0 && !CheckForDuplicates(name))
            {
                if (p->GetAttribute("name") == mPerspectivesNode->GetAttribute("current")) {
                    mPerspectivesNode->DeleteAttribute("current");
                    mPerspectivesNode->AddAttribute("current",name);
                }
                p->DeleteAttribute("name");
                p->AddAttribute("name", name);
                ListBoxPerspectives->SetString(ListBoxPerspectives->GetSelection(),name);

                wxCommandEvent eventPerspectivesChanged(EVT_PERSPECTIVES_CHANGED);
                wxPostEvent(GetParent(), eventPerspectivesChanged);
            }
        }
    }
}

void PerspectivesPanel::OnButtonDeletePerspectiveClick(wxCommandEvent& event)
{
    int selection_index = ListBoxPerspectives->GetSelection();
    if( selection_index >= 0 )
    {
        wxXmlNode* p = (wxXmlNode*)(ListBoxPerspectives->GetClientData(selection_index));
        if( p != nullptr )
        {
            mPerspectivesNode->RemoveChild(p);
            ListBoxPerspectives->Delete(selection_index);
        }
        wxCommandEvent eventPerspectives(EVT_PERSPECTIVES_CHANGED);
        wxPostEvent(GetParent(), eventPerspectives);
    }
}

void PerspectivesPanel::OnButtonSavePerspectiveClick(wxCommandEvent& event)
{
    wxCommandEvent eventSavePerspectives(EVT_SAVE_PERSPECTIVES);
    wxPostEvent(GetParent(), eventSavePerspectives);
}

bool PerspectivesPanel::CheckForDuplicates(const wxString& perspective_name)
{
    for(wxXmlNode* p=mPerspectivesNode->GetChildren(); p!=NULL; p=p->GetNext() )
    {
        if (p->GetName() == "perspective")
        {
            wxString name=p->GetAttribute("name");
            if (name == perspective_name)
            {
                return true;
            }
        }
    }
    return false;
}
