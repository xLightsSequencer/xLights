/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowFolderSearchDialog.h"

//(*InternalHeaders(ShowFolderSearchDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include "ExternalHooks.h"
#include "xLightsMain.h"

#include <log4cpp/Category.hh>
#include <wx/regex.h>

//(*IdInit(ShowFolderSearchDialog)
const wxWindowID ShowFolderSearchDialog::ID_LISTCTRL_FOLDERS = wxNewId();
const wxWindowID ShowFolderSearchDialog::ID_STATICTEXT_BACKUPFOLDER = wxNewId();
const wxWindowID ShowFolderSearchDialog::ID_BUTTON_TEMP = wxNewId();
const wxWindowID ShowFolderSearchDialog::ID_BUTTON_PERMANENT = wxNewId();
const wxWindowID ShowFolderSearchDialog::ID_BUTTON_OPEN = wxNewId();
//*)

BEGIN_EVENT_TABLE(ShowFolderSearchDialog,wxDialog)
	//(*EventTable(ShowFolderSearchDialog)
	//*)

END_EVENT_TABLE()

ShowFolderSearchDialog::ShowFolderSearchDialog(xLightsFrame* parent)
{
	//(*Initialize(ShowFolderSearchDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, _("Find Show Folder Dialog"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxDLG_UNIT(parent,wxSize(400,400)));
	SetMinSize(wxSize(800,400));
	FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListCtrl_Folders = new wxListCtrl(this, ID_LISTCTRL_FOLDERS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, _T("ID_LISTCTRL_FOLDERS"));
	FlexGridSizer1->Add(ListCtrl_Folders, 0, wxEXPAND, 5);
	StaticTextBackUpFolder = new wxStaticText(this, ID_STATICTEXT_BACKUPFOLDER, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, _T("ID_STATICTEXT_BACKUPFOLDER"));
	FlexGridSizer1->Add(StaticTextBackUpFolder, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonTemp = new wxButton(this, ID_BUTTON_TEMP, _("Change Temporary"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEMP"));
	BoxSizer1->Add(ButtonTemp, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonPermanent = new wxButton(this, ID_BUTTON_PERMANENT, _("Change Permanently"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PERMANENT"));
	BoxSizer1->Add(ButtonPermanent, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonOpen = new wxButton(this, ID_BUTTON_OPEN, _("Open in Explorer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OPEN"));
	BoxSizer1->Add(ButtonOpen, 0, wxALL, 5);
	BoxSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_LISTCTRL_FOLDERS, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&ShowFolderSearchDialog::OnListCtrl_FoldersItemSelect);
	Connect(ID_BUTTON_TEMP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ShowFolderSearchDialog::OnButtonTempClick);
	Connect(ID_BUTTON_PERMANENT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ShowFolderSearchDialog::OnButtonPermanentClick);
	Connect(ID_BUTTON_OPEN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ShowFolderSearchDialog::OnButtonOpenClick);
	//*)

    ListCtrl_Folders->AppendColumn("Folder", wxLIST_FORMAT_LEFT,
                                   wxLIST_AUTOSIZE);
    ListCtrl_Folders->AppendColumn("Opens", wxLIST_FORMAT_CENTRE);

    _xLights = parent;
    auto const& folder =  FindLogFolder();
    auto const& files = FindLogFiles(folder);
    FindShowFolders(files, folder);
    ValidateWindow();
}


ShowFolderSearchDialog::~ShowFolderSearchDialog()
{
	//(*Destroy(ShowFolderSearchDialog)
	//*)
}

void ShowFolderSearchDialog::OnInit(wxInitDialogEvent& event)
{
    Layout();
}

void ShowFolderSearchDialog::ValidateWindow()
{
    ButtonOpen->Enable(ListCtrl_Folders->GetSelectedItemCount() > 0);
    ButtonPermanent->Enable(ListCtrl_Folders->GetSelectedItemCount() > 0);
    ButtonTemp->Enable(ListCtrl_Folders->GetSelectedItemCount() > 0);
}

wxString ShowFolderSearchDialog::FindLogFolder() const
{
    wxString dir;
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    if (dir.EndsWith("/") || dir.EndsWith("\\")) {
        dir = dir.Left(dir.Length() - 1);
    }
    dir = dir + wxFileName::GetPathSeparator();
#endif
#ifdef __WXOSX__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    if (dir.EndsWith("/")) {
        dir = dir.Left(dir.Length() - 1);
    }
    dir = dir + "/Library/Logs/";
#endif
#ifdef __LINUX__
    dir = "/tmp/";
#endif
    if (!wxDir::Exists(dir)) {
        if (wxDir::Exists(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator())) {
            dir = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator();
        } else if (wxDir::Exists(wxGetCwd() + wxFileName::GetPathSeparator())) {
            dir = wxGetCwd() + wxFileName::GetPathSeparator();
        }
    }
    return dir;
}

wxArrayString ShowFolderSearchDialog::FindLogFiles(wxString const& folder) const{
    wxArrayString files;
    GetAllFilesInDir(folder, files, "xLights_l4cpp.log*", wxDIR_DEFAULT);
    files.Sort();
    return files;
}

void ShowFolderSearchDialog::FindShowFolders(wxArrayString const& logs, wxString const& folder) {
    std::map<wxString, int> showFolders;
    //Show directory set to : D:\lights_data\cubicle.
    static wxRegEx logregex("Show directory set to : (.*)\\.");
    wxProgressDialog dlg("Searching " + folder, "Searching " + folder, logs.size(), this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT);
    dlg.Show();
    int i{ 0 };
    for (auto const& log : logs) {
        if (FileExists(log)) {
            dlg.Update(i, "Reading " + log);
            wxFileName fn(log);
            wxFileInputStream input(fn.GetFullPath());
            if (input.IsOk()) {
                wxTextInputStream text(input);
                while (!input.Eof()) {
                    wxString const& line = text.ReadLine();
                    dlg.Pulse();
                    if (line.Contains("Show directory set to")) {
                        if (logregex.Matches(line)) {
                            wxString const& folder = logregex.GetMatch(line, 1);
                            if (wxDir::Exists(folder)) {
                                if (showFolders.find(folder) == showFolders.end()) {
                                    showFolders[folder] = 1;
                                } else {
                                    showFolders[folder]++;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (dlg.WasCancelled()) {
            break;
        }
#if defined(_DEBUG)
        break;
#endif
        ++i;
    }

    ListCtrl_Folders->Freeze();
    ListCtrl_Folders->DeleteAllItems();

    for (auto const& [folder,opens ]: showFolders) {
        auto item = ListCtrl_Folders->InsertItem(ListCtrl_Folders->GetItemCount(), folder);
        ListCtrl_Folders->SetItem(item, 1, wxString::Format("%i", opens));
    }
    ListCtrl_Folders->SetColumnWidth(0, wxLIST_AUTOSIZE);
    ListCtrl_Folders->Thaw();
}

void ShowFolderSearchDialog::OnListCtrl_FoldersItemSelect(wxListEvent& event)
{
    StaticTextBackUpFolder->SetLabelText(event.GetLabel());
    _selectedFolder = event.GetLabel();
    ValidateWindow();
}

void ShowFolderSearchDialog::OnButtonTempClick(wxCommandEvent& event)
{
    if (!_selectedFolder.empty()) {
        _xLights->SetDir(_selectedFolder, false);
    }
    EndDialog(0);
}

void ShowFolderSearchDialog::OnButtonPermanentClick(wxCommandEvent& event)
{
    if (!_selectedFolder.empty()) {
        _xLights->SetDir(_selectedFolder, true);
    }
    EndDialog(0);
}

void ShowFolderSearchDialog::OnButtonOpenClick(wxCommandEvent& event) {
    if (!_selectedFolder.empty()) {
        wxLaunchDefaultApplication(_selectedFolder);
    }
}
