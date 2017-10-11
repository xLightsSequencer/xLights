#ifndef GAMMADIALOG_H
#define GAMMADIALOG_H

//(*Headers(GammaDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class GammaDialog: public wxDialog
{
    float& _gamma;
    float& _gammaR;
    float& _gammaG;
    float& _gammaB;
    size_t& _startChannel;
    size_t& _nodes;
    std::string& _description;

	public:

		GammaDialog(wxWindow* parent, size_t& startChannel, size_t& channels, float& gamma, float& gammaR, float& gammaG, float& gammaB, std::string& description, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GammaDialog();

		//(*Declarations(GammaDialog)
		wxButton* Button_Ok;
		wxStaticText* StaticText_G;
		wxTextCtrl* TextCtrl_Description;
		wxCheckBox* CheckBox_Simple;
		wxStaticText* StaticText2;
		wxStaticText* StaticText_Simple;
		wxStaticText* StaticText_R;
		wxSpinCtrl* SpinCtrl_Nodes;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_B;
		wxTextCtrl* TextCtrl_G;
		wxButton* Button_Cancel;
		wxStaticText* StaticText_B;
		wxTextCtrl* TextCtrl_R;
		wxTextCtrl* TextCtrl_Simple;
		wxSpinCtrl* SpinCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(GammaDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL5;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(GammaDialog)
		void OnCheckBox_SimpleClick(wxCommandEvent& event);
		void OnTextCtrl_ValidateNumbers(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
};

#endif
