#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

//(*Headers(OptionsDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ScheduleOptions;
class CommandManager;

class OptionsDialog: public wxDialog
{
    ScheduleOptions* _options;
    bool _dragging;
    CommandManager* _commandManager;

    void LoadProjectors();
    void LoadButtons();
    void ValidateWindow();
    void OnButtonsDragEnd(wxMouseEvent& event);
    void OnButtonsDragQuit(wxMouseEvent& event);
    void OnButtonsMouseMove(wxMouseEvent& event);

	public:

		OptionsDialog(wxWindow* parent, CommandManager* commandManager, ScheduleOptions* options, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OptionsDialog();
        void EditProjector(int row);
        void EditButton(int row);

		//(*Declarations(OptionsDialog)
		wxSpinCtrl* SpinCtrl_WebServerPort;
		wxTextCtrl* TextCtrl_wwwRoot;
		wxCheckBox* CheckBox_RunBackground;
		wxButton* Button_ButtonDelete;
		wxButton* Button_Ok;
		wxStaticText* StaticText2;
		wxListView* ListView_Buttons;
		wxButton* Button_ButtonAdd;
		wxStaticText* StaticText6;
		wxButton* Button_DeleteProjector;
		wxListView* ListView_Projectors;
		wxCheckBox* CheckBox_Sync;
		wxButton* Button_Export;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Import;
		wxChoice* Choice_AudioDevice;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxButton* Button_Cancel;
		wxCheckBox* CheckBox_APIOnly;
		wxButton* Button_ProjectorEdit;
		wxButton* Button_AddProjector;
		wxCheckBox* CheckBox_SimpleMode;
		wxStaticText* StaticText4;
		wxButton* Button_ButtonEdit;
		wxTextCtrl* TextCtrl_Password;
		wxSpinCtrl* SpinCtrl_PasswordTimeout;
		wxCheckBox* CheckBox_SendOffWhenNotRunning;
		//*)

	protected:

		//(*Identifiers(OptionsDialog)
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT1;
		static const long ID_LISTVIEW2;
		static const long ID_BUTTON4;
		static const long ID_BUTTON8;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT2;
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_BUTTON7;
		static const long ID_BUTTON10;
		static const long ID_BUTTON9;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(OptionsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnGrid_ButtonsResize(wxSizeEvent& event);
		void OnGrid_ProjectorsResize(wxSizeEvent& event);
		void OnButton_AddProjectorClick(wxCommandEvent& event);
		void OnButton_ProjectorEditClick(wxCommandEvent& event);
		void OnButton_DeleteProjectorClick(wxCommandEvent& event);
		void OnButton_ButtonAddClick(wxCommandEvent& event);
		void OnButton_ButtonEditClick(wxCommandEvent& event);
		void OnButton_ButtonDeleteClick(wxCommandEvent& event);
		void OnTextCtrl_wwwRootText(wxCommandEvent& event);
		void OnListView_ButtonsBeginDrag(wxListEvent& event);
		void OnListView_ButtonsItemSelect(wxListEvent& event);
		void OnListView_ButtonsItemActivated(wxListEvent& event);
		void OnListView_ButtonsKeyDown(wxListEvent& event);
		void OnListView_ProjectorsItemSelect(wxListEvent& event);
		void OnListView_ProjectorsItemActivated(wxListEvent& event);
		void OnListView_ProjectorsKeyDown(wxListEvent& event);
		void OnButton_ImportClick(wxCommandEvent& event);
		void OnButton_ExportClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
