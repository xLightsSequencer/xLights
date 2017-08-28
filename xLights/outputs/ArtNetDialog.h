#ifndef ARTNETDIALOG_H
#define ARTNETDIALOG_H

//(*Headers(ArtNetDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
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
		wxStaticText* StaticText9;
		wxSpinCtrl* SpinCtrlUniverseOnly;
		wxNotebook* Notebook1;
		wxSpinCtrl* SpinCtrlSubnet;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrlIPAddress;
		wxStaticText* StaticText8;
		wxPanel* Panel1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrlDescription;
		wxCheckBox* CheckBox_SuppressDuplicates;
		wxButton* ButtonCancel;
		wxSpinCtrl* SpinCtrlUniverse;
		wxStaticText* StaticText5;
		wxSpinCtrl* SpinCtrlNet;
		wxStaticText* StaticText7;
		wxSpinCtrl* SpinCtrl_NumUniv;
		wxButton* ButtonOk;
		wxPanel* Panel2;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrlChannels;
		//*)

	protected:

		//(*Identifiers(ArtNetDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL3;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT8;
		static const long ID_SPINCTRL5;
		static const long ID_PANEL2;
		static const long ID_NOTEBOOK1;
		static const long ID_STATICTEXT9;
		static const long ID_SPINCTRL6;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT3;
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
