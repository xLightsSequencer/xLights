#ifndef TWINKLEPANEL_H
#define TWINKLEPANEL_H

//(*Headers(TwinklePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class TwinklePanel: public wxPanel
{
	public:

		TwinklePanel(wxWindow* parent);
		virtual ~TwinklePanel();

		//(*Declarations(TwinklePanel)
		BulkEditCheckBox* CheckBox_Twinkle_ReRandom;
		BulkEditCheckBox* CheckBox_Twinkle_Strobe;
		BulkEditSlider* Slider_Twinkle_Count;
		BulkEditSlider* Slider_Twinkle_Steps;
		wxBitmapButton* BitmapButton24;
		wxBitmapButton* BitmapButton_TwinkleCount;
		wxBitmapButton* BitmapButton_TwinkleSteps;
		wxBitmapButton* BitmapButton_TwinkleStrobe;
		wxStaticText* StaticText104;
		wxStaticText* StaticText85;
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
