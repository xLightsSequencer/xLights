#include "SeqExportDialog.h"

//(*InternalHeaders(SeqExportDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(SeqExportDialog)
const long SeqExportDialog::ID_STATICTEXT1 = wxNewId();
const long SeqExportDialog::ID_CHOICE1 = wxNewId();
const long SeqExportDialog::ID_STATICTEXT3 = wxNewId();
const long SeqExportDialog::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SeqExportDialog,wxDialog)
    //(*EventTable(SeqExportDialog)
    //*)
END_EVENT_TABLE()

SeqExportDialog::SeqExportDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(SeqExportDialog)
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Export Sequence"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Format"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceFormat = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    ChoiceFormat->SetSelection( ChoiceFormat->Append(_("LOR. *.lms or *.las")) );
    ChoiceFormat->Append(_("Lcb, LOR Clipboard *.lcb"));
    ChoiceFormat->Append(_("Vixen, Vixen sequence file *.vix"));
    ChoiceFormat->Append(_("Vir, Vixen Routine file. *.vir"));
    ChoiceFormat->Append(_("LSP, Light Show Pro "));
    ChoiceFormat->Append(_("HLS, Hinkle Lighte Sequencer *.hlsnc"));
    ChoiceFormat->Append(_("xLights, *.xseq"));
    ChoiceFormat->Append(_("Falcon, *.fseq"));
    FlexGridSizer2->Add(ChoiceFormat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlFilename = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer2->Add(TextCtrlFilename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqExportDialog::OnChoiceFormatSelect);
    //*)

}

SeqExportDialog::~SeqExportDialog()
{
    //(*Destroy(SeqExportDialog)
    //*)
}

void SeqExportDialog::ModelExportTypes()
{
    ChoiceFormat->Delete(ChoiceFormat->FindString(_("LOR. *.lms or *.las")));
    ChoiceFormat->Delete(ChoiceFormat->FindString(_("Vixen, Vixen sequence file *.vix")));
    ChoiceFormat->Delete(ChoiceFormat->FindString(_("xLights, *.xseq")));
    ChoiceFormat->Append(_("Falcon Pi Sub sequence. *.eseq"));
    ChoiceFormat->SetSelection(0);
}

void SeqExportDialog::OnChoiceFormatSelect(wxCommandEvent& event)
{
}
