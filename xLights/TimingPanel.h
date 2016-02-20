#ifndef TIMINGPANEL_H
#define TIMINGPANEL_H

//(*Headers(TimingPanel)
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
//*)

class Model;

class TimingPanel: public wxPanel
{
	public:

		TimingPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TimingPanel();

        wxString GetTimingString();

		//(*Declarations(TimingPanel)
		wxBitmapButton* BitmapButton_ZoomCycles;
		wxBitmapButton* BitmapButton_EffectBlur;
		wxBitmapButton* BitmapButton_RotoZoom;
		wxTextCtrl* TextCtrl_ZoomCycles;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_ZoomRotation;
		wxStaticText* StaticText6;
		wxChoice* BufferStyleChoice;
		wxBitmapButton* BitmapButton_ZoomRotation;
		wxPanel* Panel_Sizer;
		wxSlider* Slider_ZoomInOut;
		wxStaticText* StaticText8;
		wxTextCtrl* TextCtrl_Fadein;
		wxBitmapButton* BitmapButton_OverlayBkg;
		wxSlider* Slider_EffectLayerMix;
		wxStaticText* StaticText3;
		wxCheckBox* CheckBox_RotoZoom;
		wxBitmapButton* BitmapButton_FadeOut;
		wxChoice* BufferTransform;
		wxTextCtrl* TextCtrl_ZoomInOut;
		wxSlider* Slider_ZoomRotation;
		wxSlider* Slider_ZoomCycles;
		wxBitmapButton* BitmapButton_FadeIn;
		wxBitmapButton* BitmapButton_CheckBox_LayerMorph;
		wxCheckBox* CheckBox_LayerMorph;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxChoice* Choice_LayerMethod;
		wxBitmapButton* BitmapButton_EffectLayerMix;
		wxBitmapButton* BitmapButton_ZoomInOut;
		wxTextCtrl* TextCtrl_Fadeout;
		wxScrolledWindow* ScrolledWindowTiming;
		wxTextCtrl* txtCtlEffectBlur;
		wxCheckBox* CheckBox_OverlayBkg;
		wxTextCtrl* txtCtlEffectMix;
		wxSlider* Slider_EffectBlur;
		//*)
    
    
        void SetDefaultControls(const Model *model);

	protected:

		//(*Identifiers(TimingPanel)
		static const long ID_CHECKBOX_LayerMorph;
		static const long ID_BITMAPBUTTON_CHECKBOX_LayerMorph;
		static const long ID_CHOICE_LayerMethod;
		static const long ID_SLIDER_EffectLayerMix;
		static const long IDD_TEXTCTRL_EffectLayerMix;
		static const long ID_BITMAPBUTTON_SLIDER_EffectLayerMix;
		static const long ID_CHOICE_BufferStyle;
		static const long ID_BITMAPBUTTON_CHOICE_BufferStyle;
		static const long ID_CHOICE_BufferTransform;
		static const long ID_BITMAPBUTTON_CHOICE_BufferTransform;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_EffectBlur;
		static const long IDD_TEXTCTRL_EffectBlur;
		static const long ID_BITMAPBUTTON_SLIDER_EffectBlur;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL_Fadein;
		static const long ID_BITMAPBUTTON_TEXTCTRL_Fadein;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL_Fadeout;
		static const long ID_BITMAPBUTTON_TEXTCTRL_Fadeout;
		static const long ID_CHECKBOX_OverlayBkg;
		static const long ID_BITMAPBUTTON_OverlayBkg;
		static const long ID_CHECKBOX_RotoZoom;
		static const long ID_BITMAPBUTTON_CHECKBOX_RotoZoom;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_ZoomCycles;
		static const long IDD_TEXTCTRL_ZoomCycles;
		static const long ID_BITMAPBUTTON_SLIDER_ZoomCycles;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER_ZoomRotation;
		static const long IDD_TEXTCTRL_ZoomRotation;
		static const long ID_BITMAPBUTTON_SLIDER_ZoomRotation;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_ZoomInOut;
		static const long IDD_TEXTCTRL_ZoomInOut;
		static const long ID_BITMAPBUTTON_SLIDER_ZoomInOut;
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(TimingPanel)
        void OnResize(wxSizeEvent& event);

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
