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

//(*Headers(VAMPPluginDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "xLightsXmlFile.h"
#include <vector>
#include <map>

#include "AudioManager.h"

class xLightsFrame;

class VAMPPluginDialog: public wxDialog
{
	public:

		VAMPPluginDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~VAMPPluginDialog();

		//(*Declarations(VAMPPluginDialog)
		wxTextCtrl* TimingName;
		wxFlexGridSizer* SettingsSizer;
		wxStaticText* Label1;
		wxStaticText* Label2;
		//*)

    wxString ProcessPlugin(xLightsXmlFile* xml_file, xLightsFrame *xLightsParent,
                       const wxString &plugin, AudioManager* media);

	protected:

		//(*Identifiers(VAMPPluginDialog)
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		//*)

	private:

		//(*Handlers(VAMPPluginDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

