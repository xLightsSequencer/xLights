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
const long EffectTreeDialog::ID_BUTTON7 = wxNewId();
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
	TreeCtrl1 = new wxTreeCtrl(this, ID_TREECTRL1, wxDefaultPosition, wxSize(200,300), wxTR_HIDE_ROOT|wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	FlexGridSizer2->Add(TreeCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	btApply = new wxButton(this, ID_BUTTON6, _("&Apply Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	btApply->SetToolTip(_("Apply the selected effect Preset."));
	BoxSizer1->Add(btApply, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btNewPreset = new wxButton(this, ID_BUTTON1, _("&New Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	btNewPreset->SetToolTip(_("Create New Effect Preset from current settings."));
	BoxSizer1->Add(btNewPreset, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btUpdate = new wxButton(this, ID_BUTTON2, _("&Update Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	btUpdate->SetToolTip(_("Update the selected effect preset to reflect current effect settings."));
	BoxSizer1->Add(btUpdate, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btFavorite = new wxButton(this, ID_BUTTON5, _("Add To &Favorites"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	btFavorite->SetToolTip(_("Copy effect into favorites group."));
	BoxSizer1->Add(btFavorite, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btAddGroup = new wxButton(this, ID_BUTTON7, _("Add &Group"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	btAddGroup->SetToolTip(_("Add effect preset group."));
	BoxSizer1->Add(btAddGroup, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btRename = new wxButton(this, ID_BUTTON3, _("&Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	btRename->SetToolTip(_("Rename currently selected effect preset."));
	BoxSizer1->Add(btRename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btDelete = new wxButton(this, ID_BUTTON4, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	btDelete->SetToolTip(_("Delete curently selected effect preset."));
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

	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_ACTIVATED,(wxObjectEventFunction)&EffectTreeDialog::OnTreeCtrl1ItemActivated);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtApplyClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtNewPresetClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtUpdateClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtFavoriteClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtAddGroupClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtRenameClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtDeleteClick);
	//*)
	treeRootID = TreeCtrl1->AddRoot("Effect Presets");
    xLightParent = (xLightsFrame *)parent;

    treeFavoritesGroupID = NULL;
    treeUserGroupID = NULL;
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
    bool fixup = false;

    for(wxXmlNode *ele = EffectsNode->GetChildren(); ele!=NULL; ele=ele->GetNext() )
    {
        if (ele->GetName() == "effect")
        {
            if( NULL == treeUserGroupID)
            {
                fixup = true;
                treeUserGroupID = TreeCtrl1->AppendItem(treeRootID, name,-1,-1,NULL);
                TreeCtrl1->SetItemHasChildren(treeUserGroupID);
            }
            //This case should only be for old format rgbeffects files
            name=ele->GetAttribute("name");
            if (!name.IsEmpty())
            {
                TreeCtrl1->AppendItem(treeUserGroupID, name,-1,-1, new MyTreeItemData(ele));
            }
        }
        else if (ele->GetName() == "effectGroup")
        {
            name=ele->GetAttribute("name");
            if (name == "Favorites")
            {
                treeFavoritesGroupID = TreeCtrl1->AppendItem(treeRootID, name,-1,-1,new MyTreeItemData (ele, true));
                TreeCtrl1->SetItemHasChildren(treeFavoritesGroupID);
                AddTreeElementsRecursive(ele, treeFavoritesGroupID);
            }
            else if (name == "User Group")
            {
                treeUserGroupID = TreeCtrl1->AppendItem(treeRootID, name,-1,-1,new MyTreeItemData (ele, true));
                TreeCtrl1->SetItemHasChildren(treeUserGroupID);
                AddTreeElementsRecursive(ele, treeUserGroupID);
            }
            else
            {
                //error we should not have more than two core groups in the root of rgb_effects.xml
            }
        }
    }
    if (NULL == treeFavoritesGroupID)
    {
        wxString name = "Favorites";
        wxXmlNode *newNode = CreateEffectGroupNode(name);
        treeFavoritesGroupID = TreeCtrl1->AppendItem(treeRootID, name,-1,-1,new MyTreeItemData(newNode, true));
        TreeCtrl1->SetItemHasChildren(treeFavoritesGroupID);
        XrgbEffectsNode->AddChild(newNode);
    }
    if (fixup)
    {
        wxString name = "User Group";
        wxXmlNode *newNode = CreateEffectGroupNode(name);
        TreeCtrl1->SetItemData(treeUserGroupID,new MyTreeItemData(newNode, true));
        TreeCtrl1->SetItemText(treeUserGroupID, name);
        FixupEffectsPresets(newNode);
        XrgbEffectsNode->AddChild(newNode);
    }
    /* Enable this function when we want to include a set of effects as known 'cool' effects*/
    //AddNCcomEffects();

    SaveEffectsFile();
}

void EffectTreeDialog::FixupEffectsPresets(wxXmlNode *UserGroupNode)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId curItemID = TreeCtrl1->GetFirstChild(treeUserGroupID, cookie);
    wxXmlNode *ele, p;
    MyTreeItemData *treeData;
    while (curItemID.IsOk())
    {
        treeData = (MyTreeItemData*)TreeCtrl1->GetItemData(curItemID);
        ele=treeData->GetElement();
        wxXmlNode* p=ele->GetParent();
        if (p) p->RemoveChild(ele);
        UserGroupNode->AddChild(ele);
        curItemID = TreeCtrl1->GetNextChild(treeUserGroupID, cookie);
    }
}

void EffectTreeDialog::AddNCcomEffects()
{
    wxFileName effectsFile;
    effectsFile.AssignDir( ((xLightsFrame *)xLightParent)->CurrentDir );
    effectsFile.SetFullName(NCCOM_FILE);

    if (!NcEffectsXml.Load( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to load RGB effects file"), _("Error"));
        return;
    }
    wxXmlNode* root=NcEffectsXml.GetRoot();
    if (root->GetName() != "NutcrackerEffects")
    {
        wxMessageBox(_("Invalid RGB effects file. Please redownload."), _("Error"));
        return;
    }
    wxXmlNode* e=root->GetChildren();
    if (e->GetName() == "effects") NcEffectsNode=e;

    if (e->GetNext() != NULL)
    {
        wxMessageBox(_("Only one top level effect group allowed in nutcracker effect file."), _("Error"));
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

    for(wxXmlNode *ele = EffectsNode->GetChildren(); ele!=NULL; ele=ele->GetNext() )
    {
        if (ele->GetName() == "effect")
        {
            name=ele->GetAttribute("name");
            if (!name.IsEmpty())
            {
                TreeCtrl1->AppendItem(curGroupID, name,-1,-1, new MyTreeItemData(ele));
            }
        }
        else if (ele->GetName() == "effectGroup")
        {
            name=ele->GetAttribute("name");
            if (!name.IsEmpty())
            {
                nextGroupID = TreeCtrl1->AppendItem(curGroupID, name,-1,-1,new MyTreeItemData (ele, true));
                TreeCtrl1->SetItemHasChildren(nextGroupID);
                AddTreeElementsRecursive(ele, nextGroupID);
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

void EffectTreeDialog::ApplyEffect(bool dblClick)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if (!itemID.IsOk())
    {
         wxMessageBox(_("No effect selected."), _("ERROR"));
    }
    else if (TreeCtrl1->HasChildren(itemID))
    {
        if (dblClick)
        {
            TreeCtrl1->Toggle(itemID);
        }
        else
        {
            wxMessageBox(_("An effect group can not be applied."), _("ERROR"));
        }
    }
    else
    {
        MyTreeItemData *item = (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
        wxXmlNode *ele;
        if ( item != NULL )
        {
            ele = item->GetElement();
            wxString empty_str; //= wxEmptyString; //kludge: avoid compile error below
            ((xLightsFrame *)xLightParent)->SetEffectControls(ele->GetAttribute("settings"), empty_str);
        }
    }
}

void EffectTreeDialog::OnbtApplyClick(wxCommandEvent& event)
{
    ApplyEffect();
}

bool EffectTreeDialog::CheckValidOperation(wxTreeItemId itemID)
{
    wxTreeItemId parentID = TreeCtrl1->GetItemParent(itemID);

    if (itemID == treeNCcomGroupID || itemID == treeRootID || itemID == treeFavoritesGroupID || itemID == treeUserGroupID)
    {
        return false;
    }
    else if (parentID == treeFavoritesGroupID || parentID == treeUserGroupID )
    {
        return true;
    }
    return CheckValidOperation(parentID);
}

wxXmlNode* EffectTreeDialog::CreateEffectGroupNode(wxString& name)
{
    wxXmlNode* NewXml=new wxXmlNode(wxXML_ELEMENT_NODE, "effectGroup");
    NewXml->AddAttribute("name", name);
    return NewXml;
}

bool EffectTreeDialog::PromptForName(wxWindow* parent, wxString *name, wxString prompt, wxString errorMsg)
{
    wxTextEntryDialog dialog(/*this*/ parent,prompt,_("Name"));
    int DlgResult;
    bool ok;
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            *name=dialog.GetValue();
            name->Trim();
            if (name->IsEmpty())
            {
                ok=false;
                wxMessageBox(errorMsg, _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    return (DlgResult == wxID_OK );
}
void EffectTreeDialog::OnbtNewPresetClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxTreeItemId parentID;
    MyTreeItemData *parentData, *itemData;

    if ( itemID != treeFavoritesGroupID && itemID != treeUserGroupID && !CheckValidOperation(itemID))
    {
        wxMessageBox(_("A preset cannot be added at the currently selected location"), _("ERROR"));
        return;
    }
    wxString prompt = "Enter effect preset name";
    wxString errMsg = "Effect preset name may not be empty";
    wxString name;
    if (!PromptForName(this, &name, prompt, errMsg)) return;

    itemData = (MyTreeItemData *) TreeCtrl1->GetItemData(itemID);
    if( itemData->IsGroup())
    {
        parentID=itemID;
        parentData=itemData;
    }
    else
    {
        parentID = TreeCtrl1->GetItemParent(itemID);
        parentData=(MyTreeItemData *)TreeCtrl1->GetItemData(parentID);
    }

    wxXmlNode *node=parentData->GetElement();
    wxXmlNode *newNode=((xLightsFrame *)xLightParent)->CreateEffectNode(name);
    node->AddChild(newNode);
    TreeCtrl1->AppendItem(parentID, name, -1,-1, new MyTreeItemData(newNode));
    SaveEffectsFile();
}

void EffectTreeDialog::OnbtUpdateClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxTreeItemId parentID;
    wxString name(TreeCtrl1->GetItemText(itemID));

    if ( !CheckValidOperation(itemID) || TreeCtrl1->HasChildren(itemID))
    {
        wxMessageBox(_("You cannot store an effect on the selected item."), _("ERROR"));
        return;
    }
    parentID = TreeCtrl1->GetItemParent(itemID);
    MyTreeItemData *parentData=(MyTreeItemData *)TreeCtrl1->GetItemData(parentID);
    wxXmlNode *pnode=parentData->GetElement();

    MyTreeItemData *selData = (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    wxXmlNode *oldXml=selData->GetElement();

    pnode->RemoveChild(oldXml);
    delete oldXml;
    TreeCtrl1->Delete(itemID);

    wxXmlNode *newNode=((xLightsFrame *)xLightParent)->CreateEffectNode(name);
    pnode->AddChild(newNode);
    itemID = TreeCtrl1->AppendItem(parentID, name, -1,-1, new MyTreeItemData(newNode));
    TreeCtrl1->SelectItem(itemID);
    SaveEffectsFile();
}

void EffectTreeDialog::OnbtFavoriteClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if ( TreeCtrl1->HasChildren(itemID))
    {
        wxMessageBox(_("You cannot make a group of effects part of the favorites group."), _("ERROR"));
        return;
    }
    wxXmlNode *copyNode = ((MyTreeItemData*)(TreeCtrl1->GetItemData(itemID)))->GetElement();
    wxXmlNode *newNode= new wxXmlNode(*copyNode);
    wxXmlNode *favs = ((MyTreeItemData*)(TreeCtrl1->GetItemData(treeFavoritesGroupID)))->GetElement();
    TreeCtrl1->AppendItem(treeFavoritesGroupID, TreeCtrl1->GetItemText(itemID),-1,-1,
                          new MyTreeItemData (newNode));
    favs->AddChild(newNode);
    SaveEffectsFile();
}

void EffectTreeDialog::OnbtRenameClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if ( !CheckValidOperation(itemID))
    {
        wxMessageBox(_("You Cannot rename this item"), _("ERROR"));
        return;
    }

    wxString prompt = "Enter new effect preset name";
    wxString errMsg = "Effect preset name may not be empty";
    wxString newName;
    if (!PromptForName(this, &newName, prompt, errMsg)) return;

    MyTreeItemData *itemData= (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    wxXmlNode* e=(wxXmlNode*)itemData->GetElement();
    //isGroup=itemData->IsGroup();
    e->DeleteAttribute("name");
    e->AddAttribute("name",newName);
    //delete itemData;
    //TreeCtrl1->SetItemData(itemID, new MyTreeItemData(e));
    TreeCtrl1->SetItemText(itemID, newName);
    SaveEffectsFile();
}

void EffectTreeDialog::OnbtDeleteClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxTreeItemId parentID;

    if (!CheckValidOperation(itemID))
    {
        wxMessageBox(_("You cannot delete this item"), _("ERROR"));
        return;
    }
    parentID = TreeCtrl1->GetItemParent(itemID);
    MyTreeItemData *parentData=(MyTreeItemData *)TreeCtrl1->GetItemData(parentID);
    wxXmlNode *pnode=parentData->GetElement();

    MyTreeItemData *selData = (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    wxXmlNode *oldXml=selData->GetElement();

    pnode->RemoveChild(oldXml);
    delete oldXml;
    TreeCtrl1->Delete(itemID);
    SaveEffectsFile();
}

void EffectTreeDialog::OnbtAddGroupClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxTreeItemId parentID;
    MyTreeItemData *parentData;
    if ( !CheckValidOperation(itemID))
    {
        wxMessageBox(_("A group cannot be added at the currently selected location"), _("ERROR"));
        return;
    }
    wxString prompt = "Enter effect group name";
    wxString errMsg = "Effect group name may not be empty";
    wxString name;
    if (!PromptForName(this, &name, prompt, errMsg)) return;

    // update Choice_Presets
    MyTreeItemData *itemData=(MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    if( itemData->IsGroup())
    {
        parentID = itemID;
        parentData = itemData;
    }
    else
    {
        parentID = TreeCtrl1->GetItemParent(itemID);
        parentData=(MyTreeItemData *)TreeCtrl1->GetItemData(parentID);
    }

    wxXmlNode *node=parentData->GetElement();
    wxXmlNode *newNode=CreateEffectGroupNode(name);
    node->AddChild(newNode);
    itemID = TreeCtrl1->AppendItem(parentID, name, -1,-1, new MyTreeItemData(newNode, true));
    TreeCtrl1->SetItemHasChildren(itemID);
    SaveEffectsFile();
}

void EffectTreeDialog::OnTreeCtrl1ItemActivated(wxTreeEvent& event)
{
    ApplyEffect(true);
}

void EffectTreeDialog::SaveEffectsFile()
{
    ((xLightsFrame *)xLightParent)->SaveEffectsFile();
}

void EffectTreeDialog::OnButton_OKClick(wxCommandEvent& event)
{
    Show(false);
}
