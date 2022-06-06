/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ScriptsDialog.h"

//(*InternalHeaders(ScriptsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/listbase.h>

#include "xLightsMain.h"

#include "automation/LuaRunner.h"
#include "ExternalHooks.h"

#include <log4cpp/Category.hh>
#include <wx/mimetype.h>
#include <wx/stdpaths.h>


//(*IdInit(ScriptsDialog)
const long ScriptsDialog::ID_STATICTEXT1 = wxNewId();
const long ScriptsDialog::ID_LISTBOX_SCRIPTS = wxNewId();
const long ScriptsDialog::ID_BUTTON_RUN = wxNewId();
const long ScriptsDialog::ID_BUTTON_REFRESH = wxNewId();
const long ScriptsDialog::ID_BUTTON_CLEAR = wxNewId();
const long ScriptsDialog::ID_TEXTCTRL_LOG = wxNewId();
//*)

const long ScriptsDialog::ID_MCU_VIEWSCRIPT = wxNewId();

BEGIN_EVENT_TABLE(ScriptsDialog, wxDialog)
	//(*EventTable(ScriptsDialog)
	//*)
END_EVENT_TABLE()


ScriptsDialog::ScriptsDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _frame = (xLightsFrame*)parent;

    //(*Initialize(ScriptsDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer4;

    Create(parent, id, _("Run Scripts"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("id"));
    SetClientSize(wxSize(600,300));
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select the Lua Script to Run."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
    ListBoxScripts = new wxListBox(this, ID_LISTBOX_SCRIPTS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_SCRIPTS"));
    FlexGridSizer1->Add(ListBoxScripts, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Run = new wxButton(this, ID_BUTTON_RUN, _("Run"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RUN"));
    FlexGridSizer4->Add(Button_Run, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Refresh = new wxButton(this, ID_BUTTON_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REFRESH"));
    FlexGridSizer4->Add(Button_Refresh, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Clear = new wxButton(this, ID_BUTTON_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLEAR"));
    FlexGridSizer4->Add(Button_Clear, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Log = new wxTextCtrl(this, ID_TEXTCTRL_LOG, wxEmptyString, wxDefaultPosition, wxSize(0,300), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_LOG"));
    FlexGridSizer1->Add(TextCtrl_Log, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_LISTBOX_SCRIPTS,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&ScriptsDialog::OnListBoxScriptsDClick);
    Connect(ID_BUTTON_RUN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnButton_RunClick);
    Connect(ID_BUTTON_REFRESH,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnButton_RefreshClick);
    Connect(ID_BUTTON_CLEAR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnButton_ClearClick);
    //*)

    Connect(ID_LISTBOX_SCRIPTS, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ScriptsDialog::OnListRClick);

    _runner = std::make_unique<LuaRunner>(_frame);

    LoadScriptDir();
}

ScriptsDialog::~ScriptsDialog()
{
	//(*Destroy(ScriptsDialog)
	//*)
}

void ScriptsDialog::OnButton_RefreshClick(wxCommandEvent& event)
{
    LoadScriptDir();
}

void ScriptsDialog::OnButton_RunClick(wxCommandEvent& event)
{
    Run_Selected_Script();
}

void ScriptsDialog::OnButton_ClearClick(wxCommandEvent& event)
{
    TextCtrl_Log->Clear();
}

void ScriptsDialog::OnListRClick(wxContextMenuEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MCU_VIEWSCRIPT, "View Script");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ScriptsDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void ScriptsDialog::OnPopup(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (event.GetId() == ID_MCU_VIEWSCRIPT) {
        int sel = ListBoxScripts->GetSelection();
        if (sel == wxNOT_FOUND) {
            return;
        }

        auto const filePath = _scripts.at(sel);
        wxFileName fn(filePath);

        wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension(fn.GetExt());

        // if there is no LUA file handler treat them as text files
        if (ft == nullptr) {
            ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
        }

        if (ft) {
            wxString command = ft->GetOpenCommand(fn.GetFullPath());
            wxUnsetEnv("LD_PRELOAD");
            wxExecute(command);
        } else {
            logger_base.warn("Unable to open script as no program can open the file %s.", (const char*)filePath.c_str());
        }
    }
}

void ScriptsDialog::OnListBoxScriptsDClick(wxCommandEvent& event)
{
    Run_Selected_Script();
}


void ScriptsDialog::LoadScriptDir()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString scriptFolder = _runner->GetUserScriptFolder();

    while ((int)ListBoxScripts->GetCount() > 0) {
        ListBoxScripts->Delete(0);
    }
    _scripts.clear();

    logger_base.info("Scanning User Script folder: %s", (const char*)scriptFolder.c_str());
    if (wxDir::Exists(scriptFolder)) {
        ProcessScriptDir(scriptFolder);
    }

    logger_base.info("Scanning System Script folder: %s", (const char*)scriptFolder.c_str());
    scriptFolder = LuaRunner::GetSystemScriptFolder();
    if (wxDir::Exists(scriptFolder)) {
        ProcessScriptDir(scriptFolder);
    }
}

void ScriptsDialog::ProcessScriptDir(wxString const& dir)
{
    wxDir directory;
    directory.Open(dir);

    
    wxArrayString files;
    GetAllFilesInDir(dir, files, "*.lua");
    for (auto & file : files) {
        wxFileName fn(file);
        wxString path = fn.GetFullPath();
        _scripts.push_back(path);
        ListBoxScripts->Append(fn.GetFullName());
    }
}

void ScriptsDialog::Run_Selected_Script()
{
    int sel = ListBoxScripts->GetSelection();
    if (sel == wxNOT_FOUND) {
        DisplayError(_("Please Select an script before trying to Run it"), this);
        return;
    }
    SetCursor(wxCURSOR_WAIT);

    ListBoxScripts->Disable();
    Button_Run->Disable();
    Button_Refresh->Disable();

    auto filePath = _scripts.at(sel);

    Run_Lua_Script(filePath);

    ListBoxScripts->Enable();
    Button_Run->Enable();
    Button_Refresh->Enable();
    SetCursor(wxCURSOR_ARROW);
}

void ScriptsDialog::Run_Lua_Script(wxString const& filepath) const
{
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));


    auto LogMessage = [&](std::string const& message) {
        TextCtrl_Log->AppendText(message);
        TextCtrl_Log->AppendText("\n");
        logger_base.info("%s", (const char*)message.c_str());
    };
    _runner->Run_Script(filepath, LogMessage);
}


