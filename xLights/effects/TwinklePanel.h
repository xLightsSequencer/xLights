#ifndef TWINKLEPANEL_H
#define TWINKLEPANEL_H

//(*Headers(TwinklePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class TwinklePanel: public wxPanel
{
	public:

		TwinklePanel(wxWindow* parent);
		virtual ~TwinklePanel();

		//(*Declarations(TwinklePanel)
		BulkEditCheckBox* CheckBox_Twinkle_Strobe;
		wxBitmapButton* BitmapButton24;
		BulkEditSlider* Slider_Twinkle_Count;
		wxBitmapButton* BitmapButton_TwinkleCount;
		BulkEditSlider* Slider_Twinkle_Steps;
		wxBitmapButton* BitmapButton_TwinkleSteps;
		wxStaticText* StaticText85;
		wxStaticText* StaticText104;
		BulkEditCheckBox* CheckBox_Twinkle_ReRandom;
		wxBitmapButton* BitmapButton_TwinkleStrobe;
		//*)

	protected:

		//(*Identifiers(TwinklePanel)
		static const long ID_STATICTEXT_Twinkle_Count;
		static const long ID_SLIDER_Twinkle_Count;
		static const long IDD_TEXTCTRL_Twinkle_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Twinkle_Count;
		static const long ID_STATICTEXT_Twinkle_Steps;
		static const long ID_SLIDER_Twinkle_Steps;
		static const long IDD_TEXTCTRL_Twinkle_Steps;
		static const long ID_BITMAPBUTTON_SLIDER_Twinkle_Steps;
		static const long ID_CHECKBOX_Twinkle_Strobe;
		static const long ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe;
		static const long ID_CHECKBOX_Twinkle_ReRandom;
		static const long ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom;
		//*)

	public:

		//(*Handlers(TwinklePanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif
