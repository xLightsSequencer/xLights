#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

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
		wxButton* Button_Export;
		wxButton* Button_Import;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_APIOnly;
		wxCheckBox* CheckBox_MultithreadedTransmission;
		wxCheckBox* CheckBox_RemoteAllOff;
		wxCheckBox* CheckBox_RetryOpen;
		wxCheckBox* CheckBox_RunBackground;
		wxCheckBox* CheckBox_SendOffWhenNotRunning;
		wxCheckBox* CheckBox_SimpleMode;
		wxCheckBox* CheckBox_Sync;
		wxChoice* Choice_ARTNetTimeCodeFormat;
		wxChoice* Choice_AudioDevice;
		wxChoice* Choice_Location;
		wxChoice* Choice_OnCrash;
		wxListView* ListView_Buttons;
		wxSpinCtrl* SpinCtrl_PasswordTimeout;
		wxSpinCtrl* SpinCtrl_WebServerPort;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
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
		static const long ID_STATICTEXT2;
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_BUTTON7;
		static const long ID_BUTTON10;
		static const long ID_BUTTON9;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT9;
		static const long ID_CHOICE4;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
