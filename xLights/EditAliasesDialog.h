#ifndef EDITALIASESDIALOG_H
#define EDITALIASESDIALOG_H

//(*Headers(EditAliasesDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
//*)

class Model;

class EditAliasesDialog: public wxDialog
{
	public:

		EditAliasesDialog(wxWindow* parent, Model* mg, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EditAliasesDialog();

		//(*Declarations(EditAliasesDialog)
		wxButton* ButtonAdd;
		wxButton* ButtonCancel;
		wxButton* ButtonDelete;
		wxButton* ButtonOk;
		wxListBox* ListBoxAliases;
		//*)

	protected:

		Model* _m = nullptr;
        void ValidateWindow();

		//(*Identifiers(EditAliasesDialog)
		static const long ID_LISTBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		//*)

	private:

		//(*Handlers(EditAliasesDialog)
		void OnButtonAddClick(wxCommandEvent& event);
		void OnButtonDeleteClick(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnListBoxAliasesSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
