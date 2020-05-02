#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(SnowflakesPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class SnowflakesPanel: public wxPanel
{
    void ValidateWindow();

	public:

		SnowflakesPanel(wxWindow* parent);
		virtual ~SnowflakesPanel();

		//(*Declarations(SnowflakesPanel)
		BulkEditChoice* Choice_Falling;
		BulkEditSlider* Slider_Snowflakes_Count;
		BulkEditSlider* Slider_Snowflakes_Speed;
		BulkEditSlider* Slider_Snowflakes_Type;
		BulkEditValueCurveButton* BitmapButton_Snowflakes_Count;
		BulkEditValueCurveButton* BitmapButton_Snowflakes_Speed;
		wxStaticText* StaticText181;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText79;
		wxStaticText* StaticText80;
		xlLockButton* BitmapButton_Falling;
		xlLockButton* BitmapButton_SnowflakesCount;
		xlLockButton* BitmapButton_SnowflakesSpeed;
		xlLockButton* BitmapButton_SnowflakesType;
		//*)

	protected:

		//(*Identifiers(SnowflakesPanel)
		static const long ID_STATICTEXT_Snowflakes_Count;
		static const long ID_SLIDER_Snowflakes_Count;
		static const long ID_VALUECURVE_Snowflakes_Count;
		static const long IDD_TEXTCTRL_Snowflakes_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Snowflakes_Count;
		static const long ID_STATICTEXT_Snowflakes_Type;
		static const long ID_SLIDER_Snowflakes_Type;
		static const long IDD_TEXTCTRL_Snowflakes_Type;
		static const long ID_BITMAPBUTTON_SLIDER_Snowflakes_Type;
		static const long ID_STATICTEXT_Snowflakes_Speed;
		static const long ID_SLIDER_Snowflakes_Speed;
		static const long ID_VALUECURVE_Snowflakes_Speed;
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
		void OnChoice_FallingSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
