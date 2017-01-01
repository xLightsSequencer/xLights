#ifndef PLAYLISTITEMFSEQPANEL_H
#define PLAYLISTITEMFSEQPANEL_H

//(*Headers(PlayListItemFSEQPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
//*)

class PlayListItemFSEQ;

class PlayListItemFSEQPanel: public wxPanel
{
    PlayListItemFSEQ* _fseq;
    void ValidateWindow();

	public:

		PlayListItemFSEQPanel(wxWindow* parent, PlayListItemFSEQ* fseq, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFSEQPanel();

		//(*Declarations(PlayListItemFSEQPanel)
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxFilePickerCtrl* FilePickerCtrl_FSEQFile;
		wxCheckBox* CheckBox_OverrideAudio;
		wxFilePickerCtrl* FilePickerCtrl_AudioFile;
		//*)

	protected:

		//(*Identifiers(PlayListItemFSEQPanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemFSEQPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event);
		void OnCheckBox_OverrideAudioClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
