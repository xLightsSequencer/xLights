#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(PlayListItemVideoPanel)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemVideo;
class VideoFilePickerCtrl;

class PlayListItemVideoPanel: public wxPanel
{
    PlayListItemVideo* _video;

    void SetWindowPositionText();
	void ValidateWindow();

	public:

		PlayListItemVideoPanel(wxWindow* parent, PlayListItemVideo* video,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemVideoPanel();

		//(*Declarations(PlayListItemVideoPanel)
		VideoFilePickerCtrl* FilePickerCtrl_VideoFile;
		wxButton* Button_PositionWindow;
		wxCheckBox* CheckBox_CacheVideo;
		wxCheckBox* CheckBox_LoopVideo;
		wxCheckBox* CheckBox_SuppressVirtualMatrix;
		wxCheckBox* CheckBox_Topmost;
		wxCheckBox* CheckBox_UseMediaPlayer;
		wxSpinCtrl* SpinCtrl_FadeIn;
		wxSpinCtrl* SpinCtrl_FadeOut;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText_Position;
		wxTextCtrl* TextCtrl_Delay;
		//*)

	protected:

		//(*Identifiers(PlayListItemVideoPanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT5;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX5;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemVideoPanel)
		void OnButton_PositionWindowClick(wxCommandEvent& event);
		void OnFilePickerCtrl_VideoFileFileChanged(wxFileDirPickerEvent& event);
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnCheckBox_UseMediaPlayerClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
