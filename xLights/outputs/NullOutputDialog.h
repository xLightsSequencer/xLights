#ifndef NULLOUTPUTDIALOG_H
#define NULLOUTPUTDIALOG_H

//(*Headers(NullOutputDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class NullOutput;
class OutputManager;

class NullOutputDialog: public wxDialog
{
    NullOutput* _null;
    OutputManager* _outputManager;
    void ValidateWindow() {};

	public:

		NullOutputDialog(wxWindow* parent, NullOutput* null, OutputManager* outputManager,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NullOutputDialog();

		//(*Declarations(NullOutputDialog)
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Description;
		wxSpinCtrl* NumChannelsSpinCtrl;
		wxStaticText* StaticText1;
		wxButton* Button_Cancel;
		//*)

	protected:

		//(*Identifiers(NullOutputDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_TEXTCTRL_DESCRIPTION;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(NullOutputDialog)
		void OnTextCtrl_DescriptionText(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
