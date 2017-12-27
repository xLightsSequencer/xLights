#ifndef SNOWSTORMPANEL_H
#define SNOWSTORMPANEL_H

//(*Headers(SnowstormPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class SnowstormPanel: public wxPanel
{
	public:

		SnowstormPanel(wxWindow* parent);
		virtual ~SnowstormPanel();

		//(*Declarations(SnowstormPanel)
		wxStaticText* StaticText45;
		wxStaticText* StaticText180;
		wxBitmapButton* BitmapButton_SnowstormCount;
		BulkEditSlider* Slider_Snowstorm_Speed;
		wxBitmapButton* BitmapButton_SnowstormLength;
		BulkEditSlider* Slider_Snowstorm_Count;
		BulkEditSlider* Slider_Snowstorm_Length;
		wxStaticText* StaticText51;
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
