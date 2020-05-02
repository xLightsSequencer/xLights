/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
 
//(*InternalHeaders(FolderSelection)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/config.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/msgdlg.h>

#include "FolderSelection.h"
#include "osxMacUtils.h"

#include <log4cpp/Category.hh>
#include "UtilFunctions.h"

//(*IdInit(FolderSelection)
const long FolderSelection::ID_CHECKBOX_MEDIA_USE_SHOW = wxNewId();
const long FolderSelection::ID_TEXTCTRL_MEDIA_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_MEDIA_DIR = wxNewId();
const long FolderSelection::ID_CHECKBOX_FSEQ_USE_SHOW = wxNewId();
const long FolderSelection::ID_TEXTCTRL_FSEQ_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_FSEQ_DIR = wxNewId();
const long FolderSelection::ID_CHECKBOX_RENDERCACHE_USE_SHOW = wxNewId();
const long FolderSelection::ID_TEXTCTRL_RENDERCACHE_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_RENDERCACHE_DIR = wxNewId();
const long FolderSelection::ID_CHECKBOX_BACKUP_USE_SHOW = wxNewId();
const long FolderSelection::ID_TEXTCTRL_BACKUP_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_BACKUP_DIRECTORY = wxNewId();
const long FolderSelection::ID_CHECKBOX_ENABLE_ALT_BACKUP = wxNewId();
const long FolderSelection::ID_TEXTCTRL_ALT_BACKUP_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_ALT_BACKUP_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FOLDER_SELECT_OK = wxNewId();
const long FolderSelection::ID_BUTTON_FOLDER_SELECT_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(FolderSelection,wxDialog)
	//(*EventTable(FolderSelection)
	//*)
END_EVENT_TABLE()

FolderSelection::FolderSelection(wxWindow* parent, const wxString &showDirectory, const wxString &mediaDirectory, const wxString &fseqDirectory, const wxString& renderCacheDirectory,
						const wxString &backupDirectory, const wxString &altBackupDirectory, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
	//(*Initialize(FolderSelection)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizerAltBackupDir;
	wxStaticBoxSizer* StaticBoxSizerBackupDir;
	wxStaticBoxSizer* StaticBoxSizerFreqDir;
	wxStaticBoxSizer* StaticBoxSizerMediaDir;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticBoxSizerMediaDir = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Media Directory"));
	CheckBoxMediaUseShow = new wxCheckBox(this, ID_CHECKBOX_MEDIA_USE_SHOW, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MEDIA_USE_SHOW"));
	CheckBoxMediaUseShow->SetValue(false);
	StaticBoxSizerMediaDir->Add(CheckBoxMediaUseShow, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrlMediaDirectory = new wxTextCtrl(this, ID_TEXTCTRL_MEDIA_DIRECTORY, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MEDIA_DIRECTORY"));
	StaticBoxSizerMediaDir->Add(TextCtrlMediaDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindMediaDir = new wxButton(this, ID_BUTTON_FIND_MEDIA_DIR, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_MEDIA_DIR"));
	StaticBoxSizerMediaDir->Add(ButtonFindMediaDir, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizerMediaDir, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizerFreqDir = new wxStaticBoxSizer(wxHORIZONTAL, this, _("FSEQ Directory"));
	CheckBoxFSEQUseShow = new wxCheckBox(this, ID_CHECKBOX_FSEQ_USE_SHOW, _("Use Sequence Location"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FSEQ_USE_SHOW"));
	CheckBoxFSEQUseShow->SetValue(false);
	StaticBoxSizerFreqDir->Add(CheckBoxFSEQUseShow, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrlFSEQDirectory = new wxTextCtrl(this, ID_TEXTCTRL_FSEQ_DIRECTORY, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_FSEQ_DIRECTORY"));
	StaticBoxSizerFreqDir->Add(TextCtrlFSEQDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindFSEQDir = new wxButton(this, ID_BUTTON_FIND_FSEQ_DIR, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_FSEQ_DIR"));
	StaticBoxSizerFreqDir->Add(ButtonFindFSEQDir, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizerFreqDir, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Render Cache Directory"));
	CheckBoxRenderCacheUseShow = new wxCheckBox(this, ID_CHECKBOX_RENDERCACHE_USE_SHOW, _("Use Sequence Location"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_RENDERCACHE_USE_SHOW"));
	CheckBoxRenderCacheUseShow->SetValue(false);
	StaticBoxSizer1->Add(CheckBoxRenderCacheUseShow, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrlRenderCacheDirectory = new wxTextCtrl(this, ID_TEXTCTRL_RENDERCACHE_DIRECTORY, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_RENDERCACHE_DIRECTORY"));
	StaticBoxSizer1->Add(TextCtrlRenderCacheDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindRenderCacheDir = new wxButton(this, ID_BUTTON_FIND_RENDERCACHE_DIR, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_RENDERCACHE_DIR"));
	StaticBoxSizer1->Add(ButtonFindRenderCacheDir, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizerBackupDir = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Backup Directory"));
	CheckBoxBackupUseShow = new wxCheckBox(this, ID_CHECKBOX_BACKUP_USE_SHOW, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_BACKUP_USE_SHOW"));
	CheckBoxBackupUseShow->SetValue(false);
	StaticBoxSizerBackupDir->Add(CheckBoxBackupUseShow, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrlBackupDirectory = new wxTextCtrl(this, ID_TEXTCTRL_BACKUP_DIRECTORY, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_BACKUP_DIRECTORY"));
	StaticBoxSizerBackupDir->Add(TextCtrlBackupDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindBackupDirectory = new wxButton(this, ID_BUTTON_FIND_BACKUP_DIRECTORY, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_BACKUP_DIRECTORY"));
	StaticBoxSizerBackupDir->Add(ButtonFindBackupDirectory, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizerBackupDir, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizerAltBackupDir = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Alternative Backup Directory"));
	CheckBoxEnableAltBackup = new wxCheckBox(this, ID_CHECKBOX_ENABLE_ALT_BACKUP, _("Enable"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ENABLE_ALT_BACKUP"));
	CheckBoxEnableAltBackup->SetValue(false);
	StaticBoxSizerAltBackupDir->Add(CheckBoxEnableAltBackup, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrlAltBackupDirectory = new wxTextCtrl(this, ID_TEXTCTRL_ALT_BACKUP_DIRECTORY, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_ALT_BACKUP_DIRECTORY"));
	StaticBoxSizerAltBackupDir->Add(TextCtrlAltBackupDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindAltBackupDirectory = new wxButton(this, ID_BUTTON_FIND_ALT_BACKUP_DIRECTORY, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_ALT_BACKUP_DIRECTORY"));
	StaticBoxSizerAltBackupDir->Add(ButtonFindAltBackupDirectory, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizerAltBackupDir, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	ButtonFolderSelectOk = new wxButton(this, ID_BUTTON_FOLDER_SELECT_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FOLDER_SELECT_OK"));
	FlexGridSizer2->Add(ButtonFolderSelectOk, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonFolderSelectCancel = new wxButton(this, ID_BUTTON_FOLDER_SELECT_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FOLDER_SELECT_CANCEL"));
	FlexGridSizer2->Add(ButtonFolderSelectCancel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_RIGHT, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX_MEDIA_USE_SHOW,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxMediaUseShowClick);
	Connect(ID_TEXTCTRL_MEDIA_DIRECTORY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FolderSelection::OnTextCtrlMediaDirectoryText);
	Connect(ID_BUTTON_FIND_MEDIA_DIR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindMediaDirClick);
	Connect(ID_CHECKBOX_FSEQ_USE_SHOW,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxFSEQUseShowClick);
	Connect(ID_TEXTCTRL_FSEQ_DIRECTORY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FolderSelection::OnTextCtrlFSEQDirectoryText);
	Connect(ID_BUTTON_FIND_FSEQ_DIR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindFSEQDirClick);
	Connect(ID_CHECKBOX_RENDERCACHE_USE_SHOW,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxRenderCacheUseShowClick);
	Connect(ID_TEXTCTRL_RENDERCACHE_DIRECTORY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FolderSelection::OnTextCtrlRenderCacheDirectoryText);
	Connect(ID_BUTTON_FIND_RENDERCACHE_DIR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindRenderCacheDirClick);
	Connect(ID_CHECKBOX_BACKUP_USE_SHOW,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxBackupUseShowClick);
	Connect(ID_TEXTCTRL_BACKUP_DIRECTORY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FolderSelection::OnTextCtrlBackupDirectoryText);
	Connect(ID_BUTTON_FIND_BACKUP_DIRECTORY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindBackupDirectoryClick);
	Connect(ID_CHECKBOX_ENABLE_ALT_BACKUP,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxEnableAltBackupClick);
	Connect(ID_TEXTCTRL_ALT_BACKUP_DIRECTORY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FolderSelection::OnTextCtrlAltBackupDirectoryText);
	Connect(ID_BUTTON_FIND_ALT_BACKUP_DIRECTORY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindAltBackupDirectoryClick);
	Connect(ID_BUTTON_FOLDER_SELECT_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFolderSelectOkClick);
	Connect(ID_BUTTON_FOLDER_SELECT_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFolderSelectCancelClick);
	//*)

    ShowDirectory = showDirectory;
    MediaDirectory = mediaDirectory;
    FseqDirectory = fseqDirectory;
    RenderCacheDirectory = renderCacheDirectory;
    BackupDirectory = backupDirectory;
    AltBackupDirectory = altBackupDirectory;
    LinkMediaDir = 1;

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr)
    {
        config->Read(_("LinkFlag"), &LinkMediaDir);
        if (LinkMediaDir) {
            CheckBoxMediaUseShow->SetValue(true);
            MediaDirectory = ShowDirectory;
            TextCtrlMediaDirectory->Enable(false);
            ButtonFindMediaDir->Enable(false);
            TextCtrlMediaDirectory->SetLabel(MediaDirectory);
        } else {
            CheckBoxMediaUseShow->SetValue(false);
            TextCtrlMediaDirectory->Enable(true);
            ButtonFindMediaDir->Enable(true);
            TextCtrlMediaDirectory->SetLabel(MediaDirectory);
        }

        if (wxFileName(FseqDirectory) == wxFileName(ShowDirectory)) {
            CheckBoxFSEQUseShow->SetValue(true);
            FseqDirectory = ShowDirectory;
            TextCtrlFSEQDirectory->Enable(false);
            ButtonFindFSEQDir->Enable(false);
            TextCtrlFSEQDirectory->SetLabel(FseqDirectory);
        } else {
            CheckBoxFSEQUseShow->SetValue(false);
            TextCtrlFSEQDirectory->Enable(true);
            ButtonFindFSEQDir->Enable(true);
            TextCtrlFSEQDirectory->SetLabel(FseqDirectory);
        }

        if (wxFileName(RenderCacheDirectory) == wxFileName(ShowDirectory)) {
            CheckBoxRenderCacheUseShow->SetValue(true);
            RenderCacheDirectory = ShowDirectory;
            TextCtrlRenderCacheDirectory->Enable(false);
            ButtonFindRenderCacheDir->Enable(false);
            TextCtrlRenderCacheDirectory->SetLabel(RenderCacheDirectory);
        }
        else {
            CheckBoxRenderCacheUseShow->SetValue(false);
            TextCtrlRenderCacheDirectory->Enable(true);
            ButtonFindRenderCacheDir->Enable(true);
            TextCtrlRenderCacheDirectory->SetLabel(RenderCacheDirectory);
        }

        if (wxFileName(BackupDirectory) == wxFileName(ShowDirectory)) {
            CheckBoxBackupUseShow->SetValue(true);
            BackupDirectory = ShowDirectory;
            TextCtrlBackupDirectory->Enable(false);
            ButtonFindBackupDirectory->Enable(false);
            TextCtrlBackupDirectory->SetLabel(BackupDirectory);
        } else {
            CheckBoxBackupUseShow->SetValue(false);
            TextCtrlBackupDirectory->Enable(true);
            ButtonFindBackupDirectory->Enable(true);
            TextCtrlBackupDirectory->SetLabel(BackupDirectory);
        }
    }

    if (!altBackupDirectory.IsEmpty() ) {
        CheckBoxEnableAltBackup->SetValue(true);
        TextCtrlAltBackupDirectory->Enable(true);
        ButtonFindAltBackupDirectory->Enable(true);
        TextCtrlAltBackupDirectory->SetLabel(AltBackupDirectory);
    } else {
        AltBackupDirectory = wxString();
        CheckBoxEnableAltBackup->SetValue(false);
        TextCtrlAltBackupDirectory->Enable(false);
        TextCtrlAltBackupDirectory->SetLabel(wxString());
        ButtonFindAltBackupDirectory->Enable(false);
    }
}

FolderSelection::~FolderSelection()
{
	//(*Destroy(FolderSelection)
	//*)
}


void FolderSelection::OnButtonFolderSelectOkClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (wxFileName(MediaDirectory) == wxFileName(ShowDirectory)) {
        LinkMediaDir = 1;
        MediaDirectory = ShowDirectory;
    }

    ObtainAccessToURL(MediaDirectory.ToStdString());
    ObtainAccessToURL(FseqDirectory.ToStdString());
    ObtainAccessToURL(RenderCacheDirectory.ToStdString());
    ObtainAccessToURL(BackupDirectory.ToStdString());
    ObtainAccessToURL(AltBackupDirectory.ToStdString());

    if (!wxDir::Exists(MediaDirectory)) {
        logger_base.error("Media Directory is non-existent '%s'", (const char *)MediaDirectory.c_str());
        DisplayError("Media Directory is non-existent!");
        return;
    }

    if (wxFileName(FseqDirectory) != wxFileName(ShowDirectory)) {
        if (!wxDir::Exists(FseqDirectory)) {
            logger_base.error("FSEQ Directory is non-existent '%s'", (const char *)FseqDirectory.c_str());
            DisplayError("FSEQ Directory is non-existent!");
            return;
        }
    }

    if (wxFileName(RenderCacheDirectory) != wxFileName(ShowDirectory)) {
        if (!wxDir::Exists(RenderCacheDirectory)) {
            logger_base.error("Render Cache Directory is non-existent '%s'", (const char*)RenderCacheDirectory.c_str());
            DisplayError("Render Cache Directory is non-existent!");
            return;
        }
    }

    if (wxFileName(BackupDirectory) != wxFileName(ShowDirectory)) {
        if (!wxDir::Exists(BackupDirectory)) {
            logger_base.error("Backup Directory is non-existent '%s'", (const char *)BackupDirectory.c_str());
            DisplayError("Backup Directory is non-existent!");
            return;
        }
    }

    if (CheckBoxEnableAltBackup->IsChecked()) {
        if (!wxDir::Exists(AltBackupDirectory)) {
            logger_base.error("Alt Backup Directory is non-existent '%s'", (const char *)AltBackupDirectory.c_str());
            DisplayError("Alt Backup Directory is non-existent!");
            return;
        }
    } else {
        AltBackupDirectory = wxString();
    }

    EndDialog(wxID_OK);
}

void FolderSelection::OnButtonFolderSelectCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void FolderSelection::OnButtonFindMediaDirClick(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(MediaDirectory);
    if (dialog.ShowModal() == wxID_OK) {
        TextCtrlMediaDirectory->SetValue(dialog.GetPath());
    }
}

void FolderSelection::OnButtonFindFSEQDirClick(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(FseqDirectory);
    if (dialog.ShowModal() == wxID_OK) {
        TextCtrlFSEQDirectory->SetValue(dialog.GetPath());
    }
}

void FolderSelection::OnButtonFindRenderCacheDirClick(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(RenderCacheDirectory);
    if (dialog.ShowModal() == wxID_OK) {
        TextCtrlRenderCacheDirectory->SetValue(dialog.GetPath());
    }
}

void FolderSelection::OnButtonFindBackupDirectoryClick(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(BackupDirectory);
    if (dialog.ShowModal() == wxID_OK) {
        TextCtrlBackupDirectory->SetValue(dialog.GetPath());
    }
}

void FolderSelection::OnButtonFindAltBackupDirectoryClick(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(AltBackupDirectory);
    if (dialog.ShowModal() == wxID_OK) {
        TextCtrlAltBackupDirectory->SetValue(dialog.GetPath());
    }
}

void FolderSelection::OnCheckBoxFSEQUseShowClick(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        FseqDirectory = ShowDirectory;
        TextCtrlFSEQDirectory->Enable(false);
        ButtonFindFSEQDir->Enable(false);
        TextCtrlFSEQDirectory->SetValue(FseqDirectory);
    } else {

        TextCtrlFSEQDirectory->Enable(true);
        ButtonFindFSEQDir->Enable(true);
        TextCtrlFSEQDirectory->SetValue(FseqDirectory);
    }
}

void FolderSelection::OnCheckBoxRenderCacheUseShowClick(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        RenderCacheDirectory = ShowDirectory;
        TextCtrlRenderCacheDirectory->Enable(false);
        ButtonFindRenderCacheDir->Enable(false);
        TextCtrlRenderCacheDirectory->SetValue(RenderCacheDirectory);
    }
    else {

        TextCtrlRenderCacheDirectory->Enable(true);
        ButtonFindRenderCacheDir->Enable(true);
        TextCtrlRenderCacheDirectory->SetValue(RenderCacheDirectory);
    }
}

void FolderSelection::OnCheckBoxMediaUseShowClick(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        LinkMediaDir = 1;
        MediaDirectory = ShowDirectory;
        TextCtrlMediaDirectory->Enable(false);
        ButtonFindMediaDir->Enable(false);
        TextCtrlMediaDirectory->SetValue(MediaDirectory);
    } else {
        LinkMediaDir = 0;
        TextCtrlMediaDirectory->Enable(true);
        ButtonFindMediaDir->Enable(true);
        TextCtrlMediaDirectory->SetValue(MediaDirectory);
    }
}

void FolderSelection::OnCheckBoxBackupUseShowClick(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        BackupDirectory = ShowDirectory;
        TextCtrlBackupDirectory->Enable(false);
        ButtonFindBackupDirectory->Enable(false);
        TextCtrlBackupDirectory->SetValue(BackupDirectory);
    } else {
        TextCtrlBackupDirectory->Enable(true);
        ButtonFindBackupDirectory->Enable(true);
        TextCtrlBackupDirectory->SetValue(BackupDirectory);
    }
}

void FolderSelection::OnCheckBoxEnableAltBackupClick(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        CheckBoxEnableAltBackup->SetValue(true);
        TextCtrlAltBackupDirectory->Enable(true);
        ButtonFindAltBackupDirectory->Enable(true);
        TextCtrlAltBackupDirectory->SetValue(AltBackupDirectory);
    } else {
        AltBackupDirectory = wxString();
        CheckBoxEnableAltBackup->SetValue(false);
        TextCtrlAltBackupDirectory->Enable(false);
        TextCtrlAltBackupDirectory->SetValue(wxString());
        ButtonFindAltBackupDirectory->Enable(false);
    }
}

void FolderSelection::OnTextCtrlMediaDirectoryText(wxCommandEvent& event)
{
    MediaDirectory = event.GetString();
}

void FolderSelection::OnTextCtrlFSEQDirectoryText(wxCommandEvent& event)
{
    FseqDirectory = event.GetString();
}

void FolderSelection::OnTextCtrlRenderCacheDirectoryText(wxCommandEvent& event)
{
    RenderCacheDirectory = event.GetString();
}

void FolderSelection::OnTextCtrlBackupDirectoryText(wxCommandEvent& event)
{
    BackupDirectory = event.GetString();
}

void FolderSelection::OnTextCtrlAltBackupDirectoryText(wxCommandEvent& event)
{
    AltBackupDirectory = event.GetString();
}
