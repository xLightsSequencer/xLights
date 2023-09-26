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

//(*Headers(LyricsDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class LyricsDialog: public wxDialog
{
	public:

		LyricsDialog(int endtime, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LyricsDialog();

		//(*Declarations(LyricsDialog)
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrlLyrics;
		wxTextCtrl* TextCtrl_Lyric_EndTime;
		wxTextCtrl* TextCtrl_Lyric_StartTime;
		//*)

	protected:

		//(*Identifiers(LyricsDialog)
		static const long ID_TEXTCTRL_Lyrics;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL_LYRIC_STARTTIME;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL_LYRIC_ENDTIME;
		static const long ID_STATICTEXT2;
		//*)

	private:

		//(*Handlers(LyricsDialog)
		//*)

		DECLARE_EVENT_TABLE()
};
