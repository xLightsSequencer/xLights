#ifndef PLAYLISTITEMFSEQPANEL_H
#define PLAYLISTITEMFSEQPANEL_H

//(*Headers(PlayListItemFSEQPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
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
		wxSlider* Slider1;
		wxChoice* Choice_BlendMode;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Priority;
		wxFilePickerCtrl* FilePickerCtrl_FSEQFile;
		wxCheckBox* CheckBox_OverrideAudio;
		wxStaticText* StaticText5;
		wxStaticText* StaticText4;
		wxFilePickerCtrl* FilePickerCtrl_AudioFile;
		wxCheckBox* CheckBox_OverrideVolume;
		//*)

	protected:

		//(*Identifiers(PlayListItemFSEQPanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_CHECKBOX2;
		static const long ID_SLIDER1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemFSEQPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event);
		void OnCheckBox_OverrideAudioClick(wxCommandEvent& event);
		void OnCheckBox_OverrideVolumeClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
