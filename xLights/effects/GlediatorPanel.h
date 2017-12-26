#ifndef GLEDIATORPANEL_H
#define GLEDIATORPANEL_H

//(*Headers(GlediatorPanel)
#include <wx/panel.h>
class wxFilePickerCtrl;
class wxStaticText;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../BulkEditControls.h"

class GlediatorPanel: public wxPanel
{
	public:

		GlediatorPanel(wxWindow* parent);
		virtual ~GlediatorPanel();

		//(*Declarations(GlediatorPanel)
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		BulkEditChoice* Choice_Glediator_DurationTreatment;
		wxFilePickerCtrl* FilePickerCtrl_Glediator_Filename;
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

#endif
