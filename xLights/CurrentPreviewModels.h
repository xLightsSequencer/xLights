#ifndef CURRENTPREVIEWMODELS_H
#define CURRENTPREVIEWMODELS_H

//(*Headers(CurrentPreviewModels)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class CurrentPreviewModels: public wxDialog
{
	public:

		CurrentPreviewModels(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CurrentPreviewModels();

		//(*Declarations(CurrentPreviewModels)
		wxButton* ButtonEditGroups;
		wxStaticText* StaticText1;
		wxCheckListBox* CheckListBoxCurrentGroups;
		//*)

	protected:

		//(*Identifiers(CurrentPreviewModels)
		static const long ID_STATICTEXT1;
		static const long ID_CHECKLISTBOX_CURRENT_GROUPS;
		static const long ID_BUTTON_EDIT_GROUPS;
		//*)

	private:

		//(*Handlers(CurrentPreviewModels)
		void OnCheckListBoxCurrentGroupsToggled(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
