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

 //(*Headers(AdjustPanel)
 #include <wx/panel.h>
 class wxChoice;
 class wxFlexGridSizer;
 class wxSpinCtrl;
 class wxSpinEvent;
 class wxStaticText;
 //*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class Model;

class AdjustPanel: public xlEffectPanel
{
	std::list<Model*> GetActiveModels();

	public:

		AdjustPanel(wxWindow* parent);
		virtual ~AdjustPanel();
		virtual void ValidateWindow() override;

		//(*Declarations(AdjustPanel)
		BulkEditChoice* Choice_Action;
		BulkEditSpinCtrl* SpinCtrl_Count;
		BulkEditSpinCtrl* SpinCtrl_NthChannel;
		BulkEditSpinCtrl* SpinCtrl_StartingAt;
		BulkEditSpinCtrl* SpinCtrl_Value1;
		BulkEditSpinCtrl* SpinCtrl_Value2;
		wxFlexGridSizer* FlexGridSizer_Main;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText_Value1;
		wxStaticText* StaticText_Value2;
		//*)

	protected:

		//(*Identifiers(AdjustPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Action;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL_Value1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL_Value2;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL_NthChannel;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL_StartingAt;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL_Count;
		//*)

	public:

		//(*Handlers(AdjustPanel)
		void OnButtonRemapClick(wxCommandEvent& event);
		void OnButton_SaveAsStateClick(wxCommandEvent& event);
		void OnButton_Load_StateClick(wxCommandEvent& event);
		void OnChoice_ActionSelect(wxCommandEvent& event);
		//*)

        void OnButtonRemapRClick(wxCommandEvent& event);
        void OnChoicePopup(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};
