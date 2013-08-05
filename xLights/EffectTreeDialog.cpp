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
	btAddGroup = new wxButton(this, ID_BUTTON7, _("Add Group"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	BoxSizer1->Add(btAddGroup, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtAddGroupClick);
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
    bool foundFavs = false;
    bool foundUser = false;


    treeFavoritesGroupID = TreeCtrl1->AppendItem(treeRootID, "Favorites", -1,-1, NULL);
    treeUserGroupID = TreeCtrl1->AppendItem(treeRootID, "User Group", -1,-1, NULL);
    TreeCtrl1->Expand(treeRootID);

    for(wxXmlNode *ele = EffectsNode->GetChildren(); ele!=NULL; ele=ele->GetNext() )
    {
        if (ele->GetName() == wxT("effect"))
        {
            //This case should only be for old format rgbeffects files
            name=ele->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                TreeCtrl1->AppendItem(treeUserGroupID, name,-1,-1, new MyTreeItemData(ele));
            }
        }
        else if (ele->GetName() == wxT("effectGroup"))
        {
            name=ele->GetAttribute(wxT("name"));
            if (name == wxT("Favorites"))
            {
                AddTreeElementsRecursive(ele, treeFavoritesGroupID);
                foundFavs=true;
            }
            else
            {
                AddTreeElementsRecursive(ele, treeUserGroupID);
                foundUser=true;
            }
        }
    }
    if (!foundFavs)
    {
        wxString name = wxT("Favorites");
        wxXmlNode *newNode = CreateEffectGroupNode(name);
        TreeCtrl1->SetItemData(treeFavoritesGroupID,new MyTreeItemData(newNode));
        XrgbEffectsNode->AddChild(newNode);
    }
    if (!foundUser)
    {
        wxString name = wxT("User Group");
        wxXmlNode *newNode = CreateEffectGroupNode(name);
        TreeCtrl1->SetItemData(treeUserGroupID,new MyTreeItemData(newNode));
        FixupEffectsPresets(newNode);
        XrgbEffectsNode->AddChild(newNode);
    }
    AddNCcomEffects();
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
    if (root->GetName() != wxT("NutcrackerEffects"))
    {
        wxMessageBox(_("Invalid RGB effects file. Please redownload."), _("Error"));
        return;
    }
    wxXmlNode* e=root->GetChildren();
    if (e->GetName() == wxT("effects")) NcEffectsNode=e;

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
        if (ele->GetName() == wxT("effect"))
        {
            name=ele->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                TreeCtrl1->AppendItem(curGroupID, name,-1,-1, new MyTreeItemData(ele));
            }
        }
        else if (ele->GetName() == wxT("effectGroup"))
        {
            name=ele->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                nextGroupID = TreeCtrl1->AppendItem(curGroupID, ele->GetName(),-1,-1,new MyTreeItemData (ele));
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

bool EffectTreeDialog::CheckValidOperation(wxTreeItemId itemID)
{
    wxTreeItemId parentID = TreeCtrl1->GetItemParent(itemID);

    if (itemID == treeNCcomGroupID || itemID == treeRootID )
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
    wxXmlNode* NewXml=new wxXmlNode(wxXML_ELEMENT_NODE, wxT("effectGroup"));
    NewXml->AddAttribute(wxT("name"), name);
    return NewXml;
}

void EffectTreeDialog::OnbtNewPresetClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxTreeItemId parentID;
    if ( !CheckValidOperation(itemID))
    {
        wxMessageBox(_("A preset cannot be added at the currently selected location"), _("ERROR"));
        return;
    }
    wxTextEntryDialog dialog(this,_("Enter preset name"),_("Add New Preset"));
    int DlgResult;
    bool ok;
    wxString name;
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            name=dialog.GetValue();
            name.Trim();
            if (name.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("A preset name cannot be empty"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;

    // update Choice_Presets
    parentID = TreeCtrl1->GetItemParent(itemID);
    MyTreeItemData *parentData=(MyTreeItemData *)TreeCtrl1->GetItemData(parentID);
    wxXmlNode *node=parentData->GetElement();
    wxXmlNode *newNode=((xLightsFrame *)xLightParent)->CreateEffectNode(name);
    node->AddChild(newNode);
    TreeCtrl1->AppendItem(parentID, name, -1,-1, new MyTreeItemData(newNode));
    //Choice_Presets->SetStringSelection(name);
    //SaveEffectsFile();
}

void EffectTreeDialog::OnbtUpdateClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxTreeItemId parentID;
    if ( !CheckValidOperation(itemID))
    {
        //Generate error message
    }
    parentID = TreeCtrl1->GetItemParent(itemID);
    MyTreeItemData *parentData=(MyTreeItemData *)TreeCtrl1->GetItemData(parentID);
    wxXmlNode *pnode=parentData->GetElement();

    MyTreeItemData *selData = (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    wxXmlNode *oldXml=selData->GetElement();
    pnode->RemoveChild(oldXml);
    delete oldXml;
    delete selData;

    wxString name(TreeCtrl1->GetItemText(itemID));
    wxXmlNode *newNode=((xLightsFrame *)xLightParent)->CreateEffectNode(name);
    pnode->AddChild(newNode);
    TreeCtrl1->AppendItem(parentID, name, -1,-1, new MyTreeItemData(newNode));
}

void EffectTreeDialog::OnbtFavoriteClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if ( TreeCtrl1->HasChildren(itemID))
    {
        //Generate error message
    }
    wxXmlNode *copyNode = ((MyTreeItemData*)(TreeCtrl1->GetItemData(itemID)))->GetElement();
    wxXmlNode *newNode= new wxXmlNode(*copyNode);
    wxXmlNode *favs = ((MyTreeItemData*)(TreeCtrl1->GetItemData(treeFavoritesGroupID)))->GetElement();
    TreeCtrl1->AppendItem(treeFavoritesGroupID, TreeCtrl1->GetItemText(itemID),-1,-1,
                          new MyTreeItemData (newNode));
    favs->AddChild(newNode);


}

void EffectTreeDialog::OnbtRenameClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if ( itemID == treeFavoritesGroupID || itemID == treeUserGroupID ||!CheckValidOperation(itemID))
    {
        //Generate error message
    }
    wxTextEntryDialog dialog(this,_("Enter new name"),_("Rename Preset"),TreeCtrl1->GetItemText(itemID));
    int DlgResult;
    bool ok;
    wxString NewName;
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            NewName=dialog.GetValue();
            NewName.Trim();
            if (NewName.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("A preset name cannot be empty"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;
    MyTreeItemData *itemData= (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    wxXmlNode* e=(wxXmlNode*)itemData->GetElement();
    e->DeleteAttribute(wxT("name"));
    e->AddAttribute(wxT("name"),NewName);
    delete itemData;
    TreeCtrl1->SetItemData(itemID, new MyTreeItemData(e));
    TreeCtrl1->SetItemText(itemID, NewName);
}

void EffectTreeDialog::OnbtDeleteClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if ( !CheckValidOperation(itemID))
    {
        //Generate error message
    }
}

void EffectTreeDialog::OnbtAddGroupClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if ( !CheckValidOperation(itemID))
    {
        //Generate error message
    }
}
