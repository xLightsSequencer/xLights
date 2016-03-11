#ifndef LYRICSDIALOG_H
#define LYRICSDIALOG_H

//(*Headers(LyricsDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class LyricsDialog: public wxDialog
{
	public:

		LyricsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LyricsDialog();

		//(*Declarations(LyricsDialog)
		wxTextCtrl* TextCtrlLyrics;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(LyricsDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_Lyrics;
		static const long ID_STATICTEXT2;
		//*)

	private:

		//(*Handlers(LyricsDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
