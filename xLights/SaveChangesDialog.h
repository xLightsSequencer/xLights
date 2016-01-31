#ifndef SAVECHANGESDIALOG_H
#define SAVECHANGESDIALOG_H

//(*Headers(SaveChangesDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
//*)

class SaveChangesDialog: public wxDialog
{
	public:

		SaveChangesDialog(wxWindow* parent);
		virtual ~SaveChangesDialog();

        bool GetSaveChanges() { return mSaveChanges; }

		//(*Declarations(SaveChangesDialog)
		wxButton* Button_SaveChanges;
		wxButton* Button_Cancel;
		wxStaticText* StaticText1;
		wxButton* Button_DiscardChanges;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(SaveChangesDialog)
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON_SaveChanges;
		static const long ID_BUTTON_DiscardChanges;
		static const long ID_BUTTON_CANCEL;
		//*)

	private:
        bool mSaveChanges;

		//(*Handlers(SaveChangesDialog)
		void OnButton_SaveChangesClick(wxCommandEvent& event);
		void OnButton_DiscardChangesClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
