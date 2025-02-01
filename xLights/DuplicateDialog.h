#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(DuplicateDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class DuplicateDialog: public wxDialog
{
	public:

		DuplicateDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~DuplicateDialog();

		//(*Declarations(DuplicateDialog)
		wxButton* Button_Close;
		wxButton* Button_Ok;
		wxCheckBox* Checkbox_Retain_Duration;
		wxSpinCtrl* SpinCtrl_Count;
		wxSpinCtrl* SpinCtrl_Gap;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		//*)

        int GetCount() const
        {
            return SpinCtrl_Count->GetValue();
        }
		int GetGap() const
        {
            return SpinCtrl_Gap->GetValue();
        }
        int GetRetainDuration() const {
            return Checkbox_Retain_Duration->GetValue();
        }

	protected:

		//(*Identifiers(DuplicateDialog)
		static const long ID_SPINCTRL_COUNT;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL_GAP;
		static const long ID_CHECKBOX_RetainDur;
		static const long ID_BUTTON_OK;
		static const long ID_BUTTON_CLOSE;
		//*)

	private:

		//(*Handlers(DuplicateDialog)
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		//*)


		DECLARE_EVENT_TABLE()
};
