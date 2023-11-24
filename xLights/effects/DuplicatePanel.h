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

//(*Headers(DuplicatePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class DuplicatePanel: public xlEffectPanel
{
	public:

		DuplicatePanel(wxWindow* parent);
		virtual ~DuplicatePanel();
		virtual void ValidateWindow() override;

		//(*Declarations(DuplicatePanel)
		BulkEditCheckBox* CheckBox1;
		BulkEditCheckBox* CheckBox2;
		BulkEditCheckBox* CheckBox3;
		BulkEditCheckBox* CheckBox_Override_Palette;
		BulkEditChoice* Choice_Model;
		BulkEditSpinCtrl* SpinCtrl_Layer;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(DuplicatePanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Duplicate_Model;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL_Duplicate_Layer;
		static const long ID_CHECKBOX_Duplicate_Override_Palette;
		static const long ID_CHECKBOX_Duplicate_Override_Color;
		static const long ID_CHECKBOX_Duplicate_Override_Timing;
		static const long ID_CHECKBOX_Duplicate_Override_Buffer;
		//*)

	private:

		//(*Handlers(DuplicatePanel)
		void OnCheckBox1Click(wxCommandEvent& event);
		void OnChoice_ModelSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
