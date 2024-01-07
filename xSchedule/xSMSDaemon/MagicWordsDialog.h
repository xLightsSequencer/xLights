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

//(*Headers(MagicWordsDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
//*)

class SMSDaemonOptions;

class MagicWordsDialog : public wxDialog
{
	SMSDaemonOptions* _options = nullptr;
	void ValidateWindow();
	void LoadList();
	void EditSelected();

public:

	MagicWordsDialog(wxWindow* parent, SMSDaemonOptions* options, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~MagicWordsDialog();

	//(*Declarations(MagicWordsDialog)
	wxButton* Button_Add;
	wxButton* Button_Close;
	wxButton* Button_Delete;
	wxButton* Button_Edit;
	wxListView* ListView_Words;
	//*)

protected:

	//(*Identifiers(MagicWordsDialog)
	static const long ID_LISTVIEW1;
	static const long ID_BUTTON1;
	static const long ID_BUTTON2;
	static const long ID_BUTTON3;
	static const long ID_BUTTON4;
	//*)

private:

	//(*Handlers(MagicWordsDialog)
	void OnButton_CloseClick(wxCommandEvent& event);
	void OnButton_DeleteClick(wxCommandEvent& event);
	void OnButton_EditClick(wxCommandEvent& event);
	void OnButton_AddClick(wxCommandEvent& event);
	void OnListView_EventsItemSelect(wxListEvent& event);
	void OnListView_EventsItemActivated(wxListEvent& event);
	//*)

	DECLARE_EVENT_TABLE()
};

