#ifndef SNOWFLAKESPANEL_H
#define SNOWFLAKESPANEL_H

//(*Headers(SnowflakesPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

#include "../BulkEditControls.h"

class SnowflakesPanel: public wxPanel
{
	public:

		SnowflakesPanel(wxWindow* parent);
		virtual ~SnowflakesPanel();

		//(*Declarations(SnowflakesPanel)
		BulkEditSlider* Slider_Snowflakes_Speed;
		wxStaticText* StaticText79;
		wxStaticText* StaticText80;
		wxBitmapButton* BitmapButton_SnowflakesType;
		wxStaticText* StaticText181;
		BulkEditChoice* Choice_Falling;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton_SnowflakesSpeed;
		BulkEditSlider* Slider_Snowflakes_Type;
		wxStaticText* StaticText2;
		BulkEditSlider* Slider_Snowflakes_Count;
		wxBitmapButton* BitmapButton_Falling;
		wxBitmapButton* BitmapButton_SnowflakesCount;
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
		static const long ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed;
		static const long ID_STATICTEXT_Falling;
		static const long ID_CHOICE_Falling;
		static const long ID_STATICTEXT2;
		static const long ID_BITMAPBUTTON_CHOICE_Falling;
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
