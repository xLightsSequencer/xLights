#ifndef LIFEPANEL_H
#define LIFEPANEL_H

//(*Headers(LifePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

class LifePanel: public wxPanel
{
	public:

		LifePanel(wxWindow* parent);
		virtual ~LifePanel();

		//(*Declarations(LifePanel)
		wxBitmapButton* BitmapButton_LifeCount;
		wxStaticText* StaticText35;
		wxStaticText* StaticText37;
		wxSlider* Slider_Life_Seed;
		wxSlider* Slider_Life_Count;
		wxBitmapButton* BitmapButton_LifeSpeed;
		//*)

	protected:

		//(*Identifiers(LifePanel)
		static const long ID_STATICTEXT36;
		static const long ID_SLIDER_Life_Count;
		static const long IDD_TEXTCTRL_Life_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Life_Count;
		static const long ID_STATICTEXT37;
		static const long ID_SLIDER_Life_Seed;
		static const long IDD_TEXTCTRL_Life_Seed;
		static const long ID_BITMAPBUTTON_SLIDER_Life_Seed;
		static const long ID_SLIDER_Life_Speed;
		static const long IDD_TEXTCTRL_Life_Speed;
		//*)

	public:

		//(*Handlers(LifePanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
