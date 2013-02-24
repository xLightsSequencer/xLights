#ifndef EFFECTLISTDIALOG_H
#define EFFECTLISTDIALOG_H

//(*Headers(EffectListDialog)
#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class EffectListDialog: public wxDialog
{
	public:

		EffectListDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectListDialog();

		//(*Declarations(EffectListDialog)
		wxButton* Button_Delete;
		wxButton* Button_Rename;
		wxListBox* ListBox1;
		//*)

	protected:

		//(*Identifiers(EffectListDialog)
		static const long ID_LISTBOX1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(EffectListDialog)
		void OnButton_RenameClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
