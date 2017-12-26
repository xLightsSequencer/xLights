#ifndef LIFEPANEL_H
#define LIFEPANEL_H

//(*Headers(LifePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class LifePanel: public wxPanel
{
	public:

		LifePanel(wxWindow* parent);
		virtual ~LifePanel();

		//(*Declarations(LifePanel)
		wxBitmapButton* BitmapButton_LifeSpeed;
		wxStaticText* StaticText37;
		BulkEditSlider* Slider_Life_Speed;
		wxBitmapButton* BitmapButton_LifeCount;
		BulkEditSlider* Slider_Life_Count;
		wxStaticText* StaticText134;
		BulkEditSlider* Slider_Life_Seed;
		wxStaticText* StaticText35;
		//*)

	protected:

		//(*Identifiers(LifePanel)
		static const long ID_STATICTEXT_Life_Count;
		static const long ID_SLIDER_Life_Count;
		static const long IDD_TEXTCTRL_Life_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Life_Count;
		static const long ID_STATICTEXT_Life_Seed;
		static const long ID_SLIDER_Life_Seed;
		static const long IDD_TEXTCTRL_Life_Seed;
		static const long ID_BITMAPBUTTON_SLIDER_Life_Seed;
		static const long ID_STATICTEXT_Life_Speed;
		static const long ID_SLIDER_Life_Speed;
		static const long IDD_TEXTCTRL_Life_Speed;
		//*)

	public:

		//(*Handlers(LifePanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
