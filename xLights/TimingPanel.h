#ifndef TIMINGPANEL_H
#define TIMINGPANEL_H

//(*Headers(TimingPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

class TimingPanel: public wxPanel
{
	public:

		TimingPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TimingPanel();

        void UpdateEffectLayerMix();
        wxString GetTimingString();

		//(*Declarations(TimingPanel)
		wxSlider* Slider_EffectLayerMix;
		wxTextCtrl* TextCtrl_Fadeout;
		wxPanel* Panel_Sizer;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Fadein;
		wxTextCtrl* txtCtlEffectMix;
		wxBitmapButton* BitmapButton_FadeIn;
		wxCheckBox* CheckBox_OverlayBkg;
		wxCheckBox* CheckBox_LayerMorph;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_FadeOut;
		wxBitmapButton* BitmapButton_EffectLayerMix;
		wxChoice* Choice_LayerMethod;
		wxScrolledWindow* ScrolledWindowTiming;
		wxBitmapButton* BitmapButton_CheckBox_LayerMorph;
		wxBitmapButton* BitmapButton_OverlayBkg;
		//*)

	protected:

		//(*Identifiers(TimingPanel)
		static const long ID_STATICTEXT4;
		static const long ID_CHECKBOX_LayerMorph;
		static const long ID_BITMAPBUTTON_CHECKBOX_LayerMorph;
		static const long ID_CHOICE_LayerMethod;
		static const long ID_SLIDER_EffectLayerMix;
		static const long ID_TEXTCTRL_LayerMix;
		static const long ID_BITMAPBUTTON_SLIDER_EffectLayerMix;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL_Fadein;
		static const long ID_BITMAPBUTTON_TEXTCTRL_Fadein;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL_Fadeout;
		static const long ID_BITMAPBUTTON_TEXTCTRL_Fadeout;
		static const long ID_CHECKBOX_OverlayBkg;
		static const long ID_BITMAPBUTTON_OverlayBkg;
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(TimingPanel)
		void OnCheckBox_OverlayBkgClick(wxCommandEvent& event);
		void OnBitmapButton_FadeOutClick(wxCommandEvent& event);
		void OnBitmapButton_FadeInClick(wxCommandEvent& event);
		void OnBitmapButton_OverlayBkgClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnLeftDClick(wxMouseEvent& event);
		void OnSlider_EffectLayerMixCmdScroll(wxScrollEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
