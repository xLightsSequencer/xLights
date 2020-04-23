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

 //(*Headers(ConfigureTest)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class TestOptions;

class ConfigureTest: public wxDialog
{
    TestOptions* _testOptions = nullptr;

    public:

		ConfigureTest(wxWindow* parent, TestOptions* testOptions, wxWindowID id = wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~ConfigureTest();

		//(*Declarations(ConfigureTest)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxChoice* Choice1;
		wxSpinCtrl* SpinCtrl1;
		wxSpinCtrl* SpinCtrl2;
		wxSpinCtrl* SpinCtrl3;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(ConfigureTest)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ConfigureTest)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
