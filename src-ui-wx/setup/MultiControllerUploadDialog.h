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

#include <wx/checkbox.h>
#include <wx/srchctrl.h>
#include <wx/timer.h>

#include <vector>

#include "layout/ControllerTreeUtils.h"

class xLightsFrame;
class Controller;

class MultiControllerUploadDialog : public wxDialog
{
	xLightsFrame* _frame = nullptr;
	std::vector <Controller*> _controllers;
	std::vector<wxString> _labels;
	std::vector<bool> _checked;
	std::vector<int> _resultIcons;
	std::vector<int> _rows; // list row -> index into _controllers
	wxSearchCtrl* _filterCtrl = nullptr;
	wxCheckBox* _selectAllCB = nullptr;
	wxTimer _filterTimer;
	bool _rebuilding = false;
	ControllerTree::ControllerLedIcons _pingIcons;
	int _resultIconProcessing = -1;
	int _resultIconSuccess = -1;
	int _resultIconFailure = -1;
	wxTimer _pingRefreshTimer;
	void ValidateWindow();
	std::vector<int> GetCheckedControllers() const;
	void RefreshPingIcons();
	void RebuildList();
	bool MatchesFilter(size_t idx, const wxArrayString& tokens) const;
	void CheckRow(long row, bool checked);
	long RowForController(int idx) const;
	void UpdateSelectAll();

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
    static const long ID_MCU_CHECKSELECTED;
    static const long ID_MCU_UNCHECKSELECTED;
    static const long ID_MCU_INVERTSELECTION;

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
