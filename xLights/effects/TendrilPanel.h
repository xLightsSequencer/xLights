#ifndef TENDRILPANEL_H
#define TENDRILPANEL_H

//(*Headers(TendrilPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

#include "../BulkEditControls.h"

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
		BulkEditTextCtrl* TextCtrl_Tendril_Thickness;
		BulkEditTextCtrl* TextCtrl_Tendril_XOffset;
		BulkEditTextCtrl* TextCtrl_Tendril_Speed;
		BulkEditTextCtrl* TextCtrl_Tendril_ManualY;
		wxBitmapButton* BitmapButton_TendrilFriction;
		wxStaticText* StaticText13;
		BulkEditTextCtrl* TextCtrl_Tendril_YOffset;
		BulkEditTextCtrl* TextCtrl_Tendril_TuneMovement;
		BulkEditValueCurveButton* BitmapButton_Tendril_ManualXVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_YOffsetVC;
		BulkEditSlider* Slider_Tendril_Tension;
		BulkEditSlider* Slider_Tendril_Length;
		wxBitmapButton* BitmapButton_Tendril_YOffset;
		BulkEditTextCtrl* TextCtrl_Tendril_Dampening;
		BulkEditSlider* Slider_Tendril_TuneMovement;
		wxBitmapButton* BitmapButton_Tendril_Movement;
		BulkEditSlider* Slider_Tendril_ManualX;
		wxBitmapButton* BitmapButton_Tendril_Thickness;
		wxBitmapButton* BitmapButton_TendrilSpeed;
		wxBitmapButton* BitmapButton_Tendril_TuneMovement;
		wxStaticText* StaticText1;
		BulkEditValueCurveButton* BitmapButton_Tendril_XOffsetVC;
		wxStaticText* StaticText10;
		wxBitmapButton* BitmapButton_TendrilTension;
		wxBitmapButton* BitmapButton_Tendril_XOffset;
		BulkEditSlider* Slider_Tendril_ManualY;
		BulkEditTextCtrl* TextCtrl_Tendril_Friction;
		wxStaticText* StaticText3;
		BulkEditTextCtrl* TextCtrl_Tendril_Tension;
		BulkEditSlider* Slider_Tendril_Trails;
		BulkEditSlider* Slider_Tendril_Thickness;
		wxStaticText* StaticText68;
		BulkEditValueCurveButton* BitmapButton_Tendril_ManualYVC;
		BulkEditSlider* Slider_Tendril_Friction;
		wxStaticText* StaticText8;
		wxStaticText* StaticText12;
		BulkEditChoice* Choice_Tendril_Movement;
		BulkEditSlider* Slider_Tendril_XOffset;
		wxStaticText* StaticText7;
		BulkEditValueCurveButton* BitmapButton_Tendril_ThicknessVC;
		BulkEditTextCtrl* TextCtrl_Tendril_ManualX;
		wxStaticText* StaticText4;
		BulkEditTextCtrl* TextCtrl_Tendril_Length;
		BulkEditTextCtrl* TextCtrl_Tendril_Trails;
		wxStaticText* StaticText5;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_TendrilDampening;
		BulkEditSlider* Slider_Tendril_Dampening;
		wxBitmapButton* BitmapButton_TendrilTrails;
		wxStaticText* StaticText6;
		BulkEditValueCurveButton* BitmapButton_Tendril_TuneMovementVC;
		BulkEditSlider* Slider_Tendril_YOffset;
		BulkEditSlider* Slider_Tendril_Speed;
		wxStaticText* StaticText9;
		wxStaticText* StaticText11;
		wxBitmapButton* BitmapButton_TendrilLength;
		//*)

	protected:

		//(*Identifiers(TendrilPanel)
		static const long ID_STATICTEXT_Tendril_Movement;
		static const long ID_CHOICE_Tendril_Movement;
		static const long ID_STATICTEXT9;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Movement;
		static const long ID_STATICTEXT_Tendril_TuneMovement;
		static const long IDD_SLIDER_Tendril_TuneMovement;
		static const long ID_VALUECURVE_Tendril_TuneMovement;
		static const long ID_TEXTCTRL_Tendril_TuneMovement;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_TuneMovement;
		static const long ID_STATICTEXT_Tendril_Thickness;
		static const long IDD_SLIDER_Tendril_Thickness;
		static const long ID_VALUECURVE_Tendril_Thickness;
		static const long ID_TEXTCTRL_Tendril_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Thickness;
		static const long ID_STATICTEXT_Tendril_Friction;
		static const long IDD_SLIDER_Tendril_Friction;
		static const long ID_TEXTCTRL_Tendril_Friction;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Friction;
		static const long ID_STATICTEXT_Tendril_Dampening;
		static const long IDD_SLIDER_Tendril_Dampening;
		static const long ID_TEXTCTRL_Tendril_Dampening;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Dampening;
		static const long ID_STATICTEXT_Tendril_Tension;
		static const long IDD_SLIDER_Tendril_Tension;
		static const long ID_TEXTCTRL_Tendril_Tension;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Tension;
		static const long ID_STATICTEXT_Tendril_Trails;
		static const long IDD_SLIDER_Tendril_Trails;
		static const long ID_TEXTCTRL_Tendril_Trails;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Trails;
		static const long ID_STATICTEXT_Tendril_Length;
		static const long IDD_SLIDER_Tendril_Length;
		static const long ID_TEXTCTRL_Tendril_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Length;
		static const long ID_STATICTEXT_Tendril_Speed;
		static const long IDD_SLIDER_Tendril_Speed;
		static const long ID_TEXTCTRL_Tendril_Speed;
		static const long IID_BITMAPBUTTON_SLIDER_Tendril_Speed;
		static const long ID_STATICTEXT_Tendril_XOffset;
		static const long IDD_SLIDER_Tendril_XOffset;
		static const long ID_VALUECURVE_Tendril_XOffset;
		static const long ID_TEXTCTRL_Tendril_XOffset;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_XOffset;
		static const long ID_STATICTEXT_Tendril_YOffset;
		static const long IDD_SLIDER_Tendril_YOffset;
		static const long ID_VALUECURVE_Tendril_YOffset;
		static const long ID_TEXTCTRL_Tendril_YOffset;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_YOffset;
		static const long ID_STATICTEXT_Tendril_ManualX;
		static const long IDD_SLIDER_Tendril_ManualX;
		static const long ID_VALUECURVE_Tendril_ManualX;
		static const long ID_TEXTCTRL_Tendril_ManualX;
		static const long ID_STATICTEXT_Tendril_ManualY;
		static const long IDD_SLIDER_Tendril_ManualY;
		static const long ID_VALUECURVE_Tendril_ManualY;
		static const long ID_TEXTCTRL_Tendril_ManualY;
		//*)

	public:

		//(*Handlers(TendrilPanel)
		void OnLockButtonClick(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
		void OnChoice_Tendril_MovementSelect(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

    DECLARE_EVENT_TABLE()
};

#endif
