/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SeqExportDialog.h"

//(*InternalHeaders(SeqExportDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "xLightsMain.h"
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/filepicker.h>

//(*IdInit(SeqExportDialog)
const long SeqExportDialog::ID_STATICTEXT1 = wxNewId();
const long SeqExportDialog::ID_CHOICE1 = wxNewId();
const long SeqExportDialog::ID_STATICTEXT3 = wxNewId();
const long SeqExportDialog::ID_TEXTCTRL2 = wxNewId();
const long SeqExportDialog::ID_BUTTON1 = wxNewId();
const long SeqExportDialog::ID_BUTTON2 = wxNewId();
const long SeqExportDialog::ID_BUTTON3 = wxNewId();
//*)


BEGIN_EVENT_TABLE(SeqExportDialog, wxDialog)
//(*EventTable(SeqExportDialog)
//*)
END_EVENT_TABLE()

SeqExportDialog::SeqExportDialog(wxWindow* parent, const std::string& model, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _model = model;

    //(*Initialize(SeqExportDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;

    Create(parent, wxID_ANY, _("Export Sequence"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER, _T("wxID_ANY"));
    SetClientSize(wxSize(385, 124));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Format"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    ChoiceFormat = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(ChoiceFormat, 1, wxALL | wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    TextCtrlFilename = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(400, -1), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer3->Add(TextCtrlFilename, 1, wxALL | wxEXPAND, 2);
    ButtonFilePick = new wxButton(this, ID_BUTTON1, _("..."), wxDefaultPosition, wxSize(34, -1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(ButtonFilePick, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    ButtonOk = new wxButton(this, ID_BUTTON2, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer4->Add(ButtonOk, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    ButtonCancel = new wxButton(this, ID_BUTTON3, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer4->Add(ButtonCancel, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_CHOICE1, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&SeqExportDialog::OnChoiceFormatSelect);
    Connect(ID_TEXTCTRL2, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqExportDialog::OnTextCtrlFilenameText);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqExportDialog::OnButtonFilePickClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqExportDialog::OnButtonOkClick);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqExportDialog::OnButtonCancelClick);
    //*)

    ChoiceFormat->SetSelection(ChoiceFormat->Append(_("LOR. *.lms or *.las")));
    ChoiceFormat->Append(_("Lcb, LOR Clipboard *.lcb"));
    ChoiceFormat->Append(_("Lcb, LOR S5 Clipboard *.lcb"));
    ChoiceFormat->Append(_("Vixen, Vixen sequence file *.vix"));
    ChoiceFormat->Append(_("Vir, Vixen Routine file. *.vir"));
    ChoiceFormat->Append(_("LSP, Light Show Pro "));
    ChoiceFormat->Append(_("HLS, Hinkle Lighte Sequencer *.hlsnc"));
    ChoiceFormat->Append(_("xLights/FPP, *.fseq"));
    ChoiceFormat->Append(_("Compressed Video, *.mp4"));
    ChoiceFormat->Append(_("Uncompressed Video, *.mp4"));
    ChoiceFormat->Append(_("Uncompressed Video, *.avi"));
    ChoiceFormat->Append(_("Minleon Network Effects Controller, *.bin"));
    ChoiceFormat->Append(_("GIF Image, *.gif"));

    Fit();

    ButtonOk->SetDefault();
    SetEscapeId(ButtonCancel->GetId());
    ValidateWindow();
}

SeqExportDialog::~SeqExportDialog()
{
    //(*Destroy(SeqExportDialog)
    //*)
}

void SeqExportDialog::ModelExportTypes(bool isgroup)
{
    if (isgroup) {
        ChoiceFormat->Delete(ChoiceFormat->FindString(_("Compressed Video, *.mp4")));
        ChoiceFormat->Delete(ChoiceFormat->FindString(_("Uncompressed Video, *.mp4")));
        int idx = ChoiceFormat->FindString(_("Uncompressed Video, *.avi"));
        if (idx != -1) {
            ChoiceFormat->Delete(idx);
        }
        ChoiceFormat->Delete(ChoiceFormat->FindString(_("Minleon Network Effects Controller, *.bin")));
    }
    ChoiceFormat->Delete(ChoiceFormat->FindString(_("LOR. *.lms or *.las")));
    ChoiceFormat->Delete(ChoiceFormat->FindString(_("Vixen, Vixen sequence file *.vix")));
    ChoiceFormat->Delete(ChoiceFormat->FindString(_("xLights/FPP, *.fseq")));
    ChoiceFormat->Append(_("FPP Sub sequence. *.eseq"));
    ChoiceFormat->Append(_("FPP Compressed Sub sequence. *.eseq"));

    wxString let;
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Read("xLightsLastExportType", &let, "");
        if (let == "") {
            ChoiceFormat->SetSelection(0);
        } else {
            if (!ChoiceFormat->SetStringSelection(let)) {
                ChoiceFormat->SetSelection(0);
            }
        }
    } else {
        ChoiceFormat->SetSelection(0);
    }

    SetDefaultName();
}

void SeqExportDialog::SetExportType(bool selectedEffects, bool render)
{
    std::string title = "Export Sequence - ";

    if (render && selectedEffects) {
        title += "Render then export selected effects.";
    } else if (render) {
        title += "Render then export model.";
    } else if (selectedEffects) {
        title += "Export selected effects.";
    } else {
        title += "Export model.";
    }

    SetTitle(title);
}

void SeqExportDialog::SetDefaultName()
{
    wxString fmt = ChoiceFormat->GetStringSelection();
    wxString cwd = xLightsFrame::CurrentDir;
    wxString fsd = ((xLightsFrame*)GetParent())->GetFseqDirectory();
    if (fsd == "") {
        fsd = cwd;
    }
    wxString curV = TextCtrlFilename->GetValue();
    wxString fn = curV == "" ? cwd + wxFileName::GetPathSeparator() + _model : curV;
    wxFileName name(cwd, _model);
    if (fmt == "LOR. *.lms or *.las") {
        name.SetExt("lms");
    } else if (fmt == "Lcb, LOR Clipboard *.lcb" || fmt == "Lcb, LOR S5 Clipboard *.lcb") {
        name.SetExt("lcb");
    } else if (fmt == "Vixen, Vixen sequence file *.vix") {
        name.SetExt("vix");
    } else if (fmt == "Vir, Vixen Routine file. *.vir") {
        name.SetExt("vir");
    } else if (fmt == "LSP, Light Show Pro ") {
        name.SetExt("");
    } else if (fmt == "HLS, Hinkle Lighte Sequencer *.hlsnc") {
        name.SetExt("hlsnc");
    } else if (fmt == "FPP Sub sequence. *.eseq") {
        name.SetPath(fsd);
        name.SetExt("eseq");
    } else if (fmt == "FPP Compressed Sub sequence. *.eseq") {
        name.SetPath(fsd);
        name.SetExt("eseq");
    } else if (fmt == "xLights/FPP, *.fseq") {
        name.SetPath(fsd);
        name.SetExt("fseq");
    } else if (fmt == "Compressed Video, *.mp4") {
        name.SetExt("mp4");
    } else if (fmt == "Uncompressed Video, *.mp4") {
        name.SetExt("mp4");
    } else if (fmt == "Uncompressed Video, *.avi") {
        name.SetExt("avi");
    } else if (fmt == "Minleon Network Effects Controller, *.bin") {
        name.SetExt("bin");
    } else if (fmt == "GIF Image, *.gif") {
        name.SetExt("gif");
    }
    TextCtrlFilename->SetValue(name.GetFullPath());
}

void SeqExportDialog::OnChoiceFormatSelect(wxCommandEvent& event)
{
    SetDefaultName();

    TextCtrlFilename->SetFocus();
    ValidateWindow();
}

void SeqExportDialog::OnButtonFilePickClick(wxCommandEvent& event)
{
    wxString fmt = ChoiceFormat->GetStringSelection();
    wxFileName fn(TextCtrlFilename->GetValue());
    
    if (fmt == "LOR. *.lms or *.las") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "LOR (*.lms;*.las)|*.lms;*.las", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "Lcb, LOR Clipboard *.lcb" || fmt == "Lcb, LOR S5 Clipboard *.lcb") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "LOR Clipboard (*.lcb)|*.lcb", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "Vixen, Vixen sequence file *.vix") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Vixen Sequence File (*.vix)|*.vix", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "Vir, Vixen Routine file. *.vir") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Vixen Routine File (*.vir)|*.vir", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "LSP, Light Show Pro ") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Light Show Pro (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "HLS, Hinkle Lighte Sequencer *.hlsnc") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Hinkle Light Sequencer (*.hlsnc)|*.hlsnc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "FPP Sub sequence. *.eseq") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "FPP Sub Sequence (*.eseq)|*.eseq", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "FPP Compressed Sub sequence. *.eseq") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "FPP Compresses Sub Sequence (*.eseq)|*.eseq", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "xLights/FPP, *.fseq") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "xLights/FPP (*.fseq)|*.fseq", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "Compressed Video, *.mp4") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Video (*.mp4)|*.mp4", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "Uncompressed Video, *.avi") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Video (*.avi)|*.avi", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "Uncompressed Video, *.mp4") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Video (*.mp4)|*.mp4", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "Minleon Network Effects Controller, *.bin") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "Minleon Networks Effects Controller (*.bin)|*.bin", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    } else if (fmt == "GIF Image, *.gif") {
        TextCtrlFilename->SetValue(wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, "GIF Image (*.gif)|*.gif", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this));
    }

    ValidateWindow();
}

void SeqExportDialog::OnButtonOkClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Write("xLightsLastExportType", ChoiceFormat->GetStringSelection());
        config->Flush();
    }
    EndDialog(wxID_OK);
}

void SeqExportDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void SeqExportDialog::OnTextCtrlFilenameText(wxCommandEvent& event)
{
    ValidateWindow();
}

void SeqExportDialog::ValidateWindow()
{
    if (TextCtrlFilename->GetValue() != "") {
        wxFileName fn(TextCtrlFilename->GetValue());
        if (fn.GetPathWithSep() == "" || wxDir::Exists(fn.GetPathWithSep())) {
            ButtonOk->Enable();
        } else {
            ButtonOk->Disable();
        }
    } else {
        ButtonOk->Disable();
    }
}
