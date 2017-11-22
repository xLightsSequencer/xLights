#ifndef PlayListItemFSEQVIDEOPANEL_H
#define PlayListItemFSEQVIDEOPANEL_H

//(*Headers(PlayListItemFSEQVideoPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/button.h>
//*)

class PlayListItemFSEQVideo;
class FSEQFilePickerCtrl;
class AudioFilePickerCtrl;
class VideoFilePickerCtrl;

class PlayListItemFSEQVideoPanel: public wxPanel
{
    PlayListItemFSEQVideo* _fseq;
    void ValidateWindow();

	public:

		PlayListItemFSEQVideoPanel(wxWindow* parent, PlayListItemFSEQVideo* fseq, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFSEQVideoPanel();

		//(*Declarations(PlayListItemFSEQVideoPanel)
		wxStaticText* StaticText10;
		wxSlider* Slider1;
		wxStaticText* StaticText9;
		wxCheckBox* CheckBox_LoopVideo;
		wxSpinCtrl* SpinCtrl_FadeOut;
		wxChoice* Choice_BlendMode;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		VideoFilePickerCtrl* FilePickerCtrl_VideoFile;
		wxCheckBox* CheckBox_FastStartAudio;
		wxStaticText* StaticText8;
		wxCheckBox* CheckBox_CacheVideo;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxCheckBox* CheckBox_LimitChannels;
		wxCheckBox* CheckBox_Topmost;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Priority;
		wxCheckBox* CheckBox_OverrideAudio;
		AudioFilePickerCtrl* FilePickerCtrl_AudioFile;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		FSEQFilePickerCtrl* FilePickerCtrl_FSEQFile;
		wxCheckBox* CheckBox_SuppressVirtualMatrix;
		wxButton* Button_PositionWindow;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_StartChannel;
		wxSpinCtrl* SpinCtrl_FadeIn;
		wxCheckBox* CheckBox_OverrideVolume;
		//*)

	protected:

		//(*Identifiers(PlayListItemFSEQVideoPanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_STATICTEXT8;
		static const long ID_FILEPICKERCTRL3;
		static const long ID_BUTTON1;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX6;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX3;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL3;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX7;
		static const long ID_CHECKBOX8;
		static const long ID_CHECKBOX2;
		static const long ID_SLIDER1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT10;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT9;
		static const long ID_SPINCTRL5;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemFSEQVideoPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event);
		void OnCheckBox_OverrideAudioClick(wxCommandEvent& event);
		void OnCheckBox_OverrideVolumeClick(wxCommandEvent& event);
		void OnCheckBox_LimitChannelsClick(wxCommandEvent& event);
		void OnFilePickerCtrl_VideoFileFileChanged(wxFileDirPickerEvent& event);
		void OnButton_PositionWindowClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
