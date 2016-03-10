#ifndef LAYOUTPANEL_H
#define LAYOUTPANEL_H

//(*Headers(LayoutPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxListView;
class wxFlexGridSizer;
class wxButton;
class wxSplitterWindow;
class wxSplitterEvent;
class wxSlider;
class wxStaticText;
class wxCheckBox;
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
		wxStaticText* StaticTextCurrentPreviewSize;
		wxStaticText* StaticTextPreviewRotation;
		wxSlider* SliderPreviewScaleWidth;
		wxStaticText* StaticText37;
		wxStaticText* StaticTextStartChannel;
		wxSlider* SliderPreviewScaleHeight;
		wxTextCtrl* TextCtrlPreviewElementWidth;
		wxCheckBox* CheckBoxOverlap;
		wxFlexGridSizer* PreviewGLSizer;
		wxTextCtrl* TextCtrlModelRotationDegrees;
		wxListView* ListBoxElementList;
		wxButton* ButtonSavePreview;
		wxButton* ButtonSetBackgroundImage;
		wxButton* ButtonSelectModelGroups;
		wxSplitterWindow* SplitterWindow2;
		wxStaticText* StaticText23;
		wxPanel* PreviewGLPanel;
		wxPanel* LeftPanel;
		wxTextCtrl* TextCtrlModelStartChannel;
		wxSlider* SliderPreviewRotate;
		wxCheckBox* ScaleImageCheckbox;
		wxButton* ButtonSetPreviewSize;
		wxStaticText* StaticText5;
		wxButton* ButtonModelsPreview;
		wxStaticText* StaticText30;
		wxSlider* Slider_BackgroundBrightness;
		wxTextCtrl* TextCtrlPreviewElementHeight;
		//*)

	protected:

		//(*Identifiers(LayoutPanel)
		static const long ID_STATICTEXT_CURRENT_PREVIEW_SIZE;
		static const long ID_BUTTON_SET_PREVIEW_SIZE;
		static const long ID_BUTTON_SET_BACKGROUND_IMAGE;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT32;
		static const long ID_SLIDER_BACKGROUND_BRIGHTNESS;
		static const long ID_BUTTON_SELECT_MODEL_GROUPS;
		static const long ID_STATICTEXT21;
		static const long ID_LISTBOX_ELEMENT_LIST;
		static const long ID_CHECKBOXOVERLAP;
		static const long ID_BUTTON_MODELS_PREVIEW;
		static const long ID_BUTTON_SAVE_PREVIEW;
		static const long ID_STATICTEXT22;
		static const long ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE;
		static const long ID_SLIDER3;
		static const long ID_STATICTEXT24;
		static const long ID_TEXTCTRL3;
		static const long ID_SLIDER_PREVIEW_SCALE;
		static const long ID_STATICTEXT25;
		static const long ID_TEXTCTRL2;
		static const long ID_SLIDER_PREVIEW_ROTATE;
		static const long ID_STATICTEXT31;
		static const long ID_TEXTCTRL4;
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

		void OnButtonSetPreviewSizeClick(wxCommandEvent& event);
		void OnScaleImageCheckboxClick(wxCommandEvent& event);
		void OnButtonSetBackgroundImageClick(wxCommandEvent& event);
		void OnSlider_BackgroundBrightnessCmdScroll(wxScrollEvent& event);
		void OnButtonSelectModelGroupsClick(wxCommandEvent& event);
		void OnCheckBoxOverlapClick(wxCommandEvent& event);
		void OnButtonModelsPreviewClick(wxCommandEvent& event);
		void OnButtonSavePreviewClick(wxCommandEvent& event);
		void OnTextCtrlPreviewElementWidthText(wxCommandEvent& event);
		void OnTextCtrlPreviewElementHeightText(wxCommandEvent& event);
		void OnTextCtrlModelRotationDegreesText(wxCommandEvent& event);
		void OnTextCtrlModelStartChannelText(wxCommandEvent& event);
		void OnSliderPreviewScaleWidthCmdSliderUpdated(wxScrollEvent& event);
		void OnSliderPreviewScaleHeightCmdSliderUpdated(wxScrollEvent& event);
		void OnSliderPreviewRotateCmdSliderUpdated(wxScrollEvent& event);
		void OnListBoxElementListItemSelect(wxListEvent& event);
		void OnListBoxElementListColumnClick(wxListEvent& event);
        void OnPropertyGridChange(wxPropertyGridEvent& event);
        void OnPropertyGridChanging(wxPropertyGridEvent& event);
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

    public:
        xLightsFrame *xlights;
        ModelPreview *modelPreview;
};

#endif
