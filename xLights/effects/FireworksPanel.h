#ifndef FIREWORKSPANEL_H
#define FIREWORKSPANEL_H

//(*Headers(FireworksPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
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
		wxStaticText* StaticText91;
		BulkEditSlider* Slider_Fireworks_Num_Explosions;
		wxBitmapButton* BitmapButton_FireworksCount;
		BulkEditSlider* Slider_Fireworks_Fade;
		wxStaticText* StaticText93;
		wxBitmapButton* BitmapButton_Fireworks_Sensitivity;
		wxBitmapButton* BitmapButton_FireworksFade;
		BulkEditSlider* Slider_Fireworks_Count;
		wxStaticText* StaticText95;
		BulkEditCheckBox* CheckBox_Fireworks_UseMusic;
		wxBitmapButton* BitmapButton_FireworksVelocity;
		BulkEditSlider* Slider_Fireworks_Sensitivity;
		BulkEditTextCtrl* TextCtrl_Fireworks_Fade;
		BulkEditTextCtrl* TextCtrl_Fireworks_Velocity;
		BulkEditSlider* Slider_Fireworks_Velocity;
		wxBitmapButton* BitmapButton_FireworksNumberExplosions;
		wxStaticText* StaticText1;
		wxStaticText* StaticText94;
		BulkEditTextCtrl* TextCtrl_Fireworks_Explosions;
		BulkEditCheckBox* CheckBox_FireTiming;
		BulkEditTextCtrl* TextCtrl_Fireworks_Count;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_Fireworks_UseMusic;
		wxBitmapButton* BitmapButton1;
		BulkEditChoice* Choice_TimingTrack;
		BulkEditTextCtrl* TextCtrl_Fireworks_Sensitivity;
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
