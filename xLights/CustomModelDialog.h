#ifndef CUSTOMMODELDIALOG_H
#define CUSTOMMODELDIALOG_H

//(*Headers(CustomModelDialog)
#include <wx/dialog.h>
class wxGrid;
class wxSpinEvent;
class wxCheckBox;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxSpinCtrl;
class wxButton;
class wxStaticBoxSizer;
class wxGridEvent;
//*)

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/renderer.h>

class CustomModel;
class CopyPasteGrid;

class wxModelGridCellRenderer : public wxGridCellStringRenderer
{
public:
    wxModelGridCellRenderer(wxImage* image_, wxGrid& grid);

    virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected) wxOVERRIDE;

    void UpdateSize(wxGrid& grid, bool draw_picture_, int lightness_);
    void CreateImage();
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

    public:

		CustomModelDialog(wxWindow* parent);
		virtual ~CustomModelDialog();

		//(*Declarations(CustomModelDialog)
		wxCheckBox* CheckBoxAutoNumber;
		CopyPasteGrid* GridCustom;
		wxButton* Button_Flip_Vertical;
		wxButton* Button_Reverse;
		wxButton* ButtonWiring;
		wxCheckBox* CheckBoxAutoIncrement;
		wxBitmapButton* BitmapButtonCustomBkgrd;
		wxSpinCtrl* SpinCtrlNextChannel;
		wxButton* Button_Flip_Horizonal;
		wxButton* Button_CustomModelZoomIn;
		wxBitmapButton* BitmapButtonCustomPaste;
		wxButton* ButtonCancel;
		wxBitmapButton* BitmapButtonCustomCopy;
		wxSpinCtrl* HeightSpin;
		wxButton* ButtonOk;
		wxSlider* SliderCustomLightness;
		wxBitmapButton* BitmapButtonCustomCut;
		wxCheckBox* CheckBox_RearView;
		wxButton* Button_CustomModelZoomOut;
		wxFlexGridSizer* Sizer1;
		wxButton* Button_Renumber;
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
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON_Flip_Horizontal;
		static const long ID_BUTTON_Flip_Vertical;
		static const long ID_BUTTON_Reverse;
		static const long ID_BUTTON_RENUMBER;
		static const long ID_BITMAPBUTTON_CUSTOM_CUT;
		static const long ID_BITMAPBUTTON_CUSTOM_COPY;
		static const long ID_BITMAPBUTTON_CUSTOM_PASTE;
		static const long ID_BUTTON_CustomModelZoomIn;
		static const long ID_BUTTON_CustomModelZoomOut;
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
		void OnButton_Flip_HorizonalClick(wxCommandEvent& event);
		void OnButton_Flip_VerticalClick(wxCommandEvent& event);
		void OnButton_ReverseClick(wxCommandEvent& event);
		void OnButton_RenumberClick(wxCommandEvent& event);
		//*)

        void OnCut(wxCommandEvent& event);
        void OnCopy(wxCommandEvent& event);
        void OnPaste(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
