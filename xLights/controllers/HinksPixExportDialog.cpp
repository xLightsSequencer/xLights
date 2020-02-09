//(*InternalHeaders(HinksPixExportDialog)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/volume.h>
#include <wx/progdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/wfstream.h>

#include "HinksPixExportDialog.h"
#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "UtilFunctions.h"
#include "ControllerCaps.h"
#include "../outputs/ControllerEthernet.h"

#include <log4cpp/Category.hh>

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}

#ifndef CODEC_CAP_DELAY /* add compatibility for ffmpeg 3+ */
#define CODEC_CAP_DELAY AV_CODEC_CAP_DELAY
#endif

//(*IdInit(HinksPixExportDialog)
const long HinksPixExportDialog::ID_STATICTEXT4 = wxNewId();
const long HinksPixExportDialog::ID_CHECKLISTBOX_CONTROLLERS = wxNewId();
const long HinksPixExportDialog::ID_PANEL2 = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT1 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_FILTER = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT2 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_FOLDER = wxNewId();
const long HinksPixExportDialog::ID_BITMAPBUTTON_MOVE_UP = wxNewId();
const long HinksPixExportDialog::ID_BITMAPBUTTON_MOVE_DOWN = wxNewId();
const long HinksPixExportDialog::ID_LISTVIEW_Sequences = wxNewId();
const long HinksPixExportDialog::ID_PANEL1 = wxNewId();
const long HinksPixExportDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT3 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_SD_CARDS = wxNewId();
const long HinksPixExportDialog::ID_BUTTON_REFRESH = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT5 = wxNewId();
const long HinksPixExportDialog::ID_SPINCTRL_START_HOUR = wxNewId();
const long HinksPixExportDialog::ID_SPINCTRL_START_MIN = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT6 = wxNewId();
const long HinksPixExportDialog::ID_SPINCTRL_END_HOUR = wxNewId();
const long HinksPixExportDialog::ID_SPINCTRL_END_MIN = wxNewId();
const long HinksPixExportDialog::ID_BUTTON_EXPORT = wxNewId();
//*)

const long HinksPixExportDialog::ID_MNU_SELECTALL = wxNewId();
const long HinksPixExportDialog::ID_MNU_SELECTNONE = wxNewId();
const long HinksPixExportDialog::ID_MNU_SELECTHIGH = wxNewId();
const long HinksPixExportDialog::ID_MNU_DESELECTHIGH = wxNewId();

BEGIN_EVENT_TABLE(HinksPixExportDialog,wxDialog)
	//(*EventTable(HinksPixExportDialog)
	//*)
END_EVENT_TABLE()

inline void write2ByteUInt(uint8_t* data, uint32_t v) {
    data[0] = (uint8_t)(v & 0xFF);
    data[1] = (uint8_t)((v >> 8) & 0xFF);
}

inline void write4ByteUInt(uint8_t* data, uint32_t v) {
    data[0] = (uint8_t)(v & 0xFF);
    data[1] = (uint8_t)((v >> 8) & 0xFF);
    data[2] = (uint8_t)((v >> 16) & 0xFF);
    data[3] = (uint8_t)((v >> 24) & 0xFF);
}

void HSEQFile::writeHeader() {
    //this format was copied from Joe's HSA 2.0 JavaScript sourcecode
    //320 bytes of HSEQ header data 
    //16 bytes of garbage???, so I wrote part of old FESQ header, probably doesn't matter
    static int fixedHeaderLength = 336;
    // data offset
    uint32_t dataOffset = fixedHeaderLength;
    uint8_t header[336];
    memset(header, 0, 336);
    header[0] = 'H';
    header[1] = 'S';
    header[2] = 'E';
    header[3] = 'Q';
    header[4] = 3; //format version

    //header[9] = num_of_Addition_Controllers; //# of Slave Controllers

    write2ByteUInt(&header[16], ((44100 * m_seqStepTime) / 1000));//framerate
    write4ByteUInt(&header[20], m_seqNumFrames);//number of frames

    write4ByteUInt(&header[24], m_seqChannelCount);//total channel count of all controllers
    write2ByteUInt(&header[68], m_seqChannelCount);//total channel count of master controllers

    strcpy((char*)&header[28], (&_ipAdress)->c_str());//IP of master controllers

    header[320] = 'P';
    header[321] = 'S';
    header[322] = 'E';
    header[323] = 'Q';
    header[324] = 0x40;
    header[327] = 1;
    header[328] = 28;

    write2ByteUInt(&header[330], _orgChannelCount);

    write2ByteUInt(&header[334], m_seqNumFrames);

    write(header, 336);
    m_seqChanDataOffset = dataOffset;
    dumpInfo(false);
}

HinksPixExportDialog::HinksPixExportDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;

	//(*Initialize(HinksPixExportDialog)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer2;
	wxButton* cancelButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, _("HinksPix Export"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_3DSASH, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinimumPaneSize(10);
	SplitterWindow1->SetSashGravity(0.5);
	Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(1);
	StaticText4 = new wxStaticText(Panel2, ID_STATICTEXT4, _("HinksPix Controllers:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer5->Add(StaticText4, 0, wxALL|wxFIXED_MINSIZE, 5);
	CheckListBoxControllers = new wxCheckListBox(Panel2, ID_CHECKLISTBOX_CONTROLLERS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOX_CONTROLLERS"));
	FlexGridSizer5->Add(CheckListBoxControllers, 1, wxALL|wxEXPAND, 5);
	Panel2->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(Panel2);
	FlexGridSizer5->SetSizeHints(Panel2);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 6, 0, 0);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFilter = new wxChoice(Panel1, ID_CHOICE_FILTER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FILTER"));
	ChoiceFilter->SetSelection( ChoiceFilter->Append(_("Recursive Search")) );
	ChoiceFilter->Append(_("Only Current Directory"));
	FlexGridSizer3->Add(ChoiceFilter, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Folder:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFolder = new wxChoice(Panel1, ID_CHOICE_FOLDER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FOLDER"));
	FlexGridSizer3->Add(ChoiceFolder, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxEXPAND, 0);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	BitmapButtonMoveUp = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_MOVE_UP, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_UP")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_MOVE_UP"));
	BoxSizer2->Add(BitmapButtonMoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	BitmapButtonMoveDown = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_MOVE_DOWN, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_DOWN")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_MOVE_DOWN"));
	BoxSizer2->Add(BitmapButtonMoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer2->Add(BoxSizer2, 1, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	CheckListBox_Sequences = new wxListView(Panel1, ID_LISTVIEW_Sequences, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTVIEW_Sequences"));
	FlexGridSizer2->Add(CheckListBox_Sequences, 1, wxEXPAND, 0);
	Panel1->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel1);
	FlexGridSizer2->SetSizeHints(Panel1);
	SplitterWindow1->SplitHorizontally(Panel2, Panel1);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("SD Card:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer1->Add(StaticText3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceSDCards = new wxChoice(this, ID_CHOICE_SD_CARDS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_SD_CARDS"));
	ChoiceSDCards->SetMinSize(wxSize(40,-1));
	BoxSizer1->Add(ChoiceSDCards, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	AddRefreshButton = new wxButton(this, ID_BUTTON_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REFRESH"));
	AddRefreshButton->SetMinSize(wxSize(50,-1));
	BoxSizer1->Add(AddRefreshButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Start Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	BoxSizer1->Add(StaticText5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlStartHour = new wxSpinCtrl(this, ID_SPINCTRL_START_HOUR, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 23, 0, _T("ID_SPINCTRL_START_HOUR"));
	SpinCtrlStartHour->SetValue(_T("0"));
	SpinCtrlStartHour->SetMinSize(wxSize(50,-1));
	BoxSizer1->Add(SpinCtrlStartHour, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlStartMin = new wxSpinCtrl(this, ID_SPINCTRL_START_MIN, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 59, 0, _T("ID_SPINCTRL_START_MIN"));
	SpinCtrlStartMin->SetValue(_T("0"));
	SpinCtrlStartMin->SetMinSize(wxSize(50,-1));
	BoxSizer1->Add(SpinCtrlStartMin, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("End Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	BoxSizer1->Add(StaticText6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlEndHour = new wxSpinCtrl(this, ID_SPINCTRL_END_HOUR, _T("23"), wxDefaultPosition, wxDefaultSize, 0, 0, 23, 23, _T("ID_SPINCTRL_END_HOUR"));
	SpinCtrlEndHour->SetValue(_T("23"));
	SpinCtrlEndHour->SetMinSize(wxSize(50,-1));
	BoxSizer1->Add(SpinCtrlEndHour, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlEndMin = new wxSpinCtrl(this, ID_SPINCTRL_END_MIN, _T("59"), wxDefaultPosition, wxDefaultSize, 0, 0, 59, 59, _T("ID_SPINCTRL_END_MIN"));
	SpinCtrlEndMin->SetValue(_T("59"));
	SpinCtrlEndMin->SetMinSize(wxSize(50,-1));
	BoxSizer1->Add(SpinCtrlEndMin, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(0,0,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Export = new wxButton(this, ID_BUTTON_EXPORT, _("Export to SD Card"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EXPORT"));
	BoxSizer1->Add(Button_Export, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_CANCEL"));
	BoxSizer1->Add(cancelButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE_FILTER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&HinksPixExportDialog::OnChoiceFilterSelect);
	Connect(ID_CHOICE_FOLDER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&HinksPixExportDialog::OnChoiceFolderSelect);
	Connect(ID_BITMAPBUTTON_MOVE_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnBitmapButtonMoveUpClick);
	Connect(ID_BITMAPBUTTON_MOVE_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnBitmapButtonMoveDownClick);
	Connect(ID_LISTVIEW_Sequences,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&HinksPixExportDialog::SequenceListPopup);
	Connect(ID_BUTTON_REFRESH,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnAddRefreshButtonClick);
	Connect(ID_BUTTON_EXPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnButton_ExportClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&HinksPixExportDialog::OnClose);
	//*)

    CheckListBox_Sequences->EnableCheckBoxes();

    CreateDriveList();

    wxConfigBase* config = wxConfigBase::Get();

    wxString controllers = "";
    if (config != nullptr) {
        config->Read("HinksPixExportSelectedControllers", &controllers);
    }

    PopulateControllerList(controllers);
    GetFolderList(xLightsFrame::CurrentDir);

    if (config != nullptr) {
        wxString folderSelect = "";
        wxString driveSelect = "";
        int const filterSelect = config->ReadLong("HinksPixExportFilterSelection", -1);
        config->Read("HinksPixExportFolderSelection", &folderSelect);
        config->Read("HinksPixExportDriveSelection", &driveSelect);
        if (filterSelect != wxNOT_FOUND)
        {
            ChoiceFilter->SetSelection(filterSelect);
        }
        int const ifoldSelect = ChoiceFolder->FindString(folderSelect);
        if (ifoldSelect != wxNOT_FOUND)
        {
            ChoiceFolder->SetSelection(ifoldSelect);
        }
        else
        {
            ChoiceFolder->SetSelection(0);
        }

        int const idriveSelect = ChoiceSDCards->FindString(driveSelect);
        if (idriveSelect != wxNOT_FOUND)
        {
            ChoiceSDCards->SetSelection(idriveSelect);
        }
        SpinCtrlStartHour->SetValue(config->ReadLong("HinksPixExportStartHour", 0));
        SpinCtrlStartMin->SetValue(config->ReadLong("HinksPixExportStartMin", 0));
        SpinCtrlEndHour->SetValue(config->ReadLong("HinksPixExportEndHour", 23));
        SpinCtrlEndMin->SetValue(config->ReadLong("HinksPixExportEndMin", 59));
    }

    LoadSequences();

    int h = SplitterWindow1->GetSize().GetHeight();
    h *= 33;
    h /= 100;
    SplitterWindow1->SetSashPosition(h);
}

void HinksPixExportDialog::PopulateControllerList(wxString const& savedIPs)
{
    auto const controllers = _outputManager->GetControllers();
    for (const auto& it : controllers)
    {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth->GetIP() != "MULTICAST" && eth->GetProtocol() != OUTPUT_ZCPP && eth->GetVendor() == "HinksPix" && eth->IsManaged())
        {
            _hixControllers.push_back(eth);
            CheckListBoxControllers->AppendString(eth->GetIP() + " " + it->GetName() + " " + it->GetModel() + " " + it->GetDescription());
            if (savedIPs.Contains(eth->GetIP()))
            {
                CheckListBoxControllers->Check(CheckListBoxControllers->GetCount() - 1);
            }
        }
    }
}

void HinksPixExportDialog::OnPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    if (id == ID_MNU_SELECTALL) {
        for (int i = 0; i < CheckListBox_Sequences->GetItemCount(); i++) {
            if (!CheckListBox_Sequences->IsItemChecked(i)) {
                CheckListBox_Sequences->CheckItem(i, true);
            }
        }
    } else if (id == ID_MNU_SELECTNONE) {
        for (int i = 0; i < CheckListBox_Sequences->GetItemCount(); i++) {
            if (CheckListBox_Sequences->IsItemChecked(i)) {
                CheckListBox_Sequences->CheckItem(i, false);
            }
        }
    } else if (id == ID_MNU_SELECTHIGH) {
        for (int i = 0; i < CheckListBox_Sequences->GetItemCount(); i++) {
            if (!CheckListBox_Sequences->IsItemChecked(i) && CheckListBox_Sequences->IsSelected(i)) {
                CheckListBox_Sequences->CheckItem(i, true);
            }
        }
    } else if (id == ID_MNU_DESELECTHIGH) {
        for (int i = 0; i < CheckListBox_Sequences->GetItemCount(); i++) {
            if (CheckListBox_Sequences->IsItemChecked(i) && CheckListBox_Sequences->IsSelected(i)) {
                CheckListBox_Sequences->CheckItem(i, false);
            }
        }
    }
}

HinksPixExportDialog::~HinksPixExportDialog()
{
	//(*Destroy(HinksPixExportDialog)
	//*)
}

void HinksPixExportDialog::LoadSequencesFromFolder(wxString dir) const
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning folder for sequences for FPP upload: %s", (const char *)dir.c_str());

    const wxString fseqDir = xLightsFrame::FseqDir;

    wxDir directory;
    directory.Open(dir);

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.xml");
    static const int BUFFER_SIZE = 1024*12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer
    while (fcont) {
        if (file != "xlights_rgbeffects.xml" && file != OutputManager::GetNetworksFileName() && file != "xlights_keybindings.xml") {
            // this could be a sequence file ... lets open it and check
            //just check if <xsequence" is in the first 512 bytes, parsing every XML is way too expensive
            wxFile doc(dir + wxFileName::GetPathSeparator() + file);
            SP_XmlPullParser *parser = new SP_XmlPullParser();
            size_t read = doc.Read(&buf[0], BUFFER_SIZE);
            parser->append(&buf[0], read);
            SP_XmlPullEvent * event = parser->getNext();
            int done = 0;
            int count = 0;
            bool isSequence = false;
            bool isMedia = false;
            std::string mediaName;


            while (!done) {
                if (!event) {
                    size_t read2 = doc.Read(&buf[0], BUFFER_SIZE);
                    if (read2 == 0) {
                        done = true;
                    } else {
                        parser->append(&buf[0], read2);
                    }
                } else {
                    switch (event->getEventType()) {
                        case SP_XmlPullEvent::eEndDocument:
                            done = true;
                            break;
                        case SP_XmlPullEvent::eStartTag: {
                                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                                wxString NodeName = wxString::FromAscii(stagEvent->getName());
                                count++;
                                if (NodeName == "xsequence") {
                                    isSequence = true;
                                } else if (NodeName == "mediaFile") {
                                    isMedia = true;
                                } else {
                                    isMedia = false;
                                }
                                if (count == 100) {
                                    //media file will be very early in the file, dont waste time;
                                    done = true;
                                }
                            }
                            break;
                        case SP_XmlPullEvent::eCData:
                            if (isMedia) {
                                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                                mediaName = wxString::FromAscii(stagEvent->getText()).ToStdString();
                                done = true;
                            }
                            break;
                    }
                }
                if (!done) {
                    event = parser->getNext();
                }
            }
            delete parser;

            xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());

            std::string fseqName = frame->GetFseqDirectory() + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
            if (isSequence) {
                //need to check for existence of fseq
                if (!wxFile::Exists(fseqName)) {
                    fseqName = dir + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
                }
                if (!wxFile::Exists(fseqName)) {
                    isSequence = false;
                }
            }
            if (mediaName != "") {
                if (!wxFile::Exists(mediaName)) {
                    wxFileName fn(mediaName);
                    std::string tmn = frame->mediaDirectory + wxFileName::GetPathSeparator() + fn.GetFullName();
                    if (wxFile::Exists(tmn)) {
                        mediaName = tmn;
                    } else {
                        const std::string fixedMN = FixFile(frame->CurrentDir, mediaName);
                        if (!wxFile::Exists(fixedMN)) {
                            logger_base.info("Could not find media: %s  OR   %s   OR   %s", mediaName.c_str(), tmn.c_str(), fixedMN.c_str());
                            mediaName = "";
                        } else {
                            mediaName = fixedMN;
                        }
                    }
                }
            }
            logger_base.debug("XML:  %s   IsSeq:  %d    FSEQ:  %s   Media:  %s", file.ToStdString().c_str(), isSequence, fseqName.c_str(), mediaName.c_str());
            if (isSequence) {
                long index = CheckListBox_Sequences->GetItemCount();
                CheckListBox_Sequences->InsertItem(index, fseqName);
                if (mediaName != "") {
                    CheckListBox_Sequences->SetItem(index, 1, mediaName);
                }
            }
        }
        fcont = directory.GetNext(&file);
    }
    if (ChoiceFilter->GetSelection() == 0)
    {
        fcont = directory.GetFirst(&file, wxEmptyString, wxDIR_DIRS);
        while (fcont)
        {
            if (file != "Backup")
            {
                LoadSequencesFromFolder(dir + wxFileName::GetPathSeparator() + file);
            }
            fcont = directory.GetNext(&file);
        }
    }
}

void HinksPixExportDialog::LoadSequences()
{
    CheckListBox_Sequences->ClearAll();
    CheckListBox_Sequences->AppendColumn("Sequence");
    CheckListBox_Sequences->AppendColumn("Media");

    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxString freqDir = frame->GetFseqDirectory();

    if (ChoiceFolder->GetSelection() == 0)
    {
        LoadSequencesFromFolder(xLightsFrame::CurrentDir);
    }
    else
    {
        const wxString folder = ChoiceFolder->GetString(ChoiceFolder->GetSelection());
        LoadSequencesFromFolder(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder);
        freqDir = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder;
    }

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr)
    {
        const wxString itcsv = config->Read("HinksPixExportSelectedSequences", wxEmptyString);
        if (!itcsv.IsEmpty())
        {
            wxArrayString savedUploadItems = wxSplit(itcsv, ',');

            for (int x = 0; x < CheckListBox_Sequences->GetItemCount(); x++)
            {
                if (savedUploadItems.Index(CheckListBox_Sequences->GetItemText(x)) != wxNOT_FOUND)
                    CheckListBox_Sequences->CheckItem(x, true);
            }

            int index = 0;
            for (wxString const& seq: savedUploadItems)
            {
                int const i = CheckListBox_Sequences->FindItem(0, seq);
                if (i != -1)
                {
                    moveSequenceItem(index, i, false);
                    index++;
                }
            }
        }
    }

    CheckListBox_Sequences->SetColumnWidth(0, wxLIST_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

void HinksPixExportDialog::CreateDriveList()
{
    ChoiceSDCards->Clear();

#ifdef __WXMSW__
    wxArrayString ud = wxFSVolume::GetVolumes(wxFS_VOL_REMOVABLE | wxFS_VOL_MOUNTED, 0);
    for (const auto &a : ud) {
        ChoiceSDCards->AppendString(a);
    }
#elif defined(__WXOSX__)
    wxDir d;
    d.Open("/Volumes");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont)
    {
        ChoiceSDCards->AppendString("/Volumes/" + dir + "/");
        fcont = d.GetNext(&dir);
    }
#else
    bool done = false;
    wxDir d;
    d.Open("/media");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont) {
        wxDir d2;
        d2.Open("/media/" + dir);
        wxString dir2;
        bool fcont2 = d2.GetFirst(&dir2, wxEmptyString, wxDIR_DIRS);
        while (fcont2) 
        {
            ChoiceSDCards->AppendString("/media/" + dir + "/" + dir2);
            fcont2 = d2.GetNext(&dir2);
        }
        fcont = d.GetNext(&dir);
    }
#endif

}

void HinksPixExportDialog::SaveSettings()
{
    wxString selectedFSEQ = "";
    for (int fs = 0; fs < CheckListBox_Sequences->GetItemCount(); fs++) {
        if (CheckListBox_Sequences->IsItemChecked(fs)) {
            if (selectedFSEQ != "") {
                selectedFSEQ += ",";
            }
            selectedFSEQ += CheckListBox_Sequences->GetItemText(fs);
        }
    }

    wxString selectedController = "";
    for (int i = 0; i < CheckListBoxControllers->GetCount(); i++) {
        if (CheckListBoxControllers->IsChecked(i)) {
            if (selectedController != "") {
                selectedController += ",";
            }
            selectedController += _hixControllers[i]->GetIP();
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("HinksPixExportSelectedSequences", selectedFSEQ);
    config->Write("HinksPixExportSelectedControllers", selectedController);
    config->Write("HinksPixExportFilterSelection", ChoiceFilter->GetSelection());
    config->Write("HinksPixExportFolderSelection", ChoiceFolder->GetString(ChoiceFolder->GetSelection()));
    config->Write("HinksPixExportDriveSelection", ChoiceSDCards->GetString(ChoiceSDCards->GetSelection()));
    config->Write("HinksPixExportStartHour", SpinCtrlStartHour->GetValue());
    config->Write("HinksPixExportStartMin", SpinCtrlStartMin->GetValue());
    config->Write("HinksPixExportEndHour", SpinCtrlEndHour->GetValue());
    config->Write("HinksPixExportEndMin", SpinCtrlEndMin->GetValue());
    config->Flush();
}

void HinksPixExportDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(0);
}

void HinksPixExportDialog::SequenceListPopup(wxListEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_SELECTALL, "Select All");
    mnu.Append(ID_MNU_SELECTNONE, "Clear Selections");
    mnu.Append(ID_MNU_SELECTHIGH, "Select Highlighted");
    mnu.Append(ID_MNU_DESELECTHIGH, "Deselect Highlighted");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&HinksPixExportDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void HinksPixExportDialog::OnChoiceFolderSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void HinksPixExportDialog::OnChoiceFilterSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void HinksPixExportDialog::GetFolderList(const wxString& folder)
{
    ChoiceFolder->Append("--Show Folder--");
    wxArrayString subfolders;
    wxDir dir(folder);
    if (!dir.IsOpened())
    {
        return;
    }
    wxString strFile;

    if (dir.GetFirst(&strFile, "*", wxDIR_HIDDEN | wxDIR_DIRS))
        subfolders.Add(strFile);

    while (dir.GetNext(&strFile))
    {
        subfolders.Add(strFile);
    }
    subfolders.Sort();
    for (const auto& subfolder: subfolders)
    {
        if(subfolder.StartsWith("Backup"))
            continue;
        if (subfolder.StartsWith("."))
            continue;
        ChoiceFolder->Append(subfolder);
    }
}

void HinksPixExportDialog::OnAddRefreshButtonClick(wxCommandEvent& event)
{
    CreateDriveList();
}

void HinksPixExportDialog::OnButton_ExportClick(wxCommandEvent& event)
{
    wxString const drive = ChoiceSDCards->GetString(ChoiceSDCards->GetSelection());

    if (drive.IsEmpty())
    {
        DisplayError("No USB Drive Selected.");
        return;
    }

    wxProgressDialog prgs("Generating HinksPix Files", "Generating HinksPix Files",
        CheckListBoxControllers->GetCount() * (CheckListBox_Sequences->GetItemCount()*2) + (CheckListBoxControllers->GetCount()*2) +1,
        this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prgs.Show();

    wxArrayInt ch;
    bool error = false;
    wxString errorMsg;
    CheckListBoxControllers->GetCheckedItems(ch);
    int count = 0;
    for (auto const& index :ch)
    {
        wxString const ip = _hixControllers[index]->GetIP();
        prgs.Update(++count, "Generating HinksPix Files for " + ip);
        wxString controllerDrive = drive;
        if (ch.Count() > 1)
        {
            controllerDrive = drive + ip + wxFileName::GetPathSeparator();
            if (!wxDir::Exists(controllerDrive))
                wxMkdir(controllerDrive);
        }

        int startChan = INT32_MAX;
        int endChan = 0;
        auto const outputs = _outputManager->GetAllOutputs(ip, "");
        for (auto it = outputs.begin(); it != outputs.end(); ++it)
        {
            if ((*it)->GetStartChannel() < startChan) startChan = (*it)->GetStartChannel();
            if ((*it)->GetEndChannel() > endChan) endChan = (*it)->GetEndChannel();
        }

        std::vector<std::tuple<wxString, wxString>> songs;
        std::vector<wxString> names;

        for (int fs = 0; fs < CheckListBox_Sequences->GetItemCount(); fs++)
        {
            if (CheckListBox_Sequences->IsItemChecked(fs))
            {
                wxString const fseq = CheckListBox_Sequences->GetItemText(fs);
                wxString const media = CheckListBox_Sequences->GetItemText(fs, 1);
                wxString const shortName = createUniqueShortName(fseq, names);
                wxString const shortHseqName = shortName + ".hseq";
                prgs.Update(++count, "Generating HSEQ File " + shortHseqName);

                bool worked = Create_HinksPix_HSEQ_File(fseq, controllerDrive + wxFileName::GetPathSeparator() + shortHseqName, ip, startChan, endChan, errorMsg);

                wxString auName;
                if (worked && !media.IsEmpty())
                {
                    uint32_t Size_PCM_Bytes;
                    uint32_t Sample_Rate_Used;
                    wxString tempFile;
                    auName = shortName + ".au";
                    prgs.Update(++count, "Generating AU File " + auName);
                    worked &= Create_HinksPix_PCM_File(media, tempFile, &Size_PCM_Bytes, &Sample_Rate_Used, errorMsg);

                    if (worked)
                        worked &= Make_AU_From_PCM(tempFile, controllerDrive + wxFileName::GetPathSeparator() + auName, Size_PCM_Bytes, Sample_Rate_Used, errorMsg);
                }

                if (worked)
                {
                    songs.push_back(std::make_tuple(shortHseqName, auName));
                    names.push_back(shortName);
                }
                else
                    error = true;
            }
        }
        prgs.Update(++count, "Generating Schedule File");
        createPlayList(songs, controllerDrive);
        createSchedule(controllerDrive);
    }

    SaveSettings();
    prgs.Hide();
    if(error)
        DisplayError("HinksPix File Generation Error\n" + errorMsg);
    else
        EndDialog(wxID_CLOSE);
}

void HinksPixExportDialog::OnBitmapButtonMoveDownClick(wxCommandEvent& event)
{
    for (int i = CheckListBox_Sequences->GetItemCount()-1; i >=0 ; --i)
    {
        if (CheckListBox_Sequences->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
        {
            if (i == CheckListBox_Sequences->GetItemCount() - 1) return;
            moveSequenceItem(i+1,i);
        }
    }
}

void HinksPixExportDialog::OnBitmapButtonMoveUpClick(wxCommandEvent& event)
{
    for (int i = 0; i < CheckListBox_Sequences->GetItemCount(); ++i)
    {
        if (CheckListBox_Sequences->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
        {
            if (i == 0) return;
            moveSequenceItem(i-1, i);
        }
    }
}

void HinksPixExportDialog::moveSequenceItem(int to, int from, bool select)
{
    bool const checked = CheckListBox_Sequences->IsItemChecked(to);
    wxString const fseq = CheckListBox_Sequences->GetItemText(to);
    wxString const audio = CheckListBox_Sequences->GetItemText(to, 1);
    CheckListBox_Sequences->CheckItem(to, CheckListBox_Sequences->IsItemChecked(from));
    CheckListBox_Sequences->SetItem(to, 0, CheckListBox_Sequences->GetItemText(from));
    CheckListBox_Sequences->SetItem(to, 1, CheckListBox_Sequences->GetItemText(from, 1));
    CheckListBox_Sequences->CheckItem(from, checked);
    CheckListBox_Sequences->SetItem(from, 0, fseq);
    CheckListBox_Sequences->SetItem(from, 1, audio);
    if (select)
    {
        CheckListBox_Sequences->Select(to);
        CheckListBox_Sequences->Select(from, false);
    }
}

void HinksPixExportDialog::createPlayList(std::vector<std::tuple<wxString, wxString>> const& songs, wxString const& drive)
{
    wxArrayString main;

    int i = 0;
    for (std::tuple<wxString, wxString> const& song : songs)
    {
        wxString audio = std::get<1>(song);
        if (audio.IsEmpty()) audio = "NONE";

        main.Add(wxString::Format("{\"H\":\"%s\",\"A\":\"%s\",\"D\":5}", std::get<0>(song), audio));
    }

    wxString const filename = drive + "MAIN.ply";
    wxFile f;

    f.Open(filename, wxFile::write);

    if (f.IsOpened())
    {
        f.Write("[");
        f.Write(wxJoin(main, ',', '\0'));
        f.Write("]");
        f.Close();
    }
}

void HinksPixExportDialog::createSchedule(wxString const& drive)
{
    std::vector<wxString> const days{ "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" };
    for (auto const &day : days)
    {
        wxFile f;
        wxString const filename = drive + day + ".sched";

        f.Open(filename, wxFile::write);

        if (f.IsOpened())
        {
            auto const line = wxString::Format("[{\"S\":\"%d%02d\",\"E\":\"%d%02d\",\"P\":\"MAIN.ply\",\"Q\":0}]",
                SpinCtrlStartHour->GetValue(),SpinCtrlStartMin->GetValue(), SpinCtrlEndHour->GetValue(), SpinCtrlEndMin->GetValue());
            f.Write(line);
            f.Close();
        }
    }
}

bool HinksPixExportDialog::Create_HinksPix_HSEQ_File(wxString const& fseqFile, wxString const& shortHSEQName, wxString const ipAddress, int const startChan, int const endChan, wxString & errorMsg)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug(wxString::Format("HinksPix HSEQ Creation from %s", fseqFile));

    std::unique_ptr<FSEQFile> xf(FSEQFile::openFSEQFile(fseqFile));
    if (!xf)
    {
        errorMsg = wxString::Format("HinksPix Failed opening FSEQ %s", fseqFile);
        logger_base.error(errorMsg);
        return false;
    }

    uint32_t const xNumber_of_Frames = xf->getNumFrames();
    uint32_t const numChannels = xf->getChannelCount();
    int const xFrame_Rate = xf->getStepTime();
    if (xFrame_Rate != 50)
    {
        errorMsg = wxString::Format("HinksPix Failed Framerate must be 50Ms FSEQ %s", fseqFile);
        logger_base.error(errorMsg);
        return false;
    }

    std::vector<std::pair<uint32_t, uint32_t>> rng;
    rng.push_back(std::pair<uint32_t, uint32_t>(0, numChannels));
    xf->prepareRead(rng);

    // read file ready -- do write file
    std::unique_ptr<FSEQFile> ef(new HSEQFile(shortHSEQName, ipAddress, numChannels));
    if (!ef)
    {
        errorMsg = wxString::Format("HinksPix Failed Write opening FSEQ %s", shortHSEQName);
        logger_base.error(errorMsg);
        return false;
    }

    // acquire channel data on all controllers
   int ef_Num_Channel_To_Write = endChan - startChan + 1;

    uint8_t* src, * dest;

    ef->setChannelCount(ef_Num_Channel_To_Write);
    ef->setStepTime(xFrame_Rate);
    ef->setNumFrames(xNumber_of_Frames);

    ef->writeHeader();	// ready for frame data

    uint8_t* WriteBuf = new uint8_t[ef_Num_Channel_To_Write];

    // read buff
    uint8_t* tmpBuf = new uint8_t[numChannels];

    uint32_t frame = 0;

    //mostly copied from Joe's pull request(#1441) in Jan 2019
    while (frame < xNumber_of_Frames)
    {
        FSEQFile::FrameData* data = xf->getFrame(frame);

        data->readFrame(tmpBuf, numChannels);	// we have a read frame

        // move wanted write channels into write frame buffer
        src = tmpBuf + startChan-1;		 // start of my data with global channel array
        dest = WriteBuf;
        memmove(dest, src, ef_Num_Channel_To_Write);
        dest += ef_Num_Channel_To_Write;

        ef->addFrame(frame, WriteBuf);

        delete data;
        frame++;
    }

    ef->finalize();

    delete[]tmpBuf;
    delete[]WriteBuf;

    logger_base.debug(wxString::Format("HinksPix Completed HSEQ %s", shortHSEQName));
    return true;
}

wxString HinksPixExportDialog::createUniqueShortName(wxString const& fseqName, std::vector<wxString> const& names)
{
    //max Length is 20 according to HSA
    wxFileName fn(fseqName);
    wxString newfseqName = fn.GetName().Upper();
    newfseqName.erase(std::remove_if(newfseqName.begin(), newfseqName.end(), [](unsigned char c) { return !std::isalnum(c); }), newfseqName.end());
    if (newfseqName.Length() > 20)
        newfseqName = newfseqName.Left(20);

    //make sure name is unique
    int index = 1;
    while (std::find_if(names.begin(), names.end(), [newfseqName](auto const& e) { return newfseqName == e; }) != names.end())
    {
        //if not, add number to end
        auto const& suffix = wxString::Format("%d", index);
        if ((newfseqName.Length() + suffix.Length()) > 20)
            newfseqName = newfseqName.Left(20 - suffix.Length());
        newfseqName = newfseqName + suffix;
    }
    return newfseqName;
}

bool HinksPixExportDialog::Create_HinksPix_PCM_File(wxString const& inAudioFile, wxString& tmpPCMFile, uint32_t* Size_PCM_Bytes, uint32_t* Sample_Rate_Used, wxString& errorMsg)
{
    //mostly copied from Joe's pull request(#1441) in Jan 2019
    int outSamples;
    char* p;
    int Little_Endian = 0;
    uint16_t* W;
    int i;
    uint32_t Number_PCM_Stereo_Samples = 0;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxFile fo;

    wxFileName fn(inAudioFile);
    wxString newfseqName = fn.GetName().Upper();
    tmpPCMFile = wxFileName::CreateTempFileName(fn.GetName());

    fo.Open(tmpPCMFile, wxFile::write);
    if (!fo.IsOpened())
    {
        errorMsg = wxString::Format("Write Open Failed %s", tmpPCMFile);
        logger_base.error(errorMsg);
        return false;
    }

    // Audio samples are signed 16bit ints.  HinksPix wants audio sample in little-endian
    outSamples = 1;
    p = (char*)&outSamples;
    Little_Endian = *p;

    if (Little_Endian)
        logger_base.debug("HinksPix Audio Decoding is on a Little-Endian machine");
    else
        logger_base.debug("HinksPix Audio Decoding is on a Big-Endian machine");

    // Initialize FFmpeg
    av_register_all();	// make all codecs available

    AVFrame* frame = av_frame_alloc();	// does not create working buffers
    if (!frame)
    {
        errorMsg = wxString::Format("Error allocating the frame %s", inAudioFile);
        logger_base.error(errorMsg);
        return false;
    }

    AVFormatContext* formatContext = NULL;
    if (avformat_open_input(&formatContext, inAudioFile.c_str(), NULL, NULL) != 0)
    {
        av_frame_free(&frame);
        errorMsg = wxString::Format("Error opening the Audio file %s", inAudioFile);
        logger_base.error(errorMsg);
        return false;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0)
    {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        errorMsg = wxString::Format("Error finding the Audio stream info %s", inAudioFile);
        logger_base.error(errorMsg);
        return false;
    }

    // Find the audio stream
    AVCodec* cdc = nullptr;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
    if (streamIndex < 0)
    {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        errorMsg = wxString::Format("Could not find any audio stream in the file %s", inAudioFile);
        logger_base.error(errorMsg);
        return false;
    }

    AVStream* audioStream = formatContext->streams[streamIndex];
    AVCodecContext* codecContext = audioStream->codec;
    codecContext->codec = cdc;

    if (avcodec_open2(codecContext, codecContext->codec, NULL) != 0)	// open the codec
    {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        errorMsg = wxString::Format("Couldn't open the context with the decoder %s", inAudioFile);
        logger_base.error(errorMsg);
        return false;
    }

    logger_base.debug(wxString::Format("This Audio stream has %d channels and a sample rate of %d Hz", codecContext->channels, codecContext->sample_rate));
    logger_base.debug(wxString::Format("The Audio data is in the format %s", av_get_sample_fmt_name(codecContext->sample_fmt)));

    // setup our conversion format ... we need to conver the input to a standard format before we can process anything
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;

    *Sample_Rate_Used = out_sample_rate;

#define CONVERSION_BUFFER_SIZE 192000
    uint8_t* out_buffer = (uint8_t*)av_malloc(CONVERSION_BUFFER_SIZE * out_channels * 2); // 1 second of audio

    int64_t in_channel_layout = av_get_default_channel_layout(codecContext->channels);

    struct SwrContext* au_convert_ctx = swr_alloc_set_opts(nullptr, out_channel_layout, out_sample_fmt, out_sample_rate,
        in_channel_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, nullptr);

    if (au_convert_ctx == nullptr)
    {
        logger_base.error("DoLoadAudioData: swe_alloc_set_opts was null");
        // let it go as it may be the cause of a crash
        wxASSERT(false);
    }

    swr_init(au_convert_ctx);

    AVPacket readingPacket;
    av_init_packet(&readingPacket);

    // Read the packets in a loop
    while (av_read_frame(formatContext, &readingPacket) == 0)
    {
        if (readingPacket.stream_index == audioStream->index)
        {
            AVPacket decodingPacket = readingPacket;

            // Audio packets can have multiple audio frames in a single packet
            while (decodingPacket.size > 0)
            {
                // Try to decode the packet into a frame
                // Some frames rely on multiple packets, so we have to make sure the frame is finished before
                // we can use it
                int gotFrame = 0;
                int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &decodingPacket);

                if (result >= 0 && gotFrame)
                {
                    decodingPacket.size -= result;
                    decodingPacket.data += result;

                    // We now have a fully decoded audio frame
                    outSamples = swr_convert(au_convert_ctx, &out_buffer, CONVERSION_BUFFER_SIZE, (const uint8_t**)frame->data, frame->nb_samples);

                    // save pcm data

                    Number_PCM_Stereo_Samples += outSamples;

                    if (Little_Endian)
                        fo.Write(out_buffer, (outSamples * out_channels * 2));
                    else // big endian - need to swapp
                    {
                        W = (uint16_t*)out_buffer;

                        for (i = 0; i < (outSamples * 2); i++)	// stereo - two 16 bits words per sample
                        {
                            *W = wxUINT16_SWAP_ALWAYS(*W);
                            W++;
                        }
                    }
                }
                else
                {
                    decodingPacket.size = 0;
                    decodingPacket.data = nullptr;
                }
            }
        }

        // You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
        av_packet_unref(&readingPacket);
    }

    // Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
    // is set, there can be buffered up frames that need to be flushed, so we'll do that
    if (codecContext->codec->capabilities & CODEC_CAP_DELAY)
    {
        av_init_packet(&readingPacket);
        // Decode all the remaining frames in the buffer, until the end is reached
        int gotFrame = 0;
        while (avcodec_decode_audio4(codecContext, frame, &gotFrame, &readingPacket) >= 0 && gotFrame)
        {
            // We now have a fully decoded audio frame
            outSamples = swr_convert(au_convert_ctx, &out_buffer, CONVERSION_BUFFER_SIZE, (const uint8_t**)frame->data, frame->nb_samples);

            // save pcm data
            Number_PCM_Stereo_Samples += outSamples;

            if (Little_Endian)
                fo.Write(out_buffer, (outSamples * out_channels * 2));
            else // big endian - need to swapp
            {
                W = (uint16_t*)out_buffer;

                for (i = 0; i < (outSamples * 2); i++)	// stereo - two 16 bits words per sample
                {
                    *W = wxUINT16_SWAP_ALWAYS(*W);
                    W++;
                }
            }
        }
    }

    fo.Close();

    *Size_PCM_Bytes = Number_PCM_Stereo_Samples * 4;	// stereo - two channels 16 bits each channel

    logger_base.debug(wxString::Format("Audio PCM - %d seconds in duration", (Number_PCM_Stereo_Samples / out_sample_rate)));

    // Clean up!
    swr_free(&au_convert_ctx);
    av_free(out_buffer);
    av_frame_free(&frame);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);

    return true;
}

bool HinksPixExportDialog::Make_AU_From_PCM(wxString const& PCM_File, wxString const& AU_File, uint32_t Size_PCM_Bytes, uint32_t Sample_Rate_Used, wxString& errorMsg)
{
    //this format was copied from Joe's HSA 2.0 JavaScript sourcecode
    uint8_t header[24];
    memset(header, 0, 24);

    uint32_t magic = (0x2e << 24) | (0x73 << 16) | (0x6e << 8) | 0x64;
    write4ByteUInt(&header[0], magic);
    write4ByteUInt(&header[4], 24);

    write4ByteUInt(&header[8], Size_PCM_Bytes);

    write4ByteUInt(&header[12], 3);
    write4ByteUInt(&header[16], 44100);//bitrate
    write4ByteUInt(&header[20], 2);//channels?

    wxFile fo;
    wxFile fi;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    fo.Open(AU_File, wxFile::write);
    if (!fo.IsOpened())
    {
        errorMsg = wxString::Format("Error Creating the AU Audio file %s", AU_File);
        logger_base.error(errorMsg);
        return false;
    }

    fo.Write(&header, sizeof(header));

    fi.Open(PCM_File, wxFile::read);
    if (!fi.IsOpened())
    {
        fo.Close();
        errorMsg = wxString::Format("Error Opening the PCM Audio file %s", PCM_File);
        logger_base.error(errorMsg);
        return false;
    }

    uint8_t* Buff = (uint8_t*)malloc(Size_PCM_Bytes);
    if (Buff == 0)
    {
        fo.Close();
        fi.Close();
        errorMsg = wxString::Format("PCM Buffer is Null %s", PCM_File);
        logger_base.error(errorMsg);
        return false;
    }

    uint32_t r = fi.Read(Buff, Size_PCM_Bytes);
    if (r != Size_PCM_Bytes)
    {
        fo.Close();
        fi.Close();
        errorMsg = wxString::Format("PCM and AU file size mismatch %s %s", PCM_File, AU_File);
        logger_base.error(errorMsg);
        return false;
    }

    fo.Write(Buff, Size_PCM_Bytes);

    fo.Close();
    fi.Close();

    return true;
}
