/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ImportPreviewsModelsDialog.h"

//(*InternalHeaders(ImportPreviewsModelsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/menu.h>
#include <wx/treebase.h>
#include <wx/xml/xml.h>

#include "LayoutGroup.h"

//(*IdInit(ImportPreviewsModelsDialog)
const long ImportPreviewsModelsDialog::ID_BUTTON1 = wxNewId();
const long ImportPreviewsModelsDialog::ID_BUTTON2 = wxNewId();
//*)

const long ImportPreviewsModelsDialog::ID_MNU_IPM_SELECTALL = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_DESELECTALL = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_SELECTSIBLINGS = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_DESELECTSIBLINGS = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_DESELECTEXISTING = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_SELECTALLMODELS = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_SELECTALLMODELSGROUPS = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_EXPANDALL = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_COLLAPSEALL = wxNewId();

BEGIN_EVENT_TABLE(ImportPreviewsModelsDialog,wxDialog)
	//(*EventTable(ImportPreviewsModelsDialog)
	//*)
END_EVENT_TABLE()

ImportPreviewsModelsDialog::ImportPreviewsModelsDialog(wxWindow* parent, const wxString& filename, ModelManager& allModels, std::vector<LayoutGroup*>& layoutGroups, wxWindowID id,const wxPoint& pos,const wxSize& size) :
    _allModels(allModels), _layoutGroups(layoutGroups)
{
	//(*Initialize(ImportPreviewsModelsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, _("Import Previews and Models"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ImportPreviewsModelsDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ImportPreviewsModelsDialog::OnButton_CancelClick);
	//*)

    SetEscapeId(ID_BUTTON2);

    int idd = wxNewId();
    TreeListCtrl1 = new wxTreeListCtrl(this, idd, wxDefaultPosition, wxSize(300, 400), wxTL_MULTIPLE | wxTR_FULL_ROW_HIGHLIGHT | wxTL_CHECKBOX | wxTL_NO_HEADER, _("IPM_TREE"));
    TreeListCtrl1->AppendColumn(L"Previews & Models", 300);
    FlexGridSizer2->Add(TreeListCtrl1, 1, wxALL | wxEXPAND, 2);
    FlexGridSizer2->Layout();

    Connect(idd, wxEVT_TREELIST_ITEM_CONTEXT_MENU, (wxObjectEventFunction)& ImportPreviewsModelsDialog::OnContextMenu);
    Connect(idd, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)& ImportPreviewsModelsDialog::OnTreeListCtrlCheckboxtoggled);

    wxTreeListItem defaultItem = TreeListCtrl1->AppendItem(TreeListCtrl1->GetRootItem(), "Default");
    wxTreeListItem unassignedItem = TreeListCtrl1->AppendItem(TreeListCtrl1->GetRootItem(), "Unassigned");

    _doc.Load(filename);
    if (_doc.IsOk())
    {
        wxXmlNode* models = nullptr;
        wxXmlNode* modelgroups = nullptr;
        for (wxXmlNode* m = _doc.GetRoot(); m != nullptr; m = m->GetNext())
        {
            for (wxXmlNode* mm = m->GetChildren(); mm != nullptr; mm = mm->GetNext())
            {
                if (mm->GetName() == "models")
                {
                    models = mm;
                }
                else if (mm->GetName() == "modelGroups")
                {
                    modelgroups = mm;
                }
            }
        }

        if (models != nullptr || modelgroups != nullptr)
        {
            AddModels(TreeListCtrl1, defaultItem, models, modelgroups, "Default");
            AddModels(TreeListCtrl1, unassignedItem, models, modelgroups, "Unassigned");

            for (wxXmlNode* n = _doc.GetRoot(); n != nullptr; n = n->GetNext())
            {
                for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext())
                {
                    if (nn->GetName() == "layoutGroups")
                    {
                        for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                        {
                            if (nnn->GetName() == "layoutGroup") {
                                wxString lg = nnn->GetAttribute("name");
                                if (lg != "")
                                {
                                    wxTreeListItem t = TreeListCtrl1->AppendItem(TreeListCtrl1->GetRootItem(), lg);
                                    AddModels(TreeListCtrl1, t, models, modelgroups, lg);
                                }
                            }
                        }
                    }
                }
            }
           
        }
    }
    ValidateWindow();

    Fit();
    Layout();
}

wxArrayString ImportPreviewsModelsDialog::GetPreviews() const
{
    wxArrayString res;

    for (wxTreeListItem it = TreeListCtrl1->GetFirstChild(TreeListCtrl1->GetRootItem()); it.IsOk(); it = TreeListCtrl1->GetNextSibling(it))
    {
        if (TreeListCtrl1->GetCheckedState(it) == wxCHK_CHECKED)
        {
            res.push_back(TreeListCtrl1->GetItemText(it));
        }
    }

    return res;
}

std::list<impTreeItemData*> ImportPreviewsModelsDialog::GetModelsInPreview(wxString preview) const
{
    std::list<impTreeItemData*> res;

    if (preview == "")
    {
        for (wxTreeListItem it = TreeListCtrl1->GetFirstChild(TreeListCtrl1->GetRootItem()); it.IsOk(); it = TreeListCtrl1->GetNextSibling(it))
        {
            if (TreeListCtrl1->GetCheckedState(it) == wxCHK_UNCHECKED)
            {
                for (wxTreeListItem it2 = TreeListCtrl1->GetFirstChild(it); it2.IsOk(); it2 = TreeListCtrl1->GetNextSibling(it2))
                {
                    if (TreeListCtrl1->GetCheckedState(it2) == wxCHK_CHECKED)
                    {
                        res.push_back(((impTreeItemData*)TreeListCtrl1->GetItemData(it2)));
                    }
                }
            }
        }
    }
    else
    {
        for (wxTreeListItem it = TreeListCtrl1->GetFirstChild(TreeListCtrl1->GetRootItem()); it.IsOk(); it = TreeListCtrl1->GetNextSibling(it))
        {
            if (TreeListCtrl1->GetItemText(it) == preview)
            {
                for (wxTreeListItem it2 = TreeListCtrl1->GetFirstChild(it); it2.IsOk(); it2 = TreeListCtrl1->GetNextSibling(it2))
                {
                    if (TreeListCtrl1->GetCheckedState(it2) == wxCHK_CHECKED)
                    {
                        res.push_back(((impTreeItemData*)TreeListCtrl1->GetItemData(it2)));
                    }
                }
            }
        }
    }

    return res;
}


void ImportPreviewsModelsDialog::AddModels(wxTreeListCtrl* tree, wxTreeListItem item, wxXmlNode* models, wxXmlNode* modelgroups, wxString preview)
{
    if (modelgroups != nullptr)
    {
        for (wxXmlNode* m = modelgroups->GetChildren(); m != nullptr; m = m->GetNext())
        {
            if (m->GetAttribute("LayoutGroup") == preview)
            {
                wxString mn = m->GetAttribute("name");
                wxTreeListItem it2 = tree->AppendItem(item, mn + " - Group", -1, -1, new impTreeItemData(mn, m, true));
                if (!tree->IsExpanded(item)) tree->Expand(item);
            }
        }
    }
    if (models != nullptr)
    {
        for (wxXmlNode* m = models->GetChildren(); m != nullptr; m = m->GetNext())
        {
            if (m->GetAttribute("LayoutGroup") == preview)
            {
                wxString mn = m->GetAttribute("name");
                tree->AppendItem(item, mn, -1, -1, new impTreeItemData(mn, m, false));
                if (!tree->IsExpanded(item)) tree->Expand(item);
            }
        }
    }
}

void ImportPreviewsModelsDialog::OnTreeListCtrlCheckboxtoggled(wxTreeListEvent& event)
{
    ValidateWindow();
}

ImportPreviewsModelsDialog::~ImportPreviewsModelsDialog()
{
	//(*Destroy(ImportPreviewsModelsDialog)
	//*)
}

void ImportPreviewsModelsDialog::OnContextMenu(wxTreeListEvent& event)
{
    _item = event.GetItem();
    wxMenu mnuContext;
    mnuContext.Append(ID_MNU_IPM_EXPANDALL, "Expand All");
    mnuContext.Append(ID_MNU_IPM_COLLAPSEALL, "Collapse All");
    mnuContext.AppendSeparator();
    mnuContext.Append(ID_MNU_IPM_SELECTALL, "Select All");
    mnuContext.Append(ID_MNU_IPM_DESELECTALL, "Deselect All");
    mnuContext.AppendSeparator();
    mnuContext.Append(ID_MNU_IPM_SELECTSIBLINGS, "Select Siblings");
    mnuContext.Append(ID_MNU_IPM_DESELECTSIBLINGS, "Deselect Siblings");
    mnuContext.Append(ID_MNU_IPM_DESELECTEXISTING, "Deselect Models Already In Layout");
    mnuContext.AppendSeparator();
    mnuContext.Append(ID_MNU_IPM_SELECTALLMODELS, "Select All Models");
    mnuContext.Append(ID_MNU_IPM_SELECTALLMODELSGROUPS, "Select All Model Groups");

    mnuContext.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& ImportPreviewsModelsDialog::OnListPopup, nullptr, this);
    PopupMenu(&mnuContext);
}

void ImportPreviewsModelsDialog::OnListPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_MNU_IPM_SELECTALL)
    {
        SelectAll(true);
    }
    else if (event.GetId() == ID_MNU_IPM_DESELECTALL)
    {
        SelectAll(false);
    }
    else if (event.GetId() == ID_MNU_IPM_SELECTSIBLINGS)
    {
        SelectSiblings(_item, true);
    }
    else if (event.GetId() == ID_MNU_IPM_DESELECTSIBLINGS)
    {
        SelectSiblings(_item, false);
    }
    else if (event.GetId() == ID_MNU_IPM_DESELECTEXISTING)
    {
        DeselectExistingModels();
    }
    else if (event.GetId() == ID_MNU_IPM_SELECTALLMODELS)
    {
        SelectAllModel(true);
    }
    else if (event.GetId() == ID_MNU_IPM_SELECTALLMODELSGROUPS)
    {
        SelectAllModelGroups(true);
    }
    else if (event.GetId() == ID_MNU_IPM_EXPANDALL)
    {
        ExpandAll(true);
    }
    else if (event.GetId() == ID_MNU_IPM_COLLAPSEALL)
    {
        ExpandAll(false);
    }
    ValidateWindow();
}

void ImportPreviewsModelsDialog::ExpandAll(bool expand)
{
    for (wxTreeListItem it = TreeListCtrl1->GetFirstChild(TreeListCtrl1->GetRootItem()); it.IsOk(); it = TreeListCtrl1->GetNextSibling(it))
    {
        if (expand)
        {
            TreeListCtrl1->Expand(it);
        }
        else
        {
            TreeListCtrl1->Collapse(it);
        }
    }
}

void ImportPreviewsModelsDialog::DeselectExistingModels()
{
    for (wxTreeListItem it = TreeListCtrl1->GetFirstChild(TreeListCtrl1->GetRootItem()); it.IsOk(); it = TreeListCtrl1->GetNextSibling(it))
    {
        for (wxTreeListItem it2 = TreeListCtrl1->GetFirstChild(it); it2.IsOk(); it2 = TreeListCtrl1->GetNextSibling(it2))
        {
            if (ModelExists(TreeListCtrl1->GetItemText(it2)))
            {
                TreeListCtrl1->CheckItem(it2, wxCHK_UNCHECKED);
            }
        }
    }
}

bool ImportPreviewsModelsDialog::ModelExists(const std::string& modelName) const
{
    return _allModels.GetModel(modelName) != nullptr;
}

bool ImportPreviewsModelsDialog::LayoutExists(const std::string& layoutName) const
{
    for (const auto& it : _layoutGroups)
    {
        if (it->GetName() == layoutName) return true;
    }
    return false;
}


void ImportPreviewsModelsDialog::SelectSiblings(wxTreeListItem item, bool checked)
{
    wxTreeListItem parent = TreeListCtrl1->GetItemParent(item);
    for (wxTreeListItem it = TreeListCtrl1->GetFirstChild(parent); it.IsOk(); it = TreeListCtrl1->GetNextSibling(it))
    {
        TreeListCtrl1->CheckItem(it, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
}

void ImportPreviewsModelsDialog::SelectAll(bool checked)
{
    for (wxTreeListItem it = TreeListCtrl1->GetFirstItem(); it.IsOk(); it = TreeListCtrl1->GetNextItem(it))
    {
        TreeListCtrl1->CheckItem(it, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
}

void ImportPreviewsModelsDialog::SelectAllModel(bool checked)
{
    for (wxTreeListItem it = TreeListCtrl1->GetFirstItem(); it.IsOk(); it = TreeListCtrl1->GetNextItem(it))
    {
        if(TreeListCtrl1->GetItemData(it) != nullptr && !((impTreeItemData*)TreeListCtrl1->GetItemData(it))->IsModelGroup())
            TreeListCtrl1->CheckItem(it, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
}

void ImportPreviewsModelsDialog::SelectAllModelGroups(bool checked)
{
    for (wxTreeListItem it = TreeListCtrl1->GetFirstItem(); it.IsOk(); it = TreeListCtrl1->GetNextItem(it))
    {
        if (TreeListCtrl1->GetItemData(it) != nullptr && ((impTreeItemData*)TreeListCtrl1->GetItemData(it))->IsModelGroup())
            TreeListCtrl1->CheckItem(it, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
}

void ImportPreviewsModelsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ImportPreviewsModelsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void ImportPreviewsModelsDialog::ValidateWindow()
{
    for (wxTreeListItem it = TreeListCtrl1->GetFirstItem(); it.IsOk(); it = TreeListCtrl1->GetNextItem(it))
    {
        if (TreeListCtrl1->GetCheckedState(it) == wxCheckBoxState::wxCHK_CHECKED)
        {
            Button_Ok->Enable();
            return;
        }
    }
    Button_Ok->Disable();
}
