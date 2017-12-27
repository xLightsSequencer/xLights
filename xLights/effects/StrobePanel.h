#ifndef STROBEPANEL_H
#define STROBEPANEL_H

//(*Headers(StrobePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class StrobePanel: public wxPanel
{
	public:

		StrobePanel(wxWindow* parent);
		virtual ~StrobePanel();

		//(*Declarations(StrobePanel)
		wxBitmapButton* BitmapButton_StrobeDuration;
		BulkEditSlider* Slider_Strobe_Duration;
		wxStaticText* StaticText111;
		wxStaticText* StaticText110;
		BulkEditSlider* Slider_Number_Strobes;
		BulkEditTextCtrl* TextCtrl_Strobe_Type;
		wxBitmapButton* BitmapButton_Strobe_Type;
		BulkEditTextCtrl* TextCtrl_Strobe_Duration;
		wxBitmapButton* BitmapButton_Strobe_Music;
		wxStaticText* StaticText112;
		wxBitmapButton* BitmapButton_NumberStrobes;
		BulkEditCheckBox* CheckBox_Strobe_Music;
		BulkEditTextCtrl* TextCtrl_Number_Strobes;
		BulkEditSlider* Slider_Strobe_Type;
		//*)

	protected:

		//(*Identifiers(StrobePanel)
		static const long ID_STATICTEXT_Number_Strobes;
		static const long ID_SLIDER_Number_Strobes;
		static const long IDD_TEXTCTRL_Number_Strobes;
		static const long ID_BITMAPBUTTON49;
		static const long ID_STATICTEXT_Strobe_Duration;
		static const long ID_SLIDER_Strobe_Duration;
		static const long IDD_TEXTCTRL_Strobe_Duration;
		static const long ID_BITMAPBUTTON51;
		static const long ID_STATICTEXT_Strobe_Type;
		static const long ID_SLIDER_Strobe_Type;
		static const long IDD_TEXTCTRL_Strobe_Type;
		static const long ID_BITMAPBUTTON50;
		static const long ID_CHECKBOX_Strobe_Music;
		static const long ID_BITMAPBUTTON_Strobe_Music;
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
