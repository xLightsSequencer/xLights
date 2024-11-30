#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(OffPanel)
#include <wx/panel.h>
class wxChoice;
class wxFlexGridSizer;
class wxStaticText;
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class OffPanel: public xlEffectPanel
{
	public:

		OffPanel(wxWindow* parent);
		virtual ~OffPanel();
		virtual void ValidateWindow() override;

		//(*Declarations(OffPanel)
		wxChoice* OffStyleChoice;
		//*)

	protected:

		//(*Identifiers(OffPanel)
		static const long ID_STATICTEXT_Off_Text;
		static const long ID_CHOICE_Off_Style;
		//*)

	private:

		//(*Handlers(OffPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
