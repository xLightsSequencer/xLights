#ifndef EVENTSDIALOG_H
#define EVENTSDIALOG_H

//(*Headers(EventsDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ScheduleOptions;

class EventsDialog: public wxDialog
{
    ScheduleOptions* _scheduleOptions;
    void ValidateWindow();
    void LoadList();
    void EditSelected();

	public:

		EventsDialog(wxWindow* parent, ScheduleOptions* scheduleOptions ,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventsDialog();

		//(*Declarations(EventsDialog)
		wxButton* Button_Delete;
		wxButton* Button_Edit;
		wxButton* Button_Add;
		wxButton* Button_Close;
		wxListView* ListView_Events;
		//*)

	protected:

		//(*Identifiers(EventsDialog)
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		//*)

	private:

		//(*Handlers(EventsDialog)
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		void OnButton_EditClick(wxCommandEvent& event);
		void OnButton_AddClick(wxCommandEvent& event);
		void OnListView_EventsItemSelect(wxListEvent& event);
		void OnListView_EventsItemActivated(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
