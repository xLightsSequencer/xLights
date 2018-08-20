#ifndef VIEWOBJECTPANEL_H
#define VIEWOBJECTPANEL_H

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
    void HighlightObject(ViewObject* v);

    //(*Declarations(ViewObjectPanel)
    wxPanel* FirstPanel;
    //*)

protected:

    //(*Identifiers(ViewObjectPanel)
    static const long ID_TREELISTVIEW_PANEL;
    //*)

    static const long ID_TREELISTVIEW_OBJECTS;

private:

    //(*Handlers(ViewObjectPanel)
    //*)

    DECLARE_EVENT_TABLE()

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
    void refreshObjectList();
    int GetObjectTreeIcon(ViewObject* view_object, bool open);
    int AddObjectToTree(ViewObject *view_object, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName = false);
    void UpdateObjectList(bool full_refresh, std::vector<ViewObject*> &objects, std::string& currentLayoutGroup );
    void UpdateObjectsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<ViewObject *> &prev_objects, bool filtering);

    LayoutPanel* layoutPanel;
    ViewObjectManager& mViewObjects;
    wxTreeListCtrl* TreeListViewObjects;
    wxImageList* m_imageList;
    wxTreeListItem mSelectedGroup;

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

#endif
