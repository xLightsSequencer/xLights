#ifndef EXTRAIPSDIALOG_H
#define EXTRAIPSDIALOG_H

//(*Headers(ExtraIPsDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
//*)

#include "ScheduleOptions.h"

class ExtraIPsDialog: public wxDialog
{
    std::list<ExtraIP*>* _extraIPs;

    void EditSelected();
    void ValidateWindow();

	public:

		ExtraIPsDialog(wxWindow* parent, std::list<ExtraIP*>* extraIPs, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ExtraIPsDialog();

		//(*Declarations(ExtraIPsDialog)
		wxButton* Button_Add;
		wxButton* Button_Close;
		wxButton* Button_Delete;
		wxButton* Button_Edit;
		wxListView* ListView1;
		//*)

	protected:

		//(*Identifiers(ExtraIPsDialog)
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(ExtraIPsDialog)
		void OnButton_AddClick(wxCommandEvent& event);
		void OnButton_EditClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnListView1ItemActivated(wxListEvent& event);
		void OnListView1ItemSelect(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
