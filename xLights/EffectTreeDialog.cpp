#include "EffectTreeDialog.h"
#include "xLightsMain.h"

//(*InternalHeaders(EffectTreeDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectTreeDialog)
const long EffectTreeDialog::ID_TREECTRL1 = wxNewId();
const long EffectTreeDialog::ID_BUTTON6 = wxNewId();
const long EffectTreeDialog::ID_BUTTON1 = wxNewId();
const long EffectTreeDialog::ID_BUTTON2 = wxNewId();
const long EffectTreeDialog::ID_BUTTON7 = wxNewId();
const long EffectTreeDialog::ID_BUTTON3 = wxNewId();
const long EffectTreeDialog::ID_BUTTON4 = wxNewId();
const long EffectTreeDialog::ID_BUTTON8 = wxNewId();
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

	Create(parent, id, _("Effect Presets"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TreeCtrl1 = new wxTreeCtrl(this, ID_TREECTRL1, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	TreeCtrl1->SetMinSize(wxDLG_UNIT(this,wxSize(80,-1)));
	FlexGridSizer2->Add(TreeCtrl1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	btAddGroup = new wxButton(this, ID_BUTTON7, _("Add &Group"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	btAddGroup->SetToolTip(_("Add effect preset group."));
	BoxSizer1->Add(btAddGroup, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btRename = new wxButton(this, ID_BUTTON3, _("&Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	btRename->SetToolTip(_("Rename currently selected effect preset."));
	BoxSizer1->Add(btRename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btDelete = new wxButton(this, ID_BUTTON4, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	btDelete->SetToolTip(_("Delete curently selected effect preset."));
	BoxSizer1->Add(btDelete, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	btImport = new wxButton(this, ID_BUTTON8, _("&Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	btImport->SetToolTip(_("Import presets from another file."));
	BoxSizer1->Add(btImport, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_BEGIN_DRAG,(wxObjectEventFunction)&EffectTreeDialog::OnTreeCtrl1BeginDrag);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_END_DRAG,(wxObjectEventFunction)&EffectTreeDialog::OnTreeCtrl1EndDrag);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_ACTIVATED,(wxObjectEventFunction)&EffectTreeDialog::OnTreeCtrl1ItemActivated);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtApplyClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtNewPresetClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtUpdateClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtAddGroupClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtRenameClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtDeleteClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTreeDialog::OnbtImportClick);
	//*)
	treeRootID = TreeCtrl1->AddRoot("Effect Presets");
    xLightParent = (xLightsFrame*)parent;
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

    TreeCtrl1->SetItemData( treeRootID, new MyTreeItemData(EffectsNode, true));

    for(wxXmlNode *ele = EffectsNode->GetChildren(); ele!=NULL; ele=ele->GetNext() )
    {
        if (ele->GetName() == "effectGroup")
        {
            name = ele->GetAttribute("name");
            curGroupID = TreeCtrl1->AppendItem(treeRootID, name,-1,-1,new MyTreeItemData (ele, true));
            TreeCtrl1->SetItemHasChildren(curGroupID);
            AddTreeElementsRecursive(ele, curGroupID);
        }
    }

	TreeCtrl1->Expand(treeRootID);

    SaveEffectsFile();
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
            wxString settings;
            settings=ele->GetAttribute("settings");
            if (!settings.IsEmpty())
            {
                xLightParent->ApplyEffectsPreset(settings);
            }
        }
    }
}

void EffectTreeDialog::OnbtApplyClick(wxCommandEvent& event)
{
    ApplyEffect();
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
            if (name->IsEmpty() && !errorMsg.IsEmpty()) // !errorMsg => empty is allowed -DJ
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

    if ( !itemID.IsOk() )
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
    wxXmlNode *newNode=xLightParent->CreateEffectNode(name);
    node->AddChild(newNode);
    TreeCtrl1->AppendItem(parentID, name, -1,-1, new MyTreeItemData(newNode));

    SaveEffectsFile();
}

void EffectTreeDialog::OnbtUpdateClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxString name(TreeCtrl1->GetItemText(itemID));

    if ( TreeCtrl1->HasChildren(itemID))
    {
        wxMessageBox(_("You cannot store an effect on the selected item."), _("ERROR"));
        return;
    }
    MyTreeItemData *selData = (MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    wxXmlNode *xml_node=selData->GetElement();

    xml_node->DeleteAttribute("settings");
    xLightParent->UpdateEffectNode(xml_node);

    SaveEffectsFile();
}

void EffectTreeDialog::OnbtRenameClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    if (!itemID.IsOk())
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
    e->DeleteAttribute("name");
    e->AddAttribute("name",newName);
    TreeCtrl1->SetItemText(itemID, newName);
    SaveEffectsFile();
}

void EffectTreeDialog::OnbtDeleteClick(wxCommandEvent& event)
{
    wxTreeItemId itemID = TreeCtrl1->GetSelection();
    wxTreeItemId parentID;

    if( !itemID.IsOk() || itemID == treeRootID )
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
    wxString prompt = "Enter effect group name";
    wxString errMsg = "Effect group name may not be empty";
    wxString name;
    if (!PromptForName(this, &name, prompt, errMsg)) return;

    // update Choice_Presets
    MyTreeItemData *itemData=(MyTreeItemData *)TreeCtrl1->GetItemData(itemID);
    if( itemData->IsGroup() )
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
    xLightParent->SaveEffectsFile();
}

void EffectTreeDialog::OnButton_OKClick(wxCommandEvent& event)
{
    Show(false);
}

void EffectTreeDialog::OnTreeCtrl1BeginDrag(wxTreeEvent& event)
{
    wxTreeItemId itemID = event.GetItem();
    if( !TreeCtrl1->ItemHasChildren(itemID) )
    {
        m_draggedItem = event.GetItem();
        event.Allow();
    }
}

void EffectTreeDialog::OnTreeCtrl1EndDrag(wxTreeEvent& event)
{
    wxTreeItemId itemSrc = m_draggedItem,
                 itemDst = event.GetItem();
    m_draggedItem = (wxTreeItemId)0l;

    // where to copy the item?
    if ( itemDst.IsOk() && !TreeCtrl1->ItemHasChildren(itemDst) )
    {
        // copy to the parent then
        itemDst = TreeCtrl1->GetItemParent(itemDst);
    }

    if ( !itemDst.IsOk() )
    {
        return;
    }

    wxString name = TreeCtrl1->GetItemText(itemSrc);
    wxTreeItemId parentID = TreeCtrl1->GetItemParent(itemSrc);
    MyTreeItemData *parentData = (MyTreeItemData *)TreeCtrl1->GetItemData(parentID);
    wxXmlNode *pnode = parentData->GetElement();

    MyTreeItemData *selData = (MyTreeItemData *)TreeCtrl1->GetItemData(itemSrc);
    wxXmlNode *oldXml=selData->GetElement();

    MyTreeItemData *dstParentData = (MyTreeItemData *)TreeCtrl1->GetItemData(itemDst);
    wxXmlNode *dst_pnode = dstParentData->GetElement();

    pnode->RemoveChild(oldXml);
    dst_pnode->AddChild(oldXml);

    TreeCtrl1->Delete(itemSrc);

    wxTreeItemId itemID = TreeCtrl1->AppendItem(itemDst, name, -1,-1, new MyTreeItemData(oldXml));
    TreeCtrl1->SelectItem(itemID);

    SaveEffectsFile();
}

void EffectTreeDialog::AddImportedItemsRecursively(wxXmlNode* effects_node, wxTreeItemId curGroupID)
{
    wxString name, settings, version;
    wxTreeItemId nextGroupID;
    MyTreeItemData *parentData;

    for(wxXmlNode *ele = effects_node->GetChildren(); ele!=NULL; ele=ele->GetNext() )
    {
        if (ele->GetName() == "effect")
        {
            name=ele->GetAttribute("name");
            settings=ele->GetAttribute("settings");
            version=ele->GetAttribute("version", "0000");
            if (!name.IsEmpty())
            {
                parentData = (MyTreeItemData *)TreeCtrl1->GetItemData(curGroupID);
                wxXmlNode *node = parentData->GetElement();
                wxXmlNode *newNode = new wxXmlNode(wxXML_ELEMENT_NODE, "effect");
                newNode->AddAttribute("name", name);
                newNode->AddAttribute("settings", settings);
                newNode->AddAttribute("version", version);

                node->AddChild(newNode);
                TreeCtrl1->AppendItem(curGroupID, name, -1,-1, new MyTreeItemData(newNode));
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

void EffectTreeDialog::OnbtImportClick(wxCommandEvent& event)
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, "Choose file to Import", wxEmptyString, wxEmptyString, "XML files (*.xml)|*.xml", wxFD_OPEN, wxDefaultPosition);
    OpenDialog->SetDirectory(xLightParent->CurrentDir);
    if (OpenDialog->ShowModal() == wxID_OK)
    {
        wxString fDir = OpenDialog->GetDirectory();
        wxString filename = OpenDialog->GetFilename();
        wxFileName name_and_path(filename);
        name_and_path.SetPath(fDir);
        wxString file_to_import = name_and_path.GetFullPath();

        wxXmlDocument input_xml;
        if( !input_xml.Load(file_to_import) )  return;

        wxXmlNode* input_root=input_xml.GetRoot();

        for(wxXmlNode* e=input_root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            if (e->GetName() == "effects")
            {
                for(wxXmlNode *ele = e->GetChildren(); ele!=NULL; ele=ele->GetNext() )
                {
                    if (ele->GetName() == "effectGroup")
                    {
                        wxString name = ele->GetAttribute("name");
                        wxTreeItemIdValue cookie;
                        wxTreeItemId root = TreeCtrl1->GetRootItem();
                        bool group_exists = false;
                        for(wxTreeItemId branch = TreeCtrl1->GetFirstChild(root, cookie); branch.IsOk(); branch = TreeCtrl1->GetNextChild(root, cookie) )
                        {
                            if( TreeCtrl1->GetItemText(branch) == name )
                            {
                                AddImportedItemsRecursively(ele, branch);
                                group_exists = true;
                                break;
                            }
                        }
                        if( !group_exists )
                        {
                            AddImportedItemsRecursively(ele, treeRootID);
                        }
                    }
                }
            }
        }
        SaveEffectsFile();
    }
}
