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

//(*Headers(ScriptsDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/arrstr.h>

#include <map>
#include <memory>
#include <string>

#if __has_include("Python.h") && __has_include(<pybind11/pybind11.h>)
#define PYTHON_RUNNER
#endif

class LuaRunner;
#if defined(PYTHON_RUNNER)
class PythonRunner;
#endif
class xLightsFrame;

class ScriptsDialog : public wxDialog
{
	xLightsFrame* _frame = nullptr;
    wxArrayString _scripts;
    std::unique_ptr<LuaRunner> _runner;
	#if defined(PYTHON_RUNNER)
    std::unique_ptr<PythonRunner> _pyrunner;
	#endif

public:

	ScriptsDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~ScriptsDialog();

	//(*Declarations(ScriptsDialog)
	wxButton* Button_Clear;
	wxButton* Button_Download;
	wxButton* Button_Refresh;
	wxButton* Button_Run;
	wxListBox* ListBoxScripts;
	wxSplitterWindow* SplitterWindow1;
	wxStaticText* StaticText1;
	wxTextCtrl* TextCtrlAbout;
	wxTextCtrl* TextCtrl_Log;
	//*)



    protected:

	//(*Identifiers(ScriptsDialog)
	static const long ID_STATICTEXT1;
	static const long ID_LISTBOX_SCRIPTS;
	static const long ID_TEXTCTRL_ABOUT;
	static const long ID_SPLITTERWINDOW1;
	static const long ID_BUTTON_RUN;
	static const long ID_BUTTON_REFRESH;
	static const long ID_BUTTON_CLEAR;
	static const long ID_BUTTON_DOWNLOAD;
	static const long ID_TEXTCTRL_LOG;
	//*)

	static const long ID_MCU_VIEWSCRIPT;
	static const long ID_MCU_VIEWSCRIPTFOLDER;

private:

	//(*Handlers(ScriptsDialog)
	void OnButton_RefreshClick(wxCommandEvent& event);
	void OnButton_RunClick(wxCommandEvent& event);
	void OnButton_ClearClick(wxCommandEvent& event);
	void OnListBoxScriptsDClick(wxCommandEvent& event);
	void OnButton_DownloadClick(wxCommandEvent& event);
	void OnListBoxScriptsSelect(wxCommandEvent& event);
	//*)

	void OnListRClick(wxContextMenuEvent& event);
	void OnPopup(wxCommandEvent& event);

	void Run_Selected_Script();
	void LoadScriptDir();
    void ProcessScriptDir(wxString const& dir);
    void Run_Lua_Script(wxString const& filepath) const;
	void Run_Python_Script(wxString const& filepath) const;
	wxString ReadLuaHeader(wxString const& filepath);

	DECLARE_EVENT_TABLE()
};
