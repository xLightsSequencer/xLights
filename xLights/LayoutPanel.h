#ifndef LAYOUTPANEL_H
#define LAYOUTPANEL_H

//(*Headers(LayoutPanel)
#include <wx/panel.h>
class wxChoice;
class wxFlexGridSizer;
class wxButton;
class wxSplitterWindow;
class wxSplitterEvent;
class wxStaticText;
class wxCheckBox;
//*)

#include "wxCheckedListCtrl.h"
#include <wx/treelist.h>
#include <wx/xml/xml.h>

#include <vector>
#include <list>

class xLightsFrame;
class ModelPreview;
class Model;
class ModelGroup;
class ModelGroupPanel;
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

class CopyPasteModel
{
    bool _ok;
    wxXmlNode* _xmlNode;

public:
    CopyPasteModel(const std::string& in);
    CopyPasteModel();
    virtual ~CopyPasteModel();
    bool IsOk() const { return _ok; }
    wxXmlNode* GetModelXml() const
    {
        if (_xmlNode == nullptr) 
            return _xmlNode; 
        else 
            // we return a new copy assuming the recipient will delete it
            return new wxXmlNode(*_xmlNode);
    }
    void SetModel(Model* model);
    std::string Serialise() const;
};

class LayoutPanel: public wxPanel
{
    public:

		LayoutPanel(wxWindow* parent, xLightsFrame *xlights, wxPanel* sequencer);
		virtual ~LayoutPanel();

    private:
		//(*Declarations(LayoutPanel)
		wxCheckBox* CheckBoxOverlap;
		wxFlexGridSizer* ToolSizer;
		wxStaticText* StaticText1;
		wxButton* ButtonSavePreview;
		wxSplitterWindow* SplitterWindow2;
		wxChoice* ChoiceLayoutGroups;
		wxPanel* SecondPanel;
		wxPanel* PreviewGLPanel;
		wxPanel* FirstPanel;
		wxPanel* LeftPanel;
		wxSplitterWindow* ModelSplitter;
		//*)

		wxScrolledWindow* ModelGroupWindow;
		wxTreeListCtrl* TreeListViewModels;

	protected:

		//(*Identifiers(LayoutPanel)
		static const long ID_PANEL3;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_CHECKBOXOVERLAP;
		static const long ID_BUTTON_SAVE_PREVIEW;
		static const long ID_PANEL5;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_PREVIEWS;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW2;
		//*)

		static const long ID_TREELISTVIEW_MODELS;
        static const long ID_PREVIEW_REPLACEMODEL;
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
        static const long ID_PREVIEW_BULKEDIT_PREVIEW;
        static const long ID_PREVIEW_BULKEDIT_DIMMINGCURVES;
        static const long ID_PREVIEW_ALIGN_TOP;
        static const long ID_PREVIEW_ALIGN_BOTTOM;
        static const long ID_PREVIEW_ALIGN_LEFT;
        static const long ID_PREVIEW_ALIGN_RIGHT;
        static const long ID_PREVIEW_ALIGN_H_CENTER;
        static const long ID_PREVIEW_ALIGN_V_CENTER;
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

	public:

		//(*Handlers(LayoutPanel)
		void OnPreviewLeftUp(wxMouseEvent& event);
		void OnPreviewMouseLeave(wxMouseEvent& event);
		void OnPreviewLeftDown(wxMouseEvent& event);
		void OnPreviewRightDown(wxMouseEvent& event);
		void OnPreviewMouseMove(wxMouseEvent& event);
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
        void LockSelectedModels(bool lock);
        void PreviewSaveImage();
        void PreviewPrintImage();

    public:
        void SaveEffects();
        void UpdatePreview();
        void SelectModel(const std::string & name, bool highlight_tree = true);
        void SelectModel(Model *model, bool highlight_tree = true);
        void UnSelectAllModels(bool addBkgProps = true);
        void SetupPropGrid(Model *model);
        void AddPreviewChoice(const std::string &name);
        ModelPreview* GetMainPreview() const {return modelPreview;}
        bool GetBackgroundScaledForSelectedPreview();
        int GetBackgroundBrightnessForSelectedPreview();
        const std::string& GetCurrentLayoutGroup() const {return currentLayoutGroup;}
        void Reset();
        void SetDirtyHiLight(bool dirty);
        std::string GetCurrentPreview() const;

        void ModelGroupUpdated(ModelGroup *group, bool full_refresh);
        bool HandleLayoutKeyBinding(wxKeyEvent& event);

    protected:
        void AddModelButton(const std::string &type, const char *imageData[]);
        void UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models, bool filtering );
        void CreateModelGroupFromSelected();
        void BulkEditControllerConnection();
        void BulkEditControllerPreview();
        void BulkEditDimmingCurves();
        void ReplaceModel();
        void ShowNodeLayout();
        void ShowWiring();

        bool SelectSingleModel(int x,int y);
        bool SelectMultipleModels(int x,int y);
        void SelectAllInBoundingRect();
        void SetSelectedModelToGroupSelected();
        void Nudge(int key);

        int FindModelsClicked(int x,int y, std::vector<int> &found);

        int ModelsSelectedCount() const;
        int GetSelectedModelIndex() const;
        std::list<Model*> GetSelectedModels() const;
        void PreviewModelAlignTops();
        void PreviewModelAlignBottoms();
        void PreviewModelAlignLeft();
        void PreviewModelAlignRight();
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
        bool m_polyline_active;
        int m_previous_mouse_x, m_previous_mouse_y;
        int mPointSize;
        int mHitTestNextSelectModelIndex;
        int mNumGroups;
        bool mPropGridActive;
        wxTreeListItem mSelectedGroup;

        wxPropertyGrid *propertyEditor;
        bool updatingProperty;
        Model *selectedModel;

        void ReloadModelList();
        void refreshModelList();
        void resetPropertyGrid();
        void clearPropGrid();
        bool stringPropsVisible;
        bool appearanceVisible;
        bool sizeVisible;
        bool colSizesSet;
        std::vector<NewModelBitmapButton*> buttons;
        NewModelBitmapButton *selectedButton;
        std::string _lastXlightsModel;
        Model *newModel;
        ModelGroupPanel *model_grp_panel;
        std::string currentLayoutGroup;
        LayoutGroup* pGrp;

        std::string lastModelName;

        class UndoStep {
        public:
            std::string type;
            std::string model;
            std::string key;
            std::string data;
            std::string models;
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
            Icon_Custom,
            Icon_Dmx,
            Icon_Icicle,
            Icon_Image,
            Icon_Line,
            Icon_Matrix,
            Icon_Poly,
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
            Col_EndChan
        };

        ModelPreview *modelPreview;
        wxImage *background;
        wxString backgroundFile;
        wxString previewBackgroundFile;
        bool previewBackgroundScaled;
        int previewBackgroundBrightness;
        wxPanel* main_sequencer;
        wxImageList* m_imageList;
        bool ignore_next_event;

        void OnSelectionChanged(wxTreeListEvent& event);
        void OnItemContextMenu(wxTreeListEvent& event);

        static const long ID_MNU_DELETE_MODEL;
        static const long ID_MNU_DELETE_MODEL_GROUP;
        static const long ID_MNU_RENAME_MODEL_GROUP;
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
        void InitImageList();
        wxTreeListCtrl* CreateTreeListCtrl(long style);
        int GetModelTreeIcon(Model* model, bool open);
        int AddModelToTree(Model *model, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName = false);
        void RenameModelInTree(Model* model, const std::string new_name);
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
            xLightsFrame* xlights;
        };
        ModelListComparator comparator;

};

#endif
