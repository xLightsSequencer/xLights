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

//(*Headers(LayoutPanel)
#include <wx/panel.h>
class wxButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxNotebook;
class wxNotebookEvent;
class wxScrollBar;
class wxSplitterEvent;
class wxSplitterWindow;
class wxStaticText;
//*)

#include "wxCheckedListCtrl.h"
#include <wx/treelist.h>
#include <wx/xml/xml.h>
#include <glm/glm.hpp>

#include "ControllerConnectionDialog.h"

#include <vector>
#include <list>

class xLightsFrame;
class ModelPreview;
class BaseObject;
class Model;
class ModelGroup;
class ModelGroupPanel;
class ViewObjectPanel;
class ViewObject;
class wxListEvent;
class wxMouseEvent;
class wxPropertyGrid;
class wxPropertyGridEvent;
class NewModelBitmapButton;
class wxImageFileProperty;
class wxScrolledWindow;
class LayoutGroup;
class wxStringInputStream;

wxDECLARE_EVENT(EVT_LISTITEM_CHECKED, wxCommandEvent);

class CopyPasteBaseObject
{
    bool _ok;
	bool _viewObject;
    wxXmlNode* _xmlNode;

public:
    CopyPasteBaseObject(const std::string& in);
    CopyPasteBaseObject();
    virtual ~CopyPasteBaseObject();
    bool IsOk() const { return _ok; }
	bool IsViewObject() const { return _viewObject; }
    wxXmlNode* GetBaseObjectXml() const
    {
        if (_xmlNode == nullptr)
            return _xmlNode;
        else
            // we return a new copy assuming the recipient will delete it
            return new wxXmlNode(*_xmlNode);
    }
    void SetBaseObject(BaseObject* model);
    std::string Serialise() const;
};

class LayoutPanel: public wxPanel
{
    public:

		LayoutPanel(wxWindow* parent, xLightsFrame *xlights, wxPanel* sequencer);
		virtual ~LayoutPanel();

		friend class ViewObjectPanel;

    private:
		//(*Declarations(LayoutPanel)
		wxButton* ButtonSavePreview;
		wxCheckBox* CheckBoxOverlap;
		wxCheckBox* CheckBox_3D;
		wxChoice* ChoiceLayoutGroups;
		wxFlexGridSizer* ToolSizer;
		wxNotebook* Notebook_Objects;
		wxPanel* FirstPanel;
		wxPanel* LeftPanel;
		wxPanel* PanelModels;
		wxPanel* PanelObjects;
		wxPanel* PreviewGLPanel;
		wxPanel* SecondPanel;
		wxScrollBar* ScrollBarLayoutHorz;
		wxScrollBar* ScrollBarLayoutVert;
		wxSplitterWindow* ModelSplitter;
		wxSplitterWindow* SplitterWindow2;
		wxStaticText* StaticText1;
		//*)

		wxScrolledWindow* ViewObjectWindow;
		wxScrolledWindow* ModelGroupWindow;
		wxTreeListCtrl* TreeListViewModels;

	protected:

		//(*Identifiers(LayoutPanel)
		static const long ID_PANEL4;
		static const long ID_PANEL_Objects;
		static const long ID_NOTEBOOK_OBJECTS;
		static const long ID_PANEL3;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_CHECKBOXOVERLAP;
		static const long ID_BUTTON_SAVE_PREVIEW;
		static const long ID_PANEL5;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_PREVIEWS;
		static const long ID_CHECKBOX_3D;
		static const long ID_SCROLLBAR1;
		static const long ID_SCROLLBAR2;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW2;
		//*)

		static const long ID_TREELISTVIEW_MODELS;
        static const long ID_PREVIEW_REPLACEMODEL;
        static const long ID_PREVIEW_RESET;
        static const long ID_PREVIEW_ALIGN;
        static const long ID_PREVIEW_MODEL_NODELAYOUT;
        static const long ID_PREVIEW_MODEL_LOCK;
        static const long ID_PREVIEW_MODEL_UNLOCK;
        static const long ID_PREVIEW_MODEL_EXPORTASCUSTOM;
        static const long ID_PREVIEW_MODEL_CREATEGROUP;
        static const long ID_PREVIEW_MODEL_WIRINGVIEW;
        static const long ID_PREVIEW_MODEL_ASPECTRATIO;
        static const long ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL;
        static const long ID_PREVIEW_BULKEDIT;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERNAME;
        static const long ID_PREVIEW_BULKEDIT_SETACTIVE;
        static const long ID_PREVIEW_BULKEDIT_SETINACTIVE;
        static const long ID_PREVIEW_BULKEDIT_SMARTREMOTE;
        static const long ID_PREVIEW_BULKEDIT_TAGCOLOUR;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERNULLNODES;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT;
        static const long ID_PREVIEW_BULKEDIT_PREVIEW;
        static const long ID_PREVIEW_BULKEDIT_DIMMINGCURVES;
        static const long ID_PREVIEW_ALIGN_TOP;
        static const long ID_PREVIEW_ALIGN_BOTTOM;
        static const long ID_PREVIEW_ALIGN_GROUND;
        static const long ID_PREVIEW_ALIGN_LEFT;
        static const long ID_PREVIEW_ALIGN_RIGHT;
        static const long ID_PREVIEW_ALIGN_H_CENTER;
        static const long ID_PREVIEW_ALIGN_V_CENTER;
        static const long ID_PREVIEW_ALIGN_FRONT;
        static const long ID_PREVIEW_ALIGN_BACK;
        static const long ID_PREVIEW_DISTRIBUTE;
        static const long ID_PREVIEW_H_DISTRIBUTE;
        static const long ID_PREVIEW_V_DISTRIBUTE;
        static const long ID_PREVIEW_RESIZE;
        static const long ID_PREVIEW_RESIZE_SAMEWIDTH;
        static const long ID_PREVIEW_RESIZE_SAMEHEIGHT;
        static const long ID_PREVIEW_RESIZE_SAMESIZE;
        static const long ID_PREVIEW_DELETE_ACTIVE;
        static const long ID_PREVIEW_MODEL_ADDPOINT;
        static const long ID_PREVIEW_MODEL_DELETEPOINT;
        static const long ID_PREVIEW_MODEL_ADDCURVE;
        static const long ID_PREVIEW_MODEL_DELCURVE;
        static const long ID_PREVIEW_SAVE_LAYOUT_IMAGE;
        static const long ID_PREVIEW_PRINT_LAYOUT_IMAGE;
        static const long ID_PREVIEW_SAVE_VIEWPOINT;
        static const long ID_PREVIEW_VIEWPOINT2D;
        static const long ID_PREVIEW_VIEWPOINT3D;
        static const long ID_PREVIEW_DELETEVIEWPOINT2D;
        static const long ID_PREVIEW_DELETEVIEWPOINT3D;
        static const long ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS;
        static const long ID_ADD_OBJECT_IMAGE;
        static const long ID_ADD_OBJECT_GRIDLINES;
        static const long ID_ADD_OBJECT_MESH;
        static const long ID_ADD_DMX_MOVING_HEAD;
        static const long ID_ADD_DMX_MOVING_HEAD_3D;
        static const long ID_ADD_DMX_SERVO;
        static const long ID_ADD_DMX_SERVO_3D;
        static const long ID_ADD_DMX_SKULL;
        static const long ID_ADD_DMX_FLOODLIGHT;
        static const long ID_ADD_DMX_FLOODAREA;

	public:

		//(*Handlers(LayoutPanel)
		void OnPreviewLeftUp(wxMouseEvent& event);
		void OnPreviewMouseLeave(wxMouseEvent& event);
		void OnPreviewLeftDown(wxMouseEvent& event);
		void OnPreviewLeftDClick(wxMouseEvent& event);
		void OnPreviewRightDown(wxMouseEvent& event);
		void OnPreviewMouseMove(wxMouseEvent& event);
		void OnPreviewMouseMove3D(wxMouseEvent& event);
		void OnPreviewMouseWheel(wxMouseEvent& event);
		void OnPreviewMouseWheelDown(wxMouseEvent& event);
		void OnPreviewMouseWheelUp(wxMouseEvent& event);
		void OnPreviewMagnify(wxMouseEvent& event);
		void OnPreviewModelPopup(wxCommandEvent &event);
		void OnCheckBoxOverlapClick(wxCommandEvent& event);
		void OnButtonSavePreviewClick(wxCommandEvent& event);
		void OnPropertyGridChange(wxPropertyGridEvent& event);
		void OnPropertyGridChanging(wxPropertyGridEvent& event);
		void OnModelSplitterSashPosChanged(wxSplitterEvent& event);
		void OnSplitterWindowSashPosChanged(wxSplitterEvent& event);
		void OnNewModelTypeButtonClicked(wxCommandEvent& event);
		void OnCharHook(wxKeyEvent& event);
		void OnChar(wxKeyEvent& event);
		void OnChoiceLayoutGroupsSelect(wxCommandEvent& event);
		void OnCheckBox_3DClick(wxCommandEvent& event);
		void OnCheckBox_SelectionClick(wxCommandEvent& event);
		void OnCheckBox_XZClick(wxCommandEvent& event);
		void OnPreviewRotateGesture(wxRotateGestureEvent& event);
		void OnPreviewZoomGesture(wxZoomGestureEvent& event);
		void OnChoice_InsertObjectSelect(wxCommandEvent& event);
		void OnChoice_EditModelObjectsSelect(wxCommandEvent& event);
		void OnNotebook1PageChanged(wxNotebookEvent& event);
		void OnNotebook_ObjectsPageChanged(wxNotebookEvent& event);
		//*)

        void OnPropertyGridSelection(wxPropertyGridEvent& event);
        void OnPropertyGridItemCollapsed(wxPropertyGridEvent& event);
        void OnPropertyGridItemExpanded(wxPropertyGridEvent& event);

		DECLARE_EVENT_TABLE()

        void DoCopy(wxCommandEvent& event);
        void DoCut(wxCommandEvent& event);
        void DoPaste(wxCommandEvent& event);
        void DoUndo(wxCommandEvent& event);
        void DeleteSelectedModel();
		void DeleteSelectedObject();
        void LockSelectedModels(bool lock);
        void PreviewSaveImage();
        void PreviewPrintImage();
        void ImportModelsFromRGBEffects();

    public:
        bool IsNewModel(Model* m) const;
        void ClearUndo() { undoBuffer.clear(); }
        void SaveEffects();
        void UpdatePreview();
        void SelectBaseObject(const std::string & name, bool highlight_tree = true);
        void SelectBaseObject(BaseObject *base_object, bool highlight_tree = true);
        void SelectModel(const std::string & name, bool highlight_tree = true);
        void SelectModelGroupModels(ModelGroup* m, std::list<ModelGroup*>& processed);
        void SelectModel(Model *model, bool highlight_tree = true);
        void UnSelectAllModels(bool addBkgProps = true );
        void SelectAllModels();
        void SetupPropGrid(BaseObject *model);
        void AddPreviewChoice(const std::string &name);
        ModelPreview* GetMainPreview() const {return modelPreview;}
        bool GetBackgroundScaledForSelectedPreview();
        int GetBackgroundBrightnessForSelectedPreview();
        int GetBackgroundAlphaForSelectedPreview();
        const std::string& GetCurrentLayoutGroup() const {return currentLayoutGroup;}
        void Reset();
        void SetDirtyHiLight(bool dirty);
        std::string GetCurrentPreview() const;
        void SetDisplay2DBoundingBox(bool bb);
        void SetDisplay2DCenter0(bool bb);
        void ReloadModelList();
        void refreshModelList();
        void refreshObjectList();
        void resetPropertyGrid();
        void updatePropertyGrid();
        void ClearSelectedModelGroup();

        void ModelGroupUpdated(ModelGroup *group, bool full_refresh);
        bool HandleLayoutKeyBinding(wxKeyEvent& event);

        void OnListCharHook(wxKeyEvent& event);
        ModelGroup* GetSelectedModelGroup() const;

    protected:
        void FreezeTreeListView();
        void ThawTreeListView();
        void SetTreeListViewItemText(wxTreeListItem &item, int col, const wxString &txt);

        std::string TreeModelName(const Model* model, bool fullname);
        NewModelBitmapButton* AddModelButton(const std::string &type, const char *imageData[]);
        void UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models, bool filtering );
        void CreateModelGroupFromSelected();
        void BulkEditControllerName();
        void BulkEditActive(bool active);
        void BulkEditTagColour();
        void BulkEditControllerConnection(int type);
        void BulkEditControllerPreview();
        void BulkEditDimmingCurves();
        void ReplaceModel();
        void ShowNodeLayout();
        void ShowWiring();
        bool IsAllSelectedModelsArePixelProtocol() const;

        bool SelectSingleModel(int x,int y);
        bool SelectMultipleModels(int x,int y);
        void SelectAllInBoundingRect(bool models_and_objects);
        void HighlightAllInBoundingRect(bool models_and_objects);
        void SetSelectedModelToGroupSelected();
        void Nudge(int key);

        int FindModelsClicked(int x,int y, std::vector<int> &found);
        void GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction);
        void SetMouseStateForModels(bool value);

        int ModelsSelectedCount() const;
        int ViewObjectsSelectedCount() const;
        int GetSelectedModelIndex() const;
        std::list<BaseObject*> GetSelectedBaseObjects() const;
        void PreviewModelAlignWithGround();
        void PreviewModelAlignTops();
        void PreviewModelAlignBottoms();
        void PreviewModelAlignLeft();
        void PreviewModelAlignRight();
        void PreviewModelAlignFronts();
        void PreviewModelAlignBacks();
        void PreviewModelAlignHCenter();
        void PreviewModelAlignVCenter();
        void PreviewModelHDistribute();
        void PreviewModelVDistribute();
        void PreviewModelResize(bool sameWidth, bool sameHeight);
        Model *CreateNewModel(const std::string &type) const;

        bool _firstTreeLoad;
        bool m_dragging;
        bool m_creating_bound_rect;
        int m_bound_start_x;
        int m_bound_start_y;
        int m_bound_end_x;
        int m_bound_end_y;
        int m_over_handle;
        bool m_moving_handle;
        bool m_wheel_down;
        bool m_polyline_active;
		int m_previous_mouse_x, m_previous_mouse_y;
		int mPointSize;
        int mHitTestNextSelectModelIndex;
        int mNumGroups;
        bool mPropGridActive;
        wxTreeListItem mSelectedGroup;

        wxPropertyGrid *propertyEditor = nullptr;
        bool updatingProperty;
        BaseObject *selectedBaseObject = nullptr;
        BaseObject *highlightedBaseObject = nullptr;
        bool selectionLatched;
        int over_handle;
        glm::vec3 last_centerpos;
        glm::vec3 last_worldrotate;
        glm::vec3 last_worldscale;

        void clearPropGrid();
        bool stringPropsVisible;
        bool controllerConnectionVisible;
        bool appearanceVisible;
        bool sizeVisible;
        bool colSizesSet;
        std::vector<NewModelBitmapButton*> buttons;
        NewModelBitmapButton *selectedButton = nullptr;
        NewModelBitmapButton *obj_button = nullptr;
        std::string _lastXlightsModel;
        std::string selectedDmxModelType;
        Model *_newModel = nullptr;
        ModelGroupPanel *model_grp_panel = nullptr;
        ViewObjectPanel *objects_panel = nullptr;
        std::string currentLayoutGroup;
        LayoutGroup* pGrp = nullptr;

        std::string lastModelName;

        class UndoStep {
        public:
            std::string type;
            std::string model;
            std::string key;
            std::string data;
            std::string models;
            std::string objects;
            std::string groups;
        };
        std::vector<UndoStep> undoBuffer;
        void CreateUndoPoint(const std::string &type, const std::string &model, const std::string &key = "", const std::string &data = "");
    public:
        xLightsFrame *xlights;
        void UpdateModelList(bool full_refresh);
        void UpdateModelList(bool full_refresh, std::vector<Model*> &modelList);
        void RefreshLayout();
        void RenderLayout();
        std::string GetSelectedModelName() const;
        bool Is3d() const;
        void Set3d(bool is3d);

    private:
        enum
        {
            Icon_File,
            Icon_FolderClosed,
            Icon_FolderOpened,
            Icon_Arches,
            Icon_CandyCane,
            Icon_Circle,
            Icon_ChannelBlock,
            Icon_Cube,
            Icon_Custom,
            Icon_Dmx,
            Icon_Icicle,
            Icon_Image,
            Icon_Line,
            Icon_Matrix,
            Icon_Poly,
            Icon_Sphere,
            Icon_Spinner,
            Icon_Star,
            Icon_SubModel,
            Icon_Tree,
            Icon_Window,
            Icon_Wreath
        };

        // Tree list columns.
        enum
        {
            Col_Model,
            Col_StartChan,
            Col_EndChan,
            Col_ControllerConnection
        };

        ModelPreview *modelPreview = nullptr;
        wxImage *background = nullptr;
        wxString backgroundFile;
        wxString previewBackgroundFile;
        bool previewBackgroundScaled;
        int previewBackgroundBrightness;
        int previewBackgroundAlpha;
        wxPanel* main_sequencer = nullptr;
        wxImageList* m_imageList = nullptr;

        bool editing_models;
        bool is_3d;
        bool m_mouse_down;
        BaseObject* last_selection = nullptr;
        BaseObject* last_highlight = nullptr;
        int m_last_mouse_x, m_last_mouse_y;
        bool creating_model;
        bool mouse_state_set;

        void OnSelectionChanged(wxTreeListEvent& event);
        void OnItemContextMenu(wxTreeListEvent& event);

        static const long ID_MNU_DELETE_MODEL;
        static const long ID_MNU_DELETE_MODEL_GROUP;
        static const long ID_MNU_DELETE_EMPTY_MODEL_GROUPS;
        static const long ID_MNU_RENAME_MODEL_GROUP;
        static const long ID_MNU_CLONE_MODEL_GROUP;
        static const long ID_MNU_MAKESCVALID;
        static const long ID_MNU_MAKEALLSCVALID;
        static const long ID_MNU_MAKEALLSCNOTOVERLAPPING;
        static const long ID_MNU_ADD_MODEL_GROUP;
        void OnModelsPopup(wxCommandEvent& event);
		LayoutGroup* GetLayoutGroup(const std::string &name);
		const wxString& GetBackgroundImageForSelectedPreview();
        void SwitchChoiceToCurrentLayoutGroup();
        void DeleteCurrentPreview();
        void ShowPropGrid(bool show);
        void SetCurrentLayoutGroup(const std::string& group);
        void FinalizeModel();
        void SelectBaseObject3D();
        void ProcessLeftMouseClick3D(wxMouseEvent& event);
        void InitImageList();
        wxTreeListCtrl* CreateTreeListCtrl(long style, wxPanel* panel);
        int GetModelTreeIcon(Model* model, bool open);
        int AddModelToTree(Model *model, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName = false);
        void RenameModelInTree(Model* model, const std::string& new_name);
        void DisplayAddObjectPopup();
        void OnAddObjectPopup(wxCommandEvent& event);
        void AddObjectButton(wxMenu& mnu, const long id, const std::string &name, const char *icon[]);
        void DisplayAddDmxPopup();
        void OnAddDmxPopup(wxCommandEvent& event);
        void SelectViewObject(ViewObject *v, bool highlight_tree = true);
        //int SortElementsFunction(wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn);

        class ModelListComparator : public wxTreeListItemComparator
        {
        public:
            ModelListComparator() { xlights = nullptr; };
            virtual ~ModelListComparator() {};
            virtual int Compare(wxTreeListCtrl *treelist, unsigned column, wxTreeListItem first, wxTreeListItem second) override;
            int SortElementsFunction(wxTreeListCtrl *treelist, wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn);
            void SetFrame(xLightsFrame* frame) {xlights = frame;}
       private:
            xLightsFrame* xlights = nullptr;
        };
        ModelListComparator comparator;
        bool zoom_gesture_active;
        bool rotate_gesture_active;

};
