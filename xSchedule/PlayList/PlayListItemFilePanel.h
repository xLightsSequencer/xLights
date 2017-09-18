#ifndef PLAYLISTITEMFILEPANEL_H
#define PLAYLISTITEMFILEPANEL_H

//(*Headers(PlayListItemFilePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
//*)

class PlayListItemFile;

class PlayListItemFilePanel: public wxPanel
{
    PlayListItemFile* _file;

	public:

		PlayListItemFilePanel(wxWindow* parent, PlayListItemFile* file, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFilePanel();

		//(*Declarations(PlayListItemFilePanel)
		wxTextCtrl* TextCtrl_FName;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_FileName;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Content;
		wxTextCtrl* TextCtrl_Delay;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(PlayListItemFilePanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemFilePanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_ContentText(wxCommandEvent& event);
		void OnTextCtrl_FNameText(wxCommandEvent& event);
		void OnTextCtrl_FileNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
