#ifndef PLAYLISTITEMRUNPROCESSPANEL_H
#define PLAYLISTITEMRUNPROCESSPANEL_H

//(*Headers(PlayListItemRunProcessPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
//*)

class PlayListItemRunProcess;

class PlayListItemRunProcessPanel: public wxPanel
{
    PlayListItemRunProcess* _process;

	public:

		PlayListItemRunProcessPanel(wxWindow* parent, PlayListItemRunProcess* process, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemRunProcessPanel();

		//(*Declarations(PlayListItemRunProcessPanel)
		wxStaticText* StaticText2;
		wxCheckBox* CheckBox_WaitForCompletion;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_Command;
		wxTextCtrl* TextCtrl_RunProcessName;
		//*)

	protected:

		//(*Identifiers(PlayListItemRunProcessPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemRunProcessPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnCheckBox_WaitForCompletionClick(wxCommandEvent& event);
		void OnTextCtrl_CommandText(wxCommandEvent& event);
		void OnTextCtrl_RunProcessNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
