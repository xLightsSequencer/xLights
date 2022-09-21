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

#include "ExternalHooks.h"

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
const long RestoreBackupDialog::ID_STATICTEXT1 = wxNewId();
const long RestoreBackupDialog::ID_LISTBOX_BACKUPS = wxNewId();
const long RestoreBackupDialog::ID_PANEL1 = wxNewId();
const long RestoreBackupDialog::ID_CHECKLISTBOX_LAYOUT = wxNewId();
const long RestoreBackupDialog::ID_TREECTRL_LAYOUT = wxNewId();
const long RestoreBackupDialog::ID_SPLITTERWINDOW2 = wxNewId();
const long RestoreBackupDialog::ID_CHECKLISTBOX_SEQUENCES = wxNewId();
const long RestoreBackupDialog::ID_NOTEBOOK1 = wxNewId();
const long RestoreBackupDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long RestoreBackupDialog::ID_STATICTEXT_BACKUPFOLDER = wxNewId();
const long RestoreBackupDialog::ID_BUTTON_RUN = wxNewId();
//*)

BEGIN_EVENT_TABLE(RestoreBackupDialog,wxDialog)
	//(*EventTable(RestoreBackupDialog)
	//*)
END_EVENT_TABLE()

RestoreBackupDialog::RestoreBackupDialog(wxString const& showDir, wxString const& backupDir, wxWindow* parent, wxWindowID id):
    _showDir(showDir), _backupDir(backupDir + wxFileName::GetPathSeparator() + "Backup")
{
	//(*Initialize(RestoreBackupDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, wxID_ANY, _("Restore Dialog"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxSize(900,500));
	SetMinSize(wxSize(800,400));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxSize(602,134), wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(900,400));
	SplitterWindow1->SetMinimumPaneSize(100);
	SplitterWindow1->SetSashGravity(0.5);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Backups:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	ListBoxBackups = new wxListBox(Panel1, ID_LISTBOX_BACKUPS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_BACKUPS"));
	ListBoxBackups->SetMinSize(wxSize(250,400));
	FlexGridSizer2->Add(ListBoxBackups, 0, wxEXPAND, 0);
	Panel1->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel1);
	FlexGridSizer2->SetSizeHints(Panel1);
	Notebook1 = new wxNotebook(SplitterWindow1, ID_NOTEBOOK1, wxPoint(545,100), wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Notebook1->SetMinSize(wxSize(600,400));
	SplitterWindow2 = new wxSplitterWindow(Notebook1, ID_SPLITTERWINDOW2, wxPoint(57,24), wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW2"));
	SplitterWindow2->SetMinimumPaneSize(10);
	SplitterWindow2->SetSashGravity(0.6);
	CheckListBoxLayout = new wxCheckListBox(SplitterWindow2, ID_CHECKLISTBOX_LAYOUT, wxPoint(42,42), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOX_LAYOUT"));
	CheckListBoxLayout->SetMinSize(wxSize(0,300));
	TreeCtrlBackupLayout = new wxTreeCtrl(SplitterWindow2, ID_TREECTRL_LAYOUT, wxPoint(412,61), wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL_LAYOUT"));
	TreeCtrlBackupLayout->SetMinSize(wxSize(0,300));
	SplitterWindow2->SplitVertically(CheckListBoxLayout, TreeCtrlBackupLayout);
	CheckListBoxSequences = new wxCheckListBox(Notebook1, ID_CHECKLISTBOX_SEQUENCES, wxPoint(150,7), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOX_SEQUENCES"));
	Notebook1->AddPage(SplitterWindow2, _("Controllers/Layout"), true);
	Notebook1->AddPage(CheckListBoxSequences, _("Sequences"), false);
	SplitterWindow1->SplitVertically(Panel1, Notebook1);
	SplitterWindow1->SetSashPosition(10);
	FlexGridSizer1->Add(SplitterWindow1, 2, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticTextBackUpFolder = new wxStaticText(this, ID_STATICTEXT_BACKUPFOLDER, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_BACKUPFOLDER"));
	BoxSizer1->Add(StaticTextBackUpFolder, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonRun = new wxButton(this, ID_BUTTON_RUN, _("Restore Files"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RUN"));
	BoxSizer1->Add(ButtonRun, 0, wxALL, 5);
	BoxSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_LISTBOX_BACKUPS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&RestoreBackupDialog::OnListBoxBackupsSelect);
	Connect(ID_CHECKLISTBOX_LAYOUT,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&RestoreBackupDialog::OnCheckListBoxLayoutToggled);
	Connect(ID_CHECKLISTBOX_SEQUENCES,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&RestoreBackupDialog::OnCheckListBoxSequencesToggled);
	Connect(ID_BUTTON_RUN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RestoreBackupDialog::OnButtonRunClick);
	//*)
    SetEscapeId(wxID_CANCEL);

    ListBackupDir();
    ValidateWindow();
}

RestoreBackupDialog::~RestoreBackupDialog()
{
	//(*Destroy(RestoreBackupDialog)
	//*)
}

void RestoreBackupDialog::ValidateWindow()
{
    wxArrayInt laysel;
    CheckListBoxLayout->GetCheckedItems(laysel);
    wxArrayInt seqsel;
    CheckListBoxSequences->GetCheckedItems(seqsel);
    if (laysel.size() + seqsel.size() == 0) {
        ButtonRun->Enable(false);
    } else {
        ButtonRun->Enable(true);
    }
}

void RestoreBackupDialog::OnButtonRunClick(wxCommandEvent& event)
{
    _restoreFiles.clear();
    if (_restoreDir.empty()) {
        return;
    }

    for (size_t x = 0; x < CheckListBoxLayout->GetCount(); x++) {
        if (CheckListBoxLayout->IsChecked(x)) {
            _restoreFiles.push_back(CheckListBoxLayout->GetString(x));
        }
    }
    for (size_t x = 0; x < CheckListBoxSequences->GetCount(); x++) {
        if (CheckListBoxSequences->IsChecked(x)) {
            _restoreFiles.push_back(CheckListBoxSequences->GetString(x));
        }
    }
    if (_restoreFiles.empty()) {
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
        PopulateLayoutDataBox(folder);
        PopulateLayoutList(folder);
        PopulateSequenceList(folder);
    }
    ValidateWindow();
}

void RestoreBackupDialog::ListBackupDir()
{
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    StaticTextBackUpFolder->SetLabel("Backup Folder: " + _backupDir);

    if (!wxDir::Exists(_backupDir)) {
        logger_base.info("Backup folder doesnt exist: %s", (const char*)_backupDir.c_str());
        StaticTextBackUpFolder->SetLabel(_backupDir + " Not Found");
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

void RestoreBackupDialog::PopulateLayoutDataBox(wxString const& folder)
{
    TreeCtrlBackupLayout->DeleteAllItems();
    auto treeRootID = TreeCtrlBackupLayout->AddRoot(folder);

    auto const controllers = LoadNetworkFile(folder);
    auto const models = LoadRGBEffectsFile(folder);

    auto curGroupID = TreeCtrlBackupLayout->AppendItem(treeRootID,
        wxString::Format("Controllers [%zu Found]", controllers.size()));
    TreeCtrlBackupLayout->SetItemHasChildren(curGroupID);
    for (auto const& c : controllers) {
        auto item = TreeCtrlBackupLayout->AppendItem(curGroupID, c.Name);
        TreeCtrlBackupLayout->SetItemHasChildren(item);
        TreeCtrlBackupLayout->AppendItem(item, "IP/Com:" + c.IPCom);
        TreeCtrlBackupLayout->AppendItem(item, "Type:" + c.Type);
        TreeCtrlBackupLayout->AppendItem(item, "Protocol:" + c.Protocol);
    }

    curGroupID = TreeCtrlBackupLayout->AppendItem(treeRootID,
        wxString::Format("Models [%zu Found]", models.size()));
    TreeCtrlBackupLayout->SetItemHasChildren(curGroupID);

    for (auto const& m : models) {
        wxTreeItemId item = TreeCtrlBackupLayout->AppendItem(curGroupID, m.Name);
        TreeCtrlBackupLayout->SetItemHasChildren(item);
        TreeCtrlBackupLayout->AppendItem(item, "Type:" + m.Type);
        TreeCtrlBackupLayout->AppendItem(item, "StartChanel:" + m.StartChanel);
        TreeCtrlBackupLayout->AppendItem(item, "Preview:" + m.Layout);
    }

    TreeCtrlBackupLayout->Expand(treeRootID);
    wxTreeItemIdValue cookie;
    for (wxTreeItemId item = TreeCtrlBackupLayout->GetFirstChild(treeRootID, cookie); item.IsOk(); item = TreeCtrlBackupLayout->GetNextChild(item, cookie)) {
        TreeCtrlBackupLayout->Expand(item);
    }
    //TreeCtrlBackupLayout->ExpandAll();
}

void RestoreBackupDialog::PopulateLayoutList(wxString const& folder)
{
    CheckListBoxLayout->Clear();

    if (FileExists(wxFileName(folder, XLIGHTS_NETWORK_FILE))) {
        auto idx = CheckListBoxLayout->Append(XLIGHTS_NETWORK_FILE);
        CheckListBoxLayout->Check(idx);
    }
    if (FileExists(wxFileName(folder, XLIGHTS_RGBEFFECTS_FILE))) {
        auto idx = CheckListBoxLayout->Append(XLIGHTS_RGBEFFECTS_FILE);
        CheckListBoxLayout->Check(idx);
    }
    if (FileExists(wxFileName(folder, XLIGHTS_KEYBINDING_FILE))) {
        auto idx = CheckListBoxLayout->Append(XLIGHTS_KEYBINDING_FILE);
        CheckListBoxLayout->Check(idx);
    }

}

void RestoreBackupDialog::PopulateSequenceList(wxString const& folder)
{
    CheckListBoxSequences->Clear();
    auto const sequences = GetSeqList(folder);
    for (auto const& s : sequences) {
        CheckListBoxSequences->Append(s);
    }
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



void RestoreBackupDialog::OnCheckListBoxLayoutToggled(wxCommandEvent& event)
{
    ValidateWindow();
}

void RestoreBackupDialog::OnCheckListBoxSequencesToggled(wxCommandEvent& event)
{
    ValidateWindow();
}
