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

class TwinklePanel: public wxPanel
{
	public:

		TwinklePanel(wxWindow* parent);
		virtual ~TwinklePanel();

		//(*Declarations(TwinklePanel)
		wxCheckBox* CheckBox_Twinkle_Strobe;
		wxBitmapButton* BitmapButton24;
		wxBitmapButton* BitmapButton_TwinkleCount;
		wxSlider* Slider_Twinkle_Count;
		wxSlider* Slider_Twinkle_Steps;
		wxBitmapButton* BitmapButton_TwinkleSteps;
		wxStaticText* StaticText85;
		wxStaticText* StaticText104;
		wxBitmapButton* BitmapButton_TwinkleStrobe;
		//*)

	protected:

		//(*Identifiers(TwinklePanel)
		static const long ID_STATICTEXT86;
		static const long ID_SLIDER_Twinkle_Count;
		static const long IDD_TEXTCTRL_Twinkle_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Twinkle_Count;
		static const long ID_STATICTEXT105;
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
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
