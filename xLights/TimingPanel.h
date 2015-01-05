#ifndef TIMINGPANEL_H
#define TIMINGPANEL_H

//(*Headers(TimingPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/bmpbuttn.h>
//*)

class TimingPanel: public wxPanel
{
	public:

		TimingPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TimingPanel();

        void UpdateSpeedText();


		//(*Declarations(TimingPanel)
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_FitToTime;
		wxTextCtrl* TextCtrl_Fadein;
		wxTextCtrl* TextCtrl_Speed;
		wxBitmapButton* BitmapButton_OverlayBkg;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_FadeOut;
		wxBitmapButton* BitmapButton_Speed;
		wxBitmapButton* BitmapButton_FadeIn;
		wxTextCtrl* TextCtrl_Fadeout;
		wxSlider* Slider_Speed;
		wxCheckBox* CheckBox_OverlayBkg;
		wxStaticText* StaticText59;
		wxCheckBox* CheckBox_FitToTime;
		//*)

	protected:

		//(*Identifiers(TimingPanel)
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL_Fadein;
		static const long ID_BITMAPBUTTON_TEXTCTRL_Fadein;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL_Fadeout;
		static const long ID_BITMAPBUTTON_TEXTCTRL_Fadeout;
		static const long ID_CHECKBOX_FitToTime;
		static const long ID_BITMAPBUTTON_CHECKBOX_FitToTime;
		static const long ID_CHECKBOX_OverlayBkg;
		static const long ID_BITMAPBUTTON_OverlayBkg;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Speed;
		static const long ID_TEXTCTRL_Speed;
		//*)

	private:

		//(*Handlers(TimingPanel)
		void OnSlider_SpeedCmdScroll(wxScrollEvent& event);
		void OnCheckBox_FitToTimeClick(wxCommandEvent& event);
		void OnCheckBox_OverlayBkgClick(wxCommandEvent& event);
		void OnBitmapButton_FadeOutClick(wxCommandEvent& event);
		void OnBitmapButton_FadeInClick(wxCommandEvent& event);
		void OnBitmapButton_FitToTimeClick(wxCommandEvent& event);
		void OnBitmapButton_OverlayBkgClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
