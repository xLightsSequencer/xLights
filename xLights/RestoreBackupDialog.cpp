/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RestoreBackupDialog.h"

//(*InternalHeaders(RestoreBackupDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/xml/xml.h>

#include <log4cpp/Category.hh>

#include "../xLights/UtilFunctions.h"

#include "../include/globals.h"

struct BController {
    wxString Name;
    wxString Type;
    wxString IPCom;
    wxString Protocol;

    BController()
    {}
    BController(wxXmlNode* node)
    {
        ParseXML(node);
    }

    void ParseXML(wxXmlNode* node)
    {
        Name = node->GetAttribute("Name");
        Type = node->GetAttribute("Type", "");
        Protocol = node->GetAttribute("Protocol");
        if (node->HasAttribute("IP")) {
            IPCom = node->GetAttribute("IP");
        }
        if (node->HasAttribute("Port")) {
            IPCom = node->GetAttribute("Port");
        }
    }

    [[nodiscard]] wxString ToString() const
    {
        return wxString::Format("%s:%s:%s\n", Name, IPCom, Protocol);
    }
};

struct BModel {
    wxString Name;
    wxString Type;
    wxString Layout;
    wxString StartChanel;

    BModel()
    {}
    BModel(wxXmlNode* node)
    {
        ParseXML(node);
    }

    void ParseXML(wxXmlNode* node)
    {
        Name = node->GetAttribute("name");
        Type = node->GetAttribute("DisplayAs", "");
        Layout = node->GetAttribute("LayoutGroup");
        StartChanel = node->GetAttribute("StartChannel");
    }

    [[nodiscard]] wxString ToString() const
    {
        return wxString::Format("%s:%s:%s\n", Name, Type, StartChanel);
    }
};

//(*IdInit(RestoreBackupDialog)
const long RestoreBackupDialog::ID_LISTBOX_BACKUPS = wxNewId();
const long RestoreBackupDialog::ID_TREECTRL_BACKUP_DATA = wxNewId();
const long RestoreBackupDialog::ID_BUTTON_RUN = wxNewId();
const long RestoreBackupDialog::ID_STATICTEXT_BACKUPFOLDER = wxNewId();
const long RestoreBackupDialog::ID_STATICTEXT_SHOWFOLDER = wxNewId();
//*)

BEGIN_EVENT_TABLE(RestoreBackupDialog,wxDialog)
	//(*EventTable(RestoreBackupDialog)
	//*)
END_EVENT_TABLE()

RestoreBackupDialog::RestoreBackupDialog(wxString const& showDir, wxString const& backupDir, wxWindow* parent, wxWindowID id):
    _showDir(showDir), _backupDir(backupDir + wxFileName::GetPathSeparator() + "Backup")
{
	//(*Initialize(RestoreBackupDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Restore Dialog"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("id"));
	SetClientSize(wxSize(800,400));
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(0);
	ListBoxBackups = new wxListBox(this, ID_LISTBOX_BACKUPS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_BACKUPS"));
	FlexGridSizer1->Add(ListBoxBackups, 1, wxALL|wxEXPAND, 5);
	TreeCtrlBackupData = new wxTreeCtrl(this, ID_TREECTRL_BACKUP_DATA, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL_BACKUP_DATA"));
	FlexGridSizer1->Add(TreeCtrlBackupData, 1, wxALL|wxEXPAND, 5);
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
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_LISTBOX_BACKUPS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&RestoreBackupDialog::OnListBoxBackupsSelect);
	Connect(ID_BUTTON_RUN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RestoreBackupDialog::OnButtonRunClick);
	//*)
    SetEscapeId(wxID_CANCEL);

    ListBackupDir();
}

RestoreBackupDialog::~RestoreBackupDialog()
{
	//(*Destroy(RestoreBackupDialog)
	//*)
}

void RestoreBackupDialog::OnButtonRunClick(wxCommandEvent& event)
{
    if (_restoreDir.empty()) {
        return;
    }
    if (wxMessageBox("This will override any open file.\nAre you sure you want do this?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    EndDialog(wxID_OK);
}

void RestoreBackupDialog::OnListBoxBackupsSelect(wxCommandEvent& event)
{
    int sel = ListBoxBackups->GetSelection();
    if (sel == wxNOT_FOUND) {
        return;
    }
    auto const name = ListBoxBackups->GetString(sel);
    wxString const folder = _backupDir + wxFileName::GetPathSeparator() + name;

    if (wxDir::Exists(folder)) {
        _restoreDir = folder;
        PopulateDataBox(folder);
    }
}

void RestoreBackupDialog::ListBackupDir()
{
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    StaticTextBackUpFolder->SetLabel("Backup Folder: " + _backupDir);
    StaticTextShowFolder->SetLabel("Show Folder: " + _showDir);

    if (!wxDir::Exists(_backupDir)) {
        logger_base.info("Backup folder doesnt exist: %s", (const char*)_backupDir.c_str());
        StaticTextBackUpFolder->SetLabel(StaticTextShowFolder->GetLabel() + " Not Found");
        StaticTextBackUpFolder->SetForegroundColour(wxColor(*wxRED));
    }

    while ((int)ListBoxBackups->GetCount() > 0) {
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

void RestoreBackupDialog::PopulateDataBox(wxString const& folder)
{
    TreeCtrlBackupData->DeleteAllItems();
    auto treeRootID = TreeCtrlBackupData->AddRoot(folder);

    auto const controllers = LoadNetworkFile(folder);
    auto const models = LoadRGBEffectsFile(folder);
    auto const sequences = GetSeqList(folder);

    auto curGroupID = TreeCtrlBackupData->AppendItem(treeRootID,
        wxString::Format("Controllers [%zu Found]", controllers.size()));
    TreeCtrlBackupData->SetItemHasChildren(curGroupID);
    for (auto const& c : controllers) {

        wxTreeItemId item = FindTreeItem(curGroupID, c.Type);
        if (item.IsOk()) {
            item = TreeCtrlBackupData->AppendItem(item, c.Name);
        } else {
            auto nextID = TreeCtrlBackupData->AppendItem(curGroupID, c.Type);
            TreeCtrlBackupData->SetItemHasChildren(nextID);
            item = TreeCtrlBackupData->AppendItem(nextID, c.Name);
        }
        TreeCtrlBackupData->SetItemHasChildren(item);
        TreeCtrlBackupData->AppendItem(item, "IP/Com:" + c.IPCom);
        TreeCtrlBackupData->AppendItem(item, "Protocol:" + c.Protocol);
    }

    curGroupID = TreeCtrlBackupData->AppendItem(treeRootID,
        wxString::Format("Models [%zu Found]", models.size()));
    TreeCtrlBackupData->SetItemHasChildren(curGroupID);

    for (auto const& m : models) {

        wxTreeItemId item = FindTreeItem(curGroupID, m.Layout);
        if (item.IsOk()) {
            item = TreeCtrlBackupData->AppendItem(item, m.Name);
        } else {
            auto nextID = TreeCtrlBackupData->AppendItem(curGroupID, m.Layout);
            TreeCtrlBackupData->SetItemHasChildren(nextID);
            item = TreeCtrlBackupData->AppendItem(nextID, m.Name);
        }
        TreeCtrlBackupData->SetItemHasChildren(item);
        TreeCtrlBackupData->AppendItem(item, "Type:" + m.Type);
        TreeCtrlBackupData->AppendItem(item, "StartChanel:" + m.StartChanel);
    }

    curGroupID = TreeCtrlBackupData->AppendItem(treeRootID,
                                                wxString::Format("Sequences [%zu Found]", sequences.size()));

    for (auto const& s : sequences) {
        AddFolder(s, curGroupID);
    }

    TreeCtrlBackupData->Expand(treeRootID);
}

std::vector<BController> RestoreBackupDialog::LoadNetworkFile(wxString const& folder) const
{
    std::vector<BController> controllers;
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
        } else {
            logger_base.warn("Error loading networks file: %s.", (const char*)networkFile.GetFullPath().c_str());
        }
    } else {
        logger_base.warn("Network file not found: %s.", (const char*)networkFile.GetFullPath().c_str());
    }

    std::sort(controllers.begin(), controllers.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.Name < rhs.Name;
    });

    return controllers;
}

std::vector<BModel> RestoreBackupDialog::LoadRGBEffectsFile(wxString const& folder) const
{
    std::vector<BModel> models;

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
            } else {
                logger_base.warn("Error loading RGB Effects file: %s.", (const char*)modelFile.GetFullPath().c_str());
            }
        } else {
            logger_base.warn("Error loading RGB Effects file: %s.", (const char*)modelFile.GetFullPath().c_str());
        }
    } else {
        logger_base.warn("RGB Effects file not found: %s.", (const char*)modelFile.GetFullPath().c_str());
    }
    std::sort(models.begin(), models.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.Name < rhs.Name;
    });
    return models;
}

wxArrayString RestoreBackupDialog::GetSeqList(wxString const& folder) const
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

void RestoreBackupDialog::AddFolder( wxString path,  wxTreeItemId parent)
{
    if(path.Contains(wxFileName::GetPathSeparator()))
    {
        wxString folder = path.Left(path.First(wxFileName::GetPathSeparator()));
        wxString newPath = path.Right(path.Length() - (path.First(wxFileName::GetPathSeparator()) + 1));
        wxTreeItemId item = FindTreeItem(parent, folder);
        if (item.IsOk()) {
            return AddFolder(newPath, item);
        } else {
            auto nextID = TreeCtrlBackupData->AppendItem(parent, folder);
            TreeCtrlBackupData->SetItemHasChildren(nextID);
            return AddFolder(newPath, nextID);
        }

    } else {
        TreeCtrlBackupData->AppendItem(parent, path);
    }
}

wxTreeItemId RestoreBackupDialog::FindTreeItem(wxTreeItemId parent, wxString name) const
{
    wxTreeItemIdValue cookie;
    for (wxTreeItemId item = TreeCtrlBackupData->GetFirstChild(parent, cookie); item.IsOk(); item = TreeCtrlBackupData->GetNextChild(parent, cookie)) {
        auto treeItm = TreeCtrlBackupData->GetItemText(item);
        if (treeItm == name) {
            return item;
        }
    }

    return wxTreeItemId();
}
