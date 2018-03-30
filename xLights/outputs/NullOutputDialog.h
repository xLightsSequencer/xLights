#ifndef NULLOUTPUTDIALOG_H
#define NULLOUTPUTDIALOG_H

//(*Headers(NullOutputDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
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
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxSpinCtrl* NumChannelsSpinCtrl;
		wxSpinCtrl* SpinCtrl1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_Description;
		//*)

	protected:

		//(*Identifiers(NullOutputDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
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
