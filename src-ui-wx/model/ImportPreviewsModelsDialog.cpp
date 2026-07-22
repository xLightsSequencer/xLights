/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ImportPreviewsModelsDialog.h"

#include <cmath>
#include <spdlog/spdlog.h>
#include "models/RulerObject.h"

//(*InternalHeaders(ImportPreviewsModelsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/menu.h>
#include <wx/treebase.h>
#include <wx/tokenzr.h>
#include <algorithm>
#include <vector>

#include "layout/LayoutGroup.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"

//(*IdInit(ImportPreviewsModelsDialog)
const long ImportPreviewsModelsDialog::ID_CHECKBOX1 = wxNewId();
const long ImportPreviewsModelsDialog::ID_BUTTON1 = wxNewId();
const long ImportPreviewsModelsDialog::ID_BUTTON2 = wxNewId();
//*)

const long ImportPreviewsModelsDialog::ID_MNU_IPM_SELECTALL = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_DESELECTALL = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_SELECTHIGH = wxNewId();
const long ImportPreviewsModelsDialog::ID_MNU_IPM_DESELECTHIGH = wxNewId();
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

ImportPreviewsModelsDialog::ImportPreviewsModelsDialog(wxWindow* parent, const wxString& filename, ModelManager& allModels, const std::map<std::string, std::unique_ptr<LayoutGroup>>& layoutGroups, wxWindowID id,const wxPoint& pos,const wxSize& size) :
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
	CheckBox_IncludeEmptyModelGroups = new wxCheckBox(this, ID_CHECKBOX1, _("Include empty model groups"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_IncludeEmptyModelGroups->SetValue(false);
	FlexGridSizer1->Add(CheckBox_IncludeEmptyModelGroups, 1, wxALL|wxEXPAND, 5);
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

    // Live filter above the tree (manual code, no wxSmith/.wxs change needed).
    // Debounced so typing in a large model list does not rebuild every keystroke.
    _filterCtrl = new wxSearchCtrl(this, wxID_ANY);
    _filterCtrl->ShowCancelButton(true);
    _filterCtrl->SetDescriptiveText(_("Filter models"));
    FlexGridSizer2->Insert(0, _filterCtrl, 0, wxALL | wxEXPAND, 2);
    FlexGridSizer2->RemoveGrowableRow(0);
    FlexGridSizer2->AddGrowableRow(1);
    _filterTimer.SetOwner(this, wxNewId());
    Bind(wxEVT_TIMER, [this](wxTimerEvent&) { PopulateTree(); }, _filterTimer.GetId());
    _filterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) {
        _filter = _filterCtrl->GetValue().Lower().Trim().Trim(false);
        _filterTimer.StartOnce(200);
    });
    _filterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, [this](wxCommandEvent&) {
        ClearFilter();
    });

    pugi::xml_parse_result parseResult = _doc.load_file(filename.ToStdString().c_str());
    if (!parseResult) {
        spdlog::warn("ImportPreviewsModelsDialog: could not parse {}: {}", filename.ToStdString(), parseResult.description());
    }
    PopulateTree();
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
                        res.push_back((impTreeItemData*)TreeListCtrl1->GetItemData(it2));
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
                        res.push_back((impTreeItemData*)TreeListCtrl1->GetItemData(it2));
                    }
                }
            }
        }
    }

    return res;
}


void ImportPreviewsModelsDialog::AddModels(wxTreeListCtrl* tree, wxTreeListItem item, pugi::xml_node models, pugi::xml_node modelgroups, wxString preview, const wxString& filter)
{
    // Sort here and append in order rather than handing the control a comparator:
    // with a sort column installed every AppendItem re-sorts the parent's children,
    // which turns populating a large layout into an O(n^2) operation - and filtering
    // repopulates the whole tree on every keystroke.
    std::vector<std::pair<std::string, pugi::xml_node>> groups;
    std::vector<std::pair<std::string, pugi::xml_node>> mods;

    auto collect = [&](pugi::xml_node parent, std::vector<std::pair<std::string, pugi::xml_node>>& into) {
        for (pugi::xml_node m = parent.first_child(); m; m = m.next_sibling()) {
            if (wxString(m.attribute("LayoutGroup").as_string()) == preview) {
                wxString mn = m.attribute("name").as_string();
                if (MatchesFilter(mn, filter)) into.emplace_back(mn.ToStdString(), m);
            }
        }
    };
    if (modelgroups) collect(modelgroups, groups);
    if (models) collect(models, mods);

    if (groups.empty() && mods.empty()) return;

    auto byName = [](const auto& a, const auto& b) { return stdlistNumberAwareStringCompare(a.first, b.first); };
    std::sort(groups.begin(), groups.end(), byName);
    std::sort(mods.begin(), mods.end(), byName);

    // Model groups sort ahead of models, then each block is name-ordered.
    for (const auto& [name, node] : groups) {
        tree->AppendItem(item, wxString(name) + " - Group", -1, -1, new impTreeItemData(wxString(name), node, true));
    }
    for (const auto& [name, node] : mods) {
        tree->AppendItem(item, wxString(name), -1, -1, new impTreeItemData(wxString(name), node, false));
    }
    tree->Expand(item);
}

bool ImportPreviewsModelsDialog::MatchesFilter(const wxString& name, const wxString& filterLower)
{
    if (filterLower.empty()) return true;
    const wxString hay = name.Lower();
    wxStringTokenizer tok(filterLower);
    while (tok.HasMoreTokens()) {
        if (hay.Find(tok.GetNextToken()) == wxNOT_FOUND) return false;
    }
    return true;
}

void ImportPreviewsModelsDialog::SyncCheckedFromTree()
{
    // Merge the visible tree's check state into the persistent sets. Filtered-out
    // rows aren't visited, so their checked state is retained.
    for (wxTreeListItem p = TreeListCtrl1->GetFirstChild(TreeListCtrl1->GetRootItem()); p.IsOk(); p = TreeListCtrl1->GetNextSibling(p)) {
        const std::string pk = TreeListCtrl1->GetItemText(p).ToStdString();
        if (TreeListCtrl1->GetCheckedState(p) == wxCHK_CHECKED) _checkedPreviews.insert(pk);
        else _checkedPreviews.erase(pk);
        for (wxTreeListItem c = TreeListCtrl1->GetFirstChild(p); c.IsOk(); c = TreeListCtrl1->GetNextSibling(c)) {
            auto* d = (impTreeItemData*)TreeListCtrl1->GetItemData(c);
            if (d == nullptr) continue;
            const CheckedModel ck { d->GetName().ToStdString(), d->IsModelGroup() };
            if (TreeListCtrl1->GetCheckedState(c) == wxCHK_CHECKED) _checkedModels.insert(ck);
            else _checkedModels.erase(ck);
        }
    }
}

void ImportPreviewsModelsDialog::RestoreChecksToTree()
{
    for (wxTreeListItem p = TreeListCtrl1->GetFirstChild(TreeListCtrl1->GetRootItem()); p.IsOk(); p = TreeListCtrl1->GetNextSibling(p)) {
        if (_checkedPreviews.count(TreeListCtrl1->GetItemText(p).ToStdString()) != 0)
            TreeListCtrl1->CheckItem(p, wxCHK_CHECKED);
        for (wxTreeListItem c = TreeListCtrl1->GetFirstChild(p); c.IsOk(); c = TreeListCtrl1->GetNextSibling(c)) {
            auto* d = (impTreeItemData*)TreeListCtrl1->GetItemData(c);
            if (d == nullptr) continue;
            if (_checkedModels.count({ d->GetName().ToStdString(), d->IsModelGroup() }) != 0)
                TreeListCtrl1->CheckItem(c, wxCHK_CHECKED);
        }
    }
}

void ImportPreviewsModelsDialog::ClearFilter()
{
    _filterTimer.Stop();
    if (_filterCtrl != nullptr) _filterCtrl->ChangeValue("");
    _filter.Clear();
    PopulateTree();
}

void ImportPreviewsModelsDialog::PopulateTree()
{
    SyncCheckedFromTree();

    TreeListCtrl1->Freeze();
    TreeListCtrl1->DeleteAllItems();

    pugi::xml_node root = _doc.document_element();
    pugi::xml_node models = root.child("models");
    pugi::xml_node modelgroups = root.child("modelGroups");

    if (models || modelgroups) {
        std::vector<wxTreeListItem> previews;
        wxTreeListItem defaultItem = TreeListCtrl1->AppendItem(TreeListCtrl1->GetRootItem(), "Default");
        wxTreeListItem unassignedItem = TreeListCtrl1->AppendItem(TreeListCtrl1->GetRootItem(), "Unassigned");
        AddModels(TreeListCtrl1, defaultItem, models, modelgroups, "Default", _filter);
        AddModels(TreeListCtrl1, unassignedItem, models, modelgroups, "Unassigned", _filter);
        previews.push_back(defaultItem);
        previews.push_back(unassignedItem);

        pugi::xml_node layoutGroupsNode = root.child("layoutGroups");
        if (layoutGroupsNode) {
            for (pugi::xml_node nnn = layoutGroupsNode.first_child(); nnn; nnn = nnn.next_sibling()) {
                if (std::string_view(nnn.name()) == "layoutGroup") {
                    wxString lg = nnn.attribute("name").as_string();
                    if (lg != "") {
                        wxTreeListItem t = TreeListCtrl1->AppendItem(TreeListCtrl1->GetRootItem(), lg);
                        AddModels(TreeListCtrl1, t, models, modelgroups, lg, _filter);
                        previews.push_back(t);
                    }
                }
            }
        }

        // While filtering, drop preview rows with no matching children.
        if (!_filter.empty()) {
            for (wxTreeListItem p : previews) {
                if (!TreeListCtrl1->GetFirstChild(p).IsOk()) TreeListCtrl1->DeleteItem(p);
            }
        }
    }

    RestoreChecksToTree();

    TreeListCtrl1->Thaw();
    TreeListCtrl1->Refresh();
    _appliedFilter = _filter;
}

void ImportPreviewsModelsDialog::OnTreeListCtrlCheckboxtoggled(wxTreeListEvent& event)
{
    ValidateWindow();
}

bool ImportPreviewsModelsDialog::GetIncludeEmptyGroups() const
{
	return CheckBox_IncludeEmptyModelGroups->IsChecked();
}

ImportPreviewsModelsDialog::~ImportPreviewsModelsDialog()
{
    _filterTimer.Stop();
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
    mnuContext.Append(ID_MNU_IPM_SELECTHIGH, "Select Highlighted");
    mnuContext.Append(ID_MNU_IPM_DESELECTHIGH, "Deselect Highlighted");
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
    else if (event.GetId() == ID_MNU_IPM_SELECTHIGH) 
    {
        SelectHighlighted(true);
    } 
    else if (event.GetId() == ID_MNU_IPM_DESELECTHIGH) 
    {
        SelectHighlighted(false);
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

    // The walk above only sees what the filter left visible, but everything still in
    // the sets gets imported on OK - so drop the hidden ones too. Groups are left
    // alone, matching the tree walk (a group's row text is "<name> - Group", which
    // never matches an existing model).
    for (auto it = _checkedModels.begin(); it != _checkedModels.end();) {
        if (!it->group && ModelExists(it->name)) it = _checkedModels.erase(it);
        else ++it;
    }
}

bool ImportPreviewsModelsDialog::ModelExists(const std::string& modelName) const
{
    return _allModels.GetModel(modelName) != nullptr;
}

bool ImportPreviewsModelsDialog::LayoutExists(const std::string& layoutName) const
{
    return _layoutGroups.count(layoutName) > 0;
}

void ImportPreviewsModelsDialog::SelectRecursiveModel(wxString m, bool checked)
{
    size_t pos = m.find('/');
    std::string model = (pos != std::string::npos) ? m.substr(0, pos) : m; // remove submodel from name
    for (wxTreeListItem it = TreeListCtrl1->GetFirstItem(); it.IsOk(); it = TreeListCtrl1->GetNextItem(it)) {
        if (model == TreeListCtrl1->GetItemText(it)) {
            TreeListCtrl1->CheckItem(it, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
            auto* itm = ((impTreeItemData*)TreeListCtrl1->GetItemData(it));
            if (itm && itm->IsModelGroup()) {
                wxString const models = ((impTreeItemData*)TreeListCtrl1->GetItemData(it))->GetModelNode().attribute("models").as_string();
                wxArrayString const modelArray = wxSplit(models, ',');
                for (size_t i = 0; i < modelArray.size(); ++i) {
                    SelectRecursiveModel(modelArray[i], checked);
                }
            }
            break;
        }
    }
}

void ImportPreviewsModelsDialog::SelectSiblings(wxTreeListItem item, bool checked)
{
    if (item.IsOk()) {
        SelectRecursiveModel(TreeListCtrl1->GetItemText(item), checked);
    }
}

void ImportPreviewsModelsDialog::SelectAll(bool checked)
{
    // Deselect All means all of them, not just the rows the filter left visible.
    if (!checked) {
        _checkedModels.clear();
        _checkedPreviews.clear();
    }
    for (wxTreeListItem it = TreeListCtrl1->GetFirstItem(); it.IsOk(); it = TreeListCtrl1->GetNextItem(it)) {
        TreeListCtrl1->CheckItem(it, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
}

void ImportPreviewsModelsDialog::SelectHighlighted(bool checked)
{
    for (wxTreeListItem it = TreeListCtrl1->GetFirstItem(); it.IsOk(); it = TreeListCtrl1->GetNextItem(it)) {
        if (TreeListCtrl1->IsSelected(it)) {
            TreeListCtrl1->CheckItem(it, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
        }
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
    _filterTimer.Stop();
    EndDialog(wxID_CANCEL);
}

void ImportPreviewsModelsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    // Drop any active filter so the full tree (with every checked row) is present
    // for the caller's GetPreviews()/GetModelsInPreview() to read. This has to test
    // what the tree actually shows, not the pending _filter: emptying the box and
    // hitting Enter inside the debounce window leaves _filter empty while the tree
    // is still filtered, and the hidden checks would silently not be imported.
    if (!_appliedFilter.empty() || !_filter.empty()) {
        ClearFilter();
    } else {
        _filterTimer.Stop();
    }
    EndDialog(wxID_OK);
}

void ImportPreviewsModelsDialog::ValidateWindow()
{
    // Drive the OK button off the persistent check sets so it stays correct even
    // when checked rows are hidden by the current filter.
    SyncCheckedFromTree();
    Button_Ok->Enable(!_checkedModels.empty() || !_checkedPreviews.empty());
}

float ImportPreviewsModelsDialog::GetSourceRulerPerUnit() const
{
    pugi::xml_node root = _doc.document_element();
    pugi::xml_node viewObjects = root.child("view_objects");
    if (!viewObjects) return 0.0f;

    for (pugi::xml_node obj = viewObjects.first_child(); obj; obj = obj.next_sibling()) {
        if (std::string_view(obj.attribute("DisplayAs").as_string()) == "Ruler") {
            // X2/Y2/Z2 are offsets from WorldPos (see TwoPointScreenLocation::PrepareToDraw),
            // so the ruler pixel length is just their magnitude — no need to read WorldPos.
            float x2 = obj.attribute("X2").as_float(0);
            float y2 = obj.attribute("Y2").as_float(0);
            float z2 = obj.attribute("Z2").as_float(0);
            float realLength = obj.attribute("Length").as_float(1);
            int   srcUnits   = obj.attribute("Units").as_int(RULER_UNITS_M);
            float pixelLength = std::sqrt(x2*x2 + y2*y2 + z2*z2);
            static const char* kUnitNames[] = {"m","cm","mm","yds","ft","in"};
            const char* srcUnitName = (srcUnits >= 0 && srcUnits <= 5) ? kUnitNames[srcUnits] : "?";
            if (pixelLength > 0 && realLength > 0) {
                float realLength_m = RulerObject::Convert(srcUnits, std::string("m"), realLength);
                float srcPerUnit_m = realLength_m / pixelLength;
                spdlog::debug("ImportPreviewsModelsDialog: source ruler {:.4f}{} over {:.2f}px => {:.6f} m/px",
                              realLength, srcUnitName, pixelLength, srcPerUnit_m);
                return srcPerUnit_m;
            }
            break;
        }
    }
    return 0.0f;
}
