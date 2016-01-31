#ifndef VAMPPLUGINDIALOG_H
#define VAMPPLUGINDIALOG_H

//(*Headers(VAMPPluginDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
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
		wxStaticText* Label1;
		wxStaticText* Label2;
		wxTextCtrl* TimingName;
		wxFlexGridSizer* SettingsSizer;
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

#endif
