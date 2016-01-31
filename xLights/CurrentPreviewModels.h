#ifndef CURRENTPREVIEWMODELS_H
#define CURRENTPREVIEWMODELS_H

//(*Headers(CurrentPreviewModels)
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
//*)
#include <wx/xml/xml.h>

class CurrentPreviewModels: public wxDialog
{
	public:

		CurrentPreviewModels(wxWindow* parent, wxXmlNode* ModelGroups,wxXmlNode* Models, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CurrentPreviewModels();

		//(*Declarations(CurrentPreviewModels)
		wxStaticText* StaticText1;
		wxButton* ButtonEditGroups;
		wxCheckListBox* CheckListBoxCurrentGroups;
		//*)

	protected:

		//(*Identifiers(CurrentPreviewModels)
		static const long ID_STATICTEXT1;
		static const long ID_CHECKLISTBOX_CURRENT_GROUPS;
		static const long ID_BUTTON_EDIT_GROUPS;
		//*)

	private:
        wxXmlNode* mModelGroups;
        wxXmlNode* mModels;

        void PopulateModelGroups();
		//(*Handlers(CurrentPreviewModels)
		void OnCheckListBoxCurrentGroupsToggled(wxCommandEvent& event);
		void OnButtonEditGroupsClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
