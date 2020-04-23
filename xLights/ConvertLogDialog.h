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

 //(*Headers(ConvertLogDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
//*)

class ConvertLogDialog: public wxDialog
{
    wxString msgBuffer;

    public:

        void Done();
        void AppendConvertStatus(const wxString &msg, bool flushBuffer = true);
        ConvertLogDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ConvertLogDialog();

		//(*Declarations(ConvertLogDialog)
		wxTextCtrl* TextCtrlLog;
		//*)

	protected:

		//(*Identifiers(ConvertLogDialog)
		static const long ID_TEXTCTRL_LOG;
		//*)

	private:

		//(*Handlers(ConvertLogDialog)
		void OnButtonCloseClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

