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

#include "../../xSchedule/wxJSON/jsonreader.h"
#include "../../xSchedule/wxJSON/jsonwriter.h"

#include <log4cpp/Category.hh>
#include <wx/mimetype.h>

//(*IdInit(ScriptsDialog)
const long ScriptsDialog::ID_STATICTEXT1 = wxNewId();
const long ScriptsDialog::ID_LISTBOX_SCRIPTS = wxNewId();
const long ScriptsDialog::ID_BUTTON_RUN = wxNewId();
const long ScriptsDialog::ID_BUTTON_REFRESH = wxNewId();
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
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select the JSON Script you want to run."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
    ListBoxScripts = new wxListBox(this, ID_LISTBOX_SCRIPTS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_SCRIPTS"));
    FlexGridSizer1->Add(ListBoxScripts, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Run = new wxButton(this, ID_BUTTON_RUN, _("Run"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RUN"));
    FlexGridSizer4->Add(Button_Run, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Refresh = new wxButton(this, ID_BUTTON_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REFRESH"));
    FlexGridSizer4->Add(Button_Refresh, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Log = new wxTextCtrl(this, ID_TEXTCTRL_LOG, wxEmptyString, wxDefaultPosition, wxSize(0,300), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_LOG"));
    FlexGridSizer1->Add(TextCtrl_Log, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_BUTTON_RUN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnButton_RunClick);
    Connect(ID_BUTTON_REFRESH,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnButton_RefreshClick);
    //*)

    Connect(ID_LISTBOX_SCRIPTS, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ScriptsDialog::OnListRClick);

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
    int sel = ListBoxScripts->GetSelection();
    if (sel == wxNOT_FOUND) {
        DisplayError(_("Select an script before clicking the Run Button"), this);
        return;
    }
    SetCursor(wxCURSOR_WAIT);

    ListBoxScripts->Disable();
    Button_Run->Disable();
    Button_Refresh->Disable();

    wxString file = _scriptFolder + ListBoxScripts->GetString(sel);
    Run_Script(file);

    ListBoxScripts->Enable();
    Button_Run->Enable();
    Button_Refresh->Enable();
    SetCursor(wxCURSOR_ARROW);
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
    if (event.GetId() == ID_MCU_VIEWSCRIPT) {
        int sel = ListBoxScripts->GetSelection();
        if (sel == wxNOT_FOUND) {
            return;
        }
        wxString file = _scriptFolder + ListBoxScripts->GetString(sel);
        wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension("json");
        if (ft) {
            wxString command = ft->GetOpenCommand(file);
            wxUnsetEnv("LD_PRELOAD");
            wxExecute(command);
        }
    }
}


void ScriptsDialog::LoadScriptDir()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string scriptFolder = _frame->GetShowDirectory() + wxFileName::GetPathSeparator() + "scripts";
    if (!wxDir::Exists(scriptFolder)) {
        wxDir::Make(scriptFolder);
    }

    while ((int)ListBoxScripts->GetCount() > 0) {
        ListBoxScripts->Delete(0);
    }

    logger_base.info("Scanning Script folder: %s", (const char*)scriptFolder.c_str());

    wxDir directory;
    directory.Open(scriptFolder);

    _scriptFolder = directory.GetNameWithSep();

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.json");

    while (fcont) {
        ListBoxScripts->Append(file);
        fcont = directory.GetNext(&file);
    }
}

void ScriptsDialog::Run_Script(wxString const& filepath)
{
    TextCtrl_Log->Clear();
    wxJSONValue json;
    wxJSONReader reader;
    wxFileInputStream f(filepath);

    bool parsed = reader.Parse(f, &json) == 0;
    if (!parsed) {
        DisplayError(_("Script JSON File Cound not be Parsed.\n" + filepath), this);
        return;
    }

    if (json.IsArray()) {
        for (int x = 0; x < json.Size(); x++) {
            wxJSONValue const& aj = json[x];
            TextCtrl_Log->AppendText(_frame->ProcessxlDoAutomation(JSONtoString(aj)));
            TextCtrl_Log->AppendText("\n");
        }
    } else {
        TextCtrl_Log->AppendText(_frame->ProcessxlDoAutomation(JSONtoString(json)));
        TextCtrl_Log->AppendText("\n");
    }
}

wxString ScriptsDialog::JSONtoString(wxJSONValue const& json) const
{
    wxJSONWriter writer(wxJSONWRITER_NONE, 0, 0);
    wxString p;
    writer.Write(json, p);
    return p;
}

