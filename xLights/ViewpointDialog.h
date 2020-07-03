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

//(*Headers(ViewpointDialog)
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)

class ViewpointDialog: public wxDialog
{
	public:

		ViewpointDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ViewpointDialog();

		//(*Declarations(ViewpointDialog)
		wxButton* ApplyButton;
		wxButton* EditButton;
		wxButton* RemoveButton;
		wxButton* RenameButton;
		wxCheckListBox* CheckListBoxViewpoints;
		//*)

	protected:

		//(*Identifiers(ViewpointDialog)
		static const long ID_CHECKLISTBOX_VIEWPOINTS;
		static const long ID_BUTTON_APPLY;
		static const long ID_BUTTON_RENAME;
		static const long ID_BUTTON_EDIT;
		static const long ID_BUTTON_REMOVE;
		//*)

	private:

		//(*Handlers(ViewpointDialog)
		void OnApplyButtonClick(wxCommandEvent& event);
		void OnRenameButtonClick(wxCommandEvent& event);
		void OnEditButtonClick(wxCommandEvent& event);
		void OnRemoveButtonClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
