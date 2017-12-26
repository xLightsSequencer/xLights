#ifndef FIREWORKSPANEL_H
#define FIREWORKSPANEL_H

//(*Headers(FireworksPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
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
		BulkEditSlider* Slider_Fireworks_Velocity;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_Fireworks_Sensitivity;
		BulkEditChoice* Choice_TimingTrack;
		wxBitmapButton* BitmapButton_FireworksCount;
		BulkEditCheckBox* CheckBox_Fireworks_UseMusic;
		BulkEditTextCtrl* TextCtrl_Fireworks_Fade;
		wxStaticText* StaticText1;
		BulkEditTextCtrl* TextCtrl_Fireworks_Sensitivity;
		wxBitmapButton* BitmapButton1;
		BulkEditSlider* Slider_Fireworks_Sensitivity;
		wxStaticText* StaticText94;
		BulkEditTextCtrl* TextCtrl_Fireworks_Velocity;
		wxBitmapButton* BitmapButton_FireworksVelocity;
		BulkEditTextCtrl* TextCtrl_Fireworks_Count;
		wxStaticText* StaticText91;
		BulkEditSlider* Slider_Fireworks_Num_Explosions;
		wxStaticText* StaticText93;
		wxStaticText* StaticText95;
		BulkEditTextCtrl* TextCtrl_Fireworks_Explosions;
		wxBitmapButton* BitmapButton_FireworksFade;
		BulkEditSlider* Slider_Fireworks_Fade;
		wxBitmapButton* BitmapButton_FireworksNumberExplosions;
		wxBitmapButton* BitmapButton_Fireworks_UseMusic;
		BulkEditSlider* Slider_Fireworks_Count;
		BulkEditCheckBox* CheckBox_FireTiming;
		//*)

	protected:

		//(*Identifiers(FireworksPanel)
		static const long ID_STATICTEXT_Fireworks_Explosions;
		static const long ID_SLIDER_Fireworks_Explosions;
		static const long IDD_TEXTCTRL_Fireworks_Explosions;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions;
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
		static const long ID_BITMAPBUTTON_Fireworks_UseMusic;
		static const long ID_STATICTEXT_Fireworks_Sensitivity;
		static const long ID_SLIDER_Fireworks_Sensitivity;
		static const long IDD_TEXTCTRL_Fireworks_Sensitivity;
		static const long ID_BITMAPBUTTON_Fireworks_Sensitivity;
		static const long ID_CHECKBOX_FIRETIMING;
		static const long ID_BITMAPBUTTON_FIRETIMING;
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
