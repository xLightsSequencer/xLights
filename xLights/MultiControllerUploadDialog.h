#ifndef MULTICONTROLLERUPLOADDIALOG_H
#define MULTICONTROLLERUPLOADDIALOG_H

//(*Headers(MultiControllerUploadDialog)
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class xLightsFrame;

class MultiControllerUploadDialog: public wxDialog
{
    xLightsFrame* _frame;
    std::vector<std::string> _ips;
    std::vector<std::string> _proxies;
    void ValidateWindow();

	public:

		MultiControllerUploadDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MultiControllerUploadDialog();

		//(*Declarations(MultiControllerUploadDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Upload;
		wxCheckListBox* CheckListBox_Controllers;
		wxChoice* Choice1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Log;
		//*)

	protected:

		//(*Identifiers(MultiControllerUploadDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_CHECKLISTBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_TEXTCTRL1;
		//*)

        static const long ID_MCU_SELECTALL;
        static const long ID_MCU_SELECTNONE;

	private:

		//(*Handlers(MultiControllerUploadDialog)
		void OnButton_UploadClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnCheckListBox_ControllersToggled(wxCommandEvent& event);
        //*)

        void OnListRClick(wxContextMenuEvent& event);
        void OnPopup(wxCommandEvent &event);

		DECLARE_EVENT_TABLE()
};

#endif
