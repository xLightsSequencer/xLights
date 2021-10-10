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

//(*Headers(SnowstormPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class SnowstormPanel: public xlEffectPanel
{
	public:

		SnowstormPanel(wxWindow* parent);
		virtual ~SnowstormPanel();
		virtual void ValidateWindow() override;

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
		//*)

		DECLARE_EVENT_TABLE()
};
