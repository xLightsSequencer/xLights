#ifndef ARTNETDIALOG_H
#define ARTNETDIALOG_H

//(*Headers(ArtNetDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class ArtNetOutput;
class OutputManager;

class ArtNetDialog: public wxDialog
{
    void ValidateWindow();
    void UniverseChange();
    ArtNetOutput* _artnet;
    OutputManager* _outputManager;

	public:

		ArtNetDialog(wxWindow* parent,ArtNetOutput* artnet, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ArtNetDialog();

		//(*Declarations(ArtNetDialog)
		wxButton* ButtonCancel;
		wxButton* ButtonOk;
		wxCheckBox* CheckBox_SuppressDuplicates;
		wxNotebook* Notebook1;
		wxPanel* Panel1;
		wxSpinCtrl* SpinCtrlChannels;
		wxSpinCtrl* SpinCtrlNet;
		wxSpinCtrl* SpinCtrlSubnet;
		wxSpinCtrl* SpinCtrlUniverse;
		wxSpinCtrl* SpinCtrlUniverseOnly;
		wxSpinCtrl* SpinCtrl_NumUniv;
		wxTextCtrl* TextCtrlDescription;
		wxTextCtrl* TextCtrlIPAddress;
		//*)

	protected:

		//(*Identifiers(ArtNetDialog)
		static const long ID_TEXTCTRL1;
		static const long ID_SPINCTRL1;
		static const long ID_SPINCTRL2;
		static const long ID_SPINCTRL3;
		static const long ID_PANEL1;
		static const long ID_SPINCTRL5;
		static const long ID_PANEL2;
		static const long ID_NOTEBOOK1;
		static const long ID_SPINCTRL6;
		static const long ID_SPINCTRL4;
		static const long ID_TEXTCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ArtNetDialog)
		void OnTextCtrlIPAddressText(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnUniverseChange(wxSpinEvent& event);
		void OnNotebook1PageChanging(wxNotebookEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
