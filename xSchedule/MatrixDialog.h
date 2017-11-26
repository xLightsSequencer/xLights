#ifndef MATRIXDIALOG_H
#define MATRIXDIALOG_H

//(*Headers(MatrixDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
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
    void SetChoiceFromString(wxChoice* choice, std::string value);

	public:

		MatrixDialog(wxWindow* parent, OutputManager* outputManager, std::string& name, std::string& orientation, std::string& startLocation, int& stringLength, int& _strings, int& _strandsPerString, std::string& _startChannel, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MatrixDialog();

		//(*Declarations(MatrixDialog)
		wxChoice* Choice_Orientation;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxSpinCtrl* SpinCtrl_StringLength;
		wxStaticText* StaticText8;
		wxSpinCtrl* SpinCtrl_StrandsPerString;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_Strings;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxButton* Button_Cancel;
		wxChoice* Choice_StartLocation;
		wxTextCtrl* TextCtrl_StartChannel;
		wxStaticText* StaticText4;
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
