#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(MultiControllerUploadDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/timer.h>

#include "layout/ControllerTreeUtils.h"

class xLightsFrame;
class Controller;

class MultiControllerUploadDialog : public wxDialog
{
	xLightsFrame* _frame = nullptr;
	std::vector <Controller*> _controllers;
	ControllerTree::ControllerLedIcons _pingIcons;
	int _resultIconProcessing = -1;
	int _resultIconSuccess = -1;
	int _resultIconFailure = -1;
	wxTimer _pingRefreshTimer;
	void ValidateWindow();
	wxArrayInt GetCheckedRows() const;
	void RefreshPingIcons();

public:

	MultiControllerUploadDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~MultiControllerUploadDialog();

	//(*Declarations(MultiControllerUploadDialog)
	wxButton* Button_Cancel;
	wxButton* Button_Upload;
	wxListCtrl* ListCtrl_Controllers;
	wxStaticText* StaticText1;
	wxTextCtrl* TextCtrl_Log;
	//*)

protected:

	//(*Identifiers(MultiControllerUploadDialog)
	static const long ID_STATICTEXT1;
	static const long ID_LISTCTRL_CONTROLLERS;
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
	void OnListCtrl_ControllersItemChecked(wxListEvent& event);
	//*)

	void OnListRClick(wxContextMenuEvent& event);
	void OnPopup(wxCommandEvent& event);
    void OnProxyPopup(wxCommandEvent& event);
    void SaveChecked();
    void LoadChecked();
	DECLARE_EVENT_TABLE()
};
