#ifndef SNOWFLAKESPANEL_H
#define SNOWFLAKESPANEL_H

//(*Headers(SnowflakesPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../BulkEditControls.h"

class SnowflakesPanel: public wxPanel
{
	public:

		SnowflakesPanel(wxWindow* parent);
		virtual ~SnowflakesPanel();

		//(*Declarations(SnowflakesPanel)
		wxStaticText* StaticText80;
		wxBitmapButton* BitmapButton16;
		wxBitmapButton* BitmapButton_SnowflakesType;
		wxStaticText* StaticText2;
		wxStaticText* StaticText181;
		wxBitmapButton* BitmapButton_SnowflakesCount;
		BulkEditSlider* Slider_Snowflakes_Count;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Snowflakes_Type;
		wxBitmapButton* BitmapButton1;
		BulkEditSlider* Slider_Snowflakes_Speed;
		BulkEditChoice* Choice_Falling;
		wxStaticText* StaticText79;
		//*)

	protected:

		//(*Identifiers(SnowflakesPanel)
		static const long ID_STATICTEXT_Snowflakes_Count;
		static const long ID_SLIDER_Snowflakes_Count;
		static const long IDD_TEXTCTRL_Snowflakes_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Snowflakes_Count;
		static const long ID_STATICTEXT_Snowflakes_Type;
		static const long ID_SLIDER_Snowflakes_Type;
		static const long IDD_TEXTCTRL_Snowflakes_Type;
		static const long ID_BITMAPBUTTON_SLIDER_Snowflakes_Type;
		static const long ID_STATICTEXT_Snowflakes_Speed;
		static const long ID_SLIDER_Snowflakes_Speed;
		static const long IDD_TEXTCTRL_Snowflakes_Speed;
		static const long ID_BITMAPBUTTON34;
		static const long ID_STATICTEXT_Falling;
		static const long ID_CHOICE_Falling;
		static const long ID_STATICTEXT2;
		static const long ID_BITMAPBUTTON1;
		//*)

	public:

		//(*Handlers(SnowflakesPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
