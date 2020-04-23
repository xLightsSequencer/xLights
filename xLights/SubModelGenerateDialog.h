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

 //(*Headers(SubModelGenerateDialog)
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class SubModelGenerateDialog: public wxDialog
{
    int _modelWidth;
    int _modelHeight;
    int _modelNodes;
    int _lastCount;

    int SetSpinValue(int step, bool down);
    void ValidateWindow();

	public:

		SubModelGenerateDialog(wxWindow* parent, int modelWidth, int modelHeight, int modelNodes, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SubModelGenerateDialog();

		//(*Declarations(SubModelGenerateDialog)
		wxChoice* Choice1;
		wxSpinCtrl* SpinCtrl_Count;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_BaseName;
		//*)

        int GetCount() const;
        wxString GetBaseName() const;
        wxString GetType() const;

	protected:

		//(*Identifiers(SubModelGenerateDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		//*)

	private:

		//(*Handlers(SubModelGenerateDialog)
		void OnChoice1Select(wxCommandEvent& event);
		void OnSpinCtrl_CountChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
