/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ScriptsDialog.h"

//(*InternalHeaders(ScriptsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/listbase.h>
#include <wx/wfstream.h>

#include "xLightsMain.h"

#include "automation/LuaRunner.h"

#if defined(PYTHON_RUNNER)
#include "automation/PythonRunner.h"
#endif
#include "utils/ExternalHooks.h"
#include "shared/utils/wxUtilities.h"

#include <log.h>
#include <wx/mimetype.h>
#include <wx/progdlg.h>
#include <wx/stdpaths.h>
#include <wx/txtstrm.h>
#include <utils/CurlManager.h>
#include <nlohmann/json.hpp>


//(*IdInit(ScriptsDialog)
const long ScriptsDialog::ID_STATICTEXT1 = wxNewId();
const long ScriptsDialog::ID_LISTBOX_SCRIPTS = wxNewId();
const long ScriptsDialog::ID_TEXTCTRL_ABOUT = wxNewId();
const long ScriptsDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long ScriptsDialog::ID_SPLITTERWINDOW2 = wxNewId();
const long ScriptsDialog::ID_BUTTON_RUN = wxNewId();
const long ScriptsDialog::ID_BUTTON_REFRESH = wxNewId();
const long ScriptsDialog::ID_BUTTON_CLEAR = wxNewId();
const long ScriptsDialog::ID_BUTTON_DOWNLOAD = wxNewId();
const long ScriptsDialog::ID_TEXTCTRL_LOG = wxNewId();
//*)

const long ScriptsDialog::ID_MCU_VIEWSCRIPT = wxNewId();
const long ScriptsDialog::ID_MCU_VIEWSCRIPTFOLDER = wxNewId();

BEGIN_EVENT_TABLE(ScriptsDialog, wxDialog)
	//(*EventTable(ScriptsDialog)
	//*)
END_EVENT_TABLE()


ScriptsDialog::ScriptsDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _frame = (xLightsFrame*)parent;

    //(*Initialize(ScriptsDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxPanel* PanelBottom;
    wxPanel* PanelTop;

    Create(parent, wxID_ANY, _("Run Scripts"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("wxID_ANY"));
    SetClientSize(wxSize(700,600));
    SetMinSize(wxSize(300,200));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    SplitterWindow2 = new wxSplitterWindow(this, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW2"));
    SplitterWindow2->SetSashGravity(0.5);
    SplitterWindow2->SetMinimumPaneSize(80);
    SplitterWindow2->SetSashSize(6);
    PanelTop = new wxPanel(SplitterWindow2, wxID_ANY);
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(1);
    StaticText1 = new wxStaticText(PanelTop, ID_STATICTEXT1, _("Select the Lua Script to Run."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
    SplitterWindow1 = new wxSplitterWindow(PanelTop, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetSashGravity(0.5);
    SplitterWindow1->SetSashSize(6);
    ListBoxScripts = new wxListBox(SplitterWindow1, ID_LISTBOX_SCRIPTS, wxPoint(-237,-27), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_SCRIPTS"));
    TextCtrlAbout = new wxTextCtrl(SplitterWindow1, ID_TEXTCTRL_ABOUT, wxEmptyString, wxPoint(49,15), wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_ABOUT"));
    SplitterWindow1->SplitVertically(ListBoxScripts, TextCtrlAbout, 350);
    FlexGridSizer2->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
    PanelTop->SetSizer(FlexGridSizer2);
    PanelBottom = new wxPanel(SplitterWindow2, wxID_ANY);
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(1);
    FlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    Button_Run = new wxButton(PanelBottom, ID_BUTTON_RUN, _("Run"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RUN"));
    FlexGridSizer4->Add(Button_Run, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Refresh = new wxButton(PanelBottom, ID_BUTTON_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REFRESH"));
    FlexGridSizer4->Add(Button_Refresh, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Clear = new wxButton(PanelBottom, ID_BUTTON_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLEAR"));
    FlexGridSizer4->Add(Button_Clear, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Download = new wxButton(PanelBottom, ID_BUTTON_DOWNLOAD, _("Download"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DOWNLOAD"));
    FlexGridSizer4->Add(Button_Download, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Log = new wxTextCtrl(PanelBottom, ID_TEXTCTRL_LOG, wxEmptyString, wxDefaultPosition, wxSize(0,300), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_LOG"));
    FlexGridSizer3->Add(TextCtrl_Log, 1, wxALL|wxEXPAND, 5);
    PanelBottom->SetSizer(FlexGridSizer3);
    SplitterWindow2->SplitHorizontally(PanelTop, PanelBottom, 300);
    FlexGridSizer1->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_LISTBOX_SCRIPTS, wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&ScriptsDialog::OnListBoxScriptsSelect);
    Connect(ID_LISTBOX_SCRIPTS, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, (wxObjectEventFunction)&ScriptsDialog::OnListBoxScriptsDClick);
    Connect(ID_BUTTON_RUN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ScriptsDialog::OnButton_RunClick);
    Connect(ID_BUTTON_REFRESH, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ScriptsDialog::OnButton_RefreshClick);
    Connect(ID_BUTTON_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ScriptsDialog::OnButton_ClearClick);
    Connect(ID_BUTTON_DOWNLOAD, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ScriptsDialog::OnButton_DownloadClick);
    //*)

    Connect(ID_LISTBOX_SCRIPTS, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ScriptsDialog::OnListRClick);

    // The splitters are split before the sizer has given them their real
    // size, so the pixel sash positions passed to SplitHorizontally/
    // SplitVertically above get clamped against a near-zero window and don't
    // land at 50%. Force them to true 50% now that Layout() has given every
    // window its final size.
    SplitterWindow1->SetSashPosition(SplitterWindow1->GetClientSize().GetWidth() / 2);
    SplitterWindow2->SetSashPosition(SplitterWindow2->GetClientSize().GetHeight() / 2);

    _runner = std::make_unique<LuaRunner>(_frame);
#if defined(PYTHON_RUNNER)
    _pyrunner = std::make_unique<PythonRunner>(_frame);
#endif

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
    mnu.Append(ID_MCU_VIEWSCRIPTFOLDER, "Open Folder");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ScriptsDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void ScriptsDialog::OnPopup(wxCommandEvent& event)
{
    
    if (event.GetId() == ID_MCU_VIEWSCRIPT) {
        int sel = ListBoxScripts->GetSelection();
        if (sel == wxNOT_FOUND) {
            return;
        }

        auto const filePath = _scripts.at(sel);
        wxFileName fn(filePath);

        wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension(fn.GetExt());
        wxString command;
        if (ft != nullptr) {
            command = ft->GetOpenCommand(fn.GetFullPath());
        }
        // if there is no LUA file handler or no command to open them, treat them as text files
        if (ft == nullptr || command.empty()) {
            ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
            if (ft != nullptr) {
                command = ft->GetOpenCommand(fn.GetFullPath());
            }
        }
#ifdef __APPLE__
        if (command.empty()) {
            // just bail to the standard open in editor command
            command = "open -e " + fn.GetFullPath().ToStdString();
        }
#endif

        if (!command.empty()) {
            wxUnsetEnv("LD_PRELOAD");
            spdlog::info("Opening script '{}' via '{}'", (const char*)filePath.c_str(), command.ToStdString().c_str());
            wxExecute(command);
        } else {
            spdlog::warn("Unable to open script as no program can open the file {}.", (const char*)filePath.c_str());
        }
    }else if (event.GetId() == ID_MCU_VIEWSCRIPTFOLDER) {
        int sel = ListBoxScripts->GetSelection();
        if (sel == wxNOT_FOUND) {
            wxLaunchDefaultApplication(_runner->GetUserScriptFolder());
            return;
        }
        wxLaunchDefaultApplication(wxPathOnly(_scripts.at(sel)));
    }
}

void ScriptsDialog::OnListBoxScriptsDClick(wxCommandEvent& event)
{
    Run_Selected_Script();
}


void ScriptsDialog::LoadScriptDir()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    

    wxString scriptFolder = _runner->GetUserScriptFolder();

    while ((int)ListBoxScripts->GetCount() > 0) {
        ListBoxScripts->Delete(0);
    }
    _scripts.clear();

    spdlog::info("Scanning User Script folder: {}", (const char*)scriptFolder.c_str());
    if (wxDir::Exists(scriptFolder)) {
        ProcessScriptDir(scriptFolder);
    }

    spdlog::info("Scanning System Script folder: {}", (const char*)scriptFolder.c_str());
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

#if defined(PYTHON_RUNNER)
    GetAllFilesInDir(dir, files, "*.py");
    for (auto & file : files) {
        wxFileName fn(file);
        wxString path = fn.GetFullPath();
        _scripts.push_back(path);
        ListBoxScripts->Append(fn.GetFullName());
    }
#endif
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

    if (filePath.EndsWith(".lua")) {
        Run_Lua_Script(filePath);
    } else if (filePath.EndsWith(".py")) {
        Run_Python_Script(filePath);
    }

    ListBoxScripts->Enable();
    Button_Run->Enable();
    Button_Refresh->Enable();
    SetCursor(wxCURSOR_ARROW);
}

void ScriptsDialog::Run_Lua_Script(wxString const& filepath) const
{
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets
    


    auto LogMessage = [&](std::string const& message) {
        TextCtrl_Log->AppendText(wxString::FromUTF8(message));
        TextCtrl_Log->AppendText("\n");
        spdlog::info("{}", message);
    };
    _runner->Run_Script(filepath, LogMessage);
}

void ScriptsDialog::Run_Python_Script(wxString const& filepath) const
{
#if defined(PYTHON_RUNNER)
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets
    


    auto LogMessage = [&](std::string const& message) {
        TextCtrl_Log->AppendText(wxString::FromUTF8(message));
        TextCtrl_Log->AppendText("\n");
        spdlog::info("{}", message);
    };
    _pyrunner->Run_Script(filepath, LogMessage);
#endif
}

void ScriptsDialog::OnButton_DownloadClick(wxCommandEvent& event)
{
    //https://api.github.com/repos/xLightsSequencer/xLights/contents/resources/scripts
    std::string json_data = CurlManager::HTTPSGet(R"(https://api.github.com/repos/xLightsSequencer/xLights/contents/resources/scripts)");
    std::vector<std::pair<wxString, wxString>> scripts = std::vector<std::pair<wxString, wxString>>();

    try {
        nlohmann::json val = nlohmann::json::parse(json_data);
        if (val.is_array()) {
            for (const auto& item : val) {
                if (item.is_object() && item.contains("name") && item.contains("download_url") && item.contains("type")) {
                    auto name = item.at("name").get<std::string>();
                    auto link = item.at("download_url").get<std::string>();
                    auto type = item.at("type").get<std::string>();

                    if (type == "file" && !name.empty() && !link.empty()) {
                        scripts.emplace_back(name, link);
                    }
                }
            }
        }
        
    } catch (std::exception&)
    {}
    wxArrayString itemList;
    std::transform(scripts.begin(), scripts.end(),
                           std::back_inserter(itemList), [](auto const& str) { return str.first; });

    wxMultiChoiceDialog dlg(this, "Select Scripts", "Download Scripts", itemList);
    //OptimiseDialogPosition(&dlg);
    wxString scriptFolder = _runner->GetUserScriptFolder();
    if (dlg.ShowModal() == wxID_OK ) {
        wxProgressDialog prog = wxProgressDialog("Script download", "Downloading Script ...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
        prog.Show();
        prog.CenterOnParent();

        if (!wxDir::Exists(scriptFolder)) {
            wxDir::Make(scriptFolder);
        }
        for (auto const& idx : dlg.GetSelections()) {
            auto name = scripts[idx].first;
            auto progress = [&prog](int pos) {
                int uiPos = pos / 10;
                if (uiPos < 0) {
                    uiPos = 0;
                }
                if (uiPos > 99) {
                    uiPos = 99;
                }
                return prog.Update(uiPos);
            };
            CurlManager::HTTPSGetFile(scripts[idx].second.ToStdString(), scriptFolder + wxFileName::GetPathSeparator() + name, "", "", 600, progress);
        }
        LoadScriptDir();
    }
}

void ScriptsDialog::OnListBoxScriptsSelect(wxCommandEvent& event)
{
    TextCtrlAbout->Clear();
    int sel = ListBoxScripts->GetSelection();
    auto filePath = _scripts.at(sel);

    if (filePath.EndsWith(".lua")) {
       TextCtrlAbout->SetValue( ReadLuaHeader(filePath));
    }
}

wxString ScriptsDialog::ReadLuaHeader(wxString const& filepath)
{
    wxString help_text;
    bool comment_block = false;
    if (FileExists(filepath)) {
        wxFileInputStream input(filepath);
        if (input.IsOk()) {
            wxTextInputStream text(input);
            while (!input.Eof()) {
                wxString s = text.ReadLine();
                if (s.StartsWith("--[[")) {
                    wxString temp_text = s.substr(4).Trim().Trim(false);
                    if (temp_text.Contains("--]]")) {
                        temp_text.Replace("--]]", "");
                        help_text += temp_text;
                        help_text += "\n";
                        break;
                    }
                    help_text += temp_text;
                    help_text += "\n";
                    comment_block = true;
                } else if (s.Contains("--]]")) {
                    auto idx = s.find_last_of("--]]");
                    help_text += s.substr(0,idx - 3).Trim().Trim(false);
                    help_text += "\n";
                    break;
                } else if (s.StartsWith("--")) {
                    help_text += s.substr(2).Trim().Trim(false);
                    help_text += "\n";
                } else if (comment_block) {
                    help_text += s.Trim().Trim(false);
                    help_text += "\n";
                } else {
                    break;
                }
            }
        }
    }
    return help_text;
}
