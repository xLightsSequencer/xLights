#ifndef MATRIXDIALOG_H
#define MATRIXDIALOG_H

//(*Headers(MatrixDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OutputManager;

class MatrixDialog: public wxDialog
{
    OutputManager* _outputManager;
    std::string& _name;
    int& _stringLength;
    int& _strings;
    int& _strandsPerString;
    std::string& _startChannel;
    std::string& _orientation;
    std::string& _startingLocation;

	public:

		MatrixDialog(wxWindow* parent, OutputManager* outputManager, std::string& name, std::string& orientation, std::string& startLocation, int& stringLength, int& _strings, int& _strandsPerString, std::string& _startChannel, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MatrixDialog();

		//(*Declarations(MatrixDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxChoice* Choice_Orientation;
		wxChoice* Choice_StartLocation;
		wxSpinCtrl* SpinCtrl_StrandsPerString;
		wxSpinCtrl* SpinCtrl_StringLength;
		wxSpinCtrl* SpinCtrl_Strings;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxTextCtrl* TextCtrl_Name;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(MatrixDialog)
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT8;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(MatrixDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

        void ValidateWindow();

		DECLARE_EVENT_TABLE()
};

#endif
