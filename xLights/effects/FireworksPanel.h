#ifndef FIREWORKSPANEL_H
#define FIREWORKSPANEL_H

//(*Headers(FireworksPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include <list>
#include "../BulkEditControls.h"

class FireworksPanel: public wxPanel
{
        void ValidateWindow();
	public:

		FireworksPanel(wxWindow* parent);
		virtual ~FireworksPanel();
        void SetTimingTracks(wxCommandEvent& event);

		//(*Declarations(FireworksPanel)
		BulkEditCheckBox* CheckBox_FireTiming;
		BulkEditCheckBox* CheckBox_Fireworks_Gravity;
		BulkEditCheckBox* CheckBox_Fireworks_HoldColor;
		BulkEditCheckBox* CheckBox_Fireworks_UseMusic;
		BulkEditChoice* Choice_TimingTrack;
		BulkEditSlider* Slider_Fireworks_Count;
		BulkEditSlider* Slider_Fireworks_Fade;
		BulkEditSlider* Slider_Fireworks_Num_Explosions;
		BulkEditSlider* Slider_Fireworks_Sensitivity;
		BulkEditSlider* Slider_Fireworks_Velocity;
		BulkEditSlider* Slider_Fireworks_XLocation;
		BulkEditSlider* Slider_Fireworks_XVelocity;
		BulkEditSlider* Slider_Fireworks_YLocation;
		BulkEditSlider* Slider_Fireworks_YVelocity;
		BulkEditTextCtrl* TextCtrl1;
		BulkEditTextCtrl* TextCtrl2;
		BulkEditTextCtrl* TextCtrl_Fireworks_Count;
		BulkEditTextCtrl* TextCtrl_Fireworks_Explosions;
		BulkEditTextCtrl* TextCtrl_Fireworks_Fade;
		BulkEditTextCtrl* TextCtrl_Fireworks_Sensitivity;
		BulkEditTextCtrl* TextCtrl_Fireworks_Velocity;
		BulkEditTextCtrl* TextCtrl_Fireworks_XLocation;
		BulkEditTextCtrl* TextCtrl_Fireworks_YLocation;
		BulkEditValueCurveButton* BitmapButton_Fireworks_Count;
		BulkEditValueCurveButton* BitmapButton_Fireworks_Velocity;
		BulkEditValueCurveButton* BitmapButton_Fireworks_XLocation;
		BulkEditValueCurveButton* BitmapButton_Fireworks_XVelocity;
		BulkEditValueCurveButton* BitmapButton_Fireworks_YLocation;
		BulkEditValueCurveButton* BitmapButton_Fireworks_YVelocity;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText91;
		wxStaticText* StaticText93;
		wxStaticText* StaticText94;
		wxStaticText* StaticText95;
		xlLockButton* BitmapButton1;
		xlLockButton* BitmapButton_FireworksCount;
		xlLockButton* BitmapButton_FireworksFade;
		xlLockButton* BitmapButton_FireworksNumberExplosions;
		xlLockButton* BitmapButton_FireworksVelocity;
		xlLockButton* BitmapButton_Fireworks_Sensitivity;
		xlLockButton* BitmapButton_Fireworks_UseMusic;
		//*)

	protected:

		//(*Identifiers(FireworksPanel)
		static const long ID_STATICTEXT_Fireworks_Explosions;
		static const long ID_SLIDER_Fireworks_Explosions;
		static const long IDD_TEXTCTRL_Fireworks_Explosions;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions;
		static const long ID_STATICTEXT_Fireworks_Count;
		static const long ID_SLIDER_Fireworks_Count;
		static const long ID_VALUECURVE_Fireworks_Count;
		static const long IDD_TEXTCTRL_Fireworks_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Count;
		static const long ID_STATICTEXT_Fireworks_Velocity;
		static const long ID_SLIDER_Fireworks_Velocity;
		static const long ID_VALUECURVE_Fireworks_Velocity;
		static const long IDD_TEXTCTRL_Fireworks_Velocity;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Fireworks_XVelocity;
		static const long ID_VALUECURVE_Fireworks_XVelocity;
		static const long ID_TEXTCTRL_Fireworks_XVelocity;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Fireworks_YVelocity;
		static const long ID_VALUECURVE_Fireworks_YVelocity;
		static const long ID_TEXTCTRL_Fireworks_YVelocity;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Fireworks_XLocation;
		static const long ID_VALUECURVE_Fireworks_XLocation;
		static const long ID_TEXTCTRL_Fireworks_XLocation;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_Fireworks_YLocation;
		static const long ID_VALUECURVE_Fireworks_YLocation;
		static const long ID_TEXTCTRL_Fireworks_YLocation;
		static const long ID_CHECKBOX_Fireworks_HoldColour;
		static const long ID_CHECKBOX_Fireworks_Gravity;
		static const long ID_STATICTEXT_Fireworks_Fade;
		static const long ID_SLIDER_Fireworks_Fade;
		static const long IDD_TEXTCTRL_Fireworks_Fade;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Fade;
		static const long ID_CHECKBOX_Fireworks_UseMusic;
		static const long ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic;
		static const long ID_STATICTEXT_Fireworks_Sensitivity;
		static const long ID_SLIDER_Fireworks_Sensitivity;
		static const long IDD_TEXTCTRL_Fireworks_Sensitivity;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity;
		static const long ID_CHECKBOX_FIRETIMING;
		static const long ID_BITMAPBUTTON_CHECKBOX_FIRETIMING;
		static const long ID_STATICTEXT_FIRETIMINGTRACK;
		static const long ID_CHOICE_FIRETIMINGTRACK;
		//*)

	public:

		//(*Handlers(FireworksPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_Fireworks_UseMusicClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_FireTimingClick(wxCommandEvent& event);
		void OnChoice_TimingTrackSelect(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
