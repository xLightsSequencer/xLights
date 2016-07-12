#ifndef STARTCHANNELDIALOG_H
#define STARTCHANNELDIALOG_H

//(*Headers(StartChannelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

#include <string>
class ModelManager;

class StartChannelDialog: public wxDialog
{
	public:

		StartChannelDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~StartChannelDialog();

		//(*Declarations(StartChannelDialog)
		wxRadioButton* NoneButton;
		wxChoice* ModelChoice;
		wxSpinCtrl* StartChannel;
		wxSpinCtrl* UniverseSpin;
		wxSpinCtrl* OutputSpin;
		wxRadioButton* OutputButton;
		wxRadioButton* ModelButton;
		wxRadioButton* StartModelButton;
		wxRadioButton* UniverseButton;
		//*)
    
        void Set(const wxString &start, const ModelManager &models);
        std::string Get();

	protected:

		//(*Identifiers(StartChannelDialog)
		static const long ID_SPINCTRL1;
		static const long ID_RADIOBUTTON1;
		static const long ID_RADIOBUTTON2;
		static const long ID_SPINCTRL2;
		static const long ID_RADIOBUTTON5;
		static const long ID_SPINCTRL3;
		static const long ID_RADIOBUTTON3;
		static const long ID_CHOICE1;
		static const long ID_RADIOBUTTON4;
		//*)

	private:

		//(*Handlers(StartChannelDialog)
		void OnButtonSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
