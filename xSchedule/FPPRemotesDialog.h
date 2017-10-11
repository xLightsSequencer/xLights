#ifndef FPPREMOTESDIALOG_H
#define FPPREMOTESDIALOG_H

//(*Headers(FPPRemotesDialog)
#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <list>

class FPPRemotesDialog: public wxDialog
{
	public:

		FPPRemotesDialog(wxWindow* parent, std::list<std::string> remotes, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FPPRemotesDialog();
        std::list<std::string> GetRemotes();

		//(*Declarations(FPPRemotesDialog)
		wxListBox* ListBox_Remotes;
		wxButton* Button_Delete;
		wxButton* Button_Edit;
		wxButton* Button_Add;
		wxButton* Button_Close;
		//*)

	protected:

		//(*Identifiers(FPPRemotesDialog)
		static const long ID_LISTBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		//*)

	private:

		//(*Handlers(FPPRemotesDialog)
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnButton_AddClick(wxCommandEvent& event);
		void OnButton_EditClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		void OnListBox_RemotesDClick(wxCommandEvent& event);
		void OnListBox_RemotesSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        void EditItem(int item);
        void ValidateWindow();
};

#endif
