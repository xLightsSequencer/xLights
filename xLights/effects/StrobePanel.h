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

class StrobePanel: public wxPanel
{
	public:

		StrobePanel(wxWindow* parent);
		virtual ~StrobePanel();

		//(*Declarations(StrobePanel)
		wxBitmapButton* BitmapButton_StrobeDuration;
		wxStaticText* StaticText111;
		wxCheckBox* CheckBox_Strobe_Music;
		wxStaticText* StaticText110;
		wxSlider* Slider_Strobe_Type;
		wxBitmapButton* BitmapButton_Strobe_Type;
		wxBitmapButton* BitmapButton_Strobe_Music;
		wxTextCtrl* TextCtrl_Strobe_Duration;
		wxTextCtrl* TextCtrl_Strobe_Type;
		wxStaticText* StaticText112;
		wxBitmapButton* BitmapButton_NumberStrobes;
		wxSlider* Slider_Number_Strobes;
		wxTextCtrl* TextCtrl_Number_Strobes;
		wxSlider* Slider_Strobe_Duration;
		//*)

	protected:

		//(*Identifiers(StrobePanel)
		static const long ID_STATICTEXT112;
		static const long ID_SLIDER_Number_Strobes;
		static const long IDD_TEXTCTRL_Number_Strobes;
		static const long ID_BITMAPBUTTON49;
		static const long ID_STATICTEXT113;
		static const long ID_SLIDER_Strobe_Duration;
		static const long IDD_TEXTCTRL_Strobe_Duration;
		static const long ID_BITMAPBUTTON51;
		static const long ID_STATICTEXT114;
		static const long ID_SLIDER_Strobe_Type;
		static const long IDD_TEXTCTRL_Strobe_Type;
		static const long ID_BITMAPBUTTON50;
		static const long ID_CHECKBOX_Strobe_Music;
		static const long ID_BITMAPBUTTON_Strobe_Music;
		//*)

	public:

		//(*Handlers(StrobePanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnSlider_Strobe_TypeCmdScroll(wxScrollEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
