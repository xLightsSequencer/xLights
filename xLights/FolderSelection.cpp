#include "FolderSelection.h"

//(*InternalHeaders(FolderSelection)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/config.h>
#include <wx/dirdlg.h>
#include "xLightsMain.h"
#include "osxMacUtils.h"

//(*IdInit(FolderSelection)
const long FolderSelection::ID_CHECKBOX_MEDIA_USE_SHOW = wxNewId();
const long FolderSelection::ID_TEXTCTRL_MEDIA_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_MEDIA_DIR = wxNewId();
const long FolderSelection::ID_CHECKBOX_FSEQ_USE_SHOW = wxNewId();
const long FolderSelection::ID_TEXTCTRL_FSEQ_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_FSEQ_DIR = wxNewId();
const long FolderSelection::ID_CHECKBOX_BACKUP_USE_SHOW = wxNewId();
const long FolderSelection::ID_TEXTCTRL_BACKUP_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FIND_BACKUP_DIRECTORY = wxNewId();
const long FolderSelection::ID_BUTTON_FOLDER_SELECT_OK = wxNewId();
const long FolderSelection::ID_BUTTON_FOLDER_SELECT_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(FolderSelection,wxDialog)
	//(*EventTable(FolderSelection)
	//*)
END_EVENT_TABLE()

FolderSelection::FolderSelection(wxWindow* parent, const wxString &showDirectory, const wxString &mediaDirectory, const wxString &fseqDirectory, const wxString &backupDirectory, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
	//(*Initialize(FolderSelection)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizerBackupDir;
	wxStaticBoxSizer* StaticBoxSizerFreqDir;
	wxStaticBoxSizer* StaticBoxSizerMediaDir;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(4, 1, 0, 0);
	StaticBoxSizerMediaDir = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Media Directory"));
	CheckBoxMediaUseShow = new wxCheckBox(this, ID_CHECKBOX_MEDIA_USE_SHOW, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MEDIA_USE_SHOW"));
	CheckBoxMediaUseShow->SetValue(false);
	StaticBoxSizerMediaDir->Add(CheckBoxMediaUseShow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlMediaDirectory = new wxTextCtrl(this, ID_TEXTCTRL_MEDIA_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_MEDIA_DIRECTORY"));
	StaticBoxSizerMediaDir->Add(TextCtrlMediaDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindMediaDir = new wxButton(this, ID_BUTTON_FIND_MEDIA_DIR, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_MEDIA_DIR"));
	StaticBoxSizerMediaDir->Add(ButtonFindMediaDir, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizerMediaDir, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizerFreqDir = new wxStaticBoxSizer(wxHORIZONTAL, this, _("FSEQ Directory"));
	CheckBoxFSEQUseShow = new wxCheckBox(this, ID_CHECKBOX_FSEQ_USE_SHOW, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FSEQ_USE_SHOW"));
	CheckBoxFSEQUseShow->SetValue(false);
	StaticBoxSizerFreqDir->Add(CheckBoxFSEQUseShow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlFSEQDirectory = new wxTextCtrl(this, ID_TEXTCTRL_FSEQ_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_FSEQ_DIRECTORY"));
	StaticBoxSizerFreqDir->Add(TextCtrlFSEQDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindFSEQDir = new wxButton(this, ID_BUTTON_FIND_FSEQ_DIR, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_FSEQ_DIR"));
	StaticBoxSizerFreqDir->Add(ButtonFindFSEQDir, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizerFreqDir, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizerBackupDir = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Backup Directory"));
	CheckBoxBackupUseShow = new wxCheckBox(this, ID_CHECKBOX_BACKUP_USE_SHOW, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_BACKUP_USE_SHOW"));
	CheckBoxBackupUseShow->SetValue(false);
	StaticBoxSizerBackupDir->Add(CheckBoxBackupUseShow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlBackupDirectory = new wxTextCtrl(this, ID_TEXTCTRL_BACKUP_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_BACKUP_DIRECTORY"));
	StaticBoxSizerBackupDir->Add(TextCtrlBackupDirectory, 2, wxALL|wxEXPAND, 5);
	ButtonFindBackupDirectory = new wxButton(this, ID_BUTTON_FIND_BACKUP_DIRECTORY, _("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIND_BACKUP_DIRECTORY"));
	StaticBoxSizerBackupDir->Add(ButtonFindBackupDirectory, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizerBackupDir, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonFolderSelectOk = new wxButton(this, ID_BUTTON_FOLDER_SELECT_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FOLDER_SELECT_OK"));
	FlexGridSizer2->Add(ButtonFolderSelectOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonFolderSelectCancel = new wxButton(this, ID_BUTTON_FOLDER_SELECT_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FOLDER_SELECT_CANCEL"));
	FlexGridSizer2->Add(ButtonFolderSelectCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX_MEDIA_USE_SHOW,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxMediaUseShowClick);
	Connect(ID_BUTTON_FIND_MEDIA_DIR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindMediaDirClick);
	Connect(ID_CHECKBOX_FSEQ_USE_SHOW,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxFSEQUseShowClick);
	Connect(ID_BUTTON_FIND_FSEQ_DIR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindFSEQDirClick);
	Connect(ID_CHECKBOX_BACKUP_USE_SHOW,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FolderSelection::OnCheckBoxBackupUseShowClick);
	Connect(ID_BUTTON_FIND_BACKUP_DIRECTORY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFindBackupDirectoryClick);
	Connect(ID_BUTTON_FOLDER_SELECT_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFolderSelectOkClick);
	Connect(ID_BUTTON_FOLDER_SELECT_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FolderSelection::OnButtonFolderSelectCancelClick);
	//*)

    ShowDirectory = showDirectory;
    MediaDirectory = mediaDirectory;
    FseqDirectory = fseqDirectory;
    BackupDirectory = backupDirectory;
    LinkMediaDir = 0;
    LinkFSEQDir = 1;
    LinkBackupDir = 1;

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
        }
        else {
            CheckBoxMediaUseShow->SetValue(false);
            TextCtrlMediaDirectory->Enable(true);
            ButtonFindMediaDir->Enable(true);
            TextCtrlMediaDirectory->SetLabel(MediaDirectory);
        }
        config->Read(_("FSEQLinkFlag"), &LinkFSEQDir);
        if (LinkFSEQDir) {
            CheckBoxFSEQUseShow->SetValue(true);
            FseqDirectory = ShowDirectory;
            TextCtrlFSEQDirectory->Enable(false);
            ButtonFindFSEQDir->Enable(false);
            TextCtrlFSEQDirectory->SetLabel(FseqDirectory);
        }
        else {
            CheckBoxFSEQUseShow->SetValue(false);
            TextCtrlFSEQDirectory->Enable(true);
            ButtonFindFSEQDir->Enable(true);
            TextCtrlFSEQDirectory->SetLabel(FseqDirectory);
        }

        config->Read(_("BackupLinkFlag"), &LinkBackupDir);
        if (LinkBackupDir) {
            CheckBoxBackupUseShow->SetValue(true);
            BackupDirectory = ShowDirectory;
            TextCtrlBackupDirectory->Enable(false);
            ButtonFindBackupDirectory->Enable(false);
            TextCtrlBackupDirectory->SetLabel(BackupDirectory);
        }
        else {
            CheckBoxBackupUseShow->SetValue(false);
            TextCtrlBackupDirectory->Enable(true);
            ButtonFindBackupDirectory->Enable(true);
            TextCtrlBackupDirectory->SetLabel(BackupDirectory);
        }
    }
}

FolderSelection::~FolderSelection()
{
	//(*Destroy(FolderSelection)
	//*)
}


void FolderSelection::OnButtonFolderSelectOkClick(wxCommandEvent& event)
{
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
        MediaDirectory = dialog.GetPath();
        ObtainAccessToURL(_mediaDirectory.ToStdString());
        TextCtrlMediaDirectory->SetLabel(MediaDirectory);
    }
}

void FolderSelection::OnButtonFindFSEQDirClick(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(FseqDirectory);
    if (dialog.ShowModal() == wxID_OK) {
        FseqDirectory = dialog.GetPath();
        ObtainAccessToURL(_fseqDirectory.ToStdString());
        TextCtrlFSEQDirectory->SetLabel(FseqDirectory);
    }
}

void FolderSelection::OnCheckBoxFSEQUseShowClick(wxCommandEvent& event)
{
    if (CheckBoxFSEQUseShow->IsChecked()) {
        LinkFSEQDir = 1;
        FseqDirectory = ShowDirectory;
        TextCtrlFSEQDirectory->Enable(false);
        ButtonFindFSEQDir->Enable(false);
        TextCtrlFSEQDirectory->SetLabel(FseqDirectory);
    }
    else {
        LinkFSEQDir = 0;
        TextCtrlFSEQDirectory->Enable(true);
        ButtonFindFSEQDir->Enable(true);
        TextCtrlFSEQDirectory->SetLabel(FseqDirectory);
    }
}

void FolderSelection::OnCheckBoxMediaUseShowClick(wxCommandEvent& event)
{
    if (CheckBoxMediaUseShow->IsChecked()) {
        LinkMediaDir = 1;
        MediaDirectory = ShowDirectory;
        TextCtrlMediaDirectory->Enable(false);
        ButtonFindMediaDir->Enable(false);
        TextCtrlMediaDirectory->SetLabel(MediaDirectory);
    }
    else {
        LinkMediaDir = 0;
        TextCtrlMediaDirectory->Enable(true);
        ButtonFindMediaDir->Enable(true);
        TextCtrlMediaDirectory->SetLabel(MediaDirectory);
    }
}

void FolderSelection::OnCheckBoxBackupUseShowClick(wxCommandEvent& event)
{
    if (CheckBoxBackupUseShow->IsChecked()) {
        LinkBackupDir = 1;
        BackupDirectory = ShowDirectory;
        TextCtrlBackupDirectory->Enable(false);
        ButtonFindBackupDirectory->Enable(false);
        TextCtrlMediaDirectory->SetLabel(BackupDirectory);
    }
    else {
        LinkBackupDir = 0;
        TextCtrlBackupDirectory->Enable(true);
        ButtonFindBackupDirectory->Enable(true);
        TextCtrlMediaDirectory->SetLabel(BackupDirectory);
    }
}

void FolderSelection::OnButtonFindBackupDirectoryClick(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(BackupDirectory);
    if (dialog.ShowModal() == wxID_OK) {
        FseqDirectory = dialog.GetPath();
        ObtainAccessToURL(BackupDirectory.ToStdString());
        TextCtrlMediaDirectory->SetLabel(BackupDirectory);
    }
}
