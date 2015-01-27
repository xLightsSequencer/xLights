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
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
	ButtonAddPerspective = new wxButton(this, ID_BUTTON_ADD_PERSPECTIVE, _("+"), wxDefaultPosition, wxSize(24,24), 0, wxDefaultValidator, _T("ID_BUTTON_ADD_PERSPECTIVE"));
	ButtonAddPerspective->SetToolTip(_("Add Perspective"));
	FlexGridSizer3->Add(ButtonAddPerspective, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ButtonDeletePerspective = new wxButton(this, D_BUTTON_DELETE_PERSPECTIVE, _("-"), wxDefaultPosition, wxSize(24,24), 0, wxDefaultValidator, _T("D_BUTTON_DELETE_PERSPECTIVE"));
	ButtonDeletePerspective->SetToolTip(_("Delete Perspective"));
	FlexGridSizer3->Add(ButtonDeletePerspective, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ButtonRenamePerspective = new wxButton(this, ID_BUTTON_RENAME_PERSPECTIVE, _("Rename"), wxDefaultPosition, wxSize(70,24), 0, wxDefaultValidator, _T("ID_BUTTON_RENAME_PERSPECTIVE"));
	ButtonRenamePerspective->SetToolTip(_("Rename Perspective"));
	FlexGridSizer3->Add(ButtonRenamePerspective, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	ListBoxPerspectives = new wxListBox(this, ID_LISTBOX_PERSPECTIVES, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_PERSPECTIVES"));
	FlexGridSizer2->Add(ListBoxPerspectives, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Dbl-Clk to load"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_ADD_PERSPECTIVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnButtonAddPerspectiveClick);
	Connect(ID_BUTTON_RENAME_PERSPECTIVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnButtonRenamePerspectiveClick);
	Connect(ID_LISTBOX_PERSPECTIVES,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&PerspectivesPanel::OnListBoxPerspectivesDClick);
	Connect(wxEVT_PAINT,(wxObjectEventFunction)&PerspectivesPanel::OnPaint);
	//*)
}

PerspectivesPanel::~PerspectivesPanel()
{
	//(*Destroy(PerspectivesPanel)
	//*)
}


void PerspectivesPanel::OnButtonAddPerspectiveClick(wxCommandEvent& event)
{
    wxString name = wxGetTextFromUser("Enter name of perspective","Perspective Name");
    if(name.size()>0)
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
    wxCommandEvent eventLoadPerspective(EVT_LOAD_PERSPECTIVE);
    wxXmlNode* p = (wxXmlNode*)(ListBoxPerspectives->GetClientData(ListBoxPerspectives->GetSelection()));

    eventLoadPerspective.SetClientData(p);
    wxPostEvent(GetParent(), eventLoadPerspective);
}

void PerspectivesPanel::OnButtonRenamePerspectiveClick(wxCommandEvent& event)
{
    wxString name = wxGetTextFromUser("Enter new name for perspective","Rename Perspective ");
    if(name.size()>0)
    {
        wxXmlNode* p = (wxXmlNode*)(ListBoxPerspectives->GetClientData(ListBoxPerspectives->GetSelection()));
        p->DeleteAttribute("name");
        p->AddAttribute("name", name);
        ListBoxPerspectives->SetString(ListBoxPerspectives->GetSelection(),name);

        wxCommandEvent eventPerspectivesChanged(EVT_PERSPECTIVES_CHANGED);
        wxPostEvent(GetParent(), eventPerspectivesChanged);

    }
}
