#ifndef STROBEPANEL_H
#define STROBEPANEL_H

//(*Headers(StrobePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class StrobePanel: public wxPanel
{
	public:

		StrobePanel(wxWindow* parent);
		virtual ~StrobePanel();

		//(*Declarations(StrobePanel)
		BulkEditCheckBox* CheckBox_Strobe_Music;
		BulkEditSlider* Slider_Number_Strobes;
		BulkEditSlider* Slider_Strobe_Duration;
		BulkEditSlider* Slider_Strobe_Type;
		BulkEditTextCtrl* TextCtrl_Number_Strobes;
		BulkEditTextCtrl* TextCtrl_Strobe_Duration;
		BulkEditTextCtrl* TextCtrl_Strobe_Type;
		wxBitmapButton* BitmapButton_NumberStrobes;
		wxBitmapButton* BitmapButton_StrobeDuration;
		wxBitmapButton* BitmapButton_Strobe_Music;
		wxBitmapButton* BitmapButton_Strobe_Type;
		wxStaticText* StaticText110;
		wxStaticText* StaticText111;
		wxStaticText* StaticText112;
		//*)

	protected:

		//(*Identifiers(StrobePanel)
		static const long ID_STATICTEXT_Number_Strobes;
		static const long ID_SLIDER_Number_Strobes;
		static const long IDD_TEXTCTRL_Number_Strobes;
		static const long ID_BITMAPBUTTON_SLIDER_Number_Strobes;
		static const long ID_STATICTEXT_Strobe_Duration;
		static const long ID_SLIDER_Strobe_Duration;
		static const long IDD_TEXTCTRL_Strobe_Duration;
		static const long ID_BITMAPBUTTON_SLIDER_Strobe_Duration;
		static const long ID_STATICTEXT_Strobe_Type;
		static const long ID_SLIDER_Strobe_Type;
		static const long IDD_TEXTCTRL_Strobe_Type;
		static const long ID_BITMAPBUTTON_SLIDER_Strobe_Type;
		static const long ID_CHECKBOX_Strobe_Music;
		static const long ID_BITMAPBUTTON_CHECKBOX_Strobe_Music;
		//*)

	public:

		//(*Handlers(StrobePanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnSlider_Strobe_TypeCmdScroll(wxScrollEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
