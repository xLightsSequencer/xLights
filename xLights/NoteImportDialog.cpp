#include "NoteImportDialog.h"

//(*InternalHeaders(NoteImportDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/progdlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/textfile.h>

#include "MIDI/MidiFile.h"
#include "sequencer/SequenceElements.h"
#include "MusicXML.h"
#include "UtilFunctions.h"

//(*IdInit(NoteImportDialog)
const long NoteImportDialog::ID_STATICTEXT1 = wxNewId();
const long NoteImportDialog::ID_TEXTCTRL1 = wxNewId();
const long NoteImportDialog::ID_STATICTEXT3 = wxNewId();
const long NoteImportDialog::ID_CHOICE_Piano_Notes_Source = wxNewId();
const long NoteImportDialog::ID_STATICTEXT4 = wxNewId();
const long NoteImportDialog::ID_TEXTCTRL_Piano_File = wxNewId();
const long NoteImportDialog::ID_BUTTON_Piano_File = wxNewId();
const long NoteImportDialog::ID_STATICTEXT8 = wxNewId();
const long NoteImportDialog::ID_CHOICE_Piano_MIDITrack_APPLYLAST = wxNewId();
const long NoteImportDialog::ID_STATICTEXT5 = wxNewId();
const long NoteImportDialog::ID_SLIDER_Piano_MIDI_Start = wxNewId();
const long NoteImportDialog::IDD_TEXTCTRL_Piano_MIDI_Start = wxNewId();
const long NoteImportDialog::ID_STATICTEXT6 = wxNewId();
const long NoteImportDialog::ID_SLIDER_Piano_MIDI_Speed = wxNewId();
const long NoteImportDialog::IDD_TEXTCTRL_Piano_MIDI_Speed = wxNewId();
const long NoteImportDialog::ID_BUTTON1 = wxNewId();
const long NoteImportDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(NoteImportDialog,wxDialog)
	//(*EventTable(NoteImportDialog)
	//*)
END_EVENT_TABLE()

NoteImportDialog::NoteImportDialog(wxWindow* parent, SequenceElements& sequenceElements, bool mediaPresent, wxWindowID id,const wxPoint& pos,const wxSize& size) :
    _sequenceElements(sequenceElements)
{
	//(*Initialize(NoteImportDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Timing Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_TimingName = new wxTextCtrl(this, ID_TEXTCTRL1, _("Notes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_TimingName, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Notes Source"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Piano_Notes_Source = new wxChoice(this, ID_CHOICE_Piano_Notes_Source, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_Notes_Source"));
	Choice_Piano_Notes_Source->SetSelection( Choice_Piano_Notes_Source->Append(_("Audacity Timing File")) );
	Choice_Piano_Notes_Source->Append(_("Music XML File"));
	Choice_Piano_Notes_Source->Append(_("MIDI File"));
	Choice_Piano_Notes_Source->Append(_("Polyphonic Transcription"));
	FlexGridSizer2->Add(Choice_Piano_Notes_Source, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("File"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Piano_File = new wxTextCtrl(this, ID_TEXTCTRL_Piano_File, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_File"));
	TextCtrl_Piano_File->SetToolTip(_("The file should be a Polyphonic Transcription file consisting of 3 floating point numbers per line. #1 is the start time in seconds. #2 the end time. #3 the MIDI channel number."));
	FlexGridSizer3->Add(TextCtrl_Piano_File, 1, wxALL|wxEXPAND, 5);
	Button_Piano_File = new wxButton(this, ID_BUTTON_Piano_File, _("..."), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_Piano_File"));
	FlexGridSizer3->Add(Button_Piano_File, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer5->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Piano_MIDITrack_APPLYLAST = new wxChoice(this, ID_CHOICE_Piano_MIDITrack_APPLYLAST, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_MIDITrack_APPLYLAST"));
	FlexGridSizer5->Add(Choice_Piano_MIDITrack_APPLYLAST, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Start Time Adjust (s)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer5->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Piano_MIDI_Start = new wxSlider(this, ID_SLIDER_Piano_MIDI_Start, 0, -1000, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_MIDI_Start"));
	FlexGridSizer5->Add(Slider_Piano_MIDI_Start, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_MIDI_Start = new wxTextCtrl(this, IDD_TEXTCTRL_Piano_MIDI_Start, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Piano_MIDI_Start"));
	FlexGridSizer5->Add(TextCtrl_Piano_MIDI_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Speed Adjust %"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer5->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Piano_MIDI_Speed = new wxSlider(this, ID_SLIDER_Piano_MIDI_Speed, 100, 1, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_MIDI_Speed"));
	FlexGridSizer5->Add(Slider_Piano_MIDI_Speed, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_MIDI_Speed = new wxTextCtrl(this, IDD_TEXTCTRL_Piano_MIDI_Speed, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Piano_MIDI_Speed"));
	FlexGridSizer5->Add(TextCtrl_Piano_MIDI_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonOk = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer1->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FileDialog1 = new wxFileDialog(this, _("Select Polyphonic Transcription file"), wxEmptyString, wxEmptyString, _("Text Files|*.txt"), wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NoteImportDialog::OnTextCtrl_TimingNameText);
	Connect(ID_CHOICE_Piano_Notes_Source,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&NoteImportDialog::OnChoice_Piano_Notes_SourceSelect);
	Connect(ID_TEXTCTRL_Piano_File,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NoteImportDialog::OnTextCtrl_Piano_FileText);
	Connect(ID_BUTTON_Piano_File,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NoteImportDialog::OnButton_Piano_FileClick);
	Connect(ID_SLIDER_Piano_MIDI_Start,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&NoteImportDialog::OnSlider_Piano_MIDI_StartCmdSliderUpdated);
	Connect(IDD_TEXTCTRL_Piano_MIDI_Start,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NoteImportDialog::OnTextCtrl_Piano_MIDI_StartText);
	Connect(ID_SLIDER_Piano_MIDI_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&NoteImportDialog::OnSlider_Piano_MIDI_SpeedCmdSliderUpdated);
	Connect(IDD_TEXTCTRL_Piano_MIDI_Speed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NoteImportDialog::OnTextCtrl_Piano_MIDI_SpeedText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NoteImportDialog::OnButtonOkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NoteImportDialog::OnButtonCancelClick);
	//*)

    if (!mediaPresent)
    {
        int p = Choice_Piano_Notes_Source->FindString("Polyphonic Transcription");
        Choice_Piano_Notes_Source->Delete(p);
    }

    ValidateWindow();

    SetEscapeId(ButtonCancel->GetId());

    // this is slow so we do it outside of the commonly called ValidateWindow
    MIDIExtraValidateWindow();
    AudacityExtraValidateWindow();
    MusicXMLExtraValidateWindow();
}

NoteImportDialog::~NoteImportDialog()
{
	//(*Destroy(NoteImportDialog)
	//*)
}


void NoteImportDialog::OnChoice_Piano_Notes_SourceSelect(wxCommandEvent& event)
{
    wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
    if (notes == "Polyphonic Transcription")
    {
        DisplayWarning("Polyphonic transcription is known to be unstable and can cause xLights to crash.", this);
    }

    ValidateWindow();

    // this is slow so we do it outside of the commonly called ValidateWindow
    MIDIExtraValidateWindow();
    AudacityExtraValidateWindow();
    MusicXMLExtraValidateWindow();
}

std::vector<float> NoteImportDialog::Parse(wxString& l)
{
    std::vector<float> res;
    wxString s = l;
    while (s.Len() != 0)
    {
        int end = s.First('\t');
        if (end > 0)
        {
            res.push_back(wxAtof(s.SubString(0, end - 1)));
            s = s.Right(s.Len() - end - 1);
        }
        else
        {
            res.push_back(wxAtof(s));
            s = "";
        }
    }

    return res;
}

void NoteImportDialog::AudacityExtraValidateWindow()
{
    wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
    if (notes == "Audacity Timing File")
    {
        wxString file = TextCtrl_Piano_File->GetValue();
        wxTextFile f(file);

        if (file != "" && wxFile::Exists(file))
        {
            if (f.Open())
            {
                wxString l = f.GetNextLine();
                std::vector<float> components = Parse(l);
                if (components.size() != 3)
                {
                    TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
                    ButtonOk->Disable();
                }
            }
        }
        else
        {
            TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
            ButtonOk->Disable();
        }
    }
}

bool NoteImportDialog::IsTimingUnique(wxString name) const
{
    int num_elements = _sequenceElements.GetElementCount();
    for (int i = 0; i < num_elements; ++i)
    {
        Element* element = _sequenceElements.GetElement(i);
        if (element->GetType() == ELEMENT_TYPE_TIMING)
        {
            if (element->GetName() == name)
            {
                return false;
            }
        }
    }

    return true;
}

bool NoteImportDialog::MIDITrackContainsNotes(int track, MidiFile* midifile) const
{
    for (int i = 0; i < midifile->getNumEvents(track); i++)
    {
        MidiEvent e = midifile->getEvent(track, i);
        if (e.isNote())
        {
            return true;
        }
    }

    return false;
}

void NoteImportDialog::MIDIExtraValidateWindow()
{
    wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
    if (notes == "MIDI File")
    {
        wxString file = TextCtrl_Piano_File->GetValue();
        MidiFile midifile;
        if (file == "" || !wxFile::Exists(file) || midifile.read(file) == 0)
        {
            // invalid midi file
            TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
            Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
            ButtonOk->Disable();
        }
        else
        {
            std::string selection = std::string(Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection().c_str());
            int tracks = midifile.getNumTracks();
            Choice_Piano_MIDITrack_APPLYLAST->Clear();
            Choice_Piano_MIDITrack_APPLYLAST->Append("All");
            if (selection == "All")
            {
                Choice_Piano_MIDITrack_APPLYLAST->Select(0);
            }
            for (int i = 1; i <= tracks; i++)
            {
                if (MIDITrackContainsNotes(i - 1, &midifile))
                {
                    std::string n = std::string(wxString::Format("%i", i).c_str());
                    Choice_Piano_MIDITrack_APPLYLAST->Append(n);
                    if (selection == n)
                    {
                        Choice_Piano_MIDITrack_APPLYLAST->Select(i);
                    }
                }
            }
            if (Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection() == "")
            {
                Choice_Piano_MIDITrack_APPLYLAST->Select(0);
            }
            Choice_Piano_MIDITrack_APPLYLAST->Enable(true);
        }
    }
}

void NoteImportDialog::MusicXMLExtraValidateWindow()
{
    wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
    if (notes == "Music XML File")
    {
        wxString file = TextCtrl_Piano_File->GetValue();

        MusicXML musicxml(file.ToStdString());

        if (file == "" || !wxFile::Exists(file) || !musicxml.IsOk())
        {
            // invalid music XML file
            TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
            Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
            ButtonOk->Disable();
        }
        else
        {
            std::string selection = std::string(Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection().c_str());

            Choice_Piano_MIDITrack_APPLYLAST->Clear();
            Choice_Piano_MIDITrack_APPLYLAST->Append("All");
            if (selection == "All")
            {
                Choice_Piano_MIDITrack_APPLYLAST->Select(0);
            }

            std::list<std::string> tracks = musicxml.GetTracks();

            for (auto it =  tracks.begin(); it != tracks.end(); ++it)
            {
                Choice_Piano_MIDITrack_APPLYLAST->Append(*it);
                if (selection == *it)
                {
                    Choice_Piano_MIDITrack_APPLYLAST->SetStringSelection(*it);
                }
            }

            if (Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection() == "")
            {
                Choice_Piano_MIDITrack_APPLYLAST->Select(0);
            }
            Choice_Piano_MIDITrack_APPLYLAST->Enable(true);
        }
    }
}

void NoteImportDialog::ValidateWindow()
{
    wxString notes = Choice_Piano_Notes_Source->GetStringSelection();

    ButtonOk->Enable();

    if (IsTimingUnique(TextCtrl_TimingName->GetValue()))
    {
        TextCtrl_TimingName->SetBackgroundColour(*wxWHITE);
    }
    else
    {
        TextCtrl_TimingName->SetBackgroundColour(*wxRED);
        ButtonOk->Disable();
    }

    if (notes == "Polyphonic Transcription")
    {
        TextCtrl_Piano_File->Enable(false);
        Button_Piano_File->Enable(false);
        Slider_Piano_MIDI_Speed->Enable(false);
        Slider_Piano_MIDI_Start->Enable(false);
        TextCtrl_Piano_MIDI_Speed->Enable(false);
        TextCtrl_Piano_MIDI_Start->Enable(false);
        Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
        TextCtrl_Piano_File->SetToolTip(_(""));
    }
    else if (notes == "Audacity Timing File")
    {
        TextCtrl_Piano_File->Enable(true);
        Button_Piano_File->Enable(true);
        Slider_Piano_MIDI_Speed->Enable(false);
        Slider_Piano_MIDI_Start->Enable(false);
        TextCtrl_Piano_MIDI_Speed->Enable(false);
        TextCtrl_Piano_MIDI_Start->Enable(false);
        Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
        TextCtrl_Piano_File->SetToolTip(_("The file should be a Polyphonic Transcription file consisting of 3 floating point numbers per line. #1 is the start time in seconds. #2 the end time. #3 the MIDI channel number."));
    }
    else if (notes == "MIDI File")
    {
        TextCtrl_Piano_File->Enable(true);
        Button_Piano_File->Enable(true);
        Slider_Piano_MIDI_Speed->Enable(true);
        Slider_Piano_MIDI_Start->Enable(true);
        TextCtrl_Piano_MIDI_Speed->Enable(true);
        TextCtrl_Piano_MIDI_Start->Enable(true);
        TextCtrl_Piano_File->SetToolTip(_("The file should be a MIDI file ... typically with a .MID extension."));
    }
    else if (notes == "Music XML File")
    {
        TextCtrl_Piano_File->Enable(true);
        Button_Piano_File->Enable(true);
        Slider_Piano_MIDI_Speed->Enable(true);
        Slider_Piano_MIDI_Start->Enable(true);
        TextCtrl_Piano_MIDI_Speed->Enable(true);
        TextCtrl_Piano_MIDI_Start->Enable(true);
        TextCtrl_Piano_File->SetToolTip(_("The file should be a Music XML file ... typically with a .MXL extension"));
    }

    wxString file = TextCtrl_Piano_File->GetValue();
    if (file == "" || wxFile::Exists(file))
    {
        TextCtrl_Piano_File->SetBackgroundColour(*wxWHITE);
    }
    else
    {
        TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
        if (TextCtrl_Piano_File->IsEnabled())
        {
            ButtonOk->Disable();
        }
    }
}

void NoteImportDialog::OnButton_Piano_FileClick(wxCommandEvent& event)
{
    wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
    if (notes == "Audacity Timing File")
    {
        FileDialog1->SetWildcard("Text Files|*.txt|All Files|*.*");
    }
    else if (notes == "MIDI File")
    {
        FileDialog1->SetWildcard("MIDI Files|*.mid;*.midi|All Files|*.*");
    }
    else if (notes == "Music XML File")
    {
        FileDialog1->SetWildcard("Music XML Files|*.mxl;*.mxl;*.musicxml|All Files|*.*");
    }
    if (FileDialog1->ShowModal() == wxID_OK)
    {
        TextCtrl_Piano_File->SetValue(FileDialog1->GetDirectory() + "/" + FileDialog1->GetFilename());
        ValidateWindow();
        // this is slow so we do it outside of the commonly called ValidateWindow
        MIDIExtraValidateWindow();
        AudacityExtraValidateWindow();
        MusicXMLExtraValidateWindow();
    }
}

void NoteImportDialog::OnTextCtrl_Piano_FileText(wxCommandEvent& event)
{
    ValidateWindow();
    // this is slow so we do it outside of the commonly called ValidateWindow
    MIDIExtraValidateWindow();
    AudacityExtraValidateWindow();
    MusicXMLExtraValidateWindow();
}

void NoteImportDialog::OnButtonOkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void NoteImportDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void NoteImportDialog::OnTextCtrl_TimingNameText(wxCommandEvent& event)
{
    ValidateWindow();
    // this is slow so we do it outside of the commonly called ValidateWindow
    MIDIExtraValidateWindow();
    AudacityExtraValidateWindow();
    MusicXMLExtraValidateWindow();
}

void NoteImportDialog::OnSlider_Piano_MIDI_SpeedCmdSliderUpdated(wxScrollEvent& event)
{
    int value = Slider_Piano_MIDI_Speed->GetValue();
    wxString val = wxString::Format("%d", value);
    TextCtrl_Piano_MIDI_Speed->ChangeValue(val);
}

void NoteImportDialog::OnTextCtrl_Piano_MIDI_SpeedText(wxCommandEvent& event)
{
    int value = wxAtoi(TextCtrl_Piano_MIDI_Speed->GetValue());

    if (value > 200)
    {
        value = 200;
        wxString val_str;
        val_str << value;
        TextCtrl_Piano_MIDI_Speed->ChangeValue(val_str);
    }
    else if (value < 1)
    {
        value = 1;
        wxString val_str;
        val_str << value;
        TextCtrl_Piano_MIDI_Speed->ChangeValue(val_str);
    }
    Slider_Piano_MIDI_Speed->SetValue(value);
}

void NoteImportDialog::OnSlider_Piano_MIDI_StartCmdSliderUpdated(wxScrollEvent& event)
{
    double value = Slider_Piano_MIDI_Start->GetValue();
    value /= 100;

    wxString val = wxString::Format("%.2f", value);
    TextCtrl_Piano_MIDI_Start->ChangeValue(val);
}

void NoteImportDialog::OnTextCtrl_Piano_MIDI_StartText(wxCommandEvent& event)
{
    // 0-2000 - -10.00->10.00 seconds

    double value = wxAtof(TextCtrl_Piano_MIDI_Start->GetValue());

    if (value > 10.00)
    {
        value = 10.00;
        wxString val_str;
        val_str << value;
        TextCtrl_Piano_MIDI_Start->ChangeValue(val_str);
    }
    else if (value < -10.00)
    {
        value = -10.00;
        wxString val_str;
        val_str << value;
        TextCtrl_Piano_MIDI_Start->ChangeValue(val_str);
    }

    value *= 100.0;
    Slider_Piano_MIDI_Start->SetValue(value);
}
