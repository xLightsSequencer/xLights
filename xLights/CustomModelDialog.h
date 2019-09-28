#ifndef CUSTOMMODELDIALOG_H
#define CUSTOMMODELDIALOG_H

//(*Headers(CustomModelDialog)
#include <wx/dialog.h>
class wxBitmapButton;
class wxButton;
class wxCheckBox;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxNotebook;
class wxNotebookEvent;
class wxPanel;
class wxSlider;
class wxSpinCtrl;
class wxSpinEvent;
class wxSplitterEvent;
class wxSplitterWindow;
class wxStaticBoxSizer;
class wxStaticText;
//*)

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/renderer.h>
#include <wx/filepicker.h>
#include <wx/notebook.h>

#include <glm/mat3x3.hpp>

class CustomModel;
class CustomNotebook;
class CopyPasteGrid;
class wxModelGridCellRenderer;
class ImageFilePickerCtrl;
class ModelPreview;

wxDECLARE_EVENT(EVT_GRID_KEY, wxCommandEvent);

class CustomModelDialog: public wxDialog
{
    glm::vec3 _saveWorldPos = glm::vec3(0, 0, 0);
    glm::vec3 _saveScale = glm::vec3(1, 1, 1);
    int _saveWidth = 0;
    int _saveHeight = 0;
    int _saveDepth = 0;
    std::string _saveModelData;
    CustomModel* _model = nullptr;

    std::string GetModelData();
    void UpdatePreview(int width, int height, int depth, const std::string& modelData);
    void UpdatePreview();
    void ValidateWindow();

    static const long CUSTOMMODELDLGMNU_CUT;
    static const long CUSTOMMODELDLGMNU_COPY;
    static const long CUSTOMMODELDLGMNU_PASTE;
    static const long CUSTOMMODELDLGMNU_FLIPH;
    static const long CUSTOMMODELDLGMNU_FLIPV;
    static const long CUSTOMMODELDLGMNU_ROTATE90;
    static const long CUSTOMMODELDLGMNU_ROTATE;
    static const long CUSTOMMODELDLGMNU_REVERSE;
    static const long CUSTOMMODELDLGMNU_SHIFT;
    static const long CUSTOMMODELDLGMNU_INSERT;
    static const long CUSTOMMODELDLGMNU_COMPRESS;
    static const long CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE;
    static const long CUSTOMMODELDLGMNU_SHRINKSPACE10;
    static const long CUSTOMMODELDLGMNU_SHRINKSPACE50;
    static const long CUSTOMMODELDLGMNU_SHRINKSPACE99;
    static const long CUSTOMMODELDLGMNU_COPYLAYERFWD1;
    static const long CUSTOMMODELDLGMNU_COPYLAYERBKWD1;
    static const long CUSTOMMODELDLGMNU_COPYLAYERFWDALL;
    static const long CUSTOMMODELDLGMNU_COPYLAYERBKWDALL;

    public:

		CustomModelDialog(wxWindow* parent);
		virtual ~CustomModelDialog();

		//(*Declarations(CustomModelDialog)
		CustomNotebook* Notebook1;
		ImageFilePickerCtrl* FilePickerCtrl1;
		wxBitmapButton* BitmapButtonCustomBkgrd;
		wxBitmapButton* BitmapButtonCustomCopy;
		wxBitmapButton* BitmapButtonCustomCut;
		wxBitmapButton* BitmapButtonCustomPaste;
		wxButton* ButtonCancel;
		wxButton* ButtonOk;
		wxButton* ButtonWiring;
		wxButton* Button_CustomModelZoomIn;
		wxButton* Button_CustomModelZoomOut;
		wxCheckBox* CheckBoxAutoIncrement;
		wxCheckBox* CheckBoxAutoNumber;
		wxCheckBox* CheckBox_ShowWiring;
		wxFlexGridSizer* FlexGridSizer10;
		wxFlexGridSizer* Sizer1;
		wxPanel* Panel11;
		wxPanel* Panel1;
		wxSlider* SliderCustomLightness;
		wxSpinCtrl* HeightSpin;
		wxSpinCtrl* SpinCtrlNextChannel;
		wxSpinCtrl* SpinCtrl_Depth;
		wxSpinCtrl* WidthSpin;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* StaticText4;
		//*)


        void Setup(CustomModel *m);
        void Save(CustomModel *m);
        void ResizeCustomGrid();

	protected:
        void CutOrCopyToClipboard(bool isCut);
        void UpdateBackground();
        void Paste();

		//(*Identifiers(CustomModelDialog)
		static const long ID_SPINCTRL1;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL3;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON3;
		static const long ID_BITMAPBUTTON_CUSTOM_CUT;
		static const long ID_BITMAPBUTTON_CUSTOM_COPY;
		static const long ID_BITMAPBUTTON_CUSTOM_PASTE;
		static const long ID_BUTTON_CustomModelZoomIn;
		static const long ID_BUTTON_CustomModelZoomOut;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_SLIDER_CUSTOM_LIGHTNESS;
		static const long ID_BITMAPBUTTON_CUSTOM_BKGRD;
		static const long ID_CHECKBOX_AUTO_NUMBER;
		static const long ID_CHECKBOX_AUTO_INCREMENT;
		static const long ID_SPINCTRL_NEXT_CHANNEL;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL2;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW1;
		//*)

		std::string background_image;
		wxImage* bkg_image;
        bool bkgrd_active;
        int lightness;
        bool autonumber = false;
        bool autoincrement = false;
        int next_channel = 1;
        wxString name;
        int _selRow = 0;
        int _selCol = 0;
        std::vector<CopyPasteGrid*> _grids;
        std::vector<wxModelGridCellRenderer*> _renderers;
        ModelPreview* _modelPreview = nullptr;

	public:

		//(*Handlers(CustomModelDialog)
		void OnWidthSpinChange(wxSpinEvent& event);
		void OnHeightSpinChange(wxSpinEvent& event);
		void OnBitmapButtonCustomCutClick(wxCommandEvent& event);
		void OnBitmapButtonCustomCopyClick(wxCommandEvent& event);
		void OnBitmapButtonCustomPasteClick(wxCommandEvent& event);
		void OnButton_CustomModelZoomInClick(wxCommandEvent& event);
		void OnButton_CustomModelZoomOutClick(wxCommandEvent& event);
		void OnButtonCustomModelHelpClick(wxCommandEvent& event);
		void OnGridCustomCellChange(wxGridEvent& event);
		void OnBitmapButtonCustomBkgrdClick(wxCommandEvent& event);
		void OnSliderCustomLightnessCmdSliderUpdated(wxScrollEvent& event);
		void OnCheckBoxAutoNumberClick(wxCommandEvent& event);
		void OnCheckBoxAutoIncrementClick(wxCommandEvent& event);
		void OnSpinCtrlNextChannelChange(wxSpinEvent& event);
		void OnGridCustomCellLeftClick(wxGridEvent& event);
		void OnCheckBox_RearViewClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonWiringClick(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnGridCustomCellRightClick(wxGridEvent& event);
		void OnSpinCtrl_DepthChange(wxSpinEvent& event);
		void OnNotebook1PageChanged(wxNotebookEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnCheckBox_ShowWiringClick(wxCommandEvent& event);
		//*)

        void OnMove(wxMoveEvent& event);
        void OnCut(wxCommandEvent& event);
        void OnCopy(wxCommandEvent& event);
        void OnPaste(wxCommandEvent& event);
        void OnGridPopup(wxCommandEvent& event);
        void OnGridKey(wxCommandEvent& event);

        void Reverse();
        bool CheckScale(std::list<wxPoint>& points, float scale) const;
        void FlipHorizontal();
        void FlipVertical();
        void Rotate90();
        void Rotate();
        void CentreModel();
        void Insert(int selRow, int selCol);
        void Shift();
        void Compress();
        bool AdjustNodeBy(int node, int adjust);
        void TrimSpace();
        void ShrinkSpace(float min);
        void AddPage();
        void RemovePage();
        CopyPasteGrid* GetActiveGrid() const;
        CopyPasteGrid* GetLayerGrid(int layer) const;
        void CopyLayer(bool forward, int layers);

		DECLARE_EVENT_TABLE()
};
#endif
