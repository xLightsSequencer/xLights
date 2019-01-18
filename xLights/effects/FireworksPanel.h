#ifndef FIREWORKSPANEL_H
#define FIREWORKSPANEL_H

//(*Headers(FireworksPanel)
#include <wx/panel.h>
#include "FireworksEffect.h"
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
		BulkEditCheckBox* CheckBox_Fireworks_UseMusic;
        BulkEditCheckBox* CheckBox_Fireworks_UseLocation;
		BulkEditChoice* Choice_TimingTrack;
		BulkEditSlider* Slider_Fireworks_Count;
		BulkEditSlider* Slider_Fireworks_Fade;
		BulkEditSlider* Slider_Fireworks_Num_Explosions;
		BulkEditSlider* Slider_Fireworks_Sensitivity;
		BulkEditSlider* Slider_Fireworks_Velocity;
        BulkEditSlider* Slider_Fireworks_LocationX;
        BulkEditSlider* Slider_Fireworks_LocationY;
		BulkEditTextCtrl* TextCtrl_Fireworks_Count;
		BulkEditTextCtrl* TextCtrl_Fireworks_Explosions;
		BulkEditTextCtrl* TextCtrl_Fireworks_Fade;
		BulkEditTextCtrl* TextCtrl_Fireworks_Sensitivity;
		BulkEditTextCtrl* TextCtrl_Fireworks_Velocity;
        BulkEditTextCtrl* TextCtrl_Fireworks_LocationX;
        BulkEditTextCtrl* TextCtrl_Fireworks_LocationY;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText91;
		wxStaticText* StaticText93;
		wxStaticText* StaticText94;
		wxStaticText* StaticText95;
        wxStaticText* StaticText100;
        wxStaticText* StaticText170;
        BulkEditValueCurveButton* ValueCurve_Fireworks_LocationX;
        BulkEditValueCurveButton* ValueCurve_Fireworks_LocationY;
		xlLockButton* BitmapButton1;
		xlLockButton* BitmapButton_FireworksCount;
		xlLockButton* BitmapButton_FireworksFade;
		xlLockButton* BitmapButton_FireworksNumberExplosions;
		xlLockButton* BitmapButton_FireworksVelocity;
		xlLockButton* BitmapButton_Fireworks_Sensitivity;
		xlLockButton* BitmapButton_Fireworks_UseMusic;
        xlLockButton* BitmapButton_Fireworks_UseLocation;
        xlLockButton* BitmapButton_Fireworks_LocationX;
        xlLockButton* BitmapButton_Fireworks_LocationY;
		//*)

	protected:

		//(*Identifiers(FireworksPanel)
		static const long ID_STATICTEXT_Fireworks_Explosions;
		static const long ID_SLIDER_Fireworks_Explosions;
		static const long IDD_TEXTCTRL_Fireworks_Explosions;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions;
		static const long ID_STATICTEXT_Fireworks_Count;
		static const long ID_SLIDER_Fireworks_Count;
		static const long IDD_TEXTCTRL_Fireworks_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Count;
		static const long ID_STATICTEXT_Fireworks_Velocity;
		static const long ID_SLIDER_Fireworks_Velocity;
		static const long IDD_TEXTCTRL_Fireworks_Velocity;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity;
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
        static const long ID_CHECKBOX_Fireworks_UseLocation;
        static const long ID_SLIDER_Fireworks_LocationX;
        static const long ID_SLIDER_Fireworks_LocationY;
        static const long IDD_TEXTCTRL_Fireworks_LocationX;
        static const long IDD_TEXTCTRL_Fireworks_LocationY;
        static const long ID_STATICTEXT_Fireworks_LocationX;
        static const long ID_STATICTEXT_Fireworks_LocationY;
        static const long ID_VALUECURVE_Fireworks_LocationX;
        static const long ID_VALUECURVE_Fireworks_LocationY;
        static const long ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseLocation;
        static const long ID_BITMAPBUTTON_SLIDER_Fireworks_LocationX;
        static const long ID_BITMAPBUTTON_SLIDER_Fireworks_LocationY;
		//*)

	public:

		//(*Handlers(FireworksPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_Fireworks_UseMusicClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_FireTimingClick(wxCommandEvent& event);
		void OnChoice_TimingTrackSelect(wxCommandEvent& event);
        void OnCheckBox_UseLocationClick(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
