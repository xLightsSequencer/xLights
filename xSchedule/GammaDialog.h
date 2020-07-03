#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(GammaDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OutputManager;

class GammaDialog: public wxDialog
{
    float& _gamma;
    float& _gammaR;
    float& _gammaG;
    float& _gammaB;
    std::string& _startChannel;
    size_t& _nodes;
    std::string& _description;
    bool& _enabled;
    OutputManager* _outputManager;

    void ValidateWindow();

    public:

		GammaDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, size_t& channels, float& gamma, float& gammaR, float& gammaG, float& gammaB, std::string& description, bool& enabled, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GammaDialog();

		//(*Declarations(GammaDialog)
		wxStaticText* StaticText_StartChannel;
		wxCheckBox* CheckBox_Enabled;
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
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(GammaDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL6;
		static const long ID_STATICTEXT8;
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
		static const long ID_CHECKBOX2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(GammaDialog)
		void OnCheckBox_SimpleClick(wxCommandEvent& event);
		void OnTextCtrl_ValidateNumbers(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

