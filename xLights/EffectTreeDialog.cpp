#include "EffectTreeDialog.h"

//(*InternalHeaders(EffectTreeDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectTreeDialog)
const long EffectTreeDialog::ID_TREECTRL1 = wxNewId();
const long EffectTreeDialog::ID_BUTTON6 = wxNewId();
const long EffectTreeDialog::ID_BUTTON1 = wxNewId();
const long EffectTreeDialog::ID_BUTTON2 = wxNewId();
const long EffectTreeDialog::ID_BUTTON5 = wxNewId();
const long EffectTreeDialog::ID_BUTTON3 = wxNewId();
const long EffectTreeDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectTreeDialog,wxDialog)
	//(*EventTable(EffectTreeDialog)
	//*)
END_EVENT_TABLE()

EffectTreeDialog::EffectTreeDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EffectTreeDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	TreeCtrl1 = new wxTreeCtrl(this, ID_TREECTRL1, wxDefaultPosition, wxSize(200,300), wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	FlexGridSizer2->Add(TreeCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	btApply = new wxButton(this, ID_BUTTON6, _("Apply Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	BoxSizer1->Add(btApply, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btNewPreset = new wxButton(this, ID_BUTTON1, _("New Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer1->Add(btNewPreset, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btUpdate = new wxButton(this, ID_BUTTON2, _("Update Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(btUpdate, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btFavorite = new wxButton(this, ID_BUTTON5, _("Add To Favorites"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	BoxSizer1->Add(btFavorite, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btRename = new wxButton(this, ID_BUTTON3, _("Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	BoxSizer1->Add(btRename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btDelete = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	BoxSizer1->Add(btDelete, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtApplyClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtNewPresetClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtUpdateClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtFavoriteClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtRenameClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtDeleteClick);
	//*)
	treeRootID = TreeCtrl1->AddRoot(wxT("Effect Presets"));
    xLightParent = (xLightsFrame *)parent;
}

EffectTreeDialog::~EffectTreeDialog()
{
	//(*Destroy(EffectTreeDialog)
	//*)
}

void EffectTreeDialog::InitItems(wxXmlNode *EffectsNode)
{
    wxString name;
    wxTreeItemId curGroupID;
    XrgbEffectsNode = EffectsNode;

    treeFavoritesGroupID = TreeCtrl1->AppendItem(treeRootID, "Favorites", -1,-1, NULL);
    treeUserGroupID = TreeCtrl1->AppendItem(treeRootID, "User Group", -1,-1, NULL);
    TreeCtrl1->Expand(treeRootID);

    for(; EffectsNode!=NULL; EffectsNode=EffectsNode->GetNext() )
    {
        if (EffectsNode->GetName() == wxT("effect"))
        {
            //This case should only be for old format rgbeffects files
            name=EffectsNode->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                TreeCtrl1->AppendItem(treeUserGroupID, name,-1,-1, new MyTreeItemData(EffectsNode));
            }
        }
        else if (EffectsNode->GetName() == wxT("effectGroup"))
        {
            name=EffectsNode->GetAttribute(wxT("name"));
            if (name == wxT("Favorites"))
            {
                AddTreeElementsRecursive(EffectsNode, treeFavoritesGroupID);
            }
            else
            {
                AddTreeElementsRecursive(EffectsNode, treeUserGroupID);
            }
        }
    }
    AddNCcomEffects();
}

void EffectTreeDialog::AddNCcomEffects()
{
    wxFileName effectsFile;
    effectsFile.AssignDir( ((xLightsFrame *)xLightParent)->CurrentDir );
    effectsFile.SetFullName(NCCOM_FILE);

    if (!EffectsXml.Load( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to load RGB effects file"), _("Error"));
        return;
    }
    wxXmlNode* root=EffectsXml.GetRoot();
    if (root->GetName() != wxT("NutcrackerEffects"))
    {
        wxMessageBox(_("Invalid RGB effects file. Please redownload."), _("Error"));
        return;
    }
    wxXmlNode* e=root->GetChildren();
    if (e->GetName() == wxT("effects")) NcEffectsNode=e;

    if (e->GetNext() != NULL)
    {
        wxMessageBox(_("Only top level effect group allowed in nutcracker effect file."), _("Error"));
    }

    if (NcEffectsNode == 0)
    {
        wxMessageBox(_("No effects found in Nutcracker.com effects file"), _("Error"));
        return;
    }
    UpdateNcEffectsList();
}


void EffectTreeDialog::AddTreeElementsRecursive(wxXmlNode *EffectsNode, wxTreeItemId curGroupID)
{
    wxString name;
    wxTreeItemId nextGroupID;

    for(wxXmlNode *EffectsNode = NcEffectsNode->GetChildren(); EffectsNode!=NULL; EffectsNode=EffectsNode->GetNext() )
    {
        if (EffectsNode->GetName() == wxT("effect"))
        {
            name=EffectsNode->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                TreeCtrl1->AppendItem(curGroupID, name,-1,-1, new MyTreeItemData(EffectsNode));
            }
        }
        else if (EffectsNode->GetName() == wxT("effectGroup"))
        {
            name=EffectsNode->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                nextGroupID = TreeCtrl1->AppendItem(curGroupID, EffectsNode->GetName(),-1,-1,new MyTreeItemData (EffectsNode));
                AddTreeElementsRecursive(EffectsNode, nextGroupID);
            }
        }
    }
}

void EffectTreeDialog::UpdateNcEffectsList()
{
    wxString name;
    wxTreeItemId curGroupID;

    treeNCcomGroupID = TreeCtrl1->AppendItem(treeRootID, "Nutcraker Shared Effects", -1,-1, NULL);
    AddTreeElementsRecursive(NcEffectsNode, treeNCcomGroupID);
}


void EffectTreeDialog::OnbtApplyClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();

    if (!itemID.IsOk())
    {
        //throw erroe dialog nothign selected
    }
    else if (TreeCtrl1->HasChildren(itemID))
    {
        //Throw error dialog not an effect
    }
    else
    {
        MyTreeItemData *item = (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
        wxXmlNode *ele;
        if ( item != NULL )
        {
            ele = item->GetElement();
            ((xLightsFrame *)xLightParent)->SetEffectControls(ele->GetAttribute("settings"));

        }
    }
}

void EffectTreeDialog::CheckValidOperation()
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();

}

void EffectTreeDialog::OnbtNewPresetClick(wxCommandEvent& event)
{
    CheckValidOperation();
}

void EffectTreeDialog::OnbtUpdateClick(wxCommandEvent& event)
{
}

void EffectTreeDialog::OnbtFavoriteClick(wxCommandEvent& event)
{
}

void EffectTreeDialog::OnbtRenameClick(wxCommandEvent& event)
{
}

void EffectTreeDialog::OnbtDeleteClick(wxCommandEvent& event)
{
}
