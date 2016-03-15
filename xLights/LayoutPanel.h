#ifndef LAYOUTPANEL_H
#define LAYOUTPANEL_H

//(*Headers(LayoutPanel)
#include <wx/panel.h>
class wxSplitterWindow;
class wxCheckBox;
class wxSplitterEvent;
class wxListView;
class wxFlexGridSizer;
class wxButton;
//*)

class xLightsFrame;
class ModelPreview;
class Model;
class wxListEvent;
class wxMouseEvent;
class wxPropertyGrid;
class wxPropertyGridEvent;

#include <vector>

class LayoutPanel: public wxPanel
{
	public:

		LayoutPanel(wxWindow* parent, xLightsFrame *xlights);
		virtual ~LayoutPanel();

		//(*Declarations(LayoutPanel)
		wxFlexGridSizer* ToolSizer;
		wxListView* ListBoxElementList;
		wxButton* ButtonSelectModelGroups;
		wxButton* ButtonModelsPreview;
		wxSplitterWindow* SplitterWindow2;
		wxPanel* LeftPanel;
		wxCheckBox* CheckBoxOverlap;
		wxPanel* SecondPanel;
		wxButton* ButtonSavePreview;
		wxFlexGridSizer* PreviewGLSizer;
		wxSplitterWindow* ModelSplitter;
		wxPanel* PreviewGLPanel;
		//*)

	protected:

		//(*Identifiers(LayoutPanel)
		static const long ID_BUTTON_SELECT_MODEL_GROUPS;
		static const long ID_BUTTON_MODELS_PREVIEW;
		static const long ID_BUTTON_SAVE_PREVIEW;
		static const long ID_CHECKBOXOVERLAP;
		static const long ID_LISTBOX_ELEMENT_LIST;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_PANEL5;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW2;
		//*)

        static const long ID_PREVIEW_ALIGN;
        static const long ID_PREVIEW_MODEL_PROPERTIES;
        static const long ID_PREVIEW_ALIGN_TOP;
        static const long ID_PREVIEW_ALIGN_BOTTOM;
        static const long ID_PREVIEW_ALIGN_LEFT;
        static const long ID_PREVIEW_ALIGN_RIGHT;
        static const long ID_PREVIEW_ALIGN_H_CENTER;
        static const long ID_PREVIEW_ALIGN_V_CENTER;
        static const long ID_PREVIEW_DISTRIBUTE;
        static const long ID_PREVIEW_H_DISTRIBUTE;
        static const long ID_PREVIEW_V_DISTRIBUTE;

	public:

		//(*Handlers(LayoutPanel)
		void OnPreviewLeftUp(wxMouseEvent& event);
		void OnPreviewMouseLeave(wxMouseEvent& event);
		void OnPreviewLeftDown(wxMouseEvent& event);
		void OnPreviewRightDown(wxMouseEvent& event);
		void OnPreviewMouseMove(wxMouseEvent& event);
		void OnPreviewModelPopup(wxCommandEvent &event);
		void OnButtonSelectModelGroupsClick(wxCommandEvent& event);
		void OnCheckBoxOverlapClick(wxCommandEvent& event);
		void OnButtonModelsPreviewClick(wxCommandEvent& event);
		void OnButtonSavePreviewClick(wxCommandEvent& event);
		void OnListBoxElementListItemSelect(wxListEvent& event);
		void OnListBoxElementListColumnClick(wxListEvent& event);
		void OnPropertyGridChange(wxPropertyGridEvent& event);
		void OnPropertyGridChanging(wxPropertyGridEvent& event);
		void OnModelSplitterSashPosChanged(wxSplitterEvent& event);
		void OnSplitterWindowSashPosChanged(wxSplitterEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

    public:
        void UpdatePreview();
        void SelectModel(const std::string & name);
        void UnSelectAllModels();
        void SetupPropGrid(Model *model);

    protected:
    void ShowModelProperties();

        bool SelectSingleModel(int x,int y);
        bool SelectMultipleModels(int x,int y);
        void SelectAllInBoundingRect();
        void SetSelectedModelToGroupSelected();

        int FindModelsClicked(int x,int y, std::vector<int> &found);

        void PreviewRotationUpdated(int newRotation);
        void PreviewScaleUpdated(float xscale, float yscale);
    
        int ModelsSelectedCount();
        int GetSelectedModelIndex();
        void PreviewModelAlignTops();
        void PreviewModelAlignBottoms();
        void PreviewModelAlignLeft();
        void PreviewModelAlignRight();
        void PreviewModelAlignHCenter();
        void PreviewModelAlignVCenter();

    
        bool m_dragging;
        bool m_resizing;
        bool m_rotating;
        bool m_creating_bound_rect;
        int m_bound_start_x;
        int m_bound_start_y;
        int m_bound_end_x;
        int m_bound_end_y;
        int m_over_handle;
        int m_previous_mouse_x, m_previous_mouse_y;
        int mPointSize;
        int mHitTestNextSelectModelIndex;
    
        wxPropertyGrid *propertyEditor;
        Model *selectedModel;

        void resetPropertyGrid();
        void clearPropGrid();
        bool stringPropsVisible;
        bool appearanceVisible;
        bool sizeVisible;
    public:
        xLightsFrame *xlights;
        ModelPreview *modelPreview;
    
};

#endif
