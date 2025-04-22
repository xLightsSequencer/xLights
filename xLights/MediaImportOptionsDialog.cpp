/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(MediaImportOptionsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "MediaImportOptionsDialog.h"
#include "UtilFunctions.h"

//(*IdInit(MediaImportOptionsDialog)
const long MediaImportOptionsDialog::ID_STATICTEXT1 = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_RESTORE_DEFAULTS = wxNewId();
const long MediaImportOptionsDialog::ID_TEXTCTRL_FACES_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_FACES_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_TEXTCTRL_GLEDIATORS_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_GLEDIATORS_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_TEXTCTRL3 = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_IMAGES_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_TEXTCTRL_SHADERS_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_SHADERS_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_TEXTCTRL_VIDEOS_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_VIDEOS_FLD = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_OK = wxNewId();
const long MediaImportOptionsDialog::ID_BUTTON_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(MediaImportOptionsDialog,wxDialog)
	//(*EventTable(MediaImportOptionsDialog)
	//*)
END_EVENT_TABLE()

MediaImportOptionsDialog::MediaImportOptionsDialog(wxWindow* parent, SeqPkgImportOptions* importOptions, wxWindowID id,const wxPoint& pos,const wxSize& size) : _importOptions(importOptions)
{
	//(*Initialize(MediaImportOptionsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxGridBagSizer* SizerQuickActions;
	wxStaticBoxSizer* SizerFaces;
	wxStaticBoxSizer* SizerGlediators;
	wxStaticBoxSizer* SizerImages;
	wxStaticBoxSizer* SizerShaders;
	wxStaticBoxSizer* SizerVideos;

	Create(parent, wxID_ANY, _("Media Import Options"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select the target directories for each media type where available files will be imported into.\n\nNOTE:  Only media files from mapped effects which actually exist in the sequence package \nyou are mapping from will be imported."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer8->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	SizerQuickActions = new wxGridBagSizer(0, 0);
	ButtonRestoreDefaults = new wxButton(this, ID_BUTTON_RESTORE_DEFAULTS, _("Restore Defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RESTORE_DEFAULTS"));
	SizerQuickActions->Add(ButtonRestoreDefaults, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SizerQuickActions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(SizerQuickActions, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SizerFaces = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Faces"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FacesFolder = new wxTextCtrl(this, ID_TEXTCTRL_FACES_FLD, wxEmptyString, wxDefaultPosition, wxSize(450,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_FACES_FLD"));
	FlexGridSizer2->Add(FacesFolder, 1, wxALL|wxEXPAND, 5);
	ButtonFacesFolder = new wxButton(this, ID_BUTTON_FACES_FLD, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FACES_FLD"));
	FlexGridSizer2->Add(ButtonFacesFolder, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerFaces->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(SizerFaces, 1, wxALL|wxEXPAND, 5);
	SizerGlediators = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Glediators"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	GlediatorsFolder = new wxTextCtrl(this, ID_TEXTCTRL_GLEDIATORS_FLD, wxEmptyString, wxDefaultPosition, wxSize(450,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_GLEDIATORS_FLD"));
	FlexGridSizer3->Add(GlediatorsFolder, 1, wxALL|wxEXPAND, 5);
	ButtonGlediatorsFolder = new wxButton(this, ID_BUTTON_GLEDIATORS_FLD, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_GLEDIATORS_FLD"));
	FlexGridSizer3->Add(ButtonGlediatorsFolder, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerGlediators->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(SizerGlediators, 1, wxALL|wxEXPAND, 5);
	SizerImages = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Images"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	ImagesFolder = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(450,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer4->Add(ImagesFolder, 1, wxALL|wxEXPAND, 5);
	ButtonImagesFolder = new wxButton(this, ID_BUTTON_IMAGES_FLD, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMAGES_FLD"));
	FlexGridSizer4->Add(ButtonImagesFolder, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerImages->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(SizerImages, 1, wxALL|wxEXPAND, 5);
	SizerShaders = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Shaders"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	ShadersFolder = new wxTextCtrl(this, ID_TEXTCTRL_SHADERS_FLD, wxEmptyString, wxDefaultPosition, wxSize(450,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_SHADERS_FLD"));
	FlexGridSizer5->Add(ShadersFolder, 1, wxALL|wxEXPAND, 5);
	ButtonShadersFolder = new wxButton(this, ID_BUTTON_SHADERS_FLD, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SHADERS_FLD"));
	FlexGridSizer5->Add(ButtonShadersFolder, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerShaders->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(SizerShaders, 1, wxALL|wxEXPAND, 5);
	SizerVideos = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Videos"));
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	VideosFolder = new wxTextCtrl(this, ID_TEXTCTRL_VIDEOS_FLD, wxEmptyString, wxDefaultPosition, wxSize(450,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_VIDEOS_FLD"));
	FlexGridSizer6->Add(VideosFolder, 1, wxALL|wxEXPAND, 5);
	ButtonVideosFolder = new wxButton(this, ID_BUTTON_VIDEOS_FLD, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_VIDEOS_FLD"));
	FlexGridSizer6->Add(ButtonVideosFolder, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerVideos->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(SizerVideos, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonOK = new wxButton(this, ID_BUTTON_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OK"));
	FlexGridSizer7->Add(ButtonOK, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer7->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_RESTORE_DEFAULTS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonRestoreDefaultsClick);
	Connect(ID_BUTTON_FACES_FLD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonSelectFolderClick);
	Connect(ID_BUTTON_GLEDIATORS_FLD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonSelectFolderClick);
	Connect(ID_BUTTON_IMAGES_FLD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonSelectFolderClick);
	Connect(ID_BUTTON_SHADERS_FLD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonSelectFolderClick);
	Connect(ID_BUTTON_VIDEOS_FLD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonSelectFolderClick);
	Connect(ID_BUTTON_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonOKClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MediaImportOptionsDialog::OnButtonCancelClick);
	Connect(wxID_ANY,wxEVT_INIT_DIALOG,(wxObjectEventFunction)&MediaImportOptionsDialog::OnInit);
	//*)
}

MediaImportOptionsDialog::~MediaImportOptionsDialog()
{
	//(*Destroy(MediaImportOptionsDialog)
	//*)
}


void MediaImportOptionsDialog::OnInit(wxInitDialogEvent& event)
{
    UpdateOptions();
}

void MediaImportOptionsDialog::UpdateOptions() {

    FacesFolder->SetValue(_importOptions->GetDir(MediaTargetDir::FACES_DIR));
    GlediatorsFolder->SetValue(_importOptions->GetDir(MediaTargetDir::GLEDIATORS_DIR));
    ImagesFolder->SetValue(_importOptions->GetDir(MediaTargetDir::IMAGES_DIR));
    ShadersFolder->SetValue(_importOptions->GetDir(MediaTargetDir::SHADERS_DIR));
    VideosFolder->SetValue(_importOptions->GetDir(MediaTargetDir::VIDEOS_DIR));

    Layout();
}

void MediaImportOptionsDialog::GetCurrentLabelAndPath(int btnId, std::string& label, std::string& path) {
    wxString dlgLabel = "Select Destination for ";
    wxString fldName;

    if (btnId == ID_BUTTON_FACES_FLD) {
        fldName = "Faces";
        path = FacesFolder->GetLabel();
    } else if (btnId == ID_BUTTON_GLEDIATORS_FLD) {
        fldName = "Glediators";
        path = GlediatorsFolder->GetLabel();
    } else if (btnId == ID_BUTTON_IMAGES_FLD) {
        fldName = "Images";
        path = ImagesFolder->GetLabel();
    } else if (btnId == ID_BUTTON_SHADERS_FLD) {
        fldName = "Shaders";
        path = ShadersFolder->GetLabel();
    } else if (btnId == ID_BUTTON_VIDEOS_FLD) {
        fldName = "Videos";
        path = VideosFolder->GetLabel();
    }

    label = dlgLabel + fldName;
}

void MediaImportOptionsDialog::UpdateFolderPath(int btnId, std::string& newPath) {
    if (btnId == ID_BUTTON_FACES_FLD) {
        FacesFolder->SetValue(newPath);
        FacesFolder->SetToolTip(newPath);
    } else if (btnId == ID_BUTTON_GLEDIATORS_FLD) {
        GlediatorsFolder->SetValue(newPath);
    } else if (btnId == ID_BUTTON_IMAGES_FLD) {
        ImagesFolder->SetValue(newPath);
    } else if (btnId == ID_BUTTON_SHADERS_FLD) {
        ShadersFolder->SetValue(newPath);
    } else if (btnId == ID_BUTTON_VIDEOS_FLD) {
        VideosFolder->SetValue(newPath);
    }

    Layout();
}

void MediaImportOptionsDialog::OnButtonSelectFolderClick(wxCommandEvent& event)
{
    std::string label, path;
    GetCurrentLabelAndPath(event.GetId(), label, path);

    wxDirDialog dlg(this, label, path, wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST|wxDD_CHANGE_DIR, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        std::string newPath = dlg.GetPath().ToStdString();
        UpdateFolderPath(event.GetId(), newPath);
    }
}

void MediaImportOptionsDialog::OnButtonOKClick(wxCommandEvent& event)
{
    _importOptions->SetDir(MediaTargetDir::FACES_DIR, FacesFolder->GetValue());
    _importOptions->SetDir(MediaTargetDir::GLEDIATORS_DIR, GlediatorsFolder->GetValue());
    _importOptions->SetDir(MediaTargetDir::IMAGES_DIR, ImagesFolder->GetValue());
    _importOptions->SetDir(MediaTargetDir::SHADERS_DIR, ShadersFolder->GetValue());
    _importOptions->SetDir(MediaTargetDir::VIDEOS_DIR, VideosFolder->GetValue());

    EndDialog(wxID_OK);
}

void MediaImportOptionsDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void MediaImportOptionsDialog::OnButtonRestoreDefaultsClick(wxCommandEvent& event)
{
    _importOptions->RestoreDefaults();
    UpdateOptions();
}
