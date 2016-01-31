#ifndef FIREPANEL_H
#define FIREPANEL_H

//(*Headers(FirePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

class FirePanel: public wxPanel
{
	public:

		FirePanel(wxWindow* parent);
		virtual ~FirePanel();

		//(*Declarations(FirePanel)
		wxStaticText* StaticText132;
		wxBitmapButton* BitmapButton_FireGrow;
		wxStaticText* StaticText83;
		wxBitmapButton* BitmapButton_FireWidth;
		wxBitmapButton* BitmapButton_FireHeight;
		wxSlider* Slider_Fire_Height;
		//*)

	protected:

		//(*Identifiers(FirePanel)
		static const long ID_STATICTEXT84;
		static const long ID_SLIDER_Fire_Height;
		static const long IDD_TEXTCTRL_Fire_Height;
		static const long ID_BITMAPBUTTON_SLIDER_Fire_Height;
		static const long ID_STATICTEXT133;
		static const long ID_SLIDER_Fire_HueShift;
		static const long IDD_TEXTCTRL_Fire_HueShift;
		static const long ID_BITMAPBUTTON_SLIDER_Fire_HueShift;
		static const long IDD_SLIDER_Fire_GrowthCycles;
		static const long ID_TEXTCTRL_Fire_GrowthCycles;
		static const long ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire;
		static const long ID_CHOICE_Fire_Location;
		//*)

	public:

		//(*Handlers(FirePanel)
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
