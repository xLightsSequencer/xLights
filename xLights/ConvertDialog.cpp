#include "ConvertDialog.h"

// xml
#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../include/spxml-0.5/spxmlparser.cpp"
#include "../include/spxml-0.5/spxmlevent.cpp"
#include "../include/spxml-0.5/spxmlcodec.cpp"
#include "../include/spxml-0.5/spxmlreader.cpp"
#include "../include/spxml-0.5/spxmlutils.cpp"
#include "../include/spxml-0.5/spxmlstag.cpp"

#include <map>
#include <wx/base64.h>

//(*InternalHeaders(ConvertDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "FileConverter.h"
#include "xLightsMain.h"

#include <log4cpp/Category.hh>

//(*IdInit(ConvertDialog)
const long ConvertDialog::ID_STATICTEXT2 = wxNewId();
const long ConvertDialog::ID_STATICTEXT3 = wxNewId();
const long ConvertDialog::ID_STATICTEXT6 = wxNewId();
const long ConvertDialog::ID_BUTTON_CHOOSE_FILE = wxNewId();
const long ConvertDialog::ID_TEXTCTRL_FILENAME = wxNewId();
const long ConvertDialog::ID_STATICTEXT7 = wxNewId();
const long ConvertDialog::ID_CHOICE_OUTPUT_FORMAT = wxNewId();
const long ConvertDialog::ID_STATICTEXT8 = wxNewId();
const long ConvertDialog::ID_CHECKBOX_OFF_AT_END = wxNewId();
const long ConvertDialog::ID_STATICTEXT9 = wxNewId();
const long ConvertDialog::ID_CHECKBOX_MAP_EMPTY_CHANNELS = wxNewId();
const long ConvertDialog::ID_STATICTEXT10 = wxNewId();
const long ConvertDialog::ID_CHECKBOX_LOR_WITH_NO_CHANNELS = wxNewId();
const long ConvertDialog::ID_STATICTEXT11 = wxNewId();
const long ConvertDialog::ID_CHECKBOX_ShowChannelMapping = wxNewId();
const long ConvertDialog::ID_STATICTEXT12 = wxNewId();
const long ConvertDialog::ID_CHOICE1 = wxNewId();
const long ConvertDialog::ID_BUTTON_START_CONVERSION = wxNewId();
const long ConvertDialog::ID_STATICTEXT4 = wxNewId();
const long ConvertDialog::ID_TEXTCTRL_CONVERSION_STATUS = wxNewId();
const long ConvertDialog::ID_STATICTEXT_STATUS = wxNewId();
const long ConvertDialog::ID_BUTTON_CLOSE = wxNewId();
//*)

BEGIN_EVENT_TABLE(ConvertDialog,wxDialog)
	//(*EventTable(ConvertDialog)
	//*)
END_EVENT_TABLE()

ConvertDialog::ConvertDialog(wxWindow* parent, SeqDataType& SeqData_, NetInfoClass& NetInfo_, wxString& mediaFilename_, wxArrayString& ChannelNames_, wxArrayInt& ChannelColors_, wxArrayString& ChNames_, wxWindowID id,const wxPoint& pos,const wxSize& size) : SeqData(SeqData_), NetInfo(NetInfo_), mediaFilename(mediaFilename_), ChannelColors(ChannelColors_), ChannelNames(ChannelNames_), ChNames(ChNames_)
{
    _parent = (xLightsFrame*)parent;

	//(*Initialize(ConvertDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizerConvert;

	Create(parent, id, _("xLights File Conversion"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizerConvert = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizerConvert->AddGrowableCol(0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Entries on the Setup tab will guide the conversion. Make sure you have your lighting networks defined accurately before you start a conversion."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizerConvert->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("For LMS imports, LOR networks should be first followed by DMX starting at universe 1."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizerConvert->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT6, _("Sequence files to convert:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ButtonChooseFile = new wxButton(this, ID_BUTTON_CHOOSE_FILE, _("Choose Files"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CHOOSE_FILE"));
	FlexGridSizer2->Add(ButtonChooseFile, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrlFilename = new wxTextCtrl(this, ID_TEXTCTRL_FILENAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_FILENAME"));
	FlexGridSizer2->Add(TextCtrlFilename, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT7, _("Output Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer4->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ChoiceOutputFormat = new wxChoice(this, ID_CHOICE_OUTPUT_FORMAT, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_OUTPUT_FORMAT"));
	ChoiceOutputFormat->SetSelection( ChoiceOutputFormat->Append(_("Falcon Pi Player, *.fseq")) );
	ChoiceOutputFormat->Append(_("xLights Sequence, *.xseq"));
	ChoiceOutputFormat->Append(_("Lynx Conductor, *.seq"));
	ChoiceOutputFormat->Append(_("Vix,Vixen 2.1 *.vix sequence file"));
	ChoiceOutputFormat->Append(_("Vir, Vixen 2.1 *.vir routine file"));
	ChoiceOutputFormat->Append(_("LOR Sequence *.las or *.lms"));
	ChoiceOutputFormat->Append(_("Glediator Record File *.gled"));
	ChoiceOutputFormat->Append(_("Lcb, LOR clipboard *.lcb"));
	ChoiceOutputFormat->Append(_("HLS *.hlsnc"));
	FlexGridSizer4->Add(ChoiceOutputFormat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT8, _("All channels off at end:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer4->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBoxOffAtEnd = new wxCheckBox(this, ID_CHECKBOX_OFF_AT_END, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OFF_AT_END"));
	CheckBoxOffAtEnd->SetValue(false);
	FlexGridSizer4->Add(CheckBoxOffAtEnd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("LOR Import Options"));
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT9, _("Map Empty LMS Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer6->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBoxMapEmptyChannels = new wxCheckBox(this, ID_CHECKBOX_MAP_EMPTY_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MAP_EMPTY_CHANNELS"));
	CheckBoxMapEmptyChannels->SetValue(false);
	FlexGridSizer6->Add(CheckBoxMapEmptyChannels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT10, _("Map LMS Channels with no network"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer6->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	MapLORChannelsWithNoNetwork = new wxCheckBox(this, ID_CHECKBOX_LOR_WITH_NO_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LOR_WITH_NO_CHANNELS"));
	MapLORChannelsWithNoNetwork->SetValue(true);
	FlexGridSizer6->Add(MapLORChannelsWithNoNetwork, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT11, _("Show Verbose Channel Map:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer6->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBoxShowChannelMapping = new wxCheckBox(this, ID_CHECKBOX_ShowChannelMapping, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ShowChannelMapping"));
	CheckBoxShowChannelMapping->SetValue(false);
	FlexGridSizer6->Add(CheckBoxShowChannelMapping, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT12, _("Time Resolution"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer6->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	LORImportTimeResolution = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	LORImportTimeResolution->Append(_("25 ms"));
	LORImportTimeResolution->SetSelection( LORImportTimeResolution->Append(_("50 ms")) );
	LORImportTimeResolution->Append(_("100 ms"));
	FlexGridSizer6->Add(LORImportTimeResolution, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticBoxSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonStartConversion = new wxButton(this, ID_BUTTON_START_CONVERSION, _("Start Conversion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_START_CONVERSION"));
	FlexGridSizer5->Add(ButtonStartConversion, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Conversion Messages:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrlConversionStatus = new wxTextCtrl(this, ID_TEXTCTRL_CONVERSION_STATUS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_CONVERSION_STATUS"));
	FlexGridSizer3->Add(TextCtrlConversionStatus, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	FlexGridSizerConvert->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	StaticTextStatus = new wxStaticText(this, ID_STATICTEXT_STATUS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_STATUS"));
	FlexGridSizer7->Add(StaticTextStatus, 1, wxALL|wxEXPAND, 2);
	ButtonClose = new wxButton(this, ID_BUTTON_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLOSE"));
	FlexGridSizer7->Add(ButtonClose, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizerConvert->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizerConvert);
	FileDialogConvert = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("xLights Sequences(*.xseq)|*.xseq|\n\n\t\t\tLOR Music Sequences (*.lms)|*.lms|\n\n\t\t\tLOR Animation Sequences (*.las)|*.las|\n\n\t\t\tVixen Sequences (*.vix)|*.vix|\n\n\t\t\tFalcon Pi Player Sequences (*.fseq)|*.fseq|\n\n\t\t\tGlediator Record File (*.gled)|*.gled)|\n\n\t\t\tLynx Conductor Sequences (*.seq)|*.seq|\n\n\t\t\tHLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata"), wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FlexGridSizerConvert->Fit(this);
	FlexGridSizerConvert->SetSizeHints(this);

	Connect(ID_BUTTON_CHOOSE_FILE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ConvertDialog::OnButtonChooseFileClick);
	Connect(ID_BUTTON_START_CONVERSION,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ConvertDialog::OnButtonStartConversionClick);
	Connect(ID_BUTTON_CLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ConvertDialog::OnButtonCloseClick);
	//*)

    wxConfigBase* config = wxConfigBase::Get();
    wxString dir;
    bool ok = true;
    ok = config->Read("LastDir", &dir);
    wxString ConvertDir;
    ConvertDir.clear();
    if (ok && !config->Read("ConvertDir", &ConvertDir))
    {
        ConvertDir = dir;
    }
    FileDialogConvert->SetDirectory(ConvertDir);
}

ConvertDialog::~ConvertDialog()
{
	//(*Destroy(ConvertDialog)
	//*)
}

void ConvertDialog::PlayerError(const wxString& msg)
{
    wxMessageBox(msg, _("Error"), wxOK | wxICON_EXCLAMATION);
}

void ConvertDialog::ConversionError(const wxString& msg)
{
    wxMessageBox(msg, wxString("Error"), wxOK | wxICON_EXCLAMATION);
}

void ConvertDialog::OnButtonChooseFileClick(wxCommandEvent& event)
{
    wxArrayString ShortNames;
    wxString AllNames;
    if (FileDialogConvert->ShowModal() == wxID_OK)
    {
        FileDialogConvert->GetPaths(FileNames);
        FileDialogConvert->GetFilenames(ShortNames);
        for (size_t i = 0; i < ShortNames.GetCount(); i++)
        {
            AllNames.Append(ShortNames[i]);
            AllNames.Append("\n");
        }
        TextCtrlFilename->ChangeValue(AllNames);
        wxString ConvertDir = FileDialogConvert->GetDirectory();
        wxConfigBase* config = wxConfigBase::Get();
        config->Write(wxString("ConvertDir"), ConvertDir);
        //delete config;
    }
}

void ConvertDialog::OnButtonStartConversionClick(wxCommandEvent& event)
{
    log4cpp::Category& logger = log4cpp::Category::getRoot();
    logger.info("Converstion starting.");

    ButtonStartConversion->Enable(false);
    wxString OutputFormat = ChoiceOutputFormat->GetStringSelection();
    TextCtrlConversionStatus->Clear();

    // check inputs
    if (FileNames.size() == 0)
    {
        wxMessageBox(wxString("Please select one or more sequence files"), wxString("Error"));
    }
    else if (OutputFormat.IsEmpty())
    {
        wxMessageBox(wxString("Please select an output format"), wxString("Error"));
    }
    else
    {
        for (size_t i = 0; i < FileNames.GetCount(); i++)
        {
            DoConversion(FileNames[i], OutputFormat);
        }
        AppendConvertStatus(wxString("Finished converting all files\n"));
    }

    ButtonStartConversion->Enable(true);

    logger.info("Conversion complete.");
}

void ConvertDialog::OnButtonCloseClick(wxCommandEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void ConvertDialog::AppendConvertStatus(const wxString &msg, bool flushBuffer) {
    if (flushBuffer && !msgBuffer.IsEmpty()) {
        msgBuffer.append(msg);
        TextCtrlConversionStatus->AppendText(msgBuffer);
        msgBuffer.Clear();
    }
    else if (flushBuffer) {
        TextCtrlConversionStatus->AppendText(msg);
    }
    else {
        msgBuffer.append(msg);
        if (msgBuffer.size() > 10000) {
            TextCtrlConversionStatus->AppendText(msgBuffer);
            msgBuffer.Clear();
        }
    }

    log4cpp::Category& logger = log4cpp::Category::getRoot();
    logger.info("ConvertStatus: " + msg);
}

bool ConvertDialog::mapEmptyChannels() {
    return CheckBoxMapEmptyChannels->IsChecked();
}
bool ConvertDialog::showChannelMapping() {
    return CheckBoxShowChannelMapping->IsChecked();
}
bool ConvertDialog::isSetOffAtEnd() {
    return CheckBoxOffAtEnd->IsChecked();
}
void ConvertDialog::SetStatusText(const wxString &msg) {
    StaticTextStatus->SetLabel(msg);
}

#define string_format wxString::Format

wxString ConvertDialog::FromAscii(const char *val) {
    return wxString::FromAscii(val);
}
void RemoveAt(wxArrayString &v, int i) {
    v.RemoveAt(i);
}

#ifndef MAX_READ_BLOCK_SIZE
#define MAX_READ_BLOCK_SIZE 4096 * 1024
#endif

wxString Left(const wxString &in, int len) {
    return in.substr(0, len);
}

wxString Right(const wxString &in, int len) {
    return in.substr(in.size() - len, len);
}

bool ConvertDialog::WriteVixenFile(const wxString& filename)
{
    wxString ChannelName, TestName;
    int32_t ChannelColor;
    long TotalTime = SeqData.TotalTime();
    wxXmlNode *node, *chparent, *textnode;
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "Program");
    doc.SetRoot(root);

    // add nodes to root in reverse order



    node = new wxXmlNode(root, wxXML_ELEMENT_NODE, "EventValues");
    textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, _parent->base64_encode());


    node = new wxXmlNode(root, wxXML_ELEMENT_NODE, "Audio");
    node->AddAttribute("filename", mediaFilename);
    node->AddAttribute("duration", string_format("%ld", TotalTime));
    textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, "Music");

    chparent = new wxXmlNode(root, wxXML_ELEMENT_NODE, "Channels");


    for (int ch = 0; ch < SeqData.NumChannels(); ch++)
    {
        SetStatusText(string_format("Status: Channel %ld ", ch));

        node = new wxXmlNode(wxXML_ELEMENT_NODE, "Channel");
        node->AddAttribute("output", string_format("%d", ch));
        node->AddAttribute("id", "0");
        node->AddAttribute("enabled", "True");
        chparent->AddChild(node);

        // KW - not sure why this was this way but now test tab is removed I need to remove it
        //		if (ch < CheckListBoxTestChannels->GetCount())
        if (ch < ChNames.Count())
        {
            TestName = ChNames[ch];
            if (TestName == "") {
                TestName = NetInfo.GetChannelName(ch);
            }
        }
        else
        {
            TestName = string_format("Ch: %d", ch);
        }
        if (ch < ChannelNames.size() && ChannelNames[ch].size() != 0)
        {
            ChannelName = ChannelNames[ch];
        }
        else
        {
            ChannelName = TestName;
        }
        // LOR is BGR with high bits=0
        // Vix is RGB with high bits=1
        if (ch < ChannelColors.size() && ChannelColors[ch] > 0)
        {
            ChannelColor = 0xff000000 | (ChannelColors[ch] >> 16 & 0x0000ff) | (ChannelColors[ch] & 0x00ff00) | (ChannelColors[ch] << 16 & 0xff0000);
        }
        else if (TestName.Last() == 'R')
        {
            ChannelColor = 0xffff0000;
        }
        else if (TestName.Last() == 'G')
        {
            ChannelColor = 0xff00ff00;
        }
        else if (TestName.Last() == 'B')
        {
            ChannelColor = 0xff0000ff;
        }
        else
        {
            // default to white
            ChannelColor = 0xffffffff;
        }
        node->AddAttribute("color", string_format("%d", ChannelColor));
        textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, ChannelName);
    }

    node = new wxXmlNode(root, wxXML_ELEMENT_NODE, "AudioDevice");
    textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, "-1");

    node = new wxXmlNode(root, wxXML_ELEMENT_NODE, "MaximumLevel");
    textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, "255");

    node = new wxXmlNode(root, wxXML_ELEMENT_NODE, "MinimumLevel");
    textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, "0");

    node = new wxXmlNode(root, wxXML_ELEMENT_NODE, "EventPeriodInMilliseconds");
    textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, string_format("%ld", SeqData.FrameTime()));

    node = new wxXmlNode(root, wxXML_ELEMENT_NODE, "Time");
    textnode = new wxXmlNode(node, wxXML_TEXT_NODE, wxEmptyString, string_format("%ld", TotalTime));

    return doc.Save(filename);
}

void ConvertDialog::WriteVirFile(const wxString& filename)
{
    _parent->WriteVirFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData);
}

void ConvertDialog::WriteHLSFile(const wxString& filename)
{
    _parent->WriteHLSFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData);
}

void ConvertDialog::WriteXLightsFile(const wxString& filename)
{
    wxFile f;
    char hdr[512];
    memset(hdr, 0, 512);
    if (!f.Create(filename, true))
    {
        _parent->ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }
    if (mediaFilename.size() > 470)
    {
        _parent->ConversionError(wxString("Media file name is too long"));
        return;
    }
    if (SeqData.FrameTime() != 50) {
        _parent->ConversionError(wxString("xseq file must be 50ms timing"));
        return;
    }
    int xseq_format_version = 1;

    sprintf(hdr, "xLights %2d %8d %8d", xseq_format_version, SeqData.NumChannels(), SeqData.NumFrames());
    strncpy(&hdr[32], mediaFilename.c_str(), 470);
    f.Write(hdr, 512);
    for (int x = 0; x < SeqData.NumChannels(); x++) {
        for (int p = 0; p < SeqData.NumFrames(); p++) {
            f.Write(&SeqData[p][x], 1);
        }
    }
    f.Close();
}

void ConvertDialog::WriteLSPFile(const wxString& filename)
{
    _parent->WriteLSPFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData, 0);
}

void ConvertDialog::WriteLorFile(const wxString& filename)
{
    wxString ChannelName, TestName;
    int32_t ChannelColor;
    int ch, p, csec, StartCSec, ii;
    int intensity, LastIntensity;
    wxFile f;
    int* savedIndexes;
    int savedIndexCount = 0;
    savedIndexes = (int *)calloc(SeqData.NumChannels(), sizeof(int));

    int index = 0;
    int rgbChanIndexes[3] = { 0,0,0 };
    int curRgbChanCount = 0;

    if (!f.Create(filename, true))
    {
        _parent->ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }
    int interval = SeqData.FrameTime() / 10;  // in centiseconds
    long centiseconds = SeqData.NumFrames() * interval;

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    f.Write("<sequence saveFileVersion=\"3\"");
    if (mediaFilename.size() != 0)
    {
        f.Write(" musicFilename=\"" + mediaFilename + "\"");
    }
    f.Write(">\n");
    f.Write("\t<channels>\n");
    for (ch = 0; ch < SeqData.NumChannels(); ch++)
    {
        SetStatusText(wxString("Status: ") + string_format(" Channel %ld ", ch));

        // KW - not sure why this was this way but now test tab is removed I need to remove it
        //		if (ch < CheckListBoxTestChannels->GetCount())
        if (ch < ChNames.Count())
        {
            TestName = ChNames[ch];
            if (TestName == "") {
                TestName = NetInfo.GetChannelName(ch);
            }
        }
        else
        {
            TestName = string_format("Ch: %d", ch);
        }
        if (ch < ChannelNames.size() && ChannelNames[ch].size() != 0)
        {
            ChannelName = ChannelNames[ch];
        }
        else
        {
            ChannelName = TestName;
        }
        // LOR is BGR with high bits=0
        // Vix is RGB with high bits=1
        if (ch < ChannelColors.size() && ChannelColors[ch] > 0)
        {
            ChannelColor = ChannelColors[ch];
        }
        else if (TestName.Last() == 'R')
        {
            ChannelColor = 0x000000ff;
        }
        else if (TestName.Last() == 'G')
        {
            ChannelColor = 0x0000ff00;
        }
        else if (TestName.Last() == 'B')
        {
            ChannelColor = 0x00ff0000;
        }
        else
        {
            // default to white
            ChannelColor = 0x00ffffff;
        }
        f.Write("\t\t<channel name=\"" + ChannelName + string_format("\" color=\"%d\" centiseconds=\"%ld\" savedIndex=\"%d\">\n", ChannelColor, centiseconds, index));
        // write intensity values for this channel
        LastIntensity = 0;
        for (p = 0, csec = 0; p < SeqData.NumFrames(); p++, csec += interval)
        {
            intensity = SeqData[p][ch] * 100 / 255;
            if (intensity != LastIntensity)
            {
                if (LastIntensity != 0)
                {
                    f.Write(string_format("\t\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n", StartCSec, csec, LastIntensity));
                }
                StartCSec = csec;
            }
            LastIntensity = intensity;
        }

        if (LastIntensity != 0)
        {
            f.Write(string_format("\t\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n", StartCSec, csec, LastIntensity));
        }
        f.Write("\t\t</channel>\n");
        // KW I had to replace this because test tab has been removed
        //if (ch < CheckListBoxTestChannels->GetCount() &&
        if (ch < _parent->GetTotalChannels() &&
            (TestName.Last() == 'R' || TestName.Last() == 'G' || TestName.Last() == 'B'))
        {
            rgbChanIndexes[curRgbChanCount++] = index;
            if (curRgbChanCount == 3)
            {
                index++;
                f.Write("\t\t<rgbChannel name=\"" + Left(ChannelName, ChannelName.size() - 1) +
                    string_format("(RGB)\" totalCentiseconds=\"%d\" savedIndex=\"%d\">\n", centiseconds, index));
                savedIndexes[savedIndexCount++] = index;
                f.Write("\t\t\t<channels>\n");
                for (ii = 0; ii <3; ii++)
                {
                    f.Write(string_format("\t\t\t\t<channel savedIndex=\"%d\"/>\n", rgbChanIndexes[ii]));
                }
                f.Write("\t\t\t</channels>\n");
                f.Write("\t\t</rgbChannel>\n");
                curRgbChanCount = 0;
            }
        }
        else
        {
            savedIndexes[savedIndexCount++] = index;
        }
        index++;
    }
    f.Write("\t</channels>\n");
    f.Write("\t<tracks>\n");
    f.Write(string_format("\t\t<track totalCentiseconds=\"%ld\">\n", centiseconds));
    f.Write("\t\t\t<channels>\n");
    for (ii = 0; ii < savedIndexCount; ii++)
    {
        f.Write(string_format("\t\t\t\t<channel savedIndex=\"%d\"/>\n", savedIndexes[ii]));
    }
    f.Write("\t\t\t</channels>\n");
    f.Write("\t\t\t<timings>\n");
    for (p = 0, csec = 0; p < SeqData.NumFrames(); p++, csec += interval)
    {
        f.Write(string_format("\t\t\t\t<timing centisecond=\"%d\"/>\n", csec));
    }
    f.Write("\t\t\t</timings>\n");
    f.Write("\t\t</track>\n");
    f.Write("\t</tracks>\n");
    f.Write("</sequence>\n");
    f.Close();
    free(savedIndexes);

}
void ConvertDialog::WriteLcbFile(const wxString& filename)
{
    _parent->WriteLcbFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData);
}

void ConvertDialog::WriteConductorFile(const wxString& filename)
{
    wxFile f;
    wxUint8 buf[16384];
    size_t ch, i, j;
    if (!f.Create(filename, true))
    {
        _parent->ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }
    for (long period = 0; period < SeqData.NumFrames(); period++)
    {
        //if (period % 500 == 499) AppendConvertStatus (string_format("Writing time period %ld\n",period+1));
        wxYield();
        for (i = 0; i < 4096; i++)
        {
            for (j = 0; j < 4; j++)
            {
                ch = j * 4096 + i;
                buf[i * 4 + j] = SeqData[period][ch];
            }
        }
        f.Write(buf, 16384);
    }

    // pad the end of the file with 512 bytes of 0's
    memset(buf, 0, 512);
    f.Write(buf, 512);
    f.Close();
}

// return true on success
bool ConvertDialog::LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString& VixChannelNames)
{
    wxString tag, tempstr;
    long OutputChannel;
    wxFileName fn;
    fn.AssignDir(_parent->CurrentDir);
    fn.SetFullName(ProfileName + ".pro");
    if (!fn.FileExists())
    {
        _parent->ConversionError(wxString("Unable to find Vixen profile: ") + fn.GetFullPath() + wxString("\n\nMake sure a copy is in your xLights directory"));
        return false;
    }
    wxXmlDocument doc(fn.GetFullPath());
    if (doc.IsOk())
    {
        VixChannels.clear();
        wxXmlNode* root = doc.GetRoot();
        for (wxXmlNode* e = root->GetChildren(); e != NULL; e = e->GetNext())
        {
            tag = e->GetName();
            if (tag == wxString("ChannelObjects"))
            {
                for (wxXmlNode* p = e->GetChildren(); p != NULL; p = p->GetNext())
                {
                    if (p->GetName() == wxString("Channel"))
                    {
                        if (p->HasAttribute("output"))
                        {
                            tempstr = p->GetAttribute("output", "0");
                            OutputChannel = atol(tempstr.c_str());
                            VixChannels.push_back(OutputChannel);
                        }
                        if (p->HasAttribute("name"))
                        {
                            VixChannelNames.push_back(p->GetAttribute("name"));
                        }
                        else
                        {
                            if (p->GetChildren() != NULL) {
                                VixChannelNames.push_back(p->GetChildren()->GetContent());
                            }
                            else {
                                VixChannelNames.push_back(p->GetContent());
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
    else
    {
        _parent->ConversionError(wxString("Unable to load Vixen profile: ") + ProfileName);
    }
    return false;
}

void ConvertDialog::ReadConductorFile(const wxString& FileName)
{
    wxFile f;
    int i, j, ch;
    char row[16384];
    int period = 0;
    _parent->ConversionInit();
    wxFileDialog mediaDialog(this, wxString("Select associated media file, or cancel if this is an animation"));
    if (mediaDialog.ShowModal() == wxID_OK)
    {
        _parent->SetMediaFilename(mediaDialog.GetPath());
    }
    if (!f.Open(FileName.c_str()))
    {
        _parent->PlayerError(wxString("Unable to load sequence:\n") + FileName);
        return;
    }
    int numPeriods = f.Length() / 16384;

    SeqData.init(NetInfo.GetTotChannels(), numPeriods, 50);
    while (f.Read(row, 16384) == 16384)
    {
        wxYield();
        for (i = 0; i < 4096; i++)
        {
            for (j = 0; j < 4; j++)
            {
                ch = j * 4096 + i;
                if (ch < SeqData.NumChannels())
                {
                    SeqData[period][ch] = row[i * 4 + j];
                }
            }
        }
        period++;
    }
    f.Close();
}

void ConvertDialog::ReadGlediatorFile(const wxString& FileName)
{
    wxFile f;

    size_t fileLength;
    int j, period, x_width = 32, y_height = 32; // for now hard code matrix to be 32x32. after we get this working, we will prompt for this info during convert
    wxString filename = string_format(wxString("01 - Carol of the Bells.mp3")); // hard code a mp3 file for now
    size_t readcnt;

    _parent->ConversionInit();
    if (!f.Open(FileName.c_str()))
    {
        _parent->PlayerError(wxString("Unable to load sequence:\n") + FileName);
        return;
    }

    fileLength = f.Length();
    int numChannels = (x_width * 3 * y_height); // 3072 = 32*32*3
    char *frameBuffer = new char[SeqData.NumChannels()];

    int numFrames = (int)(fileLength / (x_width * 3 * y_height));
    _parent->SetMediaFilename(filename);
    SeqData.init(numChannels, numFrames, 50);

    wxYield();
    period = 0;
    while ((readcnt = f.Read(frameBuffer, SeqData.NumChannels())))   // Read one period of channels
    {
        for (j = 0; j<readcnt; j++)   // Loop thru all channel.s
        {
            SeqData[period][j] = frameBuffer[j];
        }
        period++;
    }

    /*
    for(i=0; i<readcnt-2; i++) {
    SeqData[i] = i%256;
    }
    for(i=0; i<readcnt-2; i+=3) { // loop thru channels, jump by 3. so this loop is pixel loop
    period = i/bytes_per_period;
    p=period * (bytes_per_period); // byte offset for start of each period
    ch=p+ (y*x_width*3) + x*3; // shows offset into source buffer
    byte =p+i;
    byte1=p+i+(1)* (SeqData.NumFrames());
    byte2=p+i+(2)* (SeqData.NumFrames());
    if ( byte2<readcnt) {
    SeqData[byte]  = row[i];
    SeqData[byte1] = row[i+1];
    SeqData[byte2] = row[i+2];
    }
    }
    //   }
    */
    f.Close();
    delete[] frameBuffer;

#ifndef NDEBUG
    AppendConvertStatus(string_format(wxString("ReadGlediatorFile SeqData.NumFrames()=%ld SeqData.NumChannels()=%ld\n"), SeqData.NumFrames(), SeqData.NumChannels()));
#endif
}


int getAttributeValueAsInt(SP_XmlStartTagEvent * stagEvent, const char * name)
{
    const char *val = stagEvent->getAttrValue(name);
    if (!val)
    {
        return 0;
    }
    return atoi(val);
}

wxString ConvertDialog::getAttributeValueSafe(SP_XmlStartTagEvent* stagEvent, const char* name)
{
    const char *val = stagEvent->getAttrValue(name);
    if (!val)
    {
        return "";
    }
    return FromAscii(val);
}


void ConvertDialog::ReadVixFile(const wxString& filename)
{
    wxString NodeName, NodeValue, msg;
    std::vector<unsigned char> VixSeqData;
    wxArrayInt VixChannels;
    wxArrayString VixChannelNames;
    long cnt = 0;
    wxArrayString context;
    long VixEventPeriod = -1;
    long MaxIntensity = 255;
    int OutputChannel;

    _parent->ConversionInit();
    AppendConvertStatus(wxString("Reading Vixen sequence\n"));

    SP_XmlPullParser *parser = new SP_XmlPullParser();
    parser->setMaxTextSize(MAX_READ_BLOCK_SIZE / 2);
    wxFile file(filename);
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);
    wxString carryOver;

    //pass 1, read the length, determine number of networks, units/network, channels per unit
    SP_XmlPullEvent * event = parser->getNext();
    int done = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch (event->getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii(stagEvent->getName());
                context.push_back(NodeName);
                cnt++;
                //msg=wxString("Element: ") + NodeName + string_format(wxString(" (%ld)\n"),cnt);
                //AppendConvertStatus (msg);
                if (cnt == 2 && (NodeName == wxString("Audio") || NodeName == wxString("Song")))
                {
                    _parent->SetMediaFilename(getAttributeValueSafe(stagEvent, "filename"));
                }
                if (cnt > 1 && context[1] == wxString("Channels") && NodeName == wxString("Channel"))
                {
                    const char *val = stagEvent->getAttrValue("output");
                    if (!val)
                    {
                        VixChannels.push_back(VixChannels.size());
                    }
                    else
                    {
                        VixChannels.push_back(atoi(val));
                    }
                }
            }
            break;
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt == 2)
                {
                    NodeValue = FromAscii(stagEvent->getText());
                    if (context[1] == wxString("MaximumLevel"))
                    {
                        MaxIntensity = atol(NodeValue.c_str());
                    }
                    else if (context[1] == wxString("EventPeriodInMilliseconds"))
                    {
                        VixEventPeriod = atol(NodeValue.c_str());
                    }
                    else if (context[1] == wxString("EventValues"))
                    {
                        //AppendConvertStatus(string_format(wxString("Chunk Size=%d\n"), NodeValue.size()));
                        if (carryOver.size() > 0) {
                            NodeValue.insert(0, carryOver);
                        }
                        int i = _parent->base64_decode(NodeValue, VixSeqData);
                        if (i != 0) {
                            int start = NodeValue.size() - i - 1;
                            carryOver = NodeValue.substr(start, start + i);
                        }
                        else {
                            carryOver.clear();
                        }
                    }
                    else if (context[1] == wxString("Profile"))
                    {
#ifndef FPP
                        LoadVixenProfile(NodeValue, VixChannels, VixChannelNames);
#endif
                    }
                    else if (context[1] == wxString("Channels") && context[2] == wxString("Channel"))
                    {
                        while (VixChannelNames.size() < VixChannels.size())
                        {
                            VixChannelNames.push_back("");
                        }
                        VixChannelNames[VixChannels.size() - 1] = NodeValue;
                    }
                }
                break;
            }
            case SP_XmlPullEvent::eEndTag:
            {
                if (cnt > 0)
                {
                    RemoveAt(context, cnt - 1);
                }
                cnt = context.size();
                break;
            }
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete[] bytes;
    delete parser;
    file.Close();

    int min = 999999;
    int max = 0;
    for (int x = 0; x < VixChannels.size(); x++)
    {
        int i = VixChannels[x];
        if (i > max)
        {
            max = i;
        }
        if (i < min)
        {
            min = i;
        }
    }

    long VixDataLen = VixSeqData.size();
    int numChannels = (max - min) + 1;
    if (numChannels < 0)
    {
        numChannels = 0;
    }
    AppendConvertStatus(string_format(wxString("Max Intensity=%ld\n"), MaxIntensity), false);
    AppendConvertStatus(string_format(wxString("# of Channels=%ld\n"), numChannels), false);
    AppendConvertStatus(string_format(wxString("Vix Event Period=%ld\n"), VixEventPeriod), false);
    AppendConvertStatus(string_format(wxString("Vix data len=%ld\n"), VixDataLen), false);
    if (numChannels == 0)
    {
        return;
    }
    long VixNumPeriods = VixDataLen / VixChannels.size();
    AppendConvertStatus(string_format(wxString("Vix # of time periods=%ld\n"), VixNumPeriods), false);
    AppendConvertStatus(wxString("Media file=") + mediaFilename + wxString("\n"), false);
    if (VixNumPeriods == 0) {
        return;
    }
    SeqData.init(numChannels, VixNumPeriods, VixEventPeriod);

    // reorder channels according to output number, scale so that max intensity is 255
    int newper, intensity;
    size_t ch;
    for (ch = 0; ch < SeqData.NumChannels(); ch++)
    {
        OutputChannel = VixChannels[ch] - min;
        if (ch < VixChannelNames.size())
        {
            ChannelNames[OutputChannel] = VixChannelNames[ch];
        }
        for (newper = 0; newper < SeqData.NumFrames(); newper++)
        {
            intensity = VixSeqData[ch*VixNumPeriods + newper];
            if (MaxIntensity != 255)
            {
                intensity = intensity * 255 / MaxIntensity;
            }
            SeqData[newper][OutputChannel] = intensity;
        }
    }
}

void ConvertDialog::ReadHLSFile(const wxString& filename)
{
    long timeCells = 0;
    long msPerCell = 50;
    long channels = 0;
    long cnt = 0;
    long tmp;
    long universe = 0;
    long channelsInUniverse = 0;
    wxString NodeName, NodeValue, Data, ChannelName;
    wxArrayString context;
    wxArrayInt map;


    SP_XmlPullParser *parser = new SP_XmlPullParser();
    wxFile file(filename);
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);

    // pass one, get the metadata
    SP_XmlPullEvent * event = parser->getNext();
    int done = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch (event->getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii(stagEvent->getName());
                context.push_back(NodeName);
                cnt++;
                break;
            }
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    NodeValue = FromAscii(stagEvent->getText());

                    if (NodeName == wxString("MilliSecPerTimeUnit"))
                    {
                        msPerCell = atol(NodeValue.c_str());
                    }
                    if (NodeName == wxString("NumberOfTimeCells"))
                    {
                        timeCells = atol(NodeValue.c_str());
                    }
                    if (NodeName == wxString("AudioSourcePcmFile"))
                    {
                        mediaFilename = NodeValue;
                        if (Right(mediaFilename, 4) == ".PCM")
                        {
                            //nothing can deal with PCM files, we'll assume this came from an mp3
                            mediaFilename = Left(mediaFilename, mediaFilename.size() - 4);
                            mediaFilename += ".mp3";
                        }
                    }
                    if (NodeName == wxString("ChannelsInUniverse"))
                    {
                        channelsInUniverse = atol(NodeValue.c_str());
                        channels += channelsInUniverse;
                    }
                    if (NodeName == wxString("UniverseNumber"))
                    {
                        tmp = atol(NodeValue.c_str());
                        universe = tmp;
                    }
                }
                break;
            }
            case SP_XmlPullEvent::eEndTag:
            {
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    if (NodeName == wxString("Universe"))
                    {
                        map.push_back(universe);
                        map.push_back(channelsInUniverse);
                        for (tmp = map.size() - 2; tmp > 0; tmp -= 2)
                        {
                            if (map[tmp] < map[tmp - 2])
                            {
                                long t1 = map[tmp];
                                long t2 = map[tmp + 1];
                                map[tmp] = map[tmp - 2];
                                map[tmp + 1] = map[tmp - 1];
                                map[tmp - 2] = t1;
                                map[tmp - 1] = t2;
                            }
                        }
                    }

                    RemoveAt(context, cnt - 1);
                }
                cnt = context.size();
                break;
            }
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete parser;

    file.Seek(0);



    channels = 0;

    for (tmp = 0; tmp < map.size(); tmp += 2)
    {
        int i = map[tmp + 1];
        int orig = NetInfo.GetNumChannels(tmp / 2);
        if (i < orig) {
            AppendConvertStatus(string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d\n"), map[tmp], i, orig), false);
            i = orig;
        }
        else if (i > orig) {
            AppendConvertStatus(string_format(wxString("WARNING Universe: %ld contains more channels than you have configured.\n"), map[tmp]), false);
            AppendConvertStatus(string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d\n"), map[tmp], i, orig), false);
        }
        else {
            AppendConvertStatus(string_format(wxString("Found Universe: %ld   Channels in Seq: %ld\n"), map[tmp], i, orig), false);
        }


        map[tmp + 1] = channels;
        channels += i;
    }

    AppendConvertStatus(string_format(wxString("TimeCells = %d\n"), timeCells), false);
    AppendConvertStatus(string_format(wxString("msPerCell = %d ms\n"), msPerCell), false);
    AppendConvertStatus(string_format(wxString("Channels = %d\n"), channels), false);
    if (channels == 0)
    {
        return;
    }
    if (timeCells == 0)
    {
        return;
    }
    SeqData.init(channels, timeCells, msPerCell);

    ChannelNames.resize(channels);
    ChannelColors.resize(channels);

    channels = 0;

    wxYield();

    parser = new SP_XmlPullParser();
    read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);


    event = parser->getNext();
    done = 0;
    int nodecnt = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch (event->getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii(stagEvent->getName());
                context.push_back(NodeName);
                cnt++;
                break;
            }
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    NodeValue = FromAscii(stagEvent->getText());

                    if (NodeName == wxString("ChanInfo"))
                    {
                        //channel name and type
                        ChannelName = NodeValue;
                    }
                    if (NodeName == wxString("Block"))
                    {
                        int idx = NodeValue.find("-");
                        Data.append(NodeValue.substr(idx + 1));
                    }
                    if (NodeName == wxString("UniverseNumber"))
                    {
                        tmp = atol(NodeValue.c_str());
                        universe = tmp;
                        for (tmp = 0; tmp < map.size(); tmp += 2)
                        {
                            if (universe == map[tmp])
                            {
                                channels = map[tmp + 1];
                            }
                        }
                    }
                }
                break;
            }
            case SP_XmlPullEvent::eEndTag:
            {
                if (nodecnt > 1000)
                {
                    nodecnt = 0;
                    wxYield();
                }
                nodecnt++;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    if (NodeName == wxString("ChannelData"))
                    {

                        //finished reading this channel, map the data
                        int idx = ChannelName.find(", ");
                        wxString type = ChannelName.substr(idx + 2);
                        wxString origName = ChannelNames[channels];
                        if (type == wxString("RGB-R"))
                        {
                            ChannelNames[channels] = Left(ChannelName, idx) + wxString("-R");
                            ChannelColors[channels] = 0x000000FF;
                        }
                        else if (type == wxString("RGB-G"))
                        {
                            ChannelNames[channels] = Left(ChannelName, idx) + wxString("-G");
                            ChannelColors[channels] = 0x0000FF00;
                        }
                        else if (type == wxString("RGB-B"))
                        {
                            ChannelNames[channels] = Left(ChannelName, idx) + wxString("-B");
                            ChannelColors[channels] = 0x00FF0000;
                        }
                        else
                        {
                            ChannelNames[channels] = Left(ChannelName, idx);
                            ChannelColors[channels] = 0x00FFFFFF;
                        }
                        wxString o2 = NetInfo.GetChannelName(channels);
                        AppendConvertStatus(string_format("Map %s -> %s (%s)\n",
                            ChannelNames[channels].c_str(),
                            origName.c_str(),
                            o2.c_str()), false);
                        for (long newper = 0; newper < SeqData.NumFrames(); newper++)
                        {
                            long intensity;
                            intensity = strtoul(Data.substr(newper * 3, 2).c_str(), NULL, 16);
                            SeqData[newper][channels] = intensity;
                        }
                        Data.clear();
                        channels++;
                    }

                    RemoveAt(context, cnt - 1);
                }
                cnt = context.size();
                break;
            }
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete[] bytes;
    delete parser;
    file.Close();
}

class LORInfo
{
public:
    wxString name;
    int savedIndex;
    wxString deviceType;
    int unit;
    int circuit;
    int network;
    bool empty;

    LORInfo(const wxString & nm, const wxString &dt, int n, int u, int c, int s) :
        name(nm), deviceType(dt), network(n), unit(u), circuit(c), savedIndex(s), empty(true)
    {
    }
    LORInfo(const LORInfo &li) :
        name(li.name),
        deviceType(li.deviceType), network(li.network), unit(li.unit),
        circuit(li.circuit), savedIndex(li.savedIndex), empty(li.empty)
    {

    }
    LORInfo() :
        name(""), deviceType(""), network(0), unit(0), circuit(0), savedIndex(0), empty(true)
    {

    }
};

typedef std::map<int, LORInfo> LORInfoMap;


static void mapLORInfo(const LORInfo &info, std::vector< std::vector<int> > *unitSizes)
{
    int unit = info.unit;
    if (unit < 0)
    {
        unit += 256;
    }
    if (unit == 0)
    {
        unit = 1;
    }

    if (info.network >= unitSizes->size())
    {
        unitSizes->resize(info.network + 1);
    }
    if (unit > (*unitSizes)[info.network].size())
    {
        (*unitSizes)[info.network].resize(unit);
    }
    if (info.circuit == 0)
    {
        (*unitSizes)[info.network][unit - 1]++;
    }
    else if (info.circuit > (*unitSizes)[info.network][unit - 1])
    {
        (*unitSizes)[info.network][unit - 1] = info.circuit;
    }
}

// return a random number between 0 and 1 inclusive
double rand01()
{
    return (double)rand() / (double)RAND_MAX;
}

void ConvertDialog::ReadLorFile(const wxString& filename, int LORImportInterval)
{
    wxString NodeName, msg, EffectType, ChannelName, deviceType, networkAsString;
    wxArrayString context;
    int unit, circuit, startcsec, endcsec, intensity, startIntensity, endIntensity, rampdiff, ChannelColor;
    int i, startper, endper, perdiff, twinklestate, nexttwinkle;
    int twinkleperiod = 400;
    int curchannel = -1;
    int MappedChannelCnt = 0;
    int MaxIntensity = 100;
    int EffectCnt = 0;
    int network, chindex = -1;
    long cnt = 0;
    std::vector< std::vector<int> > noNetworkUnitSizes;
    std::vector< std::vector<int> > lorUnitSizes;
    std::vector< std::vector<int> > dmxUnitSizes;
    LORInfoMap rgbChannels;

    LorTimingList.clear();

    _parent->ConversionInit();
    AppendConvertStatus(wxString("Reading LOR sequence\n"));
    SetStatusText(wxString("Reading LOR sequence"));


    int centisec = -1;
    int nodecnt = 0;
    int channelCount = 0;

    SP_XmlPullParser *parser = new SP_XmlPullParser();
    wxFile file(filename);
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);
    bool mapEmpty = mapEmptyChannels();
    bool showChannelMap = showChannelMapping();

    //pass 1, read the length, determine number of networks, units/network, channels per unit
    SP_XmlPullEvent * event = parser->getNext();
    int done = 0;
    int savedIndex = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch (event->getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii(stagEvent->getName());
                context.push_back(NodeName);
                cnt++;

                nodecnt++;
                if (nodecnt > 1000)
                {
                    nodecnt = 0;
                    wxYield();
                }
                if (NodeName == wxString("track"))
                {
                    centisec = getAttributeValueAsInt(stagEvent, "totalCentiseconds");
                }
                else if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
                {
                    channelCount++;
                    if ((channelCount % 1000) == 0)
                    {
                        AppendConvertStatus(string_format(wxString("Channels found so far: %d\n"), channelCount));
                        SetStatusText(string_format(wxString("Channels found so far: %d"), channelCount));
                    }

                    deviceType = FromAscii(stagEvent->getAttrValue("deviceType"));
                    network = getAttributeValueAsInt(stagEvent, "network");
                    networkAsString = FromAscii(stagEvent->getAttrValue("network"));
                    unit = getAttributeValueAsInt(stagEvent, "unit");
                    circuit = getAttributeValueAsInt(stagEvent, "circuit");
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");
                    wxString channelName = FromAscii(stagEvent->getAttrValue("name"));
                    rgbChannels[savedIndex] = LORInfo(channelName, deviceType, network, unit, circuit, savedIndex);
                    rgbChannels[savedIndex].empty = !mapEmpty;
                }
                else if (cnt > 1 && context[1] == wxString("channels") && NodeName == wxString("effect"))
                {
                    rgbChannels[savedIndex].empty = false;
                }
                else if (cnt > 3 && context[1] == wxString("channels") && context[2] == wxString("rgbChannel")
                    && context[3] == wxString("channels") && NodeName == wxString("channel"))
                {
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");
                    if (rgbChannels[savedIndex].empty == true)
                    {
                        std::vector< std::vector<int> > *unitSizes;
                        if (Left(rgbChannels[savedIndex].deviceType, 3) == "DMX")
                        {
                            unitSizes = &dmxUnitSizes;
                        }
                        else if ("" == deviceType && "" == networkAsString && !MapLORChannelsWithNoNetwork->IsChecked()) {
                            unitSizes = &noNetworkUnitSizes;
                        }
                        else
                        {
                            unitSizes = &lorUnitSizes;
                        }
                        mapLORInfo(rgbChannels[savedIndex], unitSizes);
                    }
                    rgbChannels[savedIndex].empty = false;
                }
            }
            break;
            case SP_XmlPullEvent::eEndTag:
                if (cnt == 3 && context[1] == wxString("channels") && context[2] == wxString("channel") && !rgbChannels[savedIndex].empty)
                {
                    std::vector< std::vector<int> > *unitSizes;
                    if (Left(rgbChannels[savedIndex].deviceType, 3) == "DMX") {
                        unitSizes = &dmxUnitSizes;
                    }
                    else if ("" == deviceType && "" == networkAsString && !MapLORChannelsWithNoNetwork->IsChecked()) {
                        unitSizes = &noNetworkUnitSizes;
                    }
                    else {
                        unitSizes = &lorUnitSizes;
                    }
                    mapLORInfo(rgbChannels[savedIndex], unitSizes);
                }

                if (cnt > 0)
                {
                    RemoveAt(context, cnt - 1);
                }
                cnt = context.size();
                break;
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete parser;
    AppendConvertStatus(string_format(wxString("Track 1 length = %d centiseconds\n"), centisec), false);

    if (centisec > 0)
    {
        int numFrames = centisec * 10 / LORImportInterval;
        if (numFrames == 0)
        {
            numFrames = 1;
        }
        SeqData.init(NetInfo.GetTotChannels(), numFrames, LORImportInterval);
    }
    else
    {
        _parent->ConversionError(wxString("Unable to determine the length of this LOR sequence (looked for length of track 1)"));
        return;
    }

    for (network = 0; network < lorUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < lorUnitSizes[network].size(); u++)
        {
            cnt += lorUnitSizes[network][u];
        }
        AppendConvertStatus(string_format(wxString("LOR Network %d:  %d channels\n"), network, cnt), false);
    }
    for (network = 1; network < dmxUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < dmxUnitSizes[network].size(); u++)
        {
            if (cnt < dmxUnitSizes[network][u]) {
                cnt = dmxUnitSizes[network][u];
            }
        }
        AppendConvertStatus(string_format(wxString("DMX Network %d:  %d channels\n"), network, cnt), false);
    }
    AppendConvertStatus(string_format(wxString("Total channels = %d\n"), channelCount));

    cnt = 0;
    context.clear();
    channelCount = 0;


    parser = new SP_XmlPullParser();
    file.Seek(0);
    read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);

    //pass 2, convert the data
    event = parser->getNext();
    done = 0;
    bool empty;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch (event->getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eEndTag:
                if (cnt > 0)
                {
                    RemoveAt(context, cnt - 1);
                }
                cnt = context.size();
                if (cnt == 2)
                {
                    if (empty && curchannel != -1)
                    {
                        chindex--;
                        AppendConvertStatus(wxString("WARNING: ") + ChannelNames[curchannel] + " is empty\n");
                        ChannelNames[curchannel].clear();
                        MappedChannelCnt--;
                    }
                    curchannel = -1;
                }
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii(stagEvent->getName());
                context.push_back(NodeName);
                cnt++;

                nodecnt++;
                if (nodecnt > 1000)
                {
                    nodecnt = 0;
                    wxYield();
                }
                //msg=wxString("Element: ") + NodeName + string_format(wxString(" (%ld)\n"),cnt);
                //AppendConvertStatus (msg);
                if (NodeName == wxString("sequence"))
                {
                    _parent->SetMediaFilename(getAttributeValueSafe(stagEvent, "musicFilename"));
                }
                if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
                {
                    channelCount++;
                    if ((channelCount % 1000) == 0)
                    {
                        AppendConvertStatus(string_format(wxString("Channels converted so far: %d\n"), channelCount));
                        SetStatusText(string_format(wxString("Channels converted so far: %d"), channelCount));
                    }

                    deviceType = getAttributeValueSafe(stagEvent, "deviceType");
                    networkAsString = getAttributeValueSafe(stagEvent, "network");
                    network = getAttributeValueAsInt(stagEvent, "network");

                    unit = getAttributeValueAsInt(stagEvent, "unit");
                    if (unit < 0)
                    {
                        unit += 256;
                    }
                    if (unit == 0)
                    {
                        unit = 1;
                    }
                    circuit = getAttributeValueAsInt(stagEvent, "circuit");
                    ChannelName = getAttributeValueSafe(stagEvent, "name");
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");

                    empty = rgbChannels[savedIndex].empty;
                    if (Left(deviceType, 3) == "DMX")
                    {
                        chindex = circuit - 1;
                        network--;
                        network += lorUnitSizes.size();
                        curchannel = NetInfo.CalcAbsChannel(network, chindex);
                    }
                    else if (Left(deviceType, 3) == "LOR")
                    {
                        chindex = 0;
                        for (int z = 0; z < (unit - 1); z++)
                        {
                            chindex += lorUnitSizes[network][z];
                        }
                        chindex += circuit - 1;
                        curchannel = NetInfo.CalcAbsChannel(network, chindex);
                    }
                    else if ("" == deviceType && "" == networkAsString && !MapLORChannelsWithNoNetwork->IsChecked()) {
                        curchannel = -1;
                    }
                    else {
                        chindex++;
                        if (chindex < NetInfo.GetTotChannels())
                        {
                            curchannel = chindex;
                        }
                        else
                        {
                            curchannel = -1;
                        }
                    }
                    if (curchannel >= 0)
                    {
                        //AppendConvertStatus (string_format(wxString("curchannel %d\n"),curchannel));
                        if (ChannelNames[curchannel].size() != 0)
                        {
                            AppendConvertStatus(string_format(wxString("WARNING: ") + ChannelNames[curchannel] + wxString(" and ")
                                + ChannelName + wxString(" map to the same channel %d\n"), curchannel));
                        }
                        MappedChannelCnt++;
                        ChannelNames[curchannel] = ChannelName;
                        ChannelColor = getAttributeValueAsInt(stagEvent, "color");
                        ChannelColors[curchannel] = ChannelColor;
                        if (showChannelMap)
                        {
                            AppendConvertStatus(string_format("INFO: xLights Channel: %03d = LOR Name: %s\n", curchannel + 1, ChannelName));
                        }
                    }
                    else
                    {
                        AppendConvertStatus(wxString("WARNING: channel '") + ChannelName + wxString("' is unmapped\n"));
                    }
                }
                if (cnt > 1 && context[1] == wxString("channels") && NodeName == wxString("effect") && curchannel >= 0)
                {
                    empty = false;
                    EffectCnt++;
                    startcsec = getAttributeValueAsInt(stagEvent, "startCentisecond");
                    endcsec = getAttributeValueAsInt(stagEvent, "endCentisecond");
                    intensity = getAttributeValueAsInt(stagEvent, "intensity");
                    startIntensity = getAttributeValueAsInt(stagEvent, "startIntensity");
                    endIntensity = getAttributeValueAsInt(stagEvent, "endIntensity");
                    startper = startcsec * 10 / LORImportInterval;
                    endper = endcsec * 10 / LORImportInterval;
                    perdiff = endper - startper;  // # of ticks
                    LorTimingList.insert(startper);

                    if (perdiff > 0)
                    {
                        intensity = intensity * 255 / MaxIntensity;
                        startIntensity = startIntensity * 255 / MaxIntensity;
                        endIntensity = endIntensity * 255 / MaxIntensity;
                        EffectType = getAttributeValueSafe(stagEvent, "type");
                        if (EffectType == "intensity")
                        {
                            if (intensity > 0)
                            {
                                for (i = 0; i < perdiff; i++)
                                {
                                    SeqData[startper + i][curchannel] = intensity;
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff = endIntensity - startIntensity;
                                for (i = 0; i < perdiff; i++)
                                {
                                    intensity = (int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    SeqData[startper + i][curchannel] = intensity;
                                }
                            }
                        }
                        else if (EffectType == "twinkle")
                        {
                            if (intensity == 0 && startIntensity == 0 && endIntensity == 0)
                            {
                                intensity = MaxIntensity;
                            }
                            twinklestate = static_cast<int>(rand01()*2.0) & 0x01;
                            nexttwinkle = static_cast<int>(rand01()*twinkleperiod + 100) / LORImportInterval;
                            if (intensity > 0)
                            {
                                for (i = 0; i < perdiff; i++)
                                {
                                    SeqData[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate = 1 - twinklestate;
                                        nexttwinkle = static_cast<int>(rand01()*twinkleperiod + 100) / LORImportInterval;
                                    }
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff = endIntensity - startIntensity;
                                for (i = 0; i < perdiff; i++)
                                {
                                    intensity = (int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    SeqData[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate = 1 - twinklestate;
                                        nexttwinkle = static_cast<int>(rand01()*twinkleperiod + 100) / LORImportInterval;
                                    }
                                }
                            }
                        }
                        else if (EffectType == "shimmer")
                        {
                            if (intensity == 0 && startIntensity == 0 && endIntensity == 0)
                            {
                                intensity = MaxIntensity;
                            }
                            if (intensity > 0)
                            {
                                for (i = 0; i < perdiff; i++)
                                {
                                    twinklestate = (startper + i) & 0x01;
                                    SeqData[startper + i][curchannel] = intensity * twinklestate;
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff = endIntensity - startIntensity;
                                for (i = 0; i < perdiff; i++)
                                {
                                    twinklestate = (startper + i) & 0x01;
                                    intensity = (int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    SeqData[startper + i][curchannel] = intensity * twinklestate;
                                }
                            }
                        }
                    }
                }
            }
            default:
                break;
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete[] bytes;
    delete parser;
    file.Close();


    AppendConvertStatus(string_format(wxString("# of mapped channels with effects=%d\n"), MappedChannelCnt), false);
    AppendConvertStatus(string_format(wxString("# of effects=%d\n"), EffectCnt), false);
    AppendConvertStatus(wxString("Media file=") + mediaFilename + wxString("\n"), false);
    AppendConvertStatus(string_format(wxString("New # of time periods=%ld\n"), SeqData.NumFrames()), false);
    SetStatusText(wxString("LOR sequence loaded successfully"));
}

void ConvertDialog::DoConversion(const wxString& Filename, const wxString& OutputFormat)
{
    wxString fullpath;
    wxString Out3 = Left(OutputFormat, 3);

    // read sequence file
    AppendConvertStatus(wxString("\nReading: ") + Filename + "\n");
    wxYield();
    wxFileName oName(Filename);
    wxString ext = oName.GetExt();
    if (ext == wxString("vix"))
    {
        if (Out3 == "Vix")
        {
            _parent->ConversionError(wxString("Cannot convert from Vixen to Vixen!"));
            return;
        }
        ReadVixFile(Filename);
    }
    else if (ext == wxString(XLIGHTS_SEQUENCE_EXT))
    {
        if (Out3 == "xLi")
        {
            _parent->ConversionError(wxString("Cannot convert from xLights to xLights!"));
            return;
        }
        _parent->ReadXlightsFile(Filename);
    }
    else if (ext == wxString("fseq"))
    {
        if (Out3 == "Fal")
        {
            _parent->ConversionError(wxString("Cannot convert from Falcon Player file to Falcon Player file!"));
            return;
        }
        _parent->ReadFalconFile(Filename, this);
    }
#ifndef FPP
    else if (ext == wxString("seq"))
    {
        if (Out3 == "Lyn")
        {
            _parent->ConversionError(wxString("Cannot convert from Conductor file to Conductor file!"));
            return;
        }
        ReadConductorFile(Filename);
    }
#endif
    else if (ext == wxString("gled"))
    {
        if (Out3 == "Gle")
        {
            _parent->ConversionError(wxString("Cannot convert from Glediator file to Glediator file!"));
            return;
        }
        ReadGlediatorFile(Filename);
    }
    else if (ext == wxString("hlsIdata"))
    {
        ReadHLSFile(Filename);
    }
    else if (ext == wxString("lms") || ext == wxString("las"))
    {
        if (Out3 == "LOR")
        {
            _parent->ConversionError(wxString("Cannot convert from LOR to LOR!"));
            return;
        }
        int i = LORImportTimeResolution->GetSelection();
        switch (i) {
        case 0:
            i = 25;
            break;
        case 2:
            i = 100;
            break;
        default:
            i = 50;
            break;
        }
        ReadLorFile(Filename, i);
    }
    else
    {
        _parent->ConversionError(wxString("Unknown sequence file extension"));
        return;
    }

    // check for errors
    if (SeqData.NumChannels() == 0)
    {
        AppendConvertStatus(wxString("ERROR: no channels defined\n"));
        return;
    }

    if (isSetOffAtEnd())
    {
        _parent->ClearLastPeriod();
    }
    wxYield();

    // write converted file to xLights directory
    oName.SetPath(_parent->CurrentDir);

    if (Out3 == "xLi")
    {
        oName.SetExt(wxString(XLIGHTS_SEQUENCE_EXT));
        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing xLights sequence\n"));
        WriteXLightsFile(fullpath);
        AppendConvertStatus(wxString("Finished writing new file: ") + fullpath + wxString("\n"));
    }
    else if (Out3 == "Fal")
    {
        oName.SetExt(wxString("fseq"));
        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing Falcon Player sequence\n"));
        _parent->WriteFalconPiFile(fullpath);
        AppendConvertStatus(wxString("Finished writing new file: ") + fullpath + wxString("\n"));
    }
#ifndef FPP
    else if (Out3 == "Lyn")
    {
        oName.SetExt(wxString("seq"));
        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing Lynx Conductor sequence\n"));
        WriteConductorFile(fullpath);
        AppendConvertStatus(wxString("Finished writing new file: ") + fullpath + wxString("\n"));
    }
    else if (Out3 == "Vix")
    {
        oName.SetExt(wxString("vix"));
        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing Vixen sequence\n"));
        if (WriteVixenFile(fullpath))
        {
            AppendConvertStatus(wxString("Finished writing new file: ") + fullpath + wxString("\n"));
        }
        else
        {
            _parent->ConversionError(wxString("Unable to save: ") + fullpath + wxString("\n"));
        }
    }
    else if (Out3 == "Vir")
    {
        oName.SetExt(wxString("vir"));
        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing Vixen routine\n"));
        WriteVirFile(fullpath);
    }
    else if (Out3 == "HLS")
    {
        oName.SetExt(wxString("hlsnc"));
        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing HLS routine\n"));
        WriteHLSFile(fullpath);
    }
    else if (Out3 == "LOR")
    {
        if (mediaFilename.size() == 0)
        {
            oName.SetExt(wxString("las"));
        }
        else
        {
            oName.SetExt(wxString("lms"));
        }
        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing LOR sequence\n"));
        WriteLorFile(fullpath);
        AppendConvertStatus(wxString("Finished writing LOR file: ") + fullpath + wxString("\n"));
    }
    else if (Out3 == "Lcb")
    {

        oName.SetExt(wxString("lcb"));

        fullpath = oName.GetFullPath();
        AppendConvertStatus(wxString("Writing LOR clipboard sequence\n"));
        WriteLcbFile(fullpath);
        AppendConvertStatus(wxString("Finished writing LOR lcb file: ") + fullpath + wxString("\n"));
    }
#endif
    else
    {
        AppendConvertStatus(wxString("Nothing to write - invalid output format\n"));
    }
    AppendConvertStatus("", true);
}
