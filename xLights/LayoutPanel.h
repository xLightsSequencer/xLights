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
#include <wx/treectrl.h>
#include <wx/xml/xml.h>
#include <glm/glm.hpp>

#include "ControllerConnectionDialog.h"

#include <vector>
#include <list>
#include <map>

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
class impTreeItemData;
class Motion3DEvent;

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
    std::string _lastSelProp = ""; // last selected property

    public:

		LayoutPanel(wxWindow* parent, xLightsFrame *xlights, wxPanel* sequencer);
		virtual ~LayoutPanel();

		friend class ViewObjectPanel;

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

    private:

		wxScrolledWindow* ViewObjectWindow = nullptr;
		wxScrolledWindow* ModelGroupWindow = nullptr;
		wxTreeListCtrl* TreeListViewModels = nullptr;

	protected:

		//(*Identifiers(LayoutPanel)
		static const long ID_PANEL4;
		static const long ID_PANEL_Objects;
		static const long ID_NOTEBOOK_OBJECTS;
		static const long ID_PANEL3;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_CHECKBOX_3D;
		static const long ID_CHECKBOXOVERLAP;
		static const long ID_BUTTON_SAVE_PREVIEW;
		static const long ID_PANEL5;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_PREVIEWS;
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
        static const long ID_PREVIEW_MODEL_UNLINKFROMBASE;
        static const long ID_PREVIEW_MODEL_EXPORTASCUSTOM;
        static const long ID_PREVIEW_MODEL_EXPORTASCUSTOM3D;
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
        static const long ID_PREVIEW_BULKEDIT_PIXELSIZE;
        static const long ID_PREVIEW_BULKEDIT_PIXELSTYLE;
        static const long ID_PREVIEW_BULKEDIT_TRANSPARENCY;
        static const long ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL;
        static const long ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT;
        static const long ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE;
        static const long ID_PREVIEW_BULKEDIT_PREVIEW;
        static const long ID_PREVIEW_BULKEDIT_DIMMINGCURVES;
        static const long ID_PREVIEW_ALIGN_TOP;
        static const long ID_PREVIEW_ALIGN_BOTTOM;
        static const long ID_PREVIEW_ALIGN_GROUND;
        static const long ID_PREVIEW_ALIGN_LEFT;
        static const long ID_PREVIEW_ALIGN_RIGHT;
        static const long ID_PREVIEW_ALIGN_H_CENTER;
        static const long ID_PREVIEW_ALIGN_V_CENTER;
        static const long ID_PREVIEW_ALIGN_D_CENTER;
        static const long ID_PREVIEW_ALIGN_FRONT;
        static const long ID_PREVIEW_ALIGN_BACK;
        static const long ID_PREVIEW_DISTRIBUTE;
        static const long ID_PREVIEW_H_DISTRIBUTE;
        static const long ID_PREVIEW_V_DISTRIBUTE;
        static const long ID_PREVIEW_D_DISTRIBUTE;
        static const long ID_PREVIEW_RESIZE;
        static const long ID_PREVIEW_RESIZE_SAMEWIDTH;
        static const long ID_PREVIEW_RESIZE_SAMEHEIGHT;
        static const long ID_PREVIEW_RESIZE_SAMESIZE;
        static const long ID_PREVIEW_DELETE_ACTIVE;
        static const long ID_PREVIEW_RENAME_ACTIVE;
        static const long ID_PREVIEW_MODEL_ADDPOINT;
        static const long ID_PREVIEW_MODEL_DELETEPOINT;
        static const long ID_PREVIEW_MODEL_ADDCURVE;
        static const long ID_PREVIEW_MODEL_DELCURVE;
        static const long ID_PREVIEW_SAVE_LAYOUT_IMAGE;
        static const long ID_PREVIEW_PRINT_LAYOUT_IMAGE;
        static const long ID_PREVIEW_SAVE_VIEWPOINT;
        static const long ID_PREVIEW_VIEWPOINT_DEFAULT;
        static const long ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE;
        static const long ID_PREVIEW_VIEWPOINT2D;
        static const long ID_PREVIEW_VIEWPOINT3D;
        static const long ID_PREVIEW_DELETEVIEWPOINT2D;
        static const long ID_PREVIEW_DELETEVIEWPOINT3D;
        static const long ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS;
        static const long ID_PREVIEW_IMPORT_MODELS_FROM_LORS5;
        static const long ID_ADD_OBJECT_IMAGE;
        static const long ID_ADD_OBJECT_GRIDLINES;
        static const long ID_ADD_OBJECT_TERRIAN;
        static const long ID_ADD_OBJECT_RULER;
        static const long ID_ADD_OBJECT_MESH;
        static const long ID_ADD_DMX_MOVING_HEAD;
        static const long ID_ADD_DMX_GENERAL;
        static const long ID_ADD_DMX_MOVING_HEAD_3D;
        static const long ID_ADD_DMX_SERVO;
        static const long ID_ADD_DMX_SERVO_3D;
        static const long ID_ADD_DMX_SKULL;
        static const long ID_ADD_DMX_FLOODLIGHT;
        static const long ID_ADD_DMX_FLOODAREA;
        static const long ID_PREVIEW_MODEL_CAD_EXPORT;
        static const long ID_PREVIEW_LAYOUT_DXF_EXPORT;
        static const long ID_PREVIEW_FLIP_HORIZONTAL;
        static const long ID_PREVIEW_FLIP_VERTICAL;

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
		void OnPreviewRotateGesture(wxRotateGestureEvent& event);
		void OnPreviewZoomGesture(wxZoomGestureEvent& event);
		void OnNotebook_ObjectsPageChanged(wxNotebookEvent& event);
		//*)

        void OnPreviewMotion3DButtonEvent(wxCommandEvent &event);
        void OnPreviewMotion3D(Motion3DEvent &event);
        void OnPropertyGridSelection(wxPropertyGridEvent& event);
        void OnPropertyGridItemCollapsed(wxPropertyGridEvent& event);
        void OnPropertyGridItemExpanded(wxPropertyGridEvent& event);
        void OnPropertyGridRightClick(wxPropertyGridEvent& event);
        void OnPropertyGridContextMenu(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()

        void DoCopy(wxCommandEvent& event);
        void DoCut(wxCommandEvent& event);
        void DoPaste(wxCommandEvent& event);
        void DoUndo(wxCommandEvent& event);
        void RemoveSelectedModelsFromGroup();
        void DeleteSelectedModels();
		void DeleteSelectedObject();
        void DeleteSelectedGroups();
        void LockSelectedModels(bool lock);
        void UnlinkSelectedModels();
        void PreviewSaveImage();
        void PreviewPrintImage();
        void ImportModelsFromRGBEffects();
        void ImportModelsFromLORS5();

    public:
        bool IsNewModel(Model* m) const;
        void ClearUndo() { undoBuffer.clear(); }
        bool SaveEffects();
        void UpdatePreview();
        void SelectBaseObject(const std::string & name, bool highlight_tree = true);
        void SelectBaseObject(BaseObject *base_object, bool highlight_tree = true);
        void SelectModel(const std::string & name, bool highlight_tree = true);
        void SelectModelGroupModels(ModelGroup* m, std::list<ModelGroup*>& processed);
        void SelectModel(Model *model, bool highlight_tree = true);
        void UnSelectAllModels(bool addBkgProps = true );
        void showBackgroundProperties();
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
        void SetDisplay2DGridSpacing(bool grid, long spacing);
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
    
        int calculateNodeCountOfSelected();

    protected:
        void FreezeTreeListView();
        void ThawTreeListView();
        void SetTreeListViewItemText(wxTreeListItem &item, int col, const wxString &txt);

        void SaveModelsListColumns();
        std::string TreeModelName(const Model* model, bool fullname);
        NewModelBitmapButton* AddModelButton(const std::string &type, const char *imageData[]);
        void UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models, bool filtering );
        void CreateModelGroupFromSelected();
        void AddSelectedToExistingGroups();
        void BulkEditControllerName();
        void BulkEditActive(bool active);
        void BulkEditTagColour();
        void BulkEditPixelSize();
        void BulkEditPixelStyle();
        void BulkEditTransparency();
        void BulkEditBlackTranparency();   
        void BulkEditControllerConnection(int type);
        void BulkEditControllerPreview();
        void BulkEditDimmingCurves();
        void ReplaceModel();
        void ShowNodeLayout();
        void EditSubmodels();
        void EditFaces();
        void EditStates();
        void EditModelData();
        void ShowWiring();
        void ExportModelAsCAD();
        void ExportLayoutDXF();
        bool IsAllSelectedModelsArePixelProtocol() const;
        void AddSingleModelOptionsToBaseMenu(wxMenu &menu);
        void AddBulkEditOptionsToMenu(wxMenu* bulkEditMenu);
        void AddAlignOptionsToMenu(wxMenu* mnuAlign);
        void AddDistributeOptionsToMenu(wxMenu* mnuDistribute);
        void AddResizeOptionsToMenu(wxMenu* mnuResize);
        Model* SelectSingleModel(int x,int y);
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
        Model* GetModelFromTreeItem(wxTreeListItem treeItem);
        wxTreeListItem GetTreeItemFromModel(Model* model);
        std::vector<Model*> GetSelectedModelsFromGroup(wxTreeListItem groupItem, bool nested = true);
        std::vector<Model*> GetSelectedModelsForEdit();
        void SetTreeModelSelected(Model* model, bool isPrimary);
        void SetTreeGroupModelsSelected(Model* model, bool isPrimary);
        void SetTreeSubModelSelected(Model* model, bool isPrimary);
        void CheckModelForOverlaps(Model* model);
        std::vector<std::list<std::string>> GetSelectedTreeModelPaths();
        std::list<std::string> GetTreeItemPath(wxTreeListItem item);
        wxTreeListItem GetTreeItemBranch(wxTreeListItem parent, std::string branchName);
        void ReselectTreeModels(std::vector<std::list<std::string>> modelPaths);
        void SelectModelInTree(Model* modelToSelect);
        void SelectBaseObjectInTree(BaseObject* baseObjectToSelect);
        void UnSelectModelInTree(Model* modelToUnSelect);
        void UnSelectBaseObjectInTree(BaseObject* baseObjectToUnSelect);
        void UnSelectAllModelsInTree();
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
        void PreviewModelAlignDCenter();
        void PreviewModelHDistribute();
        void PreviewModelVDistribute();
        void PreviewModelDDistribute();
        void PreviewModelResize(bool sameWidth, bool sameHeight);
        void PreviewModelFlipV();
        void PreviewModelFlipH();
        Model *CreateNewModel(const std::string &type) const;

        bool _firstTreeLoad = true;
        bool m_dragging = false;
        bool m_creating_bound_rect = false;
        int m_bound_start_x = 0;
        int m_bound_start_y = 0;
        int m_bound_end_x = 0;
        int m_bound_end_y = 0;
        int m_over_handle = -1;
        bool m_moving_handle = false;
        bool m_wheel_down = false;
        bool m_polyline_active = false;
        int m_previous_mouse_x = 0;
        int m_previous_mouse_y = 0;
		int mPointSize = 2;
        int mHitTestNextSelectModelIndex = 0;
        int mNumGroups = 0;
        bool mPropGridActive = true;
        wxTreeListItems selectedTreeGroups;
        wxTreeListItems selectedTreeModels;
        wxTreeListItems selectedTreeSubModels;

        wxPropertyGrid *propertyEditor = nullptr;
        bool updatingProperty = false;
        BaseObject *selectedBaseObject = nullptr;
        BaseObject *highlightedBaseObject = nullptr;
        wxTreeListItem selectedPrimaryTreeItem = nullptr;
        bool selectionLatched = false;
        int over_handle = -1;
        glm::vec3 last_centerpos = {0,0,0};
        glm::vec3 last_worldrotate = {0,0,0};
        glm::vec3 last_worldscale = {0,0,0};

        void clearPropGrid();
        bool stringPropsVisible = false;
        bool controllerConnectionVisible = true;
        bool appearanceVisible = false;
        bool sizeVisible = false;
        bool dimensionsVisible = false;
        bool colSizesSet = false;
        bool layersVisible = false;
        std::vector<NewModelBitmapButton*> buttons;
        NewModelBitmapButton *selectedButton = nullptr;
        NewModelBitmapButton *obj_button = nullptr;
        std::string _lastXlightsModel = "";
        std::string selectedDmxModelType;
        Model *_newModel = nullptr;
        ModelGroupPanel *model_grp_panel = nullptr;
        ViewObjectPanel *objects_panel = nullptr;
        std::string currentLayoutGroup = "Default";
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
        xLightsFrame *xlights = nullptr;
        void UpdateModelList(bool full_refresh);
        void UpdateModelList(bool full_refresh, std::vector<Model*> &modelList);
        void RefreshLayout();
        void RenderLayout();
        std::string GetSelectedModelName() const;
        bool Is3d() const;
        void Set3d(bool is3d);
        wxPropertyGrid* GetPropertyEditor() const { return propertyEditor; }

    private:
        int Col_Model = 0;
        int Col_StartChan = 1;
        int Col_EndChan = 2;
        int Col_ControllerConnection = 3;

        ModelPreview *modelPreview = nullptr;
        wxImage *background = nullptr;
        wxString backgroundFile = "";
        wxString previewBackgroundFile;
        bool previewBackgroundScaled = false;
        int previewBackgroundBrightness = 100;
        int previewBackgroundAlpha = 100;
        wxPanel* main_sequencer = nullptr;
        wxVector<wxBitmapBundle> m_imageList;

        bool editing_models = true;
        bool is_3d = false;
        bool m_mouse_down = false;
        BaseObject* last_selection = nullptr;
        BaseObject* last_highlight = nullptr;
        int m_last_mouse_x = 0;
        int m_last_mouse_y = 0;
        bool creating_model =  false;
        bool mouse_state_set = false;

        void OnSelectionChanged(wxTreeListEvent& event);
        void HandleSelectionChanged();
        void OnItemContextMenu(wxTreeListEvent& event);

        static const long ID_MNU_REMOVE_MODEL_FROM_GROUP;
        static const long ID_MNU_DELETE_MODEL;
        static const long ID_MNU_DELETE_MODEL_GROUP;
        static const long ID_MNU_DELETE_EMPTY_MODEL_GROUPS;
        static const long ID_MNU_RENAME_MODEL_GROUP;
        static const long ID_MNU_CLONE_MODEL_GROUP;
        static const long ID_MNU_MAKESCVALID;
        static const long ID_MNU_MAKEALLSCVALID;
        static const long ID_MNU_MAKEALLSCNOTOVERLAPPING;
        static const long ID_MNU_ADD_MODEL_GROUP;
        static const long ID_MNU_ADD_TO_EXISTING_GROUPS;
        void OnModelsPopup(wxCommandEvent& event);
        LayoutGroup* GetLayoutGroup(const std::string& name);
		const wxString& GetBackgroundImageForSelectedPreview();
        void SwitchChoiceToCurrentLayoutGroup();
        void DeleteCurrentPreview();
        void RenameCurrentPreview();
        void ShowPropGrid(bool show);
        void SetCurrentLayoutGroup(const std::string& group);
        void FinalizeModel();
        void SelectBaseObject3D();
        void ProcessLeftMouseClick3D(wxMouseEvent& event);
        wxTreeListCtrl* CreateTreeListCtrl(long style, wxPanel* panel);
        int AddModelToTree(Model *model, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName = false);
        void RenameModelInTree(Model* model, const std::string& new_name);
        void DisplayAddObjectPopup();
        void OnAddObjectPopup(wxCommandEvent& event);
        void AddObjectButton(wxMenu& mnu, const long id, const std::string &name, const char *icon[]);
        void DisplayAddDmxPopup();
        void OnAddDmxPopup(wxCommandEvent& event);
        void SelectViewObject(ViewObject *v, bool highlight_tree = true);
        void ImportModelsFromPreview(std::list<impTreeItemData*> models, wxString const& layoutGroup, bool includeEmptyGroups);
        int GetColumnIndex(const std::string& name) const;

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
        bool zoom_gesture_active = false;
        bool rotate_gesture_active = false;
};
