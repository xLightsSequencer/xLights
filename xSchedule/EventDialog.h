#ifndef EVENTDIALOG_H
#define EVENTDIALOG_H

//(*Headers(EventDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/choicebk.h>
//*)

class EventBase;

class EventDialog: public wxDialog
{
    EventBase* _eventBase;

	public:

		EventDialog(wxWindow* parent, EventBase* eventBase, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventDialog();
        EventBase* GetEvent() { return _eventBase; }
        void ValidateWindow();

		//(*Declarations(EventDialog)
		wxChoice* Choice_Command;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_P2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxPanel* Panel3;
		wxStaticText* StaticText5;
		wxButton* Button_Cancel;
		wxChoicebook* Choicebook_EventType;
		wxTextCtrl* TextCtrl_P3;
		wxPanel* Panel2;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_P1;
		//*)

	protected:

		//(*Identifiers(EventDialog)
		static const long ID_CHOICEBOOK1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_PANEL3;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL3;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(EventDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnChoicebook_EventTypePageChanged(wxChoicebookEvent& event);
		void OnChoice_CommandSelect(wxCommandEvent& event);
		void OnTextCtrl_P1Text(wxCommandEvent& event);
		void OnTextCtrl_P2Text(wxCommandEvent& event);
		void OnTextCtrl_P3Text(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
