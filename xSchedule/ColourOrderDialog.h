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

 //(*Headers(ColourOrderDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OutputManager;

class ColourOrderDialog: public wxDialog
{
    std::string& _startChannel;
    size_t& _nodes;
    size_t& _colourOrder;
    std::string& _description;
    bool& _enabled;
    OutputManager* _outputManager;
    void ValidateWindow();

    public:

		ColourOrderDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, size_t& nodes, size_t& colourOrder, std::string& description, bool& enabled, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColourOrderDialog();

		//(*Declarations(ColourOrderDialog)
		wxStaticText* StaticText_StartChannel;
		wxCheckBox* CheckBox_Enabled;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Description;
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Nodes;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Cancel;
		wxTextCtrl* TextCtrl_StartChannel;
		wxStaticText* StaticText4;
		wxChoice* Choice1;
		//*)

	protected:

		//(*Identifiers(ColourOrderDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ColourOrderDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

