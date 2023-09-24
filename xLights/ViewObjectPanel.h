#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(ViewObjectPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

#include <wx/treelist.h>

class ViewObjectManager;
class LayoutPanel;
class LayoutGroup;
class ViewObject;

class ViewObjectPanel: public wxPanel
{
public:

    ViewObjectPanel(wxWindow* parent, ViewObjectManager& Objects, LayoutPanel* xl,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~ViewObjectPanel();

    void UpdateObjectList(bool full_refresh, std::string& currentLayoutGroup);
    bool OnSelectionChanged(wxTreeListEvent& event, ViewObject** view_object, std::string& currentLayoutGroup);
    void OnPropertyGridChange(wxPropertyGrid *propertyEditor, wxPropertyGridEvent& event);
    void OnItemContextMenu(wxTreeListEvent& event);
    void HighlightObject(ViewObject* v);
    ViewObject* GetSelectedObject() { return mSelectedObject; }
    void refreshObjectList();

    void PreviewObjectAlignWithGround();
    void PreviewObjectAlignTops();
    void PreviewObjectAlignBottoms();
    void PreviewObjectAlignLeft();
    void PreviewObjectAlignFronts();
    void PreviewObjectAlignBacks();
    void PreviewObjectResize(bool sameWidth, bool sameHeight);
    void PreviewObjectAlignRight();
    void PreviewObjectAlignHCenter();
    void PreviewObjectAlignVCenter();
    void PreviewObjectAlignDCenter();
    void PreviewObjectHDistribute();
    void PreviewObjectVDistribute();
    void PreviewObjectDDistribute();
    void PreviewObjectFlipV();
    void PreviewObjectFlipH();
	void DeleteSelectedObject();
    void UnlinkSelectedObject();
    bool ObjectListHasFocus()
    {
        return TreeListViewObjects->HasFocus() || TreeListViewObjects->GetView()->HasFocus();
    };

    //(*Declarations(ViewObjectPanel)
    wxPanel* FirstPanel;
    //*)

protected:

    //(*Identifiers(ViewObjectPanel)
    static const long ID_TREELISTVIEW_PANEL;
    //*)

    static const long ID_TREELISTVIEW_OBJECTS;
    static const long ID_MNU_DELETE_OBJECT;
    static const long ID_MNU_UNLINKFROMBASE;

private:

    //(*Handlers(ViewObjectPanel)
    void OnChar(wxKeyEvent& event);
    //*)

	void OnCharHook(wxKeyEvent& event);

    DECLARE_EVENT_TABLE()

	void DoCopy(wxCommandEvent& event);
	void DoCut(wxCommandEvent& event);
	void DoPaste(wxCommandEvent& event);
	void DoUndo(wxCommandEvent& event);

    enum
    {
        Icon_File,
        Icon_FolderClosed,
        Icon_FolderOpened,
        Icon_Image,
        Icon_Poly
    };

    void InitImageList();
    wxTreeListCtrl* CreateTreeListCtrl(long style);
    int GetObjectTreeIcon(ViewObject* view_object, bool open);
    int AddObjectToTree(ViewObject *view_object, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName = false);
    void UpdateObjectList(bool full_refresh, std::vector<ViewObject*> &objects, std::string& currentLayoutGroup );
    void UpdateObjectsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<ViewObject *> &prev_objects, bool filtering);
    void RenameObjectInTree(ViewObject *view_object, const std::string new_name);
    void OnObjectsPopup(wxCommandEvent& event);

    LayoutPanel* layoutPanel;
    ViewObjectManager& mViewObjects;
    wxTreeListCtrl* TreeListViewObjects;
    wxVector<wxBitmapBundle> m_imageList;
    wxTreeListItem mSelectedGroup;
    ViewObject* mSelectedObject;

    class ObjectListComparator : public wxTreeListItemComparator
    {
    public:
        ObjectListComparator() {};
        virtual ~ObjectListComparator() {};
        virtual int Compare(wxTreeListCtrl *treelist, unsigned column, wxTreeListItem first, wxTreeListItem second) override;
        int SortElementsFunction(wxTreeListCtrl *treelist, wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn);
    private:
    };
    ObjectListComparator comparator;

};
