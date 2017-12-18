#include "CurrentPreviewModels.h"
#include "PreviewModels.h"
#include <wx/xml/xml.h>

//(*InternalHeaders(CurrentPreviewModels)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include "models/ModelManager.h"


//(*IdInit(CurrentPreviewModels)
const long CurrentPreviewModels::ID_CHECKLISTBOX_CURRENT_GROUPS = wxNewId();
const long CurrentPreviewModels::ID_BUTTON2 = wxNewId();
const long CurrentPreviewModels::ID_BUTTON1 = wxNewId();
const long CurrentPreviewModels::ID_BUTTON3 = wxNewId();
const long CurrentPreviewModels::ID_BUTTON_EDIT_GROUPS = wxNewId();
//*)

BEGIN_EVENT_TABLE(CurrentPreviewModels,wxDialog)
	//(*EventTable(CurrentPreviewModels)
	//*)
END_EVENT_TABLE()

CurrentPreviewModels::CurrentPreviewModels(wxWindow* parent,wxXmlNode* ModelGroups, ModelManager &Models, wxWindowID id,const wxPoint& pos,const wxSize& size) : mModels(Models)
{
	//(*Initialize(CurrentPreviewModels)
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Model Groups"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	CheckListBoxCurrentGroups = new wxCheckListBox(this, ID_CHECKLISTBOX_CURRENT_GROUPS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SORT|wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX_CURRENT_GROUPS"));
	CheckListBoxCurrentGroups->SetMinSize(wxSize(-1,400));
	FlexGridSizer2->Add(CheckListBoxCurrentGroups, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	EditButton = new wxButton(this, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(EditButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	AddButton = new wxButton(this, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(AddButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RenameButton = new wxButton(this, ID_BUTTON3, _("Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer5->Add(RenameButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RemoveButton = new wxButton(this, ID_BUTTON_EDIT_GROUPS, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EDIT_GROUPS"));
	FlexGridSizer5->Add(RemoveButton, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKLISTBOX_CURRENT_GROUPS,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&CurrentPreviewModels::OnCheckListBoxCurrentGroupsToggled);
	Connect(ID_CHECKLISTBOX_CURRENT_GROUPS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&CurrentPreviewModels::OnCurrentGroupsSelect);
	Connect(ID_CHECKLISTBOX_CURRENT_GROUPS,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&CurrentPreviewModels::OnCurrentGroupsDClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurrentPreviewModels::OnEditButtonClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurrentPreviewModels::OnAddButtonClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurrentPreviewModels::OnRenameButtonClick);
	Connect(ID_BUTTON_EDIT_GROUPS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurrentPreviewModels::OnRemoveButtonClick);
	//*)
	mModelGroups = ModelGroups;

    PopulateModelGroups();
}


CurrentPreviewModels::~CurrentPreviewModels()
{
	//(*Destroy(CurrentPreviewModels)
	//*)
}


void CurrentPreviewModels::OnCheckListBoxCurrentGroupsToggled(wxCommandEvent& event)
{
    wxString sChecked;
    for(unsigned int i=0;i<CheckListBoxCurrentGroups->GetCount();i++)
    {
        wxXmlNode* e = (wxXmlNode*)CheckListBoxCurrentGroups->GetClientData(i);
        e->DeleteAttribute("selected");
        sChecked = CheckListBoxCurrentGroups->IsChecked(i)?"1":"0";
        e->AddAttribute("selected",sChecked);
    }
}

void CurrentPreviewModels::PopulateModelGroups()
{
    wxString name;
    wxXmlNode* e;
    CheckListBoxCurrentGroups->Clear();
	for(e=mModelGroups->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "modelGroup")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                int item_index = CheckListBoxCurrentGroups->Append(name,e);
                bool isChecked = e->GetAttribute("selected")=="1"?true:false;
                CheckListBoxCurrentGroups->Check(item_index,isChecked);
            }
        }
    }
    CheckListBoxCurrentGroups->SetSelection(-1);
    EditButton->Enable(false);
    RemoveButton->Enable(false);
    RenameButton->Enable(false);
}

void CurrentPreviewModels::OnAddButtonClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
    if (dlg.ShowModal() == wxID_OK) {
        wxString name = dlg.GetValue();
        while (mModels.GetModel(name.ToStdString()) != nullptr) {
            wxTextEntryDialog dlg2(this, "Model of name " + name + " already exists. Enter name for new group", "Enter name for new group");
            if (dlg2.ShowModal() == wxID_OK) {
                name = dlg2.GetValue();
            } else {
                return;
            }
        }
        wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
        mModelGroups->AddChild(node);
        node->AddAttribute("selected", "0");
        node->AddAttribute("name", name);
        node->AddAttribute("models", "");
        node->AddAttribute("layout", "minimalGrid");
        node->AddAttribute("GridSize", "400");

        mModels.AddModel(mModels.CreateModel(node));
        PopulateModelGroups();
    }
}

void CurrentPreviewModels::OnRemoveButtonClick(wxCommandEvent& event)
{
    wxString sel = CheckListBoxCurrentGroups->GetStringSelection();
    if (wxMessageBox("Are you sure you want to remove the " + sel + " group?", "Confirm Remove?", wxICON_QUESTION | wxYES_NO) == wxYES) {
        mModels.Delete(sel.ToStdString());
        PopulateModelGroups();
    }
}


void CurrentPreviewModels::OnEditButtonClick(wxCommandEvent& event)
{
    wxString sel = CheckListBoxCurrentGroups->GetStringSelection();
    PreviewModels dialog(this,sel.ToStdString(), mModels);
    if (dialog.ShowModal() == wxID_OK) {
        dialog.UpdateModelGroup();
    }
}

void CurrentPreviewModels::OnCurrentGroupsDClick(wxCommandEvent& event)
{
    wxString sel = CheckListBoxCurrentGroups->GetStringSelection();
    PreviewModels dialog(this,sel.ToStdString(), mModels);
    if (dialog.ShowModal() == wxID_OK) {
        dialog.UpdateModelGroup();
    }
}

void CurrentPreviewModels::OnCurrentGroupsSelect(wxCommandEvent& event)
{
    EditButton->Enable(true);
    RemoveButton->Enable(true);
    RenameButton->Enable(true);
}

void CurrentPreviewModels::OnRenameButtonClick(wxCommandEvent& event)
{
    wxString sel = CheckListBoxCurrentGroups->GetStringSelection();
    wxTextEntryDialog dlg(this, "Enter new name for group " + sel, "Rename " + sel, sel);
    while (dlg.ShowModal() == wxID_OK) {
        wxString name = dlg.GetValue();

        while (mModels.GetModel(name.ToStdString()) != nullptr) {
            wxTextEntryDialog dlg2(this, "Model or Group of name " + name + " already exists. Enter new name for group", "Enter new name for group");
            if (dlg2.ShowModal() == wxID_OK) {
                name = dlg2.GetValue();
            } else {
                return;
            }
        }

        mModels.Rename(sel.ToStdString(), name.ToStdString());
        PopulateModelGroups();
    }
}
