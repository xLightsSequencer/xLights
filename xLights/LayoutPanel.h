#ifndef LAYOUTPANEL_H
#define LAYOUTPANEL_H

//(*Headers(LayoutPanel)
#include <wx/panel.h>
class wxSplitterWindow;
class wxCheckBox;
class wxSplitterEvent;
class wxStaticText;
class wxListCtrl;
class wxListView;
class wxFlexGridSizer;
class wxButton;
class wxChoice;
//*)

#include "wxCheckedListCtrl.h"

#include <vector>

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

wxDECLARE_EVENT(EVT_LISTITEM_CHECKED, wxCommandEvent);

#include <vector>

class LayoutPanel: public wxPanel
{
    public:

		LayoutPanel(wxWindow* parent, xLightsFrame *xlights, wxPanel* sequencer);
		virtual ~LayoutPanel();

    private:
		//(*Declarations(LayoutPanel)
		wxFlexGridSizer* ToolSizer;
		wxListView* ListBoxElementList;
		wxChoice* ChoiceLayoutGroups;
		wxCheckedListCtrl* ListBoxModelGroups;
		wxSplitterWindow* SplitterWindow2;
		wxPanel* LeftPanel;
		wxStaticText* StaticText1;
		wxCheckBox* CheckBoxOverlap;
		wxPanel* SecondPanel;
		wxButton* ButtonSavePreview;
		wxSplitterWindow* GroupSplitter;
		wxSplitterWindow* ModelSplitter;
		wxPanel* PreviewGLPanel;
		//*)

		wxScrolledWindow* ModelGroupWindow;

	protected:

		//(*Identifiers(LayoutPanel)
		static const long ID_CHECKLISTBOX_MODEL_GROUPS;
		static const long ID_LISTBOX_ELEMENT_LIST;
		static const long ID_SPLITTERWINDOW3;
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

        static const long ID_PREVIEW_ALIGN;
        static const long ID_PREVIEW_MODEL_NODELAYOUT;
        static const long ID_PREVIEW_MODEL_EXPORTCSV;
        static const long ID_PREVIEW_MODEL_EXPORTCUSTOMMODEL;
        static const long ID_PREVIEW_ALIGN_TOP;
        static const long ID_PREVIEW_ALIGN_BOTTOM;
        static const long ID_PREVIEW_ALIGN_LEFT;
        static const long ID_PREVIEW_ALIGN_RIGHT;
        static const long ID_PREVIEW_ALIGN_H_CENTER;
        static const long ID_PREVIEW_ALIGN_V_CENTER;
        static const long ID_PREVIEW_DISTRIBUTE;
        static const long ID_PREVIEW_H_DISTRIBUTE;
        static const long ID_PREVIEW_V_DISTRIBUTE;
        static const long ID_PREVIEW_DELETE_ACTIVE;

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
		void OnListBoxElementListItemSelect(wxListEvent& event);
		void OnListBoxElementListColumnClick(wxListEvent& event);
		void OnListBoxElementItemChecked(wxListEvent& event);
		void OnPropertyGridChange(wxPropertyGridEvent& event);
		void OnPropertyGridChanging(wxPropertyGridEvent& event);
		void OnModelSplitterSashPosChanged(wxSplitterEvent& event);
		void OnSplitterWindowSashPosChanged(wxSplitterEvent& event);
		void OnNewModelTypeButtonClicked(wxCommandEvent& event);
		void OnCharHook(wxKeyEvent& event);
		void OnChar(wxKeyEvent& event);
		void OnListBoxElementListItemRClick(wxListEvent& event);
		void OnListBoxModelGroupsItemSelect(wxListEvent& event);
		void OnListBoxModelGroupsItemFocused(wxListEvent& event);
		void OnListBoxModelGroupsItemActivated(wxListEvent& event);
		void OnGroupSplitterSashPosChanged(wxSplitterEvent& event);
		void OnListBoxModelGroupsItemDeselect(wxListEvent& event);
		void OnChoiceLayoutGroupsSelect(wxCommandEvent& event);
		void OnButtonLaunchPreviewClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        void DoCopy(wxCommandEvent& event);
        void DoCut(wxCommandEvent& event);
        void DoPaste(wxCommandEvent& event);
        void DoUndo(wxCommandEvent& event);
        void DeleteSelectedModel();

    public:
        void UpdatePreview();
        void SelectModel(const std::string & name);
        void UnSelectAllModels(bool addBkgProps = true);
        void SetupPropGrid(Model *model);
        void AddPreviewChoice(const std::string &name);
        ModelPreview* GetMainPreview() {return modelPreview;}

    protected:
        void ExportModel();
        void ExportCustomModel();
        void ImportCustomModel(Model* model);
        void AddModelButton(const std::string &type, const char *imageData[]);
        void UpdateModelGroupList();
        void ModelGroupChecked(wxCommandEvent& event);
        void DeselectModelGroupList();
        void SelectModelGroup(int index);
        void DeselectModelList();
        void MarkEffectsFileDirty();
        void SetDirtyHiLight(bool dirty);
        void UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models );

        bool SelectSingleModel(int x,int y);
        bool SelectMultipleModels(int x,int y);
        void SelectAllInBoundingRect();
        void SetSelectedModelToGroupSelected();

        int FindModelsClicked(int x,int y, std::vector<int> &found);

        int ModelsSelectedCount();
        int GetSelectedModelIndex();
        void PreviewModelAlignTops();
        void PreviewModelAlignBottoms();
        void PreviewModelAlignLeft();
        void PreviewModelAlignRight();
        void PreviewModelAlignHCenter();
        void PreviewModelAlignVCenter();
        Model *CreateNewModel(const std::string &type);


        bool m_dragging;
        bool m_creating_bound_rect;
        int m_bound_start_x;
        int m_bound_start_y;
        int m_bound_end_x;
        int m_bound_end_y;
        int m_over_handle;
        bool m_moving_handle;
        int m_previous_mouse_x, m_previous_mouse_y;
        int mPointSize;
        int mHitTestNextSelectModelIndex;
        int mNumGroups;
        bool mPropGridActive;
        int mSelectedGroup;
        wxColour mDefaultSaveBtnColor;

        wxPropertyGrid *propertyEditor;
        bool updatingProperty;
        Model *selectedModel;

        void refreshModelList();
        void resetPropertyGrid();
        void clearPropGrid();
        bool stringPropsVisible;
        bool appearanceVisible;
        bool sizeVisible;
        bool colSizesSet;
        std::vector<NewModelBitmapButton*> buttons;
        NewModelBitmapButton *selectedButton;
        wxString _lastCustomModel;
        Model *newModel;
        ModelGroupPanel *model_grp_panel;
        std::string currentLayoutGroup;

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
        void UpdateModelList(bool update_groups = true);
        void AddModelGroupItem(wxString name, ModelGroup *grp, bool selected);
        void RefreshLayout();

    private:
        ModelPreview *modelPreview;
        wxImage *background;
        wxString backgroundFile;
        wxString previewBackgroundFile;
        wxPanel* main_sequencer;

        static const long ID_MNU_DELETE_MODEL;
        static const long ID_MNU_DELETE_MODEL_GROUP;
        static const long ID_MNU_RENAME_MODEL_GROUP;
        static const long ID_MNU_ADD_MODEL_GROUP;
        void OnModelPopup(wxCommandEvent& event);
        void OnModelGroupPopup(wxCommandEvent& event);
		void OnModelGroupRightDown(wxMouseEvent& event);
		LayoutGroup* GetLayoutGroup(const std::string &name);
		const wxString& GetBackgroundImageForSelectedPreview();
        void SwitchChoiceToCurrentLayoutGroup();
        void DeleteCurrentPreview();
        void RemoveModelGroupFilters();
        void ShowPropGrid(bool show);
};

#endif
