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

//(*Headers(SuperStarImportDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class SuperStarImportDialog: public wxDialog
{
    void ValidateWindow();

	public:

		SuperStarImportDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SuperStarImportDialog();

		//(*Declarations(SuperStarImportDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxChoice* ChoiceSuperStarImportModel;
		wxChoice* Choice_LayerBlend;
		wxChoice* ImageResizeChoice;
		wxSpinCtrl* TimeAdjSpinCtrl;
		wxStaticText* StaticText1;
		wxStaticText* StaticText37;
		wxStaticText* StaticText39;
		wxStaticText* StaticText3;
		wxStaticText* StaticText40;
		wxStaticText* StaticText41;
		wxStaticText* StaticText42;
		wxStaticText* StaticText44;
		wxTextCtrl* TextCtrl_SS_X_Offset;
		wxTextCtrl* TextCtrl_SS_X_Size;
		wxTextCtrl* TextCtrl_SS_Y_Offset;
		wxTextCtrl* TextCtrl_SS_Y_Size;
		//*)

	protected:

		//(*Identifiers(SuperStarImportDialog)
		static const long ID_STATICTEXT39;
		static const long ID_CHOICE_SuperStarImportModel;
		static const long ID_STATICTEXT24;
		static const long ID_TEXTCTRL_SS_X_Size;
		static const long ID_STATICTEXT42;
		static const long ID_TEXTCTRL_SS_X_Offset;
		static const long ID_STATICTEXT40;
		static const long ID_TEXTCTRL_SS_Y_Size;
		static const long ID_STATICTEXT41;
		static const long ID_TEXTCTRL_SS_Y_Offset;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT31;
		static const long ID_BUTTON1;
		static const long ID_BUTTON_CANCEL;
		//*)

	private:

		//(*Handlers(SuperStarImportDialog)
		void OnChoiceSuperStarImportModelSelect(wxCommandEvent& event);
		void OnTextCtrl_Text(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
