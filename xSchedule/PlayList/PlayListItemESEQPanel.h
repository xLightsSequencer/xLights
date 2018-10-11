#ifndef PLAYLISTITEMESEQPANEL_H
#define PLAYLISTITEMESEQPANEL_H

//(*Headers(PlayListItemESEQPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class PlayListItemESEQ;
class ESEQFilePickerCtrl;

class PlayListItemESEQPanel: public wxPanel
{
    PlayListItemESEQ* _ESEQ;
    void ValidateWindow();

	public:

		PlayListItemESEQPanel(wxWindow* parent, PlayListItemESEQ* ESEQ, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemESEQPanel();

		//(*Declarations(PlayListItemESEQPanel)
		wxChoice* Choice_BlendMode;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		ESEQFilePickerCtrl* FilePickerCtrl_ESEQFile;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(PlayListItemESEQPanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemESEQPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
