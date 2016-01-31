#ifndef PICTURESPANEL_H
#define PICTURESPANEL_H

//(*Headers(PicturesPanel)
#include <wx/panel.h>
class wxFilePickerCtrl;
class wxTextCtrl;
class wxChoice;
class wxNotebook;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxGridBagSizer;
class wxNotebookEvent;
class wxStaticText;
class wxCheckBox;
//*)

class PicturesPanel: public wxPanel
{
	public:

		PicturesPanel(wxWindow* parent);
		virtual ~PicturesPanel();

		//(*Declarations(PicturesPanel)
		wxBitmapButton* BitmapButton_PicturesDirection;
		wxCheckBox* CheckBox_Pictures_WrapX;
		wxBitmapButton* BitmapButton_PicturesSpeed;
		wxStaticText* StaticText46;
		wxChoice* Choice_Pictures_Direction;
		wxStaticText* StaticText68;
		wxBitmapButton* BitmapButton_PicturesFilename;
		wxPanel* PictureEndPositionPanel;
		wxBitmapButton* BitmapButton6;
		//*)

	protected:

		//(*Identifiers(PicturesPanel)
		static const long ID_FILEPICKER_Pictures_Filename;
		static const long ID_BITMAPBUTTON_BUTTON_PICTURES_FILENAME;
		static const long ID_STATICTEXT46;
		static const long ID_CHOICE_Pictures_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Pictures_Direction;
		static const long ID_STATICTEXT27;
		static const long IDD_SLIDER_Pictures_Speed;
		static const long ID_TEXTCTRL_Pictures_Speed;
		static const long ID_BITMAPBUTTON25;
		static const long IDD_SLIDER_Pictures_FrameRateAdj;
		static const long ID_TEXTCTRL_Pictures_FrameRateAdj;
		static const long ID_BITMAPBUTTON_SLIDER_Pictures_GifSpeed;
		static const long ID_CHECKBOX_Pictures_PixelOffsets;
		static const long ID_SLIDER_PicturesXC;
		static const long ID_CHECKBOX_Pictures_WrapX;
		static const long IDD_TEXTCTRL_PicturesXC;
		static const long IDD_TEXTCTRL_PicturesYC;
		static const long ID_SLIDER_PicturesYC;
		static const long ID_PANEL43;
		static const long ID_SLIDER_PicturesEndXC;
		static const long IDD_TEXTCTRL_PicturesEndXC;
		static const long IDD_TEXTCTRL_PicturesEndYC;
		static const long ID_SLIDER_PicturesEndYC;
		static const long ID_PANEL45;
		static const long IDD_NOTEBOOK_Pictures_Positions;
		//*)

	public:

		//(*Handlers(PicturesPanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void OnChoicePicturesDirectionSelect(wxCommandEvent& event);

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
