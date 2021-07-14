/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xBackupMain.h"

#include "Controller.h"
#include "Model.h"

#include "../xLights/xLightsVersion.h"
#include "../xLights/UtilFunctions.h"

#include "../include/globals.h"

#include <log4cpp/Category.hh>

#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/debugrpt.h>
#include <wx/protocol/http.h>
#include <wx/filedlg.h>
#include <wx/numdlg.h>
#include <wx/progdlg.h>
#include <wx/dataview.h>
#include <wx/mimetype.h>

#include <string.h>

#include <iostream>

#ifdef __WXMSW__
#include <windows.h>
#include <wchar.h>
#include <tlhelp32.h>
#endif

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

//(*InternalHeaders(xBackupFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(xBackupFrame)
const long xBackupFrame::ID_LISTBOX_BACKUPS = wxNewId();
const long xBackupFrame::ID_TEXTCTRL_BACKUP_DATA = wxNewId();
const long xBackupFrame::ID_BUTTON_RUN = wxNewId();
const long xBackupFrame::ID_STATICTEXT_BACKUPFOLDER = wxNewId();
const long xBackupFrame::ID_STATICTEXT_SHOWFOLDER = wxNewId();
const long xBackupFrame::ID_STATUSBAR1 = wxNewId();
const long xBackupFrame::ID_MNU_SHOWFOLDER = wxNewId();
const long xBackupFrame::ID_MNU_WIPE_XLIGHTS_SETTINGS = wxNewId();
const long xBackupFrame::ID_MNU_QUIT = wxNewId();
const long xBackupFrame::ID_MNU_ABOUT = wxNewId();
const long xBackupFrame::ID_MNU_LOG = wxNewId();
//*)

BEGIN_EVENT_TABLE(xBackupFrame,wxFrame)
    //(*EventTable(xBackupFrame)
    //*)
END_EVENT_TABLE()

xBackupFrame::xBackupFrame(wxWindow* parent, wxWindowID id)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //(*Initialize(xBackupFrame)
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, _("xLights Script"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(1);
    FlexGridSizer1->AddGrowableRow(0);
    ListBoxBackups = new wxListBox(this, ID_LISTBOX_BACKUPS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_BACKUPS"));
    FlexGridSizer1->Add(ListBoxBackups, 1, wxALL|wxEXPAND, 5);
    TextCtrl_BackupData = new wxTextCtrl(this, ID_TEXTCTRL_BACKUP_DATA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_TEXTCTRL_BACKUP_DATA"));
    FlexGridSizer1->Add(TextCtrl_BackupData, 2, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonRun = new wxButton(this, ID_BUTTON_RUN, _("Restore Backup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RUN"));
    FlexGridSizer1->Add(ButtonRun, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextBackUpFolder = new wxStaticText(this, ID_STATICTEXT_BACKUPFOLDER, _("BackUpFolder"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_BACKUPFOLDER"));
    FlexGridSizer1->Add(StaticTextBackUpFolder, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextShowFolder = new wxStaticText(this, ID_STATICTEXT_SHOWFOLDER, _("ShowFolder"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_SHOWFOLDER"));
    FlexGridSizer1->Add(StaticTextShowFolder, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -10 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem_ShowFolder = new wxMenuItem(Menu1, ID_MNU_SHOWFOLDER, _("Select Show &Folder"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_ShowFolder);
    MenuItem_Wipe_xLights_Settings = new wxMenuItem(Menu1, ID_MNU_WIPE_XLIGHTS_SETTINGS, _("Wipe xLights Settings"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_Wipe_xLights_Settings);
    MenuItem_Quit = new wxMenuItem(Menu1, ID_MNU_QUIT, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem_Quit);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItem_About = new wxMenuItem(Menu2, ID_MNU_ABOUT, _("About\tF1"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem_About);
    MenuItem_Log = new wxMenuItem(Menu2, ID_MNU_LOG, _("View Log"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem_Log);
    MenuBar1->Append(Menu2, _("&Help"));
    SetMenuBar(MenuBar1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_LISTBOX_BACKUPS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&xBackupFrame::OnListBoxBackupsSelect);
    Connect(ID_BUTTON_RUN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xBackupFrame::OnButtonRunClick);
    Connect(ID_MNU_SHOWFOLDER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xBackupFrame::OnMenuItem_ShowFolderSelected);
    Connect(ID_MNU_WIPE_XLIGHTS_SETTINGS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xBackupFrame::OnMenuItem_Wipe_xLights_SettingsSelected);
    Connect(ID_MNU_QUIT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xBackupFrame::OnQuit);
    Connect(ID_MNU_ABOUT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xBackupFrame::OnAbout);
    Connect(ID_MNU_LOG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xBackupFrame::OnMenuItem_LogSelected);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xBackupFrame::OnResize);
    //*)

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(xBackupFrame::OnChar), (wxObject*)nullptr, this);

    SetTitle("xLights Backup Restore " + GetDisplayVersionString());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    SetMinSize(wxSize(400, 300));
    SetSize(800, 600);

    logger_base.debug("Loading show folder.");

    _showDir = xLightsShowDir();

    SetBackupDir();
}

xBackupFrame::~xBackupFrame()
{
    //(*Destroy(xBackupFrame)
    //*)
}

void xBackupFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void xBackupFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xBackup v%s, the xLights Backup Restore App."), GetDisplayVersionString());
    wxMessageBox(about, _("Welcome to..."));
}

void xBackupFrame::OnResize(wxSizeEvent& event)
{
    Layout();
}

void xBackupFrame::CreateDebugReport(wxDebugReportCompress *report) {
    if (wxDebugReportPreviewStd().Show(*report)) {
        report->Process();
        SendReport("crashUpload", *report);
        wxMessageBox("Crash report saved to " + report->GetCompressedFileName());
    }
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Exiting after creating debug report: " + report->GetCompressedFileName());
    delete report;
    exit(1);
}

void xBackupFrame::SendReport(const wxString &loc, wxDebugReportCompress &report) {
    wxHTTP http;
    http.Connect("dankulp.com");

    const char *bound = "--------------------------b29a7c2fe47b9481";

    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;
    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString fn = wxString::Format("xBackup-%s_%s_%s_%s.zip", wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), xlights_version_string, GetBitness(), ts);
    const char *ct = "Content-Type: application/octet-stream\n";
    std::string cd = "Content-Disposition: form-data; name=\"userfile\"; filename=\"" + fn.ToStdString() + "\"\n\n";

    wxMemoryBuffer memBuff;
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("\n", 1);
    memBuff.AppendData(ct, strlen(ct));
    memBuff.AppendData(cd.c_str(), strlen(cd.c_str()));


    wxFile f_in(report.GetCompressedFileName());
    wxFileOffset fLen = f_in.Length();
    void* tmp = memBuff.GetAppendBuf(fLen);
    size_t iRead = f_in.Read(tmp, fLen);
    memBuff.UngetAppendBuf(iRead);
    f_in.Close();

    memBuff.AppendData("\n", 1);
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("--\n", 3);

    http.SetMethod("POST");
    http.SetPostBuffer("multipart/form-data; boundary=------------------------b29a7c2fe47b9481", memBuff);
    wxInputStream * is = http.GetInputStream("/" + loc + "/index.php");
    char buf[1024];
    is->Read(buf, 1024);
    //printf("%s\n", buf);
    delete is;
    http.Close();
}

void xBackupFrame::OnButtonRunClick(wxCommandEvent& event)
{
    int sel = ListBoxBackups->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        DisplayError(_("Select an item before clicking the Run button"), this);
        return;
    }
    auto const name = ListBoxBackups->GetString(sel);
    std::string const folder = _backupDir + wxFileName::GetPathSeparator() + name;

    if (IsXlightsRunning()) {
        return;
    }

    if (wxDir::Exists(folder)) {

        if (wxMessageBox("Are you sure you want to Restore this Backup Folder?, This will OVERRIDE your Show Folder?",
            "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
            return;
        }
        wxProgressDialog prgs("Restoring Backup",
            "Restoring Backup", 100, this);
        prgs.Pulse("Back Up Show Folder First...");
        prgs.Show();
        BackUpShowDir(_showDir);

        prgs.Pulse("Restoring Backup... " + name);
        RestoreBackUp(_showDir, folder);

        ListBackupDir();
    }
    else {
        DisplayError(_("Folder Not Found: " + folder), this);
    }
}

void xBackupFrame::OnMenuItem_ShowFolderSelected(wxCommandEvent& event)
{
    wxDirDialog DirDialog1(this, _("Select Show Directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    if (DirDialog1.ShowModal() == wxID_OK) {
        wxString newdir = DirDialog1.GetPath();
        if (newdir == _showDir) return;
        _showDir = newdir;
        SetBackupDir();
    }
}

void xBackupFrame::OnMenuItem_Wipe_xLights_SettingsSelected(wxCommandEvent& event)
{
    if (IsXlightsRunning()) {
        return;
    }
    if (wxMessageBox("Are you sure you want to clear you xLights Settings, This will NOT Delete your Show Folder?",
        "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    wxConfig* xlconfig = new wxConfig(_("xLights"));
    if (xlconfig != nullptr) {
        xlconfig->DeleteAll();
        delete xlconfig;
    }
}

void xBackupFrame::OnMenuItem_LogSelected(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString dir;
    wxString fileName = "xBackup_l4cpp.log";
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    if (dir.EndsWith("/") || dir.EndsWith("\\"))
    {
        dir = dir.Left(dir.Length() - 1);
    }
    wxString filename = dir + "/" + fileName;
#endif
#ifdef __WXOSX__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    if (dir.EndsWith("/"))
    {
        dir = dir.Left(dir.Length() - 1);
    }
    wxString filename = dir + "/Library/Logs/" + fileName;
#endif
#ifdef __LINUX__
    wxString filename = "/tmp/" + fileName;
#endif
    wxString fn = "";
    if (wxFile::Exists(filename))
    {
        fn = filename;
    }
    else if (wxFile::Exists(wxFileName(_showDir, fileName).GetFullPath()))
    {
        fn = wxFileName(_showDir, fileName).GetFullPath();
    }
    else if (wxFile::Exists(wxFileName(wxGetCwd(), fileName).GetFullPath()))
    {
        fn = wxFileName(wxGetCwd(), fileName).GetFullPath();
    }

    wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
    if (fn != "" && ft)
    {
        wxString command = ft->GetOpenCommand("foo.txt");
        command.Replace("foo.txt", fn);

        logger_base.debug("Viewing log file %s.", (const char*)fn.c_str());

        wxUnsetEnv("LD_PRELOAD");
        wxExecute(command);
        delete ft;
    }
    else
    {
        DisplayError(wxString::Format("Unable to show log file '%s'.", fn).ToStdString(), this);
    }
}

void xBackupFrame::OnListBoxBackupsSelect(wxCommandEvent& event)
{
    int sel = ListBoxBackups->GetSelection();
    if (sel == wxNOT_FOUND) {
        return;
    }
    auto const name = ListBoxBackups->GetString(sel);
    std::string const folder = _backupDir + wxFileName::GetPathSeparator() + name;

    if (wxDir::Exists(folder)) {
        PopulateDataBox(folder);
    }
}

void xBackupFrame::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    if (uc == (wxChar)WXK_DELETE || uc == (wxChar)WXK_NUMPAD_DELETE) {
        DeleteFolder();
    }
}

void xBackupFrame::SetBackupDir()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    StaticTextShowFolder->SetLabel("");

    if (_showDir.Contains("Backup")) {
        DisplayError(wxString::Format("You are Using a Backup Show Directory, probably not good '%s'.", _showDir).ToStdString(), this);
    }

    _backupDir = _showDir + wxFileName::GetPathSeparator() + "Backup";

    ReadBackupFolderFromRGBEffectsFile(_showDir);

    ListBackupDir();
}

void xBackupFrame::ListBackupDir()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    StaticTextBackUpFolder->SetLabel("Backup Folder: " + _backupDir);
    StaticTextShowFolder->SetLabel("Show Folder: " + _showDir);

    if (!wxDir::Exists(_backupDir)) {
        logger_base.info("Backup folder doesnt exist: %s", (const char*)_backupDir.c_str());
        StaticTextBackUpFolder->SetLabel(StaticTextShowFolder->GetLabel() + " Not Found");
        StaticTextBackUpFolder->SetForegroundColour(wxColor(*wxRED));
    }

    while( (int)ListBoxBackups->GetCount() > 0 ) {
        ListBoxBackups->Delete(0);
    }

    logger_base.info("Scanning Backup folder: %s", (const char*)_backupDir.c_str());

    wxDir directory;
    directory.Open(_backupDir);

    wxString folder;
    bool fcont = directory.GetFirst(&folder, "*");

    while (fcont) {
        ListBoxBackups->Append(folder);
        fcont = directory.GetNext(&folder);
    }
}

wxString xBackupFrame::xLightsShowDir() const
{
    wxString showDir = "";

    wxConfig* xlconfig = new wxConfig(_("xLights"));
    if (xlconfig != nullptr)
    {
        xlconfig->Read(_("LastDir"), &showDir);
        delete xlconfig;
    }

    return showDir;
}

void xBackupFrame::PopulateDataBox(wxString const& folder)
{
    TextCtrl_BackupData->Clear();
    auto const controllers = LoadNetworkFile(folder);
    auto const models = LoadRGBEffectsFile(folder);
    auto const sequences = GetSeqList(folder);

    TextCtrl_BackupData->AppendText("Controllers:\n");
    TextCtrl_BackupData->AppendText(wxString::Format("Number Found: %zu\n", controllers.size()));
    for(auto const& c : controllers) {
        TextCtrl_BackupData->AppendText(c.ToString());
    }

    TextCtrl_BackupData->AppendText("\nModels:\n");
    TextCtrl_BackupData->AppendText(wxString::Format("Number Found: %zu\n", models.size()));
    for (auto const& m : models) {
        TextCtrl_BackupData->AppendText(m.ToString());
    }

    TextCtrl_BackupData->AppendText("\nSequences:\n");
    TextCtrl_BackupData->AppendText(wxString::Format("Number Found: %zu\n", sequences.size()));
    for (auto const& s : sequences) {
        TextCtrl_BackupData->AppendText(s + "\n");
    }

    TextCtrl_BackupData->ShowPosition(0);
}

std::vector<Controller> xBackupFrame::LoadNetworkFile(wxString const& folder) const
{
    std::vector<Controller> controllers;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxFileName networkFile;
    // load network
    networkFile.AssignDir(folder);
    networkFile.SetFullName(_(XLIGHTS_NETWORK_FILE));
    if (networkFile.FileExists()) {
        wxXmlDocument doc;
        doc.Load(networkFile.GetFullPath());

        if (doc.IsOk()) {

            for (auto e = doc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {

                if (e->GetName() == "Controller") {
                    controllers.emplace_back(e);
                }
            }
        }
        else {
            logger_base.warn("Error loading networks file: %s.", (const char*)networkFile.GetFullPath().c_str());
        }
    }
    else {
        logger_base.warn("Network file not found: %s.", (const char*)networkFile.GetFullPath().c_str());
    }

    std::sort(controllers.begin(), controllers.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.Name < rhs.Name;
        });

    return controllers;
}

std::vector<Model> xBackupFrame::LoadRGBEffectsFile(wxString const& folder) const
{
    std::vector<Model> models;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxFileName modelFile;
    // load rgb effects file
    modelFile.AssignDir(folder);
    modelFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    if (modelFile.FileExists()) {
        wxXmlDocument doc;
        doc.Load(modelFile.GetFullPath());

        if (doc.IsOk()) {
            wxXmlNode* root = doc.GetRoot();
            if (root->GetName() == "xrgb") {
                for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
                    if (e->GetName() == "models") {
                        for (wxXmlNode* model = e->GetChildren(); model != nullptr; model = model->GetNext()) {
                            if (model->GetName() == "model") {
                                models.emplace_back(model);
                            }
                        }
                    }
                    if (e->GetName() == "effects") {
                       break;
                    }
                }
            }
            else {
                logger_base.warn("Error loading RGB Effects file: %s.", (const char*)modelFile.GetFullPath().c_str());
            }
        }
        else {
            logger_base.warn("Error loading RGB Effects file: %s.", (const char*)modelFile.GetFullPath().c_str());
        }
    }
    else {
        logger_base.warn("RGB Effects file not found: %s.", (const char*)modelFile.GetFullPath().c_str());
    }
    std::sort(models.begin(), models.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.Name < rhs.Name;
        });
    return models;
}

wxArrayString xBackupFrame::GetSeqList( wxString const& folder) const
{
    wxArrayString returnList;
    wxArrayString files;
    wxDir::GetAllFiles(folder, &files, "*.x*");
    files.Sort();
    for (size_t x = 0; x < files.size(); x++) {
        wxString name = files[x].SubString(folder.length(), files[x].size());
        if (name[0] == '/' || name[0] == '\\') {
            name = name.SubString(1, name.size());
        }
        if (!name.Contains("xlights_") && (name.Lower().EndsWith("xsq") || name.Lower().EndsWith("xml"))) {
            returnList.push_back(name);
        }
    }
    return returnList;
}

bool xBackupFrame::ReadBackupFolderFromRGBEffectsFile(wxString const& folder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxFileName modelFile;
    // load rgb effects file
    modelFile.AssignDir(folder);
    modelFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    if (modelFile.FileExists()) {
        wxXmlDocument doc;
        doc.Load(modelFile.GetFullPath());

        if (doc.IsOk()) {
            wxXmlNode* root = doc.GetRoot();
            if (root->GetName() == "xrgb") {
                for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
                    if (e->GetName() == "settings") {
                        for (wxXmlNode* setting = e->GetChildren(); setting != nullptr; setting = setting->GetNext()) {
                            if (setting->GetName() == "backupDir") {
                                wxString folder = setting->GetAttribute("value");
                                if (!folder.IsEmpty() && wxDir::Exists(folder)) {
                                    folder = folder + wxFileName::GetPathSeparator() + "Backup";
                                    _backupDir = folder;
                                    logger_base.warn("Setting Backup Folder from RGB Effects file: %s.", (const char*)folder.c_str());
                                    ListBackupDir();
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            else {
                logger_base.warn("Error loading RGB Effects file: %s.", (const char*)modelFile.GetFullPath().c_str());
            }
        }
        else {
            logger_base.warn("Error loading RGB Effects file: %s.", (const char*)modelFile.GetFullPath().c_str());
        }
    }
    else {
        logger_base.warn("RGB Effects file not found: %s.", (const char*)modelFile.GetFullPath().c_str());
    }
    return false;
}

void xBackupFrame::BackUpShowDir(wxString const& folder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxFileName newDirH;
    time_t cur;
    time(&cur);
    wxDateTime curTime(cur);
    wxString newDirBackup = _backupDir;

    if (!wxDirExists(newDirBackup) && !newDirH.Mkdir(newDirBackup)) {
        DisplayError(wxString::Format("Unable to create backup directory '%s'!", newDirBackup).ToStdString());
        return;
    }

    wxString newDir = wxString::Format("%s%cxBackup_Backup_%s_%s", newDirBackup, wxFileName::GetPathSeparator(),
        curTime.FormatISODate(), curTime.Format("%H%M%S"));

    if (!newDirH.Mkdir(newDir)) {
        DisplayError(wxString::Format("Unable to create directory '%s'! Backup failed.", newDir).ToStdString());
        return;
    }
    else {
        logger_base.info("Backup directory '%s' created", (const char*)newDir.c_str());
    }

    std::string errors = "";
    BackupDirectory(folder, newDir, newDir, errors);

    if (errors != "") {
        DisplayError(errors, this);
    }
}

void xBackupFrame::RestoreBackUp(wxString const& showfolder, wxString const& backupfolder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!wxDirExists(backupfolder) )
    {
        DisplayError(wxString::Format("Unable to Restore backup directory '%s'!", backupfolder).ToStdString());
        return;
    }


    std::string errors = "";
    BackupDirectory(backupfolder, showfolder, showfolder, errors);

    if (errors != "")
    {
        DisplayError(errors, this);
    }
}

void xBackupFrame::BackupDirectory(wxString sourceDir, wxString targetDirName, wxString lastCreatedDirectory, std::string& errors)
{
    wxDir srcDir(sourceDir);

    if (!srcDir.IsOpened())
    {
        return;
    }

    if (CopyFiles("*.xsq", srcDir, targetDirName, lastCreatedDirectory, errors) +
        CopyFiles("*.xml", srcDir, targetDirName, lastCreatedDirectory, errors) +
        CopyFiles("*.xbkp", srcDir, targetDirName, lastCreatedDirectory, errors) +
        CopyFiles("*.xmap", srcDir, targetDirName, lastCreatedDirectory, errors) +
        CopyFiles("*.xschedule", srcDir, targetDirName, lastCreatedDirectory, errors) > 0)
    {
        lastCreatedDirectory = targetDirName;
    }

    // recurse through all directories but folders named Backup

    wxString dir;
    bool cont = srcDir.GetFirst(&dir, "", wxDIR_DIRS);
    while (cont)
    {
        if (dir != "Backup") {
            wxDir subdir(srcDir.GetNameWithSep() + dir);
            BackupDirectory(subdir.GetNameWithSep(), targetDirName + wxFileName::GetPathSeparator() + dir, lastCreatedDirectory, errors);
        }
        cont = srcDir.GetNext(&dir);
    }
}

bool xBackupFrame::CopyFiles(const wxString& wildcard, wxDir& srcDir, wxString& targetDirName, wxString lastCreatedDirectory, std::string& errors)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool res = false;
    wxString fname;
    wxString srcDirName = srcDir.GetNameWithSep();
    wxFileName srcFile;
    srcFile.SetPath(srcDir.GetNameWithSep());

    bool cont = srcDir.GetFirst(&fname, wildcard, wxDIR_FILES);
    while (cont)
    {
        logger_base.debug("Coping file %s.", (const char*)(srcDirName + fname).c_str());
        res = true;

        CreateMissingDirectories(targetDirName, lastCreatedDirectory, errors);

        srcFile.SetFullName(fname);

        logger_base.debug("    to %s.", (const char*)(targetDirName + wxFileName::GetPathSeparator() + fname).c_str());
        //SetStatusText("Copying File \"" + srcFile.GetFullPath());
        bool success = wxCopyFile(srcDirName + fname,
            targetDirName + wxFileName::GetPathSeparator() + fname);
        if (!success)
        {
            logger_base.error("    Copy Failed.");
            errors += "Unable to copy file \"" + srcDir.GetNameWithSep() + fname + "\"\n";
        }
        cont = srcDir.GetNext(&fname);
    }

    return res;
}

void xBackupFrame::CreateMissingDirectories(wxString targetDirName, wxString lastCreatedDirectory, std::string& errors)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (wxDir::Exists(targetDirName)) return;

    if (targetDirName.Length() > 256)
    {
        logger_base.warn("Target directory is %d characters long. This may be an issue on your operating system.", targetDirName.Length());
    }

    wxFileName tgt(targetDirName);
    wxFileName lst(lastCreatedDirectory);

    if (!tgt.GetFullPath().StartsWith(lst.GetFullPath())) return;

    wxArrayString tgtd = wxSplit(targetDirName, wxFileName::GetPathSeparator());
    wxArrayString lstd = wxSplit(lastCreatedDirectory, wxFileName::GetPathSeparator());
    wxString newDir = lastCreatedDirectory;

    bool cont = true;
    for (size_t i = lstd.Count(); cont && i < tgtd.Count(); i++)
    {
        wxDir dir(newDir);
        newDir += wxFileName::GetPathSeparator() + tgtd[i];
        if (!wxDir::Exists(newDir))
        {
            logger_base.debug("    Create folder '%s'.", (const char*)newDir.c_str());
            if (!dir.Make(newDir))
            {
                cont = false;
                errors += wxString::Format("Failed to create folder %s\n", newDir);
                logger_base.error("        Folder Create failed.");
            }
        }
    }
}

//mostly copied from stackoverflow
wxArrayString xBackupFrame::GetProcessList() const
{
    wxArrayString returnList;
#ifdef __WXMSW__

    //get child processes of this node
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (!hProcessSnap) {
        return returnList;
    }

    //Fill in the size of the structure before using it.
    PROCESSENTRY32 pe;
    memset(&pe, 0, sizeof(pe));
    pe.dwSize = sizeof(PROCESSENTRY32);

    // Walk the snapshot of the processes, and for each process,
    // kill it if its parent is pid.
    if (!Process32First(hProcessSnap, &pe)) {
        // Can't get first process.
        CloseHandle(hProcessSnap);
        return returnList;
    }

    do {
        returnList.push_back(pe.szExeFile);
    } while (Process32Next(hProcessSnap, &pe));
    CloseHandle(hProcessSnap);
#else
    //GTK and other
    wxArrayString output;
#if defined (__WXGTK__)
    ExecuteCommand(wxT("ps -A -o pid,command  --no-heading"), output);
#elif defined (__WXMAC__)
    // Mac does not like the --no-heading...
    ExecuteCommand(wxT("ps -A -o pid,command "), output);
#endif
    //untested
    for (size_t i = 0; i < output.GetCount(); i++) {
        wxString line = output.Item(i);
        //remove whitespaces
        line = line.Trim().Trim(false);

        //get the process ID
        //wxString spid = line.BeforeFirst(wxT(' '));
        //spid.ToLong(&entry.pid);
        returnList.push_back(line.AfterFirst(wxT(' ')));
    }
#endif
    return returnList;
}

//mostly copied from stackoverflow
void xBackupFrame::ExecuteCommand(const wxString& command, wxArrayString& output, long flags) const
{
#ifdef __WXMSW__
    wxExecute(command, output, flags);
#else
    FILE* fp;
    char line[512];
    memset(line, 0, sizeof(line));
    fp = popen(command.mb_str(wxConvUTF8), "r");
    while (fgets(line, sizeof line, fp)) {
        output.Add(wxString(line, wxConvUTF8));
        memset(line, 0, sizeof(line));
    }

    pclose(fp);
#endif
}

bool xBackupFrame::IsXlightsRunning() const
{
    auto const list = GetProcessList();
    for (auto const& app: list) {
        if (app.Lower().Contains("xlights")) {
            DisplayError("You Must Close xLights before Proceding");
            return true;
        }
    }
    return false;
}

void xBackupFrame::DeleteFolder()
{
    int sel = ListBoxBackups->GetSelection();
    if (sel == wxNOT_FOUND) {
        return;
    }
    auto const name = ListBoxBackups->GetString(sel);
    std::string const folder = _backupDir + wxFileName::GetPathSeparator() + name;

    if (wxDir::Exists(folder)) {
        if (wxMessageBox(wxString::Format("Are you sure you want to Delete Folder: '%s'", name),
            "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
            return;
        }

        if (!RemoveDir(folder)) {
            DisplayError(wxString::Format("Could not Delete '%s'.", folder).ToStdString(), this);
        }
        ListBackupDir();
    }
    else {
        DisplayError(wxString::Format("Folder Doesn't Exist '%s'.", folder).ToStdString(), this);
    }
}

//mostly copied from stackoverflow
bool xBackupFrame::RemoveDir(wxString rmDir) {

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // first make sure that the dir exists
    if (!wxDir::Exists(rmDir)) {
        logger_base.error("folder doesnt exist %s, Could not remove directory", (const char*)rmDir.c_str());
        return false;
    }

    // append a slash if we don't have one
    if (rmDir[rmDir.length() - 1] != wxFILE_SEP_PATH) {
        rmDir += wxFILE_SEP_PATH;
    }

    // define our directory object.  When we begin traversing it, the
    // os will not let go until the object goes out of scope.
    wxDir* dir = new wxDir(rmDir);

    // check for allocation failure
    if (dir == nullptr) {
        logger_base.error("Could not allocate new memory on the heap!");
        return false;
    }

    // our file name temp var
    wxString filename;
    // get the first filename
    bool cont = dir->GetFirst(&filename);

    // if there are files to process
    if (cont) {
        do {
            // if the next filename is actually a directory
            if (wxDirExists(rmDir + filename)) {
                // delete this directory
                RemoveDir(rmDir + filename);
            }
            else {
                // otherwise attempt to delete this file
                if (!wxRemoveFile(rmDir + filename)) {
                    // error if we couldn't
                    logger_base.error("Could not remove file \"%s%s\"", (const char*)rmDir.c_str(), (const char*)filename.c_str());
                }
            }
        }
        // get the next file name
        while (dir->GetNext(&filename));
    }

    // Remove our directory object, so the OS will let go of it and
    // allow us to delete it
    delete dir;

    // now actually try to delete it
    if (!wxFileName::Rmdir(rmDir)) {
        // error if we couldn't
        logger_base.error("Could not remove directory \"%s\"", (const char*)rmDir.c_str());
        // return accordingly
        return false;
    }
    // otherwise
    else {
        // return that we were successfull.
        return true;
    }
}
