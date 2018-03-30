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

#include "../BulkEditControls.h"

class LifePanel: public wxPanel
{
	public:

		LifePanel(wxWindow* parent);
		virtual ~LifePanel();

		//(*Declarations(LifePanel)
		wxBitmapButton* BitmapButton_LifeCount;
		wxStaticText* StaticText35;
		wxStaticText* StaticText37;
		BulkEditSlider* Slider_Life_Count;
		BulkEditSlider* Slider_Life_Seed;
		BulkEditSlider* Slider_Life_Speed;
		wxStaticText* StaticText134;
		wxBitmapButton* BitmapButton_LifeSpeed;
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
