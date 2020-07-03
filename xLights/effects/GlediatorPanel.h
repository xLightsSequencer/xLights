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

 //(*Headers(GlediatorPanel)
#include <wx/panel.h>
class wxChoice;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxStaticText;
//*)

#include "../BulkEditControls.h"

class GlediatorPanel: public wxPanel
{
	public:

		GlediatorPanel(wxWindow* parent);
		virtual ~GlediatorPanel();

		//(*Declarations(GlediatorPanel)
		BulkEditChoice* Choice_Glediator_DurationTreatment;
		BulkEditFilePickerCtrl* FilePickerCtrl_Glediator_Filename;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)
    
        wxString defaultDir;

	protected:

		//(*Identifiers(GlediatorPanel)
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL_Glediator_Filename;
		static const long ID_STATICTEXT_Glediator_DurationTreatment;
		static const long ID_CHOICE_Glediator_DurationTreatment;
		//*)

	public:

		//(*Handlers(GlediatorPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};
