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

 //(*Headers(OptionsDialog)
 #include <wx/button.h>
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/dialog.h>
 #include <wx/listctrl.h>
 #include <wx/sizer.h>
 #include <wx/spinctrl.h>
 #include <wx/stattext.h>
 #include <wx/textctrl.h>
 //*)

class ScheduleOptions;
class CommandManager;

class OptionsDialog: public wxDialog
{
    ScheduleOptions* _options;
    bool _dragging;
    CommandManager* _commandManager;

    void LoadButtons();
    void ValidateWindow();
    void OnButtonsDragEnd(wxMouseEvent& event);
    void OnButtonsDragQuit(wxMouseEvent& event);
    void OnButtonsMouseMove(wxMouseEvent& event);

	public:

		OptionsDialog(wxWindow* parent, CommandManager* commandManager, ScheduleOptions* options, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OptionsDialog();
        void EditButton(int row);

		//(*Declarations(OptionsDialog)
		wxButton* Button_ButtonAdd;
		wxButton* Button_ButtonDelete;
		wxButton* Button_ButtonEdit;
		wxButton* Button_Cancel;
		wxButton* Button_DefaultWindowLocation;
		wxButton* Button_Export;
		wxButton* Button_Import;
		wxButton* Button_Ok;
		wxCheckBox* CheckBoxSuppressDarkMode;
		wxCheckBox* CheckBox_APIOnly;
		wxCheckBox* CheckBox_AlllowPageBypass;
		wxCheckBox* CheckBox_DisableOutputOnPingFailure;
		wxCheckBox* CheckBox_HWAcceleratedVideo;
		wxCheckBox* CheckBox_KeepScreenOn;
		wxCheckBox* CheckBox_LastStartingSequenceUsesTime;
		wxCheckBox* CheckBox_MinimiseUI;
		wxCheckBox* CheckBox_MultithreadedTransmission;
		wxCheckBox* CheckBox_RemoteAllOff;
		wxCheckBox* CheckBox_RetryOpen;
		wxCheckBox* CheckBox_RunBackground;
		wxCheckBox* CheckBox_SendOffWhenNotRunning;
		wxCheckBox* CheckBox_SimpleMode;
		wxCheckBox* CheckBox_SongMMSSFormat;
		wxCheckBox* CheckBox_SuppressAudioOnRemotes;
		wxCheckBox* CheckBox_Sync;
		wxCheckBox* CheckBox_TimecodeWaitForNextSong;
		wxChoice* Choice1;
		wxChoice* Choice_ARTNetTimeCodeFormat;
		wxChoice* Choice_AudioDevice;
		wxChoice* Choice_InputAudioDevice;
		wxChoice* Choice_Location;
		wxChoice* Choice_OnCrash;
		wxChoice* Choice_SMPTEDevice;
		wxChoice* Choice_SMPTEFrameRate;
		wxListView* ListView_Buttons;
		wxSpinCtrl* SpinCtrl_PasswordTimeout;
		wxSpinCtrl* SpinCtrl_WebServerPort;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_DefaultPage;
		wxTextCtrl* TextCtrl_Password;
		wxTextCtrl* TextCtrl_wwwRoot;
		//*)

	protected:

		//(*Identifiers(OptionsDialog)
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX6;
		static const long ID_CHECKBOX7;
		static const long ID_CHECKBOX8;
		static const long ID_CHECKBOX9;
		static const long ID_CHECKBOX10;
		static const long ID_CHECKBOX11;
		static const long ID_CHECKBOX12;
		static const long ID_CHECKBOX14;
		static const long ID_CHECKBOX15;
		static const long ID_CHECKBOX16;
		static const long ID_CHECKBOX17;
		static const long ID_CHECKBOX18;
		static const long ID_STATICTEXT2;
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_BUTTON7;
		static const long ID_BUTTON10;
		static const long ID_BUTTON9;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT13;
		static const long ID_TEXTCTRL3;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX13;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT12;
		static const long ID_CHOICE7;
		static const long ID_STATICTEXT11;
		static const long ID_CHOICE6;
		static const long ID_STATICTEXT14;
		static const long ID_CHOICE8;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT9;
		static const long ID_CHOICE4;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT10;
		static const long ID_CHOICE5;
		static const long ID_BUTTONDEFAULTWINODOWLOC;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(OptionsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_ButtonAddClick(wxCommandEvent& event);
		void OnButton_ButtonEditClick(wxCommandEvent& event);
		void OnButton_ButtonDeleteClick(wxCommandEvent& event);
		void OnTextCtrl_wwwRootText(wxCommandEvent& event);
		void OnListView_ButtonsBeginDrag(wxListEvent& event);
		void OnListView_ButtonsItemSelect(wxListEvent& event);
		void OnListView_ButtonsItemActivated(wxListEvent& event);
		void OnListView_ButtonsKeyDown(wxListEvent& event);
		void OnButton_ImportClick(wxCommandEvent& event);
		void OnButton_ExportClick(wxCommandEvent& event);
		void OnCheckBox1Click(wxCommandEvent& event);
		void OnButton_DefaultWindowLocationClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
