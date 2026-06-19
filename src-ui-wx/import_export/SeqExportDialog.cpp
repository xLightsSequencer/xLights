/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SeqExportDialog.h"

//(*InternalHeaders(SeqExportDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "xLightsMain.h"
#include "settings/XLightsConfigAdapter.h"
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

//(*IdInit(SeqExportDialog)
const long SeqExportDialog::ID_STATICTEXT1 = wxNewId();
const long SeqExportDialog::ID_LISTBOX1 = wxNewId();
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
    wxFlexGridSizer* FlexGridSizer4;

    Create(parent, wxID_ANY, _("Export Sequence"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER, _T("wxID_ANY"));
    SetClientSize(wxSize(360, 440));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select export format:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    ListBoxFormat = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE, wxDefaultValidator, _T("ID_LISTBOX1"));
    FlexGridSizer1->Add(ListBoxFormat, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    ButtonOk = new wxButton(this, ID_BUTTON2, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    ButtonOk->SetDefault();
    FlexGridSizer4->Add(ButtonOk, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    ButtonCancel = new wxButton(this, ID_BUTTON3, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer4->Add(ButtonCancel, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_LISTBOX1, wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&SeqExportDialog::OnListBoxFormatSelect);
    Connect(ID_LISTBOX1, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, (wxObjectEventFunction)&SeqExportDialog::OnListBoxFormatDClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqExportDialog::OnButtonOkClick);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqExportDialog::OnButtonCancelClick);
    //*)

    ListBoxFormat->Append(_("LOR. *.lms or *.las"));
    ListBoxFormat->Append(_("Lcb, LOR Clipboard *.lcb"));
    ListBoxFormat->Append(_("Lcb, LOR S5 Clipboard *.lcb"));
    ListBoxFormat->Append(_("Vixen, Vixen sequence file *.vix"));
    ListBoxFormat->Append(_("Vir, Vixen Routine file. *.vir"));
    ListBoxFormat->Append(_("LSP, Light Show Pro "));
    ListBoxFormat->Append(_("HLS, Hinkle Lighte Sequencer *.hlsnc"));
    ListBoxFormat->Append(_("xLights/FPP, *.fseq"));
    ListBoxFormat->Append(_("Compressed Video, *.mp4"));
    ListBoxFormat->Append(_("High Quality Compressed Video, *.mp4"));
    ListBoxFormat->Append(_("HD ProRes Video, *.mov"));
#ifndef __APPLE__
    ListBoxFormat->Append(_("Uncompressed Video, *.avi"));
#endif
    ListBoxFormat->Append(_("ProRes 4444 Video, *.mov"));
    ListBoxFormat->Append(_("Lossless RGB Video, *.mov"));
    ListBoxFormat->Append(_("Minleon Network Effects Controller, *.bin"));
    ListBoxFormat->Append(_("GIF Image, *.gif"));
    ListBoxFormat->SetSelection(0);

    Fit();

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
        ListBoxFormat->Delete(ListBoxFormat->FindString(_("Compressed Video, *.mp4")));
        ListBoxFormat->Delete(ListBoxFormat->FindString(_("High Quality Compressed Video, *.mp4")));
        ListBoxFormat->Delete(ListBoxFormat->FindString(_("HD ProRes Video, *.mov")));
        int idx = ListBoxFormat->FindString(_("Uncompressed Video, *.avi"));
        if (idx != -1) {
            ListBoxFormat->Delete(idx);
        }
        ListBoxFormat->Delete(ListBoxFormat->FindString(_("Lossless RGB Video, *.mov")));
        ListBoxFormat->Delete(ListBoxFormat->FindString(_("ProRes 4444 Video, *.mov")));
        ListBoxFormat->Delete(ListBoxFormat->FindString(_("Minleon Network Effects Controller, *.bin")));
    }
    ListBoxFormat->Delete(ListBoxFormat->FindString(_("LOR. *.lms or *.las")));
    ListBoxFormat->Delete(ListBoxFormat->FindString(_("Vixen, Vixen sequence file *.vix")));
    ListBoxFormat->Delete(ListBoxFormat->FindString(_("xLights/FPP, *.fseq")));
    ListBoxFormat->Append(_("FPP Sub sequence. *.eseq"));
    ListBoxFormat->Append(_("FPP Compressed Sub sequence. *.eseq"));

    wxString let;
    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        config->Read("xLightsLastExportType", &let, "");
        if (let == "") {
            ListBoxFormat->SetSelection(0);
        } else {
            if (!ListBoxFormat->SetStringSelection(let)) {
                ListBoxFormat->SetSelection(0);
            }
        }
    } else {
        ListBoxFormat->SetSelection(0);
    }

    // Grow the listbox so every format is visible without scrolling. The item
    // count isn't known until here (ModelExportTypes adds/removes entries based
    // on the export context), so size it now and re-fit the dialog.
    int rowHeight = ListBoxFormat->GetCharHeight() + 6;
    ListBoxFormat->SetMinSize(wxSize(-1, (int)ListBoxFormat->GetCount() * rowHeight + 8));
    GetSizer()->Fit(this);
    Layout();
    CenterOnParent();

    ValidateWindow();
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

wxString SeqExportDialog::GetExportFormat() const
{
    return ListBoxFormat->GetStringSelection();
}

// Build the default output path (show/fseq dir + model name + format extension)
// used to seed the native Save As dialog.
wxString SeqExportDialog::GetDefaultName() const
{
    wxString fmt = ListBoxFormat->GetStringSelection();
    wxString cwd = xLightsFrame::CurrentDir;
    wxString fsd = ((xLightsFrame*)GetParent())->GetFseqDirectory();
    if (fsd == "") {
        fsd = cwd;
    }
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
    } else if (fmt == "High Quality Compressed Video, *.mp4") {
        name.SetExt("mp4");
    } else if (fmt == "HD ProRes Video, *.mov") {
        name.SetExt("mov");
    } else if (fmt == "Uncompressed Video, *.avi") {
        name.SetExt("avi");
    } else if (fmt == "Lossless RGB Video, *.mov") {
        name.SetExt("mov");
    } else if (fmt == "ProRes 4444 Video, *.mov") {
        name.SetExt("mov");
    } else if (fmt == "Minleon Network Effects Controller, *.bin") {
        name.SetExt("bin");
    } else if (fmt == "GIF Image, *.gif") {
        name.SetExt("gif");
    }
    return name.GetFullPath();
}

wxString SeqExportDialog::GetWildcardForFormat() const
{
    wxString fmt = ListBoxFormat->GetStringSelection();

    if (fmt == "LOR. *.lms or *.las") {
        return "LOR (*.lms;*.las)|*.lms;*.las";
    } else if (fmt == "Lcb, LOR Clipboard *.lcb" || fmt == "Lcb, LOR S5 Clipboard *.lcb") {
        return "LOR Clipboard (*.lcb)|*.lcb";
    } else if (fmt == "Vixen, Vixen sequence file *.vix") {
        return "Vixen Sequence File (*.vix)|*.vix";
    } else if (fmt == "Vir, Vixen Routine file. *.vir") {
        return "Vixen Routine File (*.vir)|*.vir";
    } else if (fmt == "LSP, Light Show Pro ") {
        return "Light Show Pro (*.*)|*.*";
    } else if (fmt == "HLS, Hinkle Lighte Sequencer *.hlsnc") {
        return "Hinkle Light Sequencer (*.hlsnc)|*.hlsnc";
    } else if (fmt == "FPP Sub sequence. *.eseq") {
        return "FPP Sub Sequence (*.eseq)|*.eseq";
    } else if (fmt == "FPP Compressed Sub sequence. *.eseq") {
        return "FPP Compresses Sub Sequence (*.eseq)|*.eseq";
    } else if (fmt == "xLights/FPP, *.fseq") {
        return "xLights/FPP (*.fseq)|*.fseq";
    } else if (fmt == "Compressed Video, *.mp4") {
        return "Video (*.mp4)|*.mp4";
    } else if (fmt == "HD ProRes Video, *.mov") {
        return "HD ProRes Video (*.mov)|*.mov";
    } else if (fmt == "Uncompressed Video, *.avi") {
        return "Video (*.avi)|*.avi";
    } else if (fmt == "High Quality Compressed Video, *.mp4") {
        return "Video (*.mp4)|*.mp4";
    } else if (fmt == "Lossless RGB Video, *.mov") {
        return "Lossless RGB Video (*.mov)|*.mov";
    } else if (fmt == "ProRes 4444 Video, *.mov") {
        return "ProRes 4444 Video (*.mov)|*.mov";
    } else if (fmt == "Minleon Network Effects Controller, *.bin") {
        return "Minleon Networks Effects Controller (*.bin)|*.bin";
    } else if (fmt == "GIF Image, *.gif") {
        return "GIF Image (*.gif)|*.gif";
    }
    return wxEmptyString;
}

// Open the native Save As dialog seeded with the default name. On macOS this is
// the only way to obtain a writable security scope for a user-chosen location;
// a path the user merely typed has no powerbox grant and ObtainAccessToURL()
// can't create a bookmark for a not-yet-existing file.
bool SeqExportDialog::PromptForFilename()
{
    wxFileName fn(GetDefaultName());
    wxString result = wxFileSelector(_("Choose output file"), fn.GetPath(), fn.GetFullName(), wxEmptyString, GetWildcardForFormat(), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this);
    if (result.IsEmpty()) {
        return false;
    }
    _filename = result.ToStdString();
    return true;
}

void SeqExportDialog::OnButtonOkClick(wxCommandEvent& event)
{
    if (ListBoxFormat->GetSelection() == wxNOT_FOUND) {
        return;
    }

    // Warn if the user picked the deprecated uncompressed AVI format. Modern
    // macOS AVFoundation has dropped its AVI decoder, so the resulting file
    // can't be read back on a Mac. The new "Lossless RGB Video, *.mov" option
    // gives the same bit-exact RGB output and decodes natively on all platforms.
    if (ListBoxFormat->GetStringSelection() == "Uncompressed Video, *.avi") {
        wxString msg =
            "You're about to export to uncompressed AVI. This format is being "
            "deprecated and will not render on upcoming versions of xLights.\n\n"
            "Consider using \"Lossless RGB Video, *.mov\" instead — same bit-exact "
            "RGB content in a format supported across all platforms.\n\n"
            "Continue with AVI?";
        int answer = wxMessageBox(msg, "AVI Export Deprecated",
                                  wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this);
        if (answer != wxYES) {
            // Leave the dialog open so the user can pick a different format.
            return;
        }
    }

    if (!PromptForFilename()) {
        // User cancelled the Save As dialog; keep the export dialog open.
        return;
    }

    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        config->Write("xLightsLastExportType", ListBoxFormat->GetStringSelection());
        config->Flush();
    }
    EndDialog(wxID_OK);
}

void SeqExportDialog::OnListBoxFormatDClick(wxCommandEvent& event)
{
    // Double-clicking a format is the same as picking it and clicking Next.
    OnButtonOkClick(event);
}

void SeqExportDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void SeqExportDialog::OnListBoxFormatSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void SeqExportDialog::ValidateWindow()
{
    if (ListBoxFormat->GetSelection() != wxNOT_FOUND) {
        ButtonOk->Enable();
    } else {
        ButtonOk->Disable();
    }
}
