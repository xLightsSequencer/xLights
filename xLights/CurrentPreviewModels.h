#ifndef CURRENTPREVIEWMODELS_H
#define CURRENTPREVIEWMODELS_H

//(*Headers(CurrentPreviewModels)
#include <wx/sizer.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)
#include <wx/xml/xml.h>

class ModelManager;

class CurrentPreviewModels: public wxDialog
{
	public:

		CurrentPreviewModels(wxWindow* parent, wxXmlNode * mg, ModelManager &Models, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CurrentPreviewModels();

		//(*Declarations(CurrentPreviewModels)
		wxCheckListBox* CheckListBoxCurrentGroups;
		wxButton* AddButton;
		wxButton* EditButton;
		wxButton* RemoveButton;
		wxButton* RenameButton;
		//*)

	protected:

		//(*Identifiers(CurrentPreviewModels)
		static const long ID_CHECKLISTBOX_CURRENT_GROUPS;
		static const long ID_BUTTON2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON_EDIT_GROUPS;
		//*)

	private:
        wxXmlNode* mModelGroups;
        ModelManager &mModels;

        void PopulateModelGroups();
		//(*Handlers(CurrentPreviewModels)
		void OnCheckListBoxCurrentGroupsToggled(wxCommandEvent& event);
		void OnAddButtonClick(wxCommandEvent& event);
		void OnRemoveButtonClick(wxCommandEvent& event);
		void OnEditButtonClick(wxCommandEvent& event);
		void OnCurrentGroupsDClick(wxCommandEvent& event);
		void OnCurrentGroupsSelect(wxCommandEvent& event);
		void OnRenameButtonClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
