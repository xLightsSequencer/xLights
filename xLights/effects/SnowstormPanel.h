#ifndef SNOWSTORMPANEL_H
#define SNOWSTORMPANEL_H

//(*Headers(SnowstormPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class SnowstormPanel: public wxPanel
{
	public:

		SnowstormPanel(wxWindow* parent);
		virtual ~SnowstormPanel();

		//(*Declarations(SnowstormPanel)
		BulkEditSlider* Slider_Snowstorm_Count;
		BulkEditSlider* Slider_Snowstorm_Length;
		BulkEditSlider* Slider_Snowstorm_Speed;
		wxStaticText* StaticText180;
		wxStaticText* StaticText45;
		wxStaticText* StaticText51;
		xlLockButton* BitmapButton_SnowstormCount;
		xlLockButton* BitmapButton_SnowstormLength;
		xlLockButton* BitmapButton_SnowstormSpeed;
		//*)

	protected:

		//(*Identifiers(SnowstormPanel)
		static const long ID_STATICTEXT_Snowstorm_Count;
		static const long ID_SLIDER_Snowstorm_Count;
		static const long IDD_TEXTCTRL_Snowstorm_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Snowstorm_Count;
		static const long ID_STATICTEXT_Snowstorm_Length;
		static const long ID_SLIDER_Snowstorm_Length;
		static const long IDD_TEXTCTRL_Snowstorm_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Snowstorm_Length;
		static const long ID_STATICTEXT_Snowstorm_Speed;
		static const long ID_SLIDER_Snowstorm_Speed;
		static const long IDD_TEXTCTRL_Snowstorm_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Snowstorm_Speed;
		//*)

	public:

		//(*Handlers(SnowstormPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
