#ifndef PLAYLISTITEMCURLPANEL_H
#define PLAYLISTITEMCURLPANEL_H

//(*Headers(PlayListItemCURLPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class PlayListItemCURL;

class PlayListItemCURLPanel: public wxPanel
{
    PlayListItemCURL* _curl;

	public:

		PlayListItemCURLPanel(wxWindow* parent, PlayListItemCURL* process, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemCURLPanel();

		//(*Declarations(PlayListItemCURLPanel)
		wxTextCtrl* TextCtrl_CURLName;
		wxTextCtrl* TextCtrl_Body;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_URL;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxStaticText* StaticText5;
		wxChoice* Choice_Type;
		wxStaticText* StaticText4;
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
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemCURLPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnCheckBox_WaitForCompletionClick(wxCommandEvent& event);
		void OnTextCtrl_CommandText(wxCommandEvent& event);
		void OnTextCtrl_CURLNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
