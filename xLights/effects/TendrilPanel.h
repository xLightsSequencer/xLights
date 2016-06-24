#ifndef TENDRILPANEL_H
#define TENDRILPANEL_H

//(*Headers(TendrilPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class TendrilPanel: public wxPanel
{
    void ValidateWindow();
    // these are only used for validation
    int __tuneMovement;
    int __thickness;
    int __friction;
    int __dampening;
    int __tension;
    int __trails;
    int __length;
    int __speed;
    int __horizontalOffset;
    int __verticalOffset;

	public:

		TendrilPanel(wxWindow* parent);
		virtual ~TendrilPanel();

		//(*Declarations(TendrilPanel)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_Tendril_Length;
		ValueCurveButton* BitmapButton_Tendril_ThicknessVC;
		wxTextCtrl* TextCtrl_Tendril_Dampening;
		wxBitmapButton* BitmapButton_Tendril_XOffset;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_TendrilSpeed;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Tendril_Trails;
		wxSlider* Slider_Tendril_Dampening;
		wxSlider* Slider_Tendril_ManualX;
		ValueCurveButton* BitmapButton_Tendril_ManualYVC;
		ValueCurveButton* BitmapButton_Tendril_ManualXVC;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxBitmapButton* BitmapButton_TendrilDampening;
		ValueCurveButton* BitmapButton_Tendril_TuneMovementVC;
		wxTextCtrl* TextCtrl_Tendril_ManualY;
		wxTextCtrl* TextCtrl_Tendril_Thickness;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_Tendril_Tension;
		wxTextCtrl* TextCtrl_Tendril_Speed;
		wxStaticText* StaticText3;
		ValueCurveButton* BitmapButton_Tendril_XOffsetVC;
		wxSlider* Slider_Tendril_TuneMovement;
		wxBitmapButton* BitmapButton_TendrilTension;
		wxTextCtrl* TextCtrl_Tendril_YOffset;
		wxBitmapButton* BitmapButton_TendrilFriction;
		ValueCurveButton* BitmapButton_Tendril_YOffsetVC;
		wxSlider* Slider_Tendril_Speed;
		wxSlider* Slider_Tendril_XOffset;
		wxStaticText* StaticText5;
		wxBitmapButton* BitmapButton_TendrilLength;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_Tendril_Movement;
		wxBitmapButton* BitmapButton_Tendril_Thickness;
		wxSlider* Slider_Tendril_Friction;
		wxTextCtrl* TextCtrl_Tendril_Friction;
		wxStaticText* StaticText12;
		wxStaticText* StaticText68;
		wxChoice* Choice_Tendril_Movement;
		wxSlider* Slider_Tendril_Thickness;
		wxSlider* Slider_Tendril_YOffset;
		wxTextCtrl* TextCtrl_Tendril_XOffset;
		wxSlider* Slider_Tendril_Length;
		wxBitmapButton* BitmapButton_Tendril_YOffset;
		wxBitmapButton* BitmapButton_TendrilTrails;
		wxSlider* Slider_Tendril_ManualY;
		wxStaticText* StaticText4;
		wxSlider* Slider_Tendril_Trails;
		wxBitmapButton* BitmapButton_Tendril_TuneMovement;
		wxTextCtrl* TextCtrl_Tendril_ManualX;
		wxTextCtrl* TextCtrl_Tendril_TuneMovement;
		wxSlider* Slider_Tendril_Tension;
		//*)

	protected:

		//(*Identifiers(TendrilPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Tendril_Movement;
		static const long ID_STATICTEXT9;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Movement;
		static const long ID_STATICTEXT8;
		static const long IDD_SLIDER_Tendril_TuneMovement;
		static const long ID_VALUECURVE_Tendril_TuneMovement;
		static const long ID_TEXTCTRL_Tendril_TuneMovement;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement;
		static const long ID_STATICTEXT7;
		static const long IDD_SLIDER_Tendril_Thickness;
		static const long ID_VALUECURVE_Tendril_Thickness;
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
		static const long ID_STATICTEXT10;
		static const long IDD_SLIDER_Tendril_XOffset;
		static const long ID_VALUECURVE_Tendril_XOffset;
		static const long ID_TEXTCTRL_Tendril_XOffset;
		static const long ID_BITMAPBUTTON_Tendril_XOffset;
		static const long ID_STATICTEXT11;
		static const long IDD_SLIDER_Tendril_YOffset;
		static const long ID_VALUECURVE_Tendril_YOffset;
		static const long ID_TEXTCTRL_Tendril_YOffset;
		static const long ID_BITMAPBUTTON_Tendril_YOffset;
		static const long ID_STATICTEXT12;
		static const long IDD_SLIDER_Tendril_ManualX;
		static const long ID_VALUECURVE_Tendril_ManualX;
		static const long ID_TEXTCTRL_Tendril_ManualX;
		static const long ID_STATICTEXT13;
		static const long IDD_SLIDER_Tendril_ManualY;
		static const long ID_VALUECURVE_Tendril_ManualY;
		static const long ID_TEXTCTRL_Tendril_ManualY;
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
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnChoiceTendrilDirectionSelect(wxCommandEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnChoice_Tendril_MovementSelect(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
