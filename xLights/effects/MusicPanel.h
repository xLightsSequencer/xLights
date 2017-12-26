#ifndef MUSICPANEL_H
#define MUSICPANEL_H

//(*Headers(MusicPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../BulkEditControls.h"

class MusicPanel: public wxPanel
{

	public:

		MusicPanel(wxWindow* parent);
		virtual ~MusicPanel();

		//(*Declarations(MusicPanel)
		BulkEditChoice* Choice_Music_Scaling;
		BulkEditCheckBox* CheckBox_Music_Fade;
		BulkEditSlider* Slider_Music_EndNote;
		BulkEditTextCtrl* TextCtrl_Music_StartNote;
		BulkEditChoice* Choice_Music_Colour;
		wxBitmapButton* BitmapButton_Music_EndNote;
		wxStaticText* StaticText2;
		BulkEditSlider* Slider_Music_Bars;
		BulkEditTextCtrl* TextCtrl_Music_Sensitivity;
		wxStaticText* StaticText6;
		BulkEditTextCtrl* TextCtrl_Music_Bars;
		BulkEditTextCtrl* TextCtrl_Music_EndNote;
		wxStaticText* StaticText11;
		wxBitmapButton* BitmapButton_Music_Sensitivity;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Music_StartNote;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_Music_Bars;
		BulkEditTextCtrl* TextCtrl_Music_Offset;
		wxStaticText* StaticText5;
		wxBitmapButton* BitmapButton_Music_Colour;
		BulkEditChoice* Choice_Music_Type;
		BulkEditSlider* Slider_Music_Offset;
		wxStaticText* StaticText12;
		wxBitmapButton* BitmapButton_Music_Offset;
		wxBitmapButton* BitmapButton_Music_Type;
		BulkEditSlider* Slider_Music_Sensitivity;
		wxBitmapButton* BitmapButton_Music_Fade;
		wxBitmapButton* BitmapButton_Music_StartNote;
		wxBitmapButton* BitmapButton_Music_Scale;
		wxStaticText* StaticText4;
		BulkEditCheckBox* CheckBox_Music_Scale;
		wxBitmapButton* BitmapButton_Music_Scaling;
		BulkEditValueCurveButton* BitmapButton_Music_OffsetVC;
		//*)

	protected:

		//(*Identifiers(MusicPanel)
		static const long ID_STATICTEXT_Music_Bars;
		static const long ID_SLIDER_Music_Bars;
		static const long IDD_TEXTCTRL_Music_Bars;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Bars;
		static const long ID_STATICTEXT_Music_Type;
		static const long ID_CHOICE_Music_Type;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Type;
		static const long ID_STATICTEXT_Music_StartNote;
		static const long ID_SLIDER_Music_StartNote;
		static const long IDD_TEXTCTRL_Music_StartNote;
		static const long ID_BITMAPBUTTON_Music_StartNote;
		static const long ID_STATICTEXT_Music_EndNote;
		static const long ID_SLIDER_Music_EndNote;
		static const long IDD_TEXTCTRL_Music_EndNote;
		static const long ID_BITMAPBUTTON_Music_EndNote;
		static const long ID_STATICTEX_Music_Sensitivity;
		static const long ID_SLIDER_Music_Sensitivity;
		static const long IDD_TEXTCTRL_Music_Sensitivity;
		static const long ID_BITMAPBUTTON_Music_Sensitivty;
		static const long ID_STATICTEXT_Music_Offset;
		static const long ID_SLIDER_Music_Offset;
		static const long ID_VALUECURVE_Music_Offset;
		static const long IDD_TEXTCTRL_Music_Offset;
		static const long ID_BITMAPBUTTON_Music_Offset;
		static const long ID_CHECKBOX_Music_Scale;
		static const long ID_BITMAPBUTTON_Music_Scale;
		static const long ID_STATICTEXT_Music_Scaling;
		static const long ID_CHOICE_Music_Scaling;
		static const long ID_BITMAPBUTTON_Music_Scaling;
		static const long ID_STATICTEXT_Music_Colour;
		static const long ID_CHOICE_Music_Colour;
		static const long ID_BITMAPBUTTON_Music_Colour;
		static const long ID_CHECKBOX_Music_Fade;
		static const long ID_BITMAPBUTTON_Music_Fade;
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
