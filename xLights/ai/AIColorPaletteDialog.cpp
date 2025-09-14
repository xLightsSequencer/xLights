#include "AIColorPaletteDialog.h"

//(*InternalHeaders(AIColorPaletteDialog)
#include <wx/string.h>
//*)

//(*IdInit(AIColorPaletteDialog)
const wxWindowID AIColorPaletteDialog::ID_RADIOBUTTON1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_TEXTCTRL1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_RADIOBUTTON2 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_TEXTCTRL2 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_HTMLWINDOW1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_BUTTON1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_OK = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(AIColorPaletteDialog,wxDialog)
    //(*EventTable(AIColorPaletteDialog)
    //*)
END_EVENT_TABLE()

#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../sequencer/MainSequencer.h"
#include "../ai/aiBase.h"

AIColorPaletteDialog::AIColorPaletteDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(AIColorPaletteDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticBoxSizer* StaticBoxSizer2;

    Create(parent, id, _T("Generate Color Palette"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Parameters"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    SongRadioButton = new wxRadioButton(this, ID_RADIOBUTTON1, _T("Song"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    SongRadioButton->SetValue(true);
    FlexGridSizer2->Add(SongRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SongTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(600,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    SongTextCtrl->SetMaxLength(250);
    FlexGridSizer2->Add(SongTextCtrl, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FreeFormRadioButton = new wxRadioButton(this, ID_RADIOBUTTON2, _T("Free Form"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    FlexGridSizer2->Add(FreeFormRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FreeFormText = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FreeFormText->Disable();
    FlexGridSizer2->Add(FreeFormText, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Results"));
    ResultHTMLCtrl = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(-1,300), wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
    StaticBoxSizer2->Add(ResultHTMLCtrl, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    GenerateButton = new wxButton(this, ID_BUTTON1, _T("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    GenerateButton->SetDefault();
    FlexGridSizer3->Add(GenerateButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OkButon = new wxButton(this, ID_OK, _T("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_OK"));
    FlexGridSizer3->Add(OkButon, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CancelButton = new wxButton(this, ID_CANCEL, _T("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CANCEL"));
    FlexGridSizer3->Add(CancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_RADIOBUTTON1, wxEVT_COMMAND_RADIOBUTTON_SELECTED, (wxObjectEventFunction)&AIColorPaletteDialog::OnSongRadioButtonSelect);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&AIColorPaletteDialog::OnSongTextCtrlText);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_ENTER, (wxObjectEventFunction)&AIColorPaletteDialog::OnSongTextCtrlTextEnter);
    Connect(ID_RADIOBUTTON2, wxEVT_COMMAND_RADIOBUTTON_SELECTED, (wxObjectEventFunction)&AIColorPaletteDialog::OnFreeFormRadioButtonSelect);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIColorPaletteDialog::OnGenerateButtonClick);
    Connect(ID_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIColorPaletteDialog::OnOkButonClick);
    Connect(ID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIColorPaletteDialog::OnCancelButtonClick);
    //*)

    if (xLightsFrame::CurrentSeqXmlFile->GetMedia()) {
        auto title = xLightsFrame::CurrentSeqXmlFile->GetMedia()->Title();
        auto artist = xLightsFrame::CurrentSeqXmlFile->GetMedia()->Artist();
        if (!artist.empty()) {
            title = title + " by " + artist;
        }
        SongTextCtrl->SetValue(title);
        createFreeFormFromSong();
    }

}

AIColorPaletteDialog::~AIColorPaletteDialog()
{
    //(*Destroy(AIColorPaletteDialog)
    //*)
}
void AIColorPaletteDialog::createFreeFormFromSong() {
    auto song = SongTextCtrl->GetValue();
    if (!song.empty()) {
        song = "from the song " + song;
    }
    FreeFormText->SetValue(song);
}


void AIColorPaletteDialog::OnSongRadioButtonSelect(wxCommandEvent& event)
{
    SongTextCtrl->Enable();
    FreeFormText->Disable();
}

void AIColorPaletteDialog::OnSongTextCtrlTextEnter(wxCommandEvent& event)
{
    createFreeFormFromSong();
}

void AIColorPaletteDialog::OnFreeFormRadioButtonSelect(wxCommandEvent& event)
{
    SongTextCtrl->Disable();
    FreeFormText->Enable();
}

void AIColorPaletteDialog::OnGenerateButtonClick(wxCommandEvent& event)
{
    auto prompt = FreeFormText->GetValue();
    aiBase::AIColorPalette cp = xLightsApp::GetFrame()->GetAIService(aiType::COLORPALETTES)->GenerateColorPalette(prompt);
    colors.clear();
    std::string html;
    html += "<html><body>\n";
    if (cp.error.empty()) {
        html += "<b>Description:</b> " + cp.description + "<br>\n";
        if (!cp.song.empty()) {
            html += "<b>Song:</b> " + cp.song + "<br>\n";
        }
        if (!cp.artist.empty()) {
            html += "<b>Artist:</b> " + cp.artist + "<br>\n";
        }
        html += "<br><ul>\n";
        for (int x  = 0; x < cp.colors.size(); x++) {
            html += "<li>";
            html += "<span style='background-color: black; color: " + cp.colors[x].hexValue + "'><b>" + cp.colors[x].name + "</b></span> (" + cp.colors[x].hexValue + ")- " + cp.colors[x].description;
            html += "</li>\n";
            colors.push_back(cp.colors[x].hexValue);
        }
        
        html += "</ul>";
    } else {
        html += "<b>Error: </b> "  + cp.error;
    }
    html += "</body></html>";
    ResultHTMLCtrl->SetPage(html);
}

void AIColorPaletteDialog::OnOkButonClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void AIColorPaletteDialog::OnCancelButtonClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}


wxArrayString AIColorPaletteDialog::GetColorStrings() {
    return colors;
}

void AIColorPaletteDialog::OnSongTextCtrlText(wxCommandEvent& event)
{
    createFreeFormFromSong();
}
