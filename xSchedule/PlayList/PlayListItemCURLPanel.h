#ifndef PLAYLISTITEMCURLPANEL_H
#define PLAYLISTITEMCURLPANEL_H

//(*Headers(PlayListItemCURLPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemCURL;

class PlayListItemCURLPanel: public wxPanel
{
    PlayListItemCURL* _curl;

    void ValidateWindow();

	public:

		PlayListItemCURLPanel(wxWindow* parent, PlayListItemCURL* process, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemCURLPanel();

		//(*Declarations(PlayListItemCURLPanel)
		wxChoice* Choice_ContentType;
		wxChoice* Choice_Type;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Body;
		wxTextCtrl* TextCtrl_CURLName;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_URL;
		//*)

	protected:

		//(*Identifiers(PlayListItemCURLPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemCURLPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_CURLNameText(wxCommandEvent& event);
		void OnChoice_TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
