#ifndef CUSTOMMODELDIALOG_H
#define CUSTOMMODELDIALOG_H

//(*Headers(CustomModelDialog)
#include <wx/dialog.h>
class wxBitmapButton;
class wxButton;
class wxCheckBox;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxGrid;
class wxGridEvent;
class wxSlider;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticBoxSizer;
class wxStaticText;
//*)

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/renderer.h>
#include <wx/filepicker.h>

class CustomModel;
class CopyPasteGrid;

class wxModelGridCellRenderer : public wxGridCellStringRenderer
{
public:
    wxModelGridCellRenderer(wxImage* image_, wxGrid& grid);
    virtual ~wxModelGridCellRenderer() {}

    virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected) wxOVERRIDE;

    void UpdateSize(wxGrid& grid, bool draw_picture_, int lightness_);
    void CreateImage();
    void SetImage(wxImage* image);
    void DetermineGridSize(wxGrid& grid);

private:
    wxImage* image;
    wxBitmap bmp;
    int width;
    int height;
    bool draw_picture;
    int lightness;
};

class CustomModelDialog: public wxDialog
{
    void ValidateWindow();

    static const long CUSTOMMODELDLGMNU_CUT;
    static const long CUSTOMMODELDLGMNU_COPY;
    static const long CUSTOMMODELDLGMNU_PASTE;
    static const long CUSTOMMODELDLGMNU_FLIPH;
    static const long CUSTOMMODELDLGMNU_FLIPV;
    static const long CUSTOMMODELDLGMNU_REVERSE;
    static const long CUSTOMMODELDLGMNU_SHIFT;
    static const long CUSTOMMODELDLGMNU_INSERT;
    static const long CUSTOMMODELDLGMNU_COMPRESS;
    static const long CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE;
    static const long CUSTOMMODELDLGMNU_SHRINKSPACE10;
    static const long CUSTOMMODELDLGMNU_SHRINKSPACE50;
    static const long CUSTOMMODELDLGMNU_SHRINKSPACE99;

    public:

		CustomModelDialog(wxWindow* parent);
		virtual ~CustomModelDialog();

		//(*Declarations(CustomModelDialog)
		CopyPasteGrid* GridCustom;
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
		wxFilePickerCtrl* FilePickerCtrl1;
		wxFlexGridSizer* Sizer1;
		wxSlider* SliderCustomLightness;
		wxSpinCtrl* HeightSpin;
		wxSpinCtrl* SpinCtrlNextChannel;
		wxSpinCtrl* WidthSpin;
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
		static const long ID_GRID_Custom;
		//*)

		std::string background_image;
		wxImage* bkg_image;
		wxModelGridCellRenderer* renderer;
        bool bkgrd_active;
        int lightness;
        bool autonumber;
        bool autoincrement;
        int next_channel;
        wxString name;
        int _selRow;
        int _selCol;

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
		//*)

        void OnCut(wxCommandEvent& event);
        void OnCopy(wxCommandEvent& event);
        void OnPaste(wxCommandEvent& event);
        void OnGridPopup(wxCommandEvent& event);

        void Reverse();
        bool CheckScale(std::list<wxPoint>& points, float scale);
        void FlipHorizontal();
        void FlipVertical();
        void Insert(int selRow, int selCol);
        void Shift();
        void Compress();
        bool AdjustNodeBy(int node, int adjust);
        void TrimSpace();
        void ShrinkSpace(float min);

		DECLARE_EVENT_TABLE()
};

#endif
