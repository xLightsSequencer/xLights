#ifndef MUSICPANEL_H
#define MUSICPANEL_H

//(*Headers(MusicPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class MusicPanel: public wxPanel
{

	public:

		MusicPanel(wxWindow* parent);
		virtual ~MusicPanel();

		//(*Declarations(MusicPanel)
		wxBitmapButton* BitmapButton_Music_Bars;
		wxBitmapButton* BitmapButton_Music_Scale;
		BulkEditTextCtrl* TextCtrl_Music_EndNote;
		BulkEditCheckBox* CheckBox_Music_Fade;
		BulkEditChoice* Choice_Music_Colour;
		wxBitmapButton* BitmapButton_Music_StartNote;
		BulkEditChoice* Choice_Music_Scaling;
		BulkEditSlider* Slider_Music_Offset;
		BulkEditValueCurveButton* BitmapButton_Music_OffsetVC;
		BulkEditSlider* Slider_Music_Sensitivity;
		wxBitmapButton* BitmapButton_Music_Scaling;
		BulkEditSlider* Slider_Music_Bars;
		wxBitmapButton* BitmapButton_Music_EndNote;
		BulkEditSlider* Slider_Music_StartNote;
		BulkEditTextCtrl* TextCtrl_Music_Bars;
		BulkEditChoice* Choice_Music_Type;
		BulkEditTextCtrl* TextCtrl_Music_Sensitivity;
		wxStaticText* StaticText1;
		BulkEditTextCtrl* TextCtrl_Music_StartNote;
		wxStaticText* StaticText3;
		BulkEditCheckBox* CheckBox_Music_Scale;
		BulkEditSlider* Slider_Music_EndNote;
		wxStaticText* StaticText12;
		BulkEditTextCtrl* TextCtrl_Music_Offset;
		wxStaticText* StaticText4;
		wxBitmapButton* BitmapButton_Music_Colour;
		wxStaticText* StaticText5;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_Music_Sensitivity;
		wxStaticText* StaticText6;
		wxBitmapButton* BitmapButton_Music_Type;
		wxBitmapButton* BitmapButton_Music_Fade;
		wxBitmapButton* BitmapButton_Music_Offset;
		wxStaticText* StaticText11;
		//*)

	protected:

		//(*Identifiers(MusicPanel)
		static const long ID_STATICTEXT_Music_Bars;
		static const long ID_SLIDER_Music_Bars;
		static const long IDD_TEXTCTRL_Music_Bars;
		static const long ID_BITMAPBUTTON_SLIDER_Music_Bars;
		static const long ID_STATICTEXT_Music_Type;
		static const long ID_CHOICE_Music_Type;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Type;
		static const long ID_STATICTEXT_Music_StartNote;
		static const long ID_SLIDER_Music_StartNote;
		static const long IDD_TEXTCTRL_Music_StartNote;
		static const long ID_BITMAPBUTTON_SLIDER_Music_StartNote;
		static const long ID_STATICTEXT_Music_EndNote;
		static const long ID_SLIDER_Music_EndNote;
		static const long IDD_TEXTCTRL_Music_EndNote;
		static const long ID_BITMAPBUTTON_SLIDER_Music_EndNote;
		static const long ID_STATICTEX_Music_Sensitivity;
		static const long ID_SLIDER_Music_Sensitivity;
		static const long IDD_TEXTCTRL_Music_Sensitivity;
		static const long ID_BITMAPBUTTON_SLIDER_Music_Sensitivity;
		static const long ID_STATICTEXT_Music_Offset;
		static const long ID_SLIDER_Music_Offset;
		static const long ID_VALUECURVE_Music_Offset;
		static const long IDD_TEXTCTRL_Music_Offset;
		static const long ID_BITMAPBUTTON_SLIDER_Music_Offset;
		static const long ID_CHECKBOX_Music_Scale;
		static const long ID_BITMAPBUTTON_CHECKBOX_Music_Scale;
		static const long ID_STATICTEXT_Music_Scaling;
		static const long ID_CHOICE_Music_Scaling;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Scaling;
		static const long ID_STATICTEXT_Music_Colour;
		static const long ID_CHOICE_Music_Colour;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Colour;
		static const long ID_CHECKBOX_Music_Fade;
		static const long ID_BITMAPBUTTON_CHECKBOX_Music_Fade;
		//*)

	public:

		//(*Handlers(MusicPanel)
		void OnLockButtonClick(wxCommandEvent& event);
        void OnChoice_Music_TypeSelect(wxCommandEvent& event);
		void OnSlider_Music_StartNoteCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_Music_StartNoteText(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()

	public:

		void ValidateWindow();
};

#endif
