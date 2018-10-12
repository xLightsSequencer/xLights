#ifndef STARTCHANNELDIALOG_H
#define STARTCHANNELDIALOG_H

//(*Headers(StartChannelDialog)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

#include <string>
#include <map>
class ModelManager;
class OutputManager;

class StartChannelDialog: public wxDialog
{
	public:

		StartChannelDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~StartChannelDialog();

		//(*Declarations(StartChannelDialog)
		wxCheckBox* CheckBox_FromThisPreviewOnly;
		wxChoice* ModelChoice;
		wxChoice* OutputChoice;
		wxChoice* ipChoice;
		wxChoice* universeChoice;
		wxRadioButton* ModelButton;
		wxRadioButton* NoneButton;
		wxRadioButton* OutputButton;
		wxRadioButton* StartModelButton;
		wxRadioButton* UniverseButton;
		wxSpinCtrl* StartChannel;
		//*)
    
        void Set(const wxString &start, const ModelManager &models, const std::string& preview);
        void SetUniverseOptionsBasedOnIP(wxString ip);
        std::string Get();
        void UpdateModels();
        OutputManager* _outputManager;
        std::map<std::string, std::string> _modelsPreview;
        std::string _preview;

	protected:

		//(*Identifiers(StartChannelDialog)
		static const long ID_SPINCTRL1;
		static const long ID_RADIOBUTTON1;
		static const long ID_RADIOBUTTON2;
		static const long ID_CHOICE2;
		static const long ID_RADIOBUTTON5;
		static const long ID_CHOICE3;
		static const long ID_CHOICE4;
		static const long ID_RADIOBUTTON3;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX1;
		static const long ID_RADIOBUTTON4;
		//*)

	private:

		//(*Handlers(StartChannelDialog)
		void OnButtonSelect(wxCommandEvent& event);
		void OnipChoiceSelect(wxCommandEvent& event);
		void OnCheckBox_FromThisPreviewOnlyClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
