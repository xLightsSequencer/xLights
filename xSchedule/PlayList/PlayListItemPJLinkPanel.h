#ifndef PLAYLISTITEMPJLinkPANEL_H
#define PLAYLISTITEMPJLinkPANEL_H

//(*Headers(PlayListItemPJLinkPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class PlayListItemPJLink;

class PlayListItemPJLinkPanel: public wxPanel
{
    PlayListItemPJLink* _PJLink;
    void ValidateWindow();

	public:

		PlayListItemPJLinkPanel(wxWindow* parent, PlayListItemPJLink* PJLink, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemPJLinkPanel();

		//(*Declarations(PlayListItemPJLinkPanel)
		wxChoice* Choice_Command;
		wxStaticText* StaticText2;
		wxChoice* Choice_Projector;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxStaticText* StaticText5;
		wxChoice* Choice_Parameter;
		//*)

	protected:

		//(*Identifiers(PlayListItemPJLinkPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemPJLinkPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnCheckBox_OverrideAudioClick(wxCommandEvent& event);
		void OnChoice_CommandSelect(wxCommandEvent& event);
		void OnChoice_ProjectorSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
