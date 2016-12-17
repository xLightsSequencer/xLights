#ifndef FIREWORKSPANEL_H
#define FIREWORKSPANEL_H

//(*Headers(FireworksPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include <list>

class FireworksPanel: public wxPanel
{
        void ValidateWindow();
	public:

		FireworksPanel(wxWindow* parent);
		virtual ~FireworksPanel();
        void SetTimingTrack(std::list<std::string> timingtracks);

		//(*Declarations(FireworksPanel)
		wxSlider* Slider_Fireworks_Velocity;
		wxSlider* Slider_Fireworks_Sensitivity;
		wxStaticText* StaticText2;
		wxChoice* Choice_TimingTrack;
		wxBitmapButton* BitmapButton_Fireworks_Sensitivity;
		wxBitmapButton* BitmapButton_FireworksCount;
		wxSlider* Slider_Fireworks_Fade;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton1;
		wxStaticText* StaticText94;
		wxCheckBox* CheckBox_Fireworks_UseMusic;
		wxBitmapButton* BitmapButton_FireworksVelocity;
		wxSlider* Slider_Fireworks_Num_Explosions;
		wxStaticText* StaticText91;
		wxStaticText* StaticText93;
		wxStaticText* StaticText95;
		wxCheckBox* CheckBox_FireTiming;
		wxBitmapButton* BitmapButton_FireworksFade;
		wxSlider* Slider_Fireworks_Count;
		wxBitmapButton* BitmapButton_FireworksNumberExplosions;
		wxBitmapButton* BitmapButton_Fireworks_UseMusic;
		//*)

	protected:

		//(*Identifiers(FireworksPanel)
		static const long ID_STATICTEXT95;
		static const long ID_SLIDER_Fireworks_Explosions;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions;
		static const long ID_STATICTEXT92;
		static const long ID_SLIDER_Fireworks_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Count;
		static const long ID_STATICTEXT94;
		static const long ID_SLIDER_Fireworks_Velocity;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity;
		static const long ID_STATICTEXT96;
		static const long ID_SLIDER_Fireworks_Fade;
		static const long ID_BITMAPBUTTON_SLIDER_Fireworks_Fade;
		static const long ID_CHECKBOX_Fireworks_UseMusic;
		static const long ID_BITMAPBUTTON_Fireworks_UseMusic;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Fireworks_Sensitivity;
		static const long ID_BITMAPBUTTON_Fireworks_Sensitivity;
		static const long ID_CHECKBOX_FIRETIMING;
		static const long ID_BITMAPBUTTON_FIRETIMING;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_FIRETIMINGTRACK;
		//*)

	public:

		//(*Handlers(FireworksPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_Fireworks_UseMusicClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_FireTimingClick(wxCommandEvent& event);
		void OnChoice_TimingTrackSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
