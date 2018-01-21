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

#include <wx/filepicker.h>
#include "../BulkEditControls.h"

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
		BulkEditSlider* Slider_PicturesEndYC;
		wxBitmapButton* BitmapButton_PicturesDirection;
		BulkEditSliderF1* Slider_Pictures_Speed;
		wxStaticText* StaticText_Pictures_YC;
		BulkEditSlider* Slider_PicturesYC;
		BulkEditSliderF1* Slider_Pictures_FR;
		xlPictureFilePickerCtrl* FilePickerCtrl1;
		wxStaticText* StaticText161;
		wxStaticText* StaticText_Pictures_XC;
		wxBitmapButton* BitmapButton_PicturesSpeed;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_PicturesEndXC;
		BulkEditSlider* Slider_Pictures_EndScale;
		BulkEditSlider* Slider_PicturesXC;
		wxStaticText* StaticText46;
		BulkEditCheckBox* CheckBox_Pictures_Shimmer;
		wxBitmapButton* BitmapButton_PicturesFrameRateAdj;
		wxStaticText* StaticText68;
		BulkEditCheckBox* CheckBox_Pictures_WrapX;
		BulkEditCheckBox* CheckBox_Pictures_PixelOffsets;
		wxPanel* PictureEndPositionPanel;
		wxStaticText* StaticText160;
		wxStaticText* StaticText2;
		BulkEditChoice* Choice_Scaling;
		BulkEditCheckBox* CheckBox_LoopGIF;
		BulkEditSlider* Slider_Pictures_StartScale;
		BulkEditChoice* Choice_Pictures_Direction;
		wxStaticText* StaticText96;
		//*)

	protected:

		//(*Identifiers(PicturesPanel)
		static const long ID_FILEPICKER_Pictures_Filename;
		static const long ID_STATICTEXT_Pictures_Direction;
		static const long ID_CHOICE_Pictures_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Pictures_Direction;
		static const long ID_STATICTEXT_Pictures_Speed;
		static const long IDD_SLIDER_Pictures_Speed;
		static const long ID_TEXTCTRL_Pictures_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Pictures_Speed;
		static const long ID_STATICTEXT_Pictures_FrameRateAdj;
		static const long IDD_SLIDER_Pictures_FrameRateAdj;
		static const long ID_TEXTCTRL_Pictures_FrameRateAdj;
		static const long ID_BITMAPBUTTON_SLIDER_Pictures_FrameRateAdj;
		static const long ID_CHECKBOX_Pictures_PixelOffsets;
		static const long ID_CHOICE_Scaling;
		static const long ID_CHECKBOX_Pictures_Shimmer;
		static const long ID_CHECKBOX_LoopGIF;
		static const long ID_STATICTEXT_PicturesXC;
		static const long ID_SLIDER_PicturesXC;
		static const long ID_CHECKBOX_Pictures_WrapX;
		static const long IDD_TEXTCTRL_PicturesXC;
		static const long ID_STATICTEXT_PicturesYC;
		static const long IDD_TEXTCTRL_PicturesYC;
		static const long ID_SLIDER_PicturesYC;
		static const long ID_PANEL43;
		static const long ID_STATICTEXT_PicturesEndXC;
		static const long ID_SLIDER_PicturesEndXC;
		static const long IDD_TEXTCTRL_PicturesEndXC;
		static const long ID_STATICTEXT_PicturesEndYC;
		static const long IDD_TEXTCTRL_PicturesEndYC;
		static const long ID_SLIDER_PicturesEndYC;
		static const long ID_PANEL45;
		static const long ID_STATICTEXT_Pictures_StartScale;
		static const long ID_SLIDER_Pictures_StartScale;
		static const long IDD_TEXTCTRL_Pictures_StartScale;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT_Pictures_EndScale;
		static const long ID_SLIDER_Pictures_EndScale;
		static const long IDD_TEXTCTRL_Pictures_EndScale;
		static const long ID_PANEL2;
		static const long IDD_NOTEBOOK_Pictures_Positions;
		//*)

	public:

		//(*Handlers(PicturesPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoicePicturesDirectionSelect(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
