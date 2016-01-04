#ifndef FIREWORKSPANEL_H
#define FIREWORKSPANEL_H

//(*Headers(FireworksPanel)
#include <wx/panel.h>
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class FireworksPanel: public wxPanel
{
	public:

		FireworksPanel(wxWindow* parent);
		virtual ~FireworksPanel();

		//(*Declarations(FireworksPanel)
		wxSlider* Slider_Fireworks_Velocity;
		wxBitmapButton* BitmapButton_FireworksCount;
		wxSlider* Slider_Fireworks_Fade;
		wxStaticText* StaticText94;
		wxBitmapButton* BitmapButton_FireworksVelocity;
		wxStaticText* StaticText91;
		wxStaticText* StaticText93;
		wxStaticText* StaticText95;
		wxBitmapButton* BitmapButton_FireworksFade;
		wxSlider* Slider_Fireworks_Count;
		wxBitmapButton* BitmapButton_FireworksNumberExplosions;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
