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

 //(*Headers(LifePanel)
 #include <wx/panel.h>
 class wxBitmapButton;
 class wxFlexGridSizer;
 class wxSlider;
 class wxStaticText;
 class wxTextCtrl;
 //*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class LifePanel: public xlEffectPanel
{
	public:

		LifePanel(wxWindow* parent);
		virtual ~LifePanel();
		virtual void ValidateWindow() override;

		//(*Declarations(LifePanel)
		BulkEditSlider* Slider_Life_Count;
		BulkEditSlider* Slider_Life_Seed;
		BulkEditSlider* Slider_Life_Speed;
		wxStaticText* StaticText134;
		wxStaticText* StaticText35;
		wxStaticText* StaticText37;
		xlLockButton* BitmapButton_LifeCount;
		xlLockButton* BitmapButton_LifeSpeed;
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
		//*)

		DECLARE_EVENT_TABLE()
};
