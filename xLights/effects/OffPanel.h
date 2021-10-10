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

//(*Headers(OffPanel)
#include <wx/panel.h>
class wxCheckBox;
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
		BulkEditCheckBox* CheckBox_Transparent;
		//*)

	protected:

		//(*Identifiers(OffPanel)
		static const long ID_STATICTEXT_Off_Text;
		static const long ID_CHECKBOX_Off_Transparent;
		//*)

	private:

		//(*Handlers(OffPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
