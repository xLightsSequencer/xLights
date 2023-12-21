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

//(*Headers(MultiControllerUploadDialog)
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class xLightsFrame;
class Controller;

class MultiControllerUploadDialog : public wxDialog
{
	xLightsFrame* _frame = nullptr;
	std::vector <Controller*> _controllers;
	void ValidateWindow();

public:

	MultiControllerUploadDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~MultiControllerUploadDialog();

	//(*Declarations(MultiControllerUploadDialog)
	wxButton* Button_Cancel;
	wxButton* Button_Upload;
	wxCheckListBox* CheckListBox_Controllers;
	wxStaticText* StaticText1;
	wxTextCtrl* TextCtrl_Log;
	//*)

protected:

	//(*Identifiers(MultiControllerUploadDialog)
	static const long ID_STATICTEXT1;
	static const long ID_CHECKLISTBOX1;
	static const long ID_BUTTON1;
	static const long ID_BUTTON2;
	static const long ID_TEXTCTRL1;
	//*)

	static const long ID_MCU_SELECTALL;
	static const long ID_MCU_SELECTNONE;
    static const long ID_MCU_SELECTACTIVE;
    static const long ID_MCU_DESELECTINACTIVE;
    static const long ID_MCU_SELECTAUTO;

private:

	//(*Handlers(MultiControllerUploadDialog)
	void OnButton_UploadClick(wxCommandEvent& event);
	void OnButton_CancelClick(wxCommandEvent& event);
	void OnCheckListBox_ControllersToggled(wxCommandEvent& event);
	//*)

	void OnListRClick(wxContextMenuEvent& event);
	void OnPopup(wxCommandEvent& event);
    void OnProxyPopup(wxCommandEvent& event);
    void SaveChecked();
    void LoadChecked();
	DECLARE_EVENT_TABLE()
};
