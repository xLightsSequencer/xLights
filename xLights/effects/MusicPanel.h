#ifndef MUSICPANEL_H
#define MUSICPANEL_H

//(*Headers(MusicPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class MusicPanel: public wxPanel
{

	public:

		MusicPanel(wxWindow* parent);
		virtual ~MusicPanel();

		//(*Declarations(MusicPanel)
		BulkEditCheckBox* CheckBox_Music_Fade;
		BulkEditCheckBox* CheckBox_Music_LogarithmicXAxis;
		BulkEditCheckBox* CheckBox_Music_Scale;
		BulkEditChoice* Choice_Music_Colour;
		BulkEditChoice* Choice_Music_Scaling;
		BulkEditChoice* Choice_Music_Type;
		BulkEditSlider* Slider_Music_Bars;
		BulkEditSlider* Slider_Music_EndNote;
		BulkEditSlider* Slider_Music_Offset;
		BulkEditSlider* Slider_Music_Sensitivity;
		BulkEditSlider* Slider_Music_StartNote;
		BulkEditTextCtrl* TextCtrl_Music_Bars;
		BulkEditTextCtrl* TextCtrl_Music_EndNote;
		BulkEditTextCtrl* TextCtrl_Music_Offset;
		BulkEditTextCtrl* TextCtrl_Music_Sensitivity;
		BulkEditTextCtrl* TextCtrl_Music_StartNote;
		BulkEditValueCurveButton* BitmapButton_Music_OffsetVC;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		xlLockButton* BitmapButton_Music_Bars;
		xlLockButton* BitmapButton_Music_Colour;
		xlLockButton* BitmapButton_Music_EndNote;
		xlLockButton* BitmapButton_Music_Fade;
		xlLockButton* BitmapButton_Music_LogarithmicXAxis;
		xlLockButton* BitmapButton_Music_Offset;
		xlLockButton* BitmapButton_Music_Scale;
		xlLockButton* BitmapButton_Music_Scaling;
		xlLockButton* BitmapButton_Music_Sensitivity;
		xlLockButton* BitmapButton_Music_StartNote;
		xlLockButton* BitmapButton_Music_Type;
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
		static const long ID_CHECKBOX_Music_LogarithmicX;
		static const long ID_BITMAPBUTTON_CHECKBOX_Music_LogarithmicX;
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
