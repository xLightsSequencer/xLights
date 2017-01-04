#ifndef PLAYLISTPANEL_H
#define PLAYLISTPANEL_H

//(*Headers(PlayListPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
//*)

class PlayList;

class PlayListPanel: public wxPanel
{
    PlayList* _playlist;

	public:

		PlayListPanel(wxWindow* parent, PlayList* pl, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListPanel();

		//(*Declarations(PlayListPanel)
		wxStaticText* StaticText2;
		wxCheckBox* CheckBox_LastStepOnce;
		wxStaticText* StaticText1;
		wxSpinCtrl* SpinCtrl_Priority;
		wxCheckBox* CheckBox_FirstOnce;
		wxTextCtrl* TextCtrl_PlayListName;
		//*)

	protected:

		//(*Identifiers(PlayListPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		//*)

	private:

		//(*Handlers(PlayListPanel)
		void OnTextCtrl_PlayListNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
