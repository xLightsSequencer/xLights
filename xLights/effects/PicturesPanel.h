#ifndef PICTURESPANEL_H
#define PICTURESPANEL_H

//(*Headers(PicturesPanel)
#include <wx/panel.h>
class wxFilePickerCtrl;
class wxGridBagSizer;
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include <wx/filepicker.h>

class xlPictureFilePickerCtrl : public wxFilePickerCtrl {
public:
    xlPictureFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr)
        : wxFilePickerCtrl(parent, id, path, message, wxImage::GetImageExtWildcard(), pos, size, style, validator, name) {

    }
    virtual ~xlPictureFilePickerCtrl() {}
};

class PicturesPanel: public wxPanel
{
	public:

		PicturesPanel(wxWindow* parent);
		virtual ~PicturesPanel();

		//(*Declarations(PicturesPanel)
		wxSlider* Slider_PicturesEndXC;
		wxCheckBox* CheckBox_Pictures_PixelOffsets;
		wxCheckBox* CheckBox_Pictures_ScaleToFit;
		wxSlider* Slider_Pictures_Speed;
		wxCheckBox* CheckBox_Pictures_WrapX;
		wxChoice* Choice_Pictures_Direction;
		wxBitmapButton* BitmapButton_PicturesDirection;
		wxBitmapButton* BitmapButton_PicturesSpeed;
		wxSlider* Slider_Pictures_EndScale;
		wxSlider* Slider_PicturesXC;
		wxSlider* Slider_Pictures_StartScale;
		xlPictureFilePickerCtrl* FilePickerCtrl1;
		wxSlider* Slider_PicturesYC;
		wxCheckBox* CheckBox_Pictures_Shimmer;
		wxBitmapButton* BitmapButton6;
		wxPanel* PictureEndPositionPanel;
		wxBitmapButton* BitmapButton_PicturesFilename;
		wxStaticText* StaticText68;
		wxSlider* Slider_Pictures_FR;
		wxSlider* Slider_PicturesEndYC;
		wxStaticText* StaticText46;
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
		static const long ID_CHECKBOX_Pictures_ScaleToFit;
		static const long ID_CHECKBOX_Pictures_Shimmer;
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
		static const long ID_SLIDER_Pictures_StartScale;
		static const long IDD_TEXTCTRL_Pictures_StartScale;
		static const long ID_PANEL1;
		static const long ID_SLIDER_Pictures_EndScale;
		static const long IDD_TEXTCTRL_Pictures_EndScale;
		static const long ID_PANEL2;
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
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
