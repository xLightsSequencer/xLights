#ifndef TENDRILPANEL_H
#define TENDRILPANEL_H

//(*Headers(TendrilPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class TendrilPanel: public wxPanel
{
	public:

		TendrilPanel(wxWindow* parent);
		virtual ~TendrilPanel();

		//(*Declarations(TendrilPanel)
		wxTextCtrl* TextCtrl_Tendril_Length;
		wxTextCtrl* TextCtrl_Tendril_Dampening;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Tendril_Movement;
		wxBitmapButton* BitmapButton_TendrilSpeed;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Tendril_Trails;
		wxSlider* Slider_Tendril_Dampening;
		wxStaticText* StaticText8;
		wxBitmapButton* BitmapButton_TendrilDampening;
		wxTextCtrl* TextCtrl_Tendril_Thickness;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_Tendril_Tension;
		wxTextCtrl* TextCtrl_Tendril_Speed;
		wxStaticText* StaticText3;
		wxSlider* Slider_Tendril_TuneMovement;
		wxBitmapButton* BitmapButton_TendrilTension;
		wxBitmapButton* BitmapButton_TendrilFriction;
		wxSlider* Slider_Tendril_Speed;
		wxStaticText* StaticText5;
		wxBitmapButton* BitmapButton_TendrilLength;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_Tendril_Movement;
		wxBitmapButton* BitmapButton_Tendril_Thickness;
		wxSlider* Slider_Tendril_Friction;
		wxTextCtrl* TextCtrl_Tendril_Friction;
		wxStaticText* StaticText68;
		wxSlider* Slider_Tendril_Thickness;
		wxSlider* Slider_Tendril_Length;
		wxSlider* Slider_Tendril_Movement;
		wxBitmapButton* BitmapButton_TendrilTrails;
		wxStaticText* StaticText4;
		wxSlider* Slider_Tendril_Trails;
		wxBitmapButton* BitmapButton_Tendril_TuneMovement;
		wxTextCtrl* TextCtrl_Tendril_TuneMovement;
		wxSlider* Slider_Tendril_Tension;
		//*)

	protected:

		//(*Identifiers(TendrilPanel)
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Tendril_Movement;
		static const long ID_TEXTCTRL_Tendril_Movement;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Movement;
		static const long ID_STATICTEXT8;
		static const long IDD_SLIDER_Tendril_TuneMovement;
		static const long ID_TEXTCTRL_Tendril_TuneMovement;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement;
		static const long ID_STATICTEXT7;
		static const long IDD_SLIDER_Tendril_Thickness;
		static const long ID_TEXTCTRL_Tendril_Thickness;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Thickness;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_Tendril_Friction;
		static const long ID_TEXTCTRL_Tendril_Friction;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Friction;
		static const long ID_STATICTEXT3;
		static const long IDD_SLIDER_Tendril_Dampening;
		static const long ID_TEXTCTRL_Tendril_Dampening;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Dampening;
		static const long ID_STATICTEXT4;
		static const long IDD_SLIDER_Tendril_Tension;
		static const long ID_TEXTCTRL_Tendril_Tension;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Tension;
		static const long ID_STATICTEXT5;
		static const long IDD_SLIDER_Tendril_Trails;
		static const long ID_TEXTCTRL_Tendril_Trails;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Trails;
		static const long ID_STATICTEXT6;
		static const long IDD_SLIDER_Tendril_Length;
		static const long ID_TEXTCTRL_Tendril_Length;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Length;
		static const long ID_STATICTEXT27;
		static const long IDD_SLIDER_Tendril_Speed;
		static const long ID_TEXTCTRL_Tendril_Speed;
		static const long IID_BITMAPBUTTON_CHOICE_Tendril_Speed;
		//*)

	public:

		//(*Handlers(TendrilPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoiceTendrilDirectionSelect(wxCommandEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
