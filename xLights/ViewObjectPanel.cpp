/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/artprov.h>
#include <wx/treebase.h>
#include <wx/dataview.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "LayoutPanel.h"
#include "ViewObjectPanel.h"
#include "UtilFunctions.h"
#include "models/ViewObject.h"
#include "xLightsMain.h"
#include <log4cpp/Category.hh>

//(*InternalHeaders(ViewObjectPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ViewObjectPanel)
const long ViewObjectPanel::ID_TREELISTVIEW_PANEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(ViewObjectPanel,wxPanel)
	//(*EventTable(ViewObjectPanel)
	//*)
END_EVENT_TABLE()

const long ViewObjectPanel::ID_TREELISTVIEW_OBJECTS = wxNewId();
const long ViewObjectPanel::ID_MNU_DELETE_OBJECT = wxNewId();
const long ViewObjectPanel::ID_MNU_UNLINKFROMBASE = wxNewId();

ViewObjectPanel::ViewObjectPanel(wxWindow* parent,ViewObjectManager &Objects,LayoutPanel *xl,wxWindowID id,const wxPoint& pos,const wxSize& size)
:   layoutPanel(xl), mViewObjects(Objects), mSelectedObject(nullptr)
{
	//(*Initialize(ViewObjectPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	SetMinSize(wxSize(200,200));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	FirstPanel = new wxPanel(this, ID_TREELISTVIEW_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_TREELISTVIEW_PANEL"));
	FlexGridSizer2->Add(FirstPanel, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer1);

	Connect(wxEVT_CHAR,(wxObjectEventFunction)&ViewObjectPanel::OnChar);
	//*)

    InitImageList();
    TreeListViewObjects = CreateTreeListCtrl(wxTL_DEFAULT_STYLE);
    wxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(TreeListViewObjects, wxSizerFlags(2).Expand());
    FirstPanel->SetSizer(sizer1);
    sizer1->SetSizeHints(FirstPanel);

	TreeListViewObjects->GetView()->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&ViewObjectPanel::DoCut, nullptr, this);
	TreeListViewObjects->GetView()->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&ViewObjectPanel::DoCopy, nullptr, this);
	TreeListViewObjects->GetView()->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&ViewObjectPanel::DoPaste, nullptr, this);
	TreeListViewObjects->GetView()->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&ViewObjectPanel::DoUndo, nullptr, this);
	TreeListViewObjects->GetView()->Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(ViewObjectPanel::OnCharHook), nullptr, this);

    refreshObjectList();
}

ViewObjectPanel::~ViewObjectPanel()
{
    TreeListViewObjects->DeleteAllItems();
}

class ObjectTreeData : public wxTreeItemData {
public:
    ObjectTreeData(ViewObject *v, int NativeOrder) :wxTreeItemData(), view_object(v) {
        nativeOrder = NativeOrder;
    };
    virtual ~ObjectTreeData() {};

    ViewObject *GetViewObject() const
    {
        return view_object;
    }

    int nativeOrder;
private:
    ViewObject *view_object;
};

void ViewObjectPanel::InitImageList()
{
    m_imageList.push_back(wxArtProvider::GetBitmapBundle("wxART_NORMAL_FILE", wxART_LIST));
    m_imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_GROUP_CLOSED", wxART_LIST));
    m_imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_GROUP_OPEN", wxART_LIST));
    m_imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_IMAGE_ICON", wxART_LIST));
    m_imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_POLY_ICON", wxART_LIST));
}

wxTreeListCtrl* ViewObjectPanel::CreateTreeListCtrl(long style)
{
    wxTreeListCtrl* const
    tree = new wxTreeListCtrl(FirstPanel, ID_TREELISTVIEW_OBJECTS,
                              wxDefaultPosition, wxDefaultSize,
                              style, "ID_TREELISTVIEW_OBJECTS");
    tree->SetImages(m_imageList);

    tree->AppendColumn("Object / Group",
                       wxCOL_WIDTH_AUTOSIZE,
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);
    tree->SetSortColumn(0, true);
    return tree;
}

void ViewObjectPanel::refreshObjectList() {

    /*TreeListViewObjects->Freeze();

    for ( wxTreeListItem item = TreeListViewObjects->GetFirstItem();
          item.IsOk();
          item = TreeListViewObjects->GetNextItem(item) )
    {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewObjects->GetItemData(item));
        Model *model = data != nullptr ? data->GetModel() : nullptr;

        if (model != nullptr ) {
            int end_channel = model->GetLastChannel()+1;
            wxString endStr = model->GetLastChannelInStartChannelFormat(xlights->GetOutputManager(), nullptr);
            if( model->GetDisplayAs() != "ModelGroup" ) {
                wxString cv = TreeListViewObjects->GetItemText(item, Col_StartChan);
                wxString startStr = model->GetStartChannelInDisplayFormat();
                if (cv != startStr) {
                    data->startingChannel = model->GetNumberFromChannelString(model->ModelStartChannel);
                    if ((model->CouldComputeStartChannel || model->GetDisplayAs() == "SubModel") && model->IsValidStartChannelString())
                    {
                        TreeListViewObjects->SetItemText(item, Col_StartChan, startStr);
                    }
                    else
                    {
                        TreeListViewObjects->SetItemText(item, Col_StartChan, "*** " + startStr);
                    }
                }
                cv = TreeListViewObjects->GetItemText(item, Col_EndChan);
                if (cv != endStr) {
                    data->endingChannel = end_channel;
                    TreeListViewObjects->SetItemText(item, Col_EndChan, endStr);
                }
            }
        }
    }
    TreeListViewObjects->Thaw();
    TreeListViewObjects->Refresh();*/
}

int ViewObjectPanel::GetObjectTreeIcon(ViewObject* view_object, bool open) {
    if( view_object->GetDisplayAs() == "ModelGroup" ) {
        return open ? Icon_FolderOpened : Icon_FolderClosed;
    } else {
        const std::string type = view_object->GetDisplayAs();
        if (type == "Image") {
            return Icon_Image;
        } else if( type == "Poly Line" ) {
            return Icon_Poly;
        }
    }
    return Icon_File;
}

int ViewObjectPanel::AddObjectToTree(ViewObject *view_object, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName) {
    int width = 0;

    if (view_object == nullptr) return width;

    wxTreeListItem item = TreeListViewObjects->AppendItem(*parent, fullName ? view_object->GetName() : view_object->name,
                                                         GetObjectTreeIcon(view_object, false),
                                                         GetObjectTreeIcon(view_object, true),
                                                         new ObjectTreeData(view_object, nativeOrder));

    /*if( model->GetDisplayAs() == "ModelGroup" ) {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        ModelGroup *grp = (ModelGroup*)model;
        int i = 0;
        for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); ++it) {
            Model *m = xlights->AllModels[*it];

            if (m == nullptr)
            {
                logger_base.error("Model group %s thought it contained model. '%s' but it didnt. This would have crashed.", (const char *)grp->GetName().c_str(), (const char *)it->c_str());
            }
            else if (m == grp)
            {
                // This is bad ... a model group contains itself
                logger_base.error("Model group contains itself. '%s'", (const char *)grp->GetName().c_str());
            }
            else
            {
                AddModelToTree(m, &item, false, i, true);
                i++;
            }
        }
    }*/

    if (expanded) TreeListViewObjects->Expand(item);

    return width;
}

void ViewObjectPanel::UpdateObjectList(bool full_refresh, std::string& currentLayoutGroup) {
    std::vector<ViewObject *> objects;
    UpdateObjectList(full_refresh, objects, currentLayoutGroup);
}

void ViewObjectPanel::UpdateObjectList(bool full_refresh, std::vector<ViewObject*> &objects, std::string& currentLayoutGroup ) {

    TreeListViewObjects->Freeze();
    unsigned sortcol;
    bool ascending;
    bool sorted = TreeListViewObjects->GetSortColumn(&sortcol, &ascending);

    //if (full_refresh) {
    //    UnSelectAllModels();
    //}
    std::vector<ViewObject *> dummy_objects;

    // Update all the custom previews
    /*for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        dummy_objects.clear();
        if (grp->GetName() == currentLayoutGroup) {
            UpdateModelsForPreview(currentLayoutGroup, grp, models, true);
        } else {
            UpdateModelsForPreview(grp->GetName(), grp, dummy_objects, false);
        }
    }*/

    // update the Layout tab preview for default options
    if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
        UpdateObjectsForPreview(currentLayoutGroup, nullptr, objects, true);
    }

    if (full_refresh) {
        int width = 0;
        //turn off the column width auto-resize.  Makes it REALLY slow to populate the tree
        TreeListViewObjects->SetColumnWidth(0, 10);
        //turn off the sorting as that is ALSO really slow
        TreeListViewObjects->SetItemComparator(nullptr);
        if (sorted) {
            //UnsetAsSortKey may be unimplemented on all  platforms so we'll set a
            //sort column to 0 which is faster due to straight string compare
            TreeListViewObjects->GetDataView()->GetSortingColumn()->UnsetAsSortKey();
        }

        //delete all items will attempt to resort as each item is deleted, however, our Model pointers
        //stored in the items may be invalid
        wxTreeListItem child = TreeListViewObjects->GetFirstItem();
        std::list<std::string> expanded;
        while (child.IsOk()) {
            if (TreeListViewObjects->IsExpanded(child)) {
                expanded.push_back(TreeListViewObjects->GetItemText(child));
            }
            TreeListViewObjects->DeleteItem(child);
            child = TreeListViewObjects->GetFirstItem();
        }
        TreeListViewObjects->DeleteAllItems();
        if (sorted) {
            //UnsetAsSortKey may be unimplemented on all  platforms so we'll set a
            //sort column to 0 which is faster due to straight string compare
            TreeListViewObjects->SetSortColumn(0, true);
            //then turn it off again so platforms that DO support this can benefit
            TreeListViewObjects->GetDataView()->GetSortingColumn()->UnsetAsSortKey();
        }

        wxTreeListItem root = TreeListViewObjects->GetRootItem();
        // add all the object groups
        /*for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
            Model *model = it->second;
            if (model->GetDisplayAs() == "ModelGroup") {
                if (currentLayoutGroup == "All Models" || model->GetLayoutGroup() == currentLayoutGroup
                    || (model->GetLayoutGroup() == "All Previews" && currentLayoutGroup != "Unassigned")) {
                    bool expand = (std::find(expanded.begin(), expanded.end(), model->GetName()) != expanded.end());
                    width = std::max(width, AddObjectToTree(model, &root, expand, 0));
                }
            }
        }*/

        // add all the objects
        for (auto it = objects.begin(); it != objects.end(); ++it) {
            ViewObject *view_object = *it;
            if (view_object->GetDisplayAs() != "ModelGroup") {
                bool expand = (std::find(expanded.begin(), expanded.end(), view_object->GetName()) != expanded.end());
                width = std::max(width, AddObjectToTree(view_object, &root, expand, 0));
            }
        }

        TreeListViewObjects->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);

        // we should have calculated a size, now turn off the auto-sizes as it's SLOW to update anything later
        int i = TreeListViewObjects->GetColumnWidth(0);

#ifdef LINUX // Calculate size on linux as GTK doesn't size the window in time

        i = TreeListViewObjects->GetSize().GetWidth() - (width * 2);
#endif
        if (i > 10) {
            TreeListViewObjects->SetColumnWidth(0, i);
        }
        //turn the sorting back on
        TreeListViewObjects->SetItemComparator(&comparator);
        if (sorted) {
            TreeListViewObjects->SetSortColumn(sortcol, ascending);
        }
    }
    //modelPreview->SetModels(models);
    //UpdatePreview();

    TreeListViewObjects->Thaw();
    TreeListViewObjects->Refresh();
}

void ViewObjectPanel::UpdateObjectsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<ViewObject *> &prev_objects, bool filtering)
{
    std::set<std::string> objectsAdded;

    for (const auto& it : layoutPanel->xlights->AllObjects) {
        ViewObject *view_object = it.second;
        if (view_object->GetDisplayAs() != "ObjectGroup") {
            if (group == "All Models" ||
                view_object->GetLayoutGroup() == group ||
                (view_object->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                prev_objects.push_back(view_object);
                objectsAdded.insert(view_object->name);
            }
        }
    }

    // add in any models that were not in preview but belong to a group that is in the preview
    /*std::string selected_group_name = "";
    if (mSelectedGroup.IsOk() && filtering) {
        selected_group_name = TreeListViewObjects->GetItemText(mSelectedGroup);
    }

    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        bool mark_selected = false;
        if (mSelectedGroup.IsOk() && filtering && (view_object->name == selected_group_name)) {
            mark_selected = true;
        }
        if (view_object->GetDisplayAs() == "ObjectGroup") {
            ModelGroup *grp = (ModelGroup*)(view_object);
            if (group == "All Models" ||
                view_object->GetLayoutGroup() == group ||
                (view_object->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                for (auto it2 = grp->ModelNames().begin(); it2 != grp->ModelNames().end(); ++it2) {
                    ViewObject *m = layoutPanel->xlights->AllObjects[*it2];
                    if (m != nullptr) {
                        if (mark_selected) {
                            if (selectedModel == nullptr)
                            {
                                SelectModel(m, false);
                            }
                            m->GroupSelected = true;
                            m->Highlighted = true;
                        }
                        if (m->DisplayAs == "SubModel") {
                            if (mark_selected) {
                                prev_objects.push_back(m);
                            }
                        }
                        else if (m->DisplayAs == "ModelGroup") {
                            ModelGroup *mg = (ModelGroup*)m;
                            if (mark_selected) {
                                for (auto it3 = mg->Models().begin(); it3 != mg->Models().end(); ++it3) {
                                    if ((*it3)->DisplayAs != "ModelGroup") {
                                        if (selectedModel == nullptr)
                                        {
                                            SelectModel((*it3), false);
                                        }
                                        (*it3)->GroupSelected = true;
                                        (*it3)->Highlighted = true;
                                        prev_objects.push_back(*it3);
                                    }
                                }
                            }
                        }
                        else if (objectsAdded.find(*it2) == objectsAdded.end()) {
                            objectsAdded.insert(*it2);
                            prev_objects.push_back(m);
                        }
                    }
                }
            }
        }
    }*/

   /* // only run this for layout group previews
    if (layout_grp != nullptr) {
        layout_grp->SetModels(prev_objects);
        ModelPreview* preview = layout_grp->GetModelPreview();
        if (layout_grp->GetPreviewCreated()) {
            preview->SetModels(layout_grp->GetModels());
            if (preview->GetActive()) {
                preview->Refresh();
                preview->Update();
            }
        }
    }*/
}

void ViewObjectPanel::HighlightObject(ViewObject* v)
{
    for ( wxTreeListItem item = TreeListViewObjects->GetFirstItem();
          item.IsOk();
          item = TreeListViewObjects->GetNextSibling(item) )
    {
        if (TreeListViewObjects->GetItemData(item) != nullptr)
        {
            ObjectTreeData *mitem = dynamic_cast<ObjectTreeData*>(TreeListViewObjects->GetItemData(item));
            if (mitem != nullptr && mitem->GetViewObject() == v) {
                TreeListViewObjects->Select(item);
                TreeListViewObjects->EnsureVisible(item);
                mSelectedObject = v;
                break;
            }
        }
    }
}

void ViewObjectPanel::RenameObjectInTree(ViewObject *view_object, const std::string new_name)
{
    for ( wxTreeListItem item = TreeListViewObjects->GetFirstItem();
          item.IsOk();
          item = TreeListViewObjects->GetNextItem(item) )
    {
        ObjectTreeData *data = dynamic_cast<ObjectTreeData*>(TreeListViewObjects->GetItemData(item));
        if (data != nullptr && data->GetViewObject() == view_object) {
            TreeListViewObjects->SetItemText(item, wxString(new_name.c_str()));
        }
    }
}

bool ViewObjectPanel::OnSelectionChanged(wxTreeListEvent& event, ViewObject** view_object, std::string& currentLayoutGroup)
{
    bool show_prop_grid = false;

    wxTreeListItem item = event.GetItem();
    if (item.IsOk()) {
        ObjectTreeData* data = (ObjectTreeData*)TreeListViewObjects->GetItemData(item);
        *view_object = ((data != nullptr) ? data->GetViewObject() : nullptr);
        if (*view_object != nullptr) {
            if ((*view_object)->GetDisplayAs() == "ObjectGroup") {
                mSelectedGroup = item;
                UpdateObjectList(false, currentLayoutGroup);
                // model_grp_panel->UpdatePanel(view_object->name);
            } else {
                mSelectedGroup = nullptr;
                mSelectedObject = *view_object;
                show_prop_grid = true;
            }

            if ((*view_object)->IsFromBase()) {
                TreeListViewObjects->SetToolTip("From Base Show Folder");
            } else {
                TreeListViewObjects->UnsetToolTip();
            }
        } else {
            mSelectedGroup = nullptr;
            mSelectedObject = nullptr;
            show_prop_grid = true;
            // UnSelectAllObjects(true);
            TreeListViewObjects->UnsetToolTip();
        }
#ifndef LINUX
        TreeListViewObjects->SetFocus();
#endif
    }
    return show_prop_grid;
}

void ViewObjectPanel::OnPropertyGridChange(wxPropertyGrid *propertyEditor, wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    if (mSelectedObject != nullptr) {
        if ("ObjectName" == name) {
            std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());

            if (safename != event.GetValue().GetString().ToStdString())
            {
                // need to update the property grid with the modified name
                wxPGProperty* prop = propertyEditor->GetPropertyByName("ObjectName");
                if (prop != nullptr) {
                    prop->SetValue(safename);
                }
            }
            std::string oldname = mSelectedObject->name;
            if (oldname != safename) {
                RenameObjectInTree(mSelectedObject, safename);
                mSelectedObject = nullptr;
                layoutPanel->xlights->RenameObject(oldname, safename);
                layoutPanel->SelectBaseObject(safename);
                mSelectedObject = dynamic_cast<ViewObject*>(layoutPanel->selectedBaseObject);
                layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ViewObjectPanel::OnPropertyGridChange::ObjectName");
                layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ViewObjectPanel::OnPropertyGridChange::ObjectName");
                layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_OBJECTLIST, "ViewObjectPanel::OnPropertyGridChange::ObjectName");
            }
        } else {
            int i = mSelectedObject->OnPropertyGridChange(propertyEditor, event);
            wxASSERT(i == 0 || i == GRIDCHANGE_SUPPRESS_HOLDSIZE);
        }
    }

    if (mSelectedObject != nullptr && mSelectedObject->IsFromBase()) {
        propertyEditor->SetToolTip("This object comes from the base folder and its properties cannot be edited.");
        auto it = propertyEditor->GetIterator(wxPG_ITERATE_ALL, nullptr);
        while (!it.AtEnd()) {
            it.GetProperty()->Enable(false);
            it.Next(true);
        }

    } else {
        propertyEditor->UnsetToolTip();
    }
}

void ViewObjectPanel::OnItemContextMenu(wxTreeListEvent& event)
{
    wxMenu mnuContext;
    wxTreeListItem item = event.GetItem();
    if( item.IsOk() ) {
        ObjectTreeData *data = dynamic_cast<ObjectTreeData*>(TreeListViewObjects->GetItemData(item));
        ViewObject* view_object = data != nullptr ? data->GetViewObject() : nullptr;
        if( view_object != nullptr ) {
            if( view_object->GetDisplayAs() == "ObjectGroup" ) {
                mSelectedGroup = item;
            } else {
                mSelectedGroup = nullptr;
                mSelectedObject = view_object;
                //SelectModel(model, false);
            }
        }
    } else {
        return;
    }

    if (mSelectedObject != nullptr ) {
        mnuContext.Append(ID_MNU_DELETE_OBJECT,"Delete");
        mnuContext.Append(ID_MNU_UNLINKFROMBASE, "Unlink Models from Base Show Folder")->Enable(mSelectedObject->IsFromBase());
    }

    mnuContext.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ViewObjectPanel::OnObjectsPopup, nullptr, this);
    PopupMenu(&mnuContext);
}

void ViewObjectPanel::OnObjectsPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int id = event.GetId();
    if(id == ID_MNU_DELETE_OBJECT)
    {
        logger_base.debug("ViewObjectPanel::OnObjectsPopup DELETE_OBJECT");
        DeleteSelectedObject();
    } else if (id == ID_MNU_UNLINKFROMBASE) {
        logger_base.debug("ViewObjectPanel::OnObjectsPopup UNLINKFROMBASE");
        UnlinkSelectedObject();
    }
}

void ViewObjectPanel::UnlinkSelectedObject()
{
    if (mSelectedObject != nullptr && mSelectedObject->IsFromBase()) {
        mSelectedObject->SetFromBase(false);
    }

    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ViewObjectPanel::UnlinkSelectedObject");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::UnlinkSelectedObject");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::UnlinkSelectedObject");
}

void ViewObjectPanel::DeleteSelectedObject()
{
    // dont block deletes of objects from base
    if( mSelectedObject != nullptr && !mSelectedObject->GetObjectScreenLocation().IsLocked()) {
        layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
        // This should delete all selected models
        layoutPanel->xlights->AllObjects.Delete(mSelectedObject->name);
        mSelectedObject = nullptr;
        layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::DeleteSelectedObject");
        layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ViewObjectPanel::DeleteSelectedObject");
        layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::DeleteSelectedObject");
        layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::DeleteSelectedObject");
    }
}

int ViewObjectPanel::ObjectListComparator::SortElementsFunction(wxTreeListCtrl *treelist, wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn)
{
    unsigned col;
    bool ascending;
    treelist->GetSortColumn(&col, &ascending);

    ObjectTreeData *data1 = dynamic_cast<ObjectTreeData*>(treelist->GetItemData(item1));
    ObjectTreeData *data2 = dynamic_cast<ObjectTreeData*>(treelist->GetItemData(item2));

    ViewObject* a = data1->GetViewObject();
    ViewObject* b = data2->GetViewObject();

    if (a == nullptr || b == nullptr) {
        return 0;
    }

    if (a->GetDisplayAs() == "ObjectGroup") {
        if (b->GetDisplayAs() == "ObjectGroup") {
            return NumberAwareStringCompare(a->name, b->name);
        }
        else {
            if (ascending)
                return -1;
            else
                return 1;
        }
    }
    else if (b->GetDisplayAs() == "ObjectGroup") {
        if (ascending)
            return 1;
        else
            return -1;
    }

    // Dont sort things with parents
    auto parent1 = treelist->GetItemParent(item1);
    auto parent2 = treelist->GetItemParent(item2);
    auto root = treelist->GetRootItem();
    if ((parent1 != root || parent2 != root) && parent1 == parent2)
    {
        int ia = data1->nativeOrder;
        int ib = data2->nativeOrder;
        if (ia > ib)
        {
            if (ascending)
            {
                return 1;
            }
            return -1;
        }
        if (ia < ib)
        {
            if (ascending)
            {
                return -1;
            }
            return 1;
        }
    }

    return NumberAwareStringCompare(a->name, b->name);
}

int ViewObjectPanel::ObjectListComparator::Compare(wxTreeListCtrl *treelist, unsigned column, wxTreeListItem first, wxTreeListItem second)
{
    return SortElementsFunction(treelist, first, second, column);
}

void ViewObjectPanel::PreviewObjectAlignWithGround()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if (view_object->GroupSelected || view_object->Selected)
        {
            view_object->SetBottom(0.0f);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignWithGround");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignWithGround");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignWithGround", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignTops()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float top = mSelectedObject->GetTop();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if(view_object->GroupSelected)
        {
            view_object->SetTop(top);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignTops");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignTops");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignTops", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignBottoms()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float bottom = mSelectedObject->GetBottom();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if(view_object->GroupSelected)
        {
            view_object->SetBottom(bottom);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignBottoms");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignBottoms");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignBottoms", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignLeft()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float left = mSelectedObject->GetLeft();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if(view_object->GroupSelected)
        {
            view_object->SetLeft(left);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignLeft");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignLeft");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignLeft", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignFronts()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float front = mSelectedObject->GetFront();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if (view_object->GroupSelected)
        {
            view_object->SetFront(front);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignFronts");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignFronts");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignFronts", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignBacks()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float back = mSelectedObject->GetBack();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if (view_object->GroupSelected)
        {
            view_object->SetBack(back);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignBacks");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignBacks");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignBacks", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectResize(bool sameWidth, bool sameHeight)
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);

    if (sameWidth)
    {
        int width = mSelectedObject->GetWidth();
        for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
            ViewObject *view_object = it->second;
            if (view_object->GroupSelected)
            {
                view_object->SetWidth(width);
                bool z_scale = view_object->GetBaseObjectScreenLocation().GetSupportsZScaling();
                if (z_scale) {
                    view_object->GetBaseObjectScreenLocation().SetMDepth(width);
                }
            }
        }
    }

    if (sameHeight)
    {
        int height = mSelectedObject->GetHeight();
        for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
            ViewObject *view_object = it->second;
            if (view_object->GroupSelected)
            {
                view_object->SetHeight(height);
            }
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectResize");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectResize");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectResize", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignRight()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float right = mSelectedObject->GetRight();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if(view_object->GroupSelected)
        {
            view_object->SetRight(right);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignRight");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignRight");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignRight", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignHCenter()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float center = mSelectedObject->GetHcenterPos();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if(view_object->GroupSelected)
        {
            view_object->SetHcenterPos(center);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignHCenter");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignHCenter");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignHCenter", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignVCenter()
{
    if (mSelectedObject == nullptr) return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float center = mSelectedObject->GetVcenterPos();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if(view_object->GroupSelected)
        {
            view_object->SetVcenterPos(center);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignVCenter");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignVCenter");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignVCenter", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectAlignDCenter()
{
    if (mSelectedObject == nullptr)
        return;

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);
    float center = mSelectedObject->GetDcenterPos();
    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject* view_object = it->second;
        if (view_object->GroupSelected) {
            view_object->SetDcenterPos(center);
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectAlignDCenter");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectAlignDCenter");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectAlignDCenter", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

bool SortObjectX(const ViewObject* first, const ViewObject* second)
{
    float firstmodelX = first->GetBaseObjectScreenLocation().GetHcenterPos();
    float secondmodelX = second->GetBaseObjectScreenLocation().GetHcenterPos();

    return firstmodelX < secondmodelX;
}

bool SortObjectY(const ViewObject* first, const ViewObject* second)
{
    float firstmodelY = first->GetBaseObjectScreenLocation().GetVcenterPos();
    float secondmodelY = second->GetBaseObjectScreenLocation().GetVcenterPos();

    return firstmodelY < secondmodelY;
}

bool SortObjectZ(const ViewObject* first, const ViewObject* second)
{
    float firstmodelZ = first->GetBaseObjectScreenLocation().GetDcenterPos();
    float secondmodelZ = second->GetBaseObjectScreenLocation().GetDcenterPos();

    return firstmodelZ < secondmodelZ;
}

void ViewObjectPanel::PreviewObjectHDistribute()
{
    int count = 0;
    float minx = 999999;
    float maxx = -999999;

    std::list<ViewObject*> objects;

    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if (view_object->GroupSelected || view_object->Selected)
        {
            count++;
            float x = view_object->GetHcenterPos();

            if (x < minx) minx = x;
            if (x > maxx) maxx = x;
            objects.push_back(view_object);
        }
    }

    if (count <= 2) return;

    objects.sort(SortObjectX);

    float space = (maxx - minx) / (count - 1);

    layoutPanel->CreateUndoPoint("All", objects.front()->name);

    float x = -1;
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        if (it == objects.begin())
        {
            x = (*it)->GetHcenterPos() + space;
        }
        else if (*it == objects.back())
        {
            // do nothing
        }
        else
        {
            (*it)->SetHcenterPos(x);
            x += space;
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectHDistribute");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectHDistribute");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectHDistribute", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectVDistribute()
{
    int count = 0;
    float miny = 999999;
    float maxy = -999999;

    std::list<ViewObject*> objects;

    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        if (view_object->GroupSelected || view_object->Selected)
        {
            count++;
            float y = view_object->GetVcenterPos();

            if (y < miny) miny = y;
            if (y > maxy) maxy = y;
            objects.push_back(view_object);
        }
    }

    if (count <= 2) return;

    objects.sort(SortObjectY);

    float space = (maxy - miny) / (count - 1);

    layoutPanel->CreateUndoPoint("All", objects.front()->name);

    float y = -1;
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        if (it == objects.begin())
        {
            y = (*it)->GetVcenterPos() + space;
        }
        else if (*it == objects.back())
        {
            // do nothing
        }
        else
        {
            (*it)->SetVcenterPos(y);
            y += space;
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectVDistribute");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectVDistribute");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectVDistribute", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}
void ViewObjectPanel::PreviewObjectDDistribute()
{
    int count = 0;
    float minz = 999999;
    float maxz = -999999;

    std::list<ViewObject*> objects;

    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject* view_object = it->second;
        if (view_object->GroupSelected || view_object->Selected) {
            count++;
            float z = view_object->GetDcenterPos();

            if (z < minz)
                minz = z;
            if (z > maxz)
                maxz = z;
            objects.push_back(view_object);
        }
    }

    if (count <= 2)
        return;

    objects.sort(SortObjectZ);

    float space = (maxz - minz) / (count - 1);

    layoutPanel->CreateUndoPoint("All", objects.front()->name);

    float z = -1;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (it == objects.begin()) {
            z = (*it)->GetDcenterPos() + space;
        } else if (*it == objects.back()) {
            // do nothing
        } else {
            (*it)->SetDcenterPos(z);
            z += space;
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectDDistribute");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectDDistribute");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectDDistribute", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectFlipV() {
    if (mSelectedObject == nullptr) {
        return;
    }

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);

    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject* view_object = it->second;
        if (view_object->Selected) {
            view_object->FlipVertical();
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectFlipVertical");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectFlipVertical");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectFlipVertical", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::PreviewObjectFlipH() {
    if (mSelectedObject == nullptr) {
        return;
    }

    layoutPanel->CreateUndoPoint("All", mSelectedObject->name);

    for (auto it = layoutPanel->xlights->AllObjects.begin(); it != layoutPanel->xlights->AllObjects.end(); ++it) {
        ViewObject* view_object = it->second;
        if (view_object->Selected) {
            view_object->FlipHorizontal();
        }
    }
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ViewObjectPanel::PreviewObjectFlipHorizontal");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ViewObjectPanel::PreviewObjectFlipHorizontal");
    layoutPanel->xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ViewObjectPanel::PreviewObjectFlipHorizontal", nullptr, nullptr, layoutPanel->GetSelectedModelName());
}

void ViewObjectPanel::DoCut(wxCommandEvent& event)
{
	layoutPanel->DoCut(event);
}

void ViewObjectPanel::DoCopy(wxCommandEvent& event)
{
	layoutPanel->DoCopy(event);
}

void ViewObjectPanel::DoPaste(wxCommandEvent& event)
{
	layoutPanel->DoPaste(event);
}

void ViewObjectPanel::DoUndo(wxCommandEvent& event)
{
	layoutPanel->DoUndo(event);
}

void ViewObjectPanel::OnCharHook(wxKeyEvent& event)
{
	layoutPanel->OnListCharHook(event);
}

void ViewObjectPanel::OnChar(wxKeyEvent& event)
{
	layoutPanel->OnChar(event);
}
