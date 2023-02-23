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

//(*Headers(LinkJukeboxButtonDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class ButtonControl;
class MainSequencer;

class LinkJukeboxButtonDialog: public wxDialog
{
    MainSequencer* _mainSequencer;

    void ValidateWindow();
    void LoadChoices();

	public:
		LinkJukeboxButtonDialog(wxWindow* parent, int button, ButtonControl* buttonControl, MainSequencer* mainSequencer, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LinkJukeboxButtonDialog();

		//(*Declarations(LinkJukeboxButtonDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxButton* Button_Unlink;
		wxCheckBox* CheckBox_LoopEffect;
		wxChoice* Choice_Description;
		wxChoice* Choice_Layer;
		wxChoice* Choice_Model;
		wxChoice* Choice_Time;
		wxRadioButton* RadioButton_ED;
		wxRadioButton* RadioButton_MLT;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Number;
		wxTextCtrl* TextCtrl_Tooltip;
		//*)

	protected:

		//(*Identifiers(LinkJukeboxButtonDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_RADIOBUTTON2;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE3;
		static const long ID_RADIOBUTTON1;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE4;
		static const long ID_BUTTON3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(LinkJukeboxButtonDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnChoice_DescriptionSelect(wxCommandEvent& event);
		void OnChoice_TimeSelect(wxCommandEvent& event);
		void OnChoice_LayerSelect(wxCommandEvent& event);
		void OnChoice_ModelSelect(wxCommandEvent& event);
		void OnRadioButton_Select(wxCommandEvent& event);
		void OnButton_UnlinkClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
