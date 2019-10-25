#ifndef xxxDIALOG_H
#define xxxDIALOG_H

//(*Headers(xxxDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "xxxEthernetOutput.h"

class xxxEthernetOutput;
class OutputManager;

class xxxDialog: public wxDialog
{
    xxxEthernetOutput* _xxx;
    OutputManager* _outputManager;
    void ValidateWindow();

	public:

		xxxDialog(wxWindow* parent, xxxEthernetOutput* xxx, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~xxxDialog();

		//(*Declarations(xxxDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_SuppressDuplicates;
		wxSpinCtrl* SpinCtrl_Id;
		wxSpinCtrl* SpinCtrl_Pixels;
		wxSpinCtrl* SpinCtrl_Port;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Description;
		wxTextCtrl* TextCtrl_IPAddress;
		//*)

	protected:

		//(*Identifiers(xxxDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_BUTTON_OK;
		static const long ID_BUTTON_CANCEL;
		//*)

	private:

		//(*Handlers(xxxDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnTextCtrl_IPAddressText(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_DescriptionText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
