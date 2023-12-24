//(*InternalHeaders(HinksPixExportDialog)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dir.h>
#include <wx/progdlg.h>
#include <wx/volume.h>
#include <wx/wfstream.h>
#include <wx/artprov.h>
#include <wx/numdlg.h>

#include "AudioManager.h"
#include "ControllerCaps.h"
#include "ControllerUploadData.h"
#include "../ExternalHooks.h"
#include "HinksPixExportDialog.h"
#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include "../outputs/ControllerEthernet.h"
#include "models/ModelManager.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include <log4cpp/Category.hh>

#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../include/spxml-0.5/spxmlparser.hpp"

static const std::string CHECK_COL = "ID_UPLOAD_";
static const std::string MODE_COL = "ID_MODE_";
static const std::string SLAVE1_COL = "ID_SLAVE1_";
static const std::string SLAVE2_COL = "ID_SLAVE2_";
static const std::string DISK_COL = "ID_DISK_";
static const std::string SEL_DISK = "Select Folder";

enum class ScheduleColumn : int { PlayList = 0,
                                  StartHour,
                                  StartMin,
                                  EndHour,
                                  EndMin,
                                  Enabled };

//(*IdInit(HinksPixExportDialog)
const long HinksPixExportDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT3 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_PLAYLISTS = wxNewId();
const long HinksPixExportDialog::ID_BUTTON_ADD_PLAYLIST = wxNewId();
const long HinksPixExportDialog::ID_BUTTON_REMOVE = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT1 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_FILTER = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT2 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_FOLDER = wxNewId();
const long HinksPixExportDialog::ID_BITMAPBUTTON_MOVE_UP = wxNewId();
const long HinksPixExportDialog::ID_BITMAPBUTTON_MOVE_DOWN = wxNewId();
const long HinksPixExportDialog::ID_LISTVIEW_Sequences = wxNewId();
const long HinksPixExportDialog::ID_PANEL1 = wxNewId();
const long HinksPixExportDialog::ID_GRID_SCHEDULE = wxNewId();
const long HinksPixExportDialog::ID_PANEL4 = wxNewId();
const long HinksPixExportDialog::ID_NOTEBOOK_EXPORT_ITEMS = wxNewId();
const long HinksPixExportDialog::ID_BUTTON_REFRESH = wxNewId();
const long HinksPixExportDialog::ID_BUTTON_EXPORT = wxNewId();
const long HinksPixExportDialog::wxID_Close = wxNewId();
//*)

const long HinksPixExportDialog::ID_MNU_SELECTALL = wxNewId();
const long HinksPixExportDialog::ID_MNU_SELECTNONE = wxNewId();
const long HinksPixExportDialog::ID_MNU_SELECTHIGH = wxNewId();
const long HinksPixExportDialog::ID_MNU_DESELECTHIGH = wxNewId();

const long HinksPixExportDialog::ID_MNU_SETALL = wxNewId();
const long HinksPixExportDialog::ID_MNU_SETALLPLAY = wxNewId();

BEGIN_EVENT_TABLE(HinksPixExportDialog, wxDialog)
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

inline std::array<int, 4> getIPBytes(const wxString& ip) {
    wxArrayString const ips = wxSplit(ip, '.');
    if (ips.size() != 4) {
        return { 0, 0, 0, 0 };
    }
    return { wxAtoi(ips[0]), wxAtoi(ips[1]), wxAtoi(ips[2]), wxAtoi(ips[3]) };
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

    int num_of_Addition_Controllers = 0;
    if (m_slave1) {
        num_of_Addition_Controllers++;
    }
    if (m_slave2) {
        num_of_Addition_Controllers++;
    }

    header[9] = num_of_Addition_Controllers; //# of Slave Controllers

    write2ByteUInt(&header[16], ((44100 * m_seqStepTime) / 1000)); //framerate
    write4ByteUInt(&header[20], m_seqNumFrames);                   //number of frames

    write4ByteUInt(&header[24], m_seqChannelCount);     //total channel count of all controllers
    write2ByteUInt(&header[68], m_hinx->GetChannels()); //total channel count of master controllers

    strcpy((char*)&header[28], m_hinx->GetIP().c_str()); //IP of master controllers

    if (m_slave1) {
        write2ByteUInt(&header[76], m_slave1->GetChannels());

        auto const slave1IP = getIPBytes(m_slave1->GetIP());
        header[72] = slave1IP[0];
        header[73] = slave1IP[1];
        header[74] = slave1IP[2];
        header[75] = slave1IP[3];
        int j = 0;
        for (auto* const output : m_slave1->GetOutputs()) {
            write2ByteUInt(&header[80 + (j * 2)], output->GetUniverse());
            write2ByteUInt(&header[148 + (j * 2)], output->GetChannels());
            j++;
        }
    }
    if (m_slave2) {
        write2ByteUInt(&header[220], m_slave2->GetChannels());

        auto const slave2IP = getIPBytes(m_slave2->GetIP());
        header[216] = slave2IP[0];
        header[217] = slave2IP[1];
        header[218] = slave2IP[2];
        header[219] = slave2IP[3];
        int j = 0;
        for (auto* const output : m_slave2->GetOutputs()) {
            write2ByteUInt(&header[224 + (j * 2)], output->GetUniverse());
            write2ByteUInt(&header[258 + (j * 2)], output->GetChannels());
            j++;
        }
    }

    header[320] = 'P';
    header[321] = 'S';
    header[322] = 'E';
    header[323] = 'Q';
    header[324] = 0x40;
    header[327] = 1;
    header[328] = 28;

    write2ByteUInt(&header[330], m_orgChannelCount);

    write2ByteUInt(&header[334], m_seqNumFrames);

    write(header, 336);
    m_seqChanDataOffset = dataOffset;
    dumpInfo(false);
}

void HinksChannelMap::Dump() const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug(" xLights StartChannel %d Channel Count %d Hinkspix StartChannel %d",
                      OrgStartChannel,
                      ChannelCount,
                      HinksStartChannel);
}

HinksPixExportDialog::HinksPixExportDialog(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    m_outputManager(outputManager), m_modelManager(modelManager) {
	//(*Initialize(HinksPixExportDialog)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	wxButton* Button_Close;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, _("HinksPix Export"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	NotebookExportItems = new wxNotebook(this, ID_NOTEBOOK_EXPORT_ITEMS, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_EXPORT_ITEMS"));
	NotebookExportItems->SetMinSize(wxSize(1000,400));
	HinkControllerList = new wxScrolledWindow(NotebookExportItems, ID_SCROLLEDWINDOW1, wxPoint(-124,-53), wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	HinkControllerList->SetMinSize(wxDLG_UNIT(NotebookExportItems,wxSize(-1,150)));
	HinkControllerSizer = new wxFlexGridSizer(0, 8, 0, 0);
	HinkControllerList->SetSizer(HinkControllerSizer);
	HinkControllerSizer->Fit(HinkControllerList);
	HinkControllerSizer->SetSizeHints(HinkControllerList);
	Panel1 = new wxPanel(NotebookExportItems, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("Playlist:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer3->Add(StaticText3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChoicePlaylists = new wxChoice(Panel1, ID_CHOICE_PLAYLISTS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PLAYLISTS"));
	BoxSizer3->Add(ChoicePlaylists, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonAddPlaylist = new wxButton(Panel1, ID_BUTTON_ADD_PLAYLIST, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_PLAYLIST"));
	BoxSizer3->Add(ButtonAddPlaylist, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonRemove = new wxButton(Panel1, ID_BUTTON_REMOVE, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REMOVE"));
	BoxSizer3->Add(ButtonRemove, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer3->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFilter = new wxChoice(Panel1, ID_CHOICE_FILTER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FILTER"));
	ChoiceFilter->SetSelection( ChoiceFilter->Append(_("Recursive Search")) );
	ChoiceFilter->Append(_("Only Current Directory"));
	BoxSizer3->Add(ChoiceFilter, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Folder:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BoxSizer3->Add(StaticText2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFolder = new wxChoice(Panel1, ID_CHOICE_FOLDER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FOLDER"));
	BoxSizer3->Add(ChoiceFolder, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(BoxSizer3, 1, wxEXPAND, 0);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	BitmapButtonMoveUp = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_MOVE_UP, wxArtProvider::GetBitmapBundle("wxART_GO_UP", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_MOVE_UP"));
	BoxSizer2->Add(BitmapButtonMoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	BitmapButtonMoveDown = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_MOVE_DOWN, wxArtProvider::GetBitmapBundle("wxART_GO_DOWN", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_MOVE_DOWN"));
	BoxSizer2->Add(BitmapButtonMoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer2->Add(BoxSizer2, 1, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	CheckListBox_Sequences = new wxListView(Panel1, ID_LISTVIEW_Sequences, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTVIEW_Sequences"));
	FlexGridSizer2->Add(CheckListBox_Sequences, 1, wxEXPAND, 0);
	Panel1->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel1);
	FlexGridSizer2->SetSizeHints(Panel1);
	Panel4 = new wxPanel(NotebookExportItems, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	GridSchedule = new wxGrid(Panel4, ID_GRID_SCHEDULE, wxPoint(0,-112), wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID_SCHEDULE"));
	GridSchedule->CreateGrid(0,6);
	GridSchedule->EnableEditing(true);
	GridSchedule->EnableGridLines(true);
	GridSchedule->SetColLabelValue(0, _("PlayList"));
	GridSchedule->SetColLabelValue(1, _("Start Hour"));
	GridSchedule->SetColLabelValue(2, _("Start Min"));
	GridSchedule->SetColLabelValue(3, _("End Hour"));
	GridSchedule->SetColLabelValue(4, _("End Min"));
	GridSchedule->SetColLabelValue(5, _("Enabled"));
	GridSchedule->SetDefaultCellFont( GridSchedule->GetFont() );
	GridSchedule->SetDefaultCellTextColour( GridSchedule->GetForegroundColour() );
	FlexGridSizer4->Add(GridSchedule, 0, wxEXPAND, 0);
	Panel4->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel4);
	FlexGridSizer4->SetSizeHints(Panel4);
	NotebookExportItems->AddPage(HinkControllerList, _("Controllers"), false);
	NotebookExportItems->AddPage(Panel1, _("Playlists"), false);
	NotebookExportItems->AddPage(Panel4, _("Schedule"), false);
	FlexGridSizer1->Add(NotebookExportItems, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	AddRefreshButton = new wxButton(this, ID_BUTTON_REFRESH, _("Refresh USB Drives"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REFRESH"));
	BoxSizer1->Add(AddRefreshButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Export = new wxButton(this, ID_BUTTON_EXPORT, _("Export to SD Card"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EXPORT"));
	BoxSizer1->Add(Button_Export, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, wxID_Close, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_Close"));
	BoxSizer1->Add(Button_Close, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_ADD_PLAYLIST,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnButtonAddPlaylistClick);
	Connect(ID_BUTTON_REMOVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnButtonRemoveClick);
	Connect(ID_BITMAPBUTTON_MOVE_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnBitmapButtonMoveUpClick);
	Connect(ID_BITMAPBUTTON_MOVE_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnBitmapButtonMoveDownClick);
	Connect(ID_LISTVIEW_Sequences,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&HinksPixExportDialog::SequenceListPopup);
	Connect(ID_GRID_SCHEDULE,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&HinksPixExportDialog::OnGridScheduleCellRightClick);
	Connect(ID_GRID_SCHEDULE,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&HinksPixExportDialog::OnGridScheduleCellChanged);
	Connect(ID_BUTTON_REFRESH,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnAddRefreshButtonClick);
	Connect(ID_BUTTON_EXPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnButton_ExportClick);
	Connect(wxID_Close,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&HinksPixExportDialog::OnButton_CloseClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&HinksPixExportDialog::OnClose);
	//*)

    AddInstanceHeader("Create");
    AddInstanceHeader("IP Address");
    AddInstanceHeader("Description");
    AddInstanceHeader("Model");
    AddInstanceHeader("Mode");
    AddInstanceHeader("Drive");
    AddInstanceHeader("Slave1");
    AddInstanceHeader("Slave2");

    CheckListBox_Sequences->EnableCheckBoxes();

    CreateDriveList();
    PopulateControllerList(outputManager);
    GetFolderList(xLightsFrame::CurrentDir);
    LoadSettings();
}

HinksPixExportDialog::~HinksPixExportDialog() {
    //(*Destroy(HinksPixExportDialog)
    //*)
}

void HinksPixExportDialog::PopulateControllerList(OutputManager* outputManager) {
    HinkControllerList->Freeze();
    //remove all the children from the first upload checkbox on
    wxWindow* w = HinkControllerList->FindWindow(CHECK_COL + "0");
    while (w) {
        wxWindow* tmp = w->GetNextSibling();
        w->Destroy();
        w = tmp;
    }

    auto const controllers = outputManager->GetControllers();
    wxArrayString otherControllers;
    for (const auto& it : controllers) {
        auto* eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && eth->GetIP() != "MULTICAST" && eth->GetProtocol() != OUTPUT_ZCPP && eth->IsManaged()) {
            if (eth->GetVendor() == "HinksPix") {
                m_hixControllers.push_back(eth);
            } else if (eth->GetProtocol() == OUTPUT_E131) {
                m_otherControllers.push_back(eth);
                otherControllers.push_back(eth->GetName());
            }
        }
    }
    int row = 0;
    for (const auto& it : m_hixControllers) {
        std::string rowStr = std::to_string(row);
        wxCheckBox* CheckBox1 = new wxCheckBox(HinkControllerList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, CHECK_COL + rowStr);
        HinkControllerSizer->Add(CheckBox1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        std::string l = it->GetIP();
        wxStaticText* label = new wxStaticText(HinkControllerList, wxID_ANY, l, wxDefaultPosition, wxDefaultSize, 0, _T("ID_IPADDRESS_" + rowStr));
        HinkControllerSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 1);
        label = new wxStaticText(HinkControllerList, wxID_ANY, it->GetName(), wxDefaultPosition, wxDefaultSize, 0, _T("ID_DESCRIPTION_" + rowStr));
        HinkControllerSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 1);
        label = new wxStaticText(HinkControllerList, wxID_ANY, it->GetModel(), wxDefaultPosition, wxDefaultSize, 0, _T("ID_MODEL_" + rowStr));
        HinkControllerSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 1);

        wxChoice* Choice1 = new wxChoice(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, MODE_COL + rowStr);

        Choice1->Append(_("Master"));
        Choice1->Append(_("Remote"));
        Choice1->Append(_("Don't Set"));
        if (it == m_hixControllers.front()) {
            Choice1->SetSelection(0);
        } else {
            Choice1->SetSelection(1);
        }
        HinkControllerSizer->Add(Choice1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);

        Choice1 = new wxChoice(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, DISK_COL + rowStr);
        Choice1->Append(m_drives);

        HinkControllerSizer->Add(Choice1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);

        wxChoice* Choice2 = new wxChoice(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, SLAVE1_COL + rowStr);
        Choice2->Append(_(""));
        wxChoice* Choice3 = new wxChoice(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, SLAVE2_COL + rowStr);
        Choice3->Append(_(""));
        for (wxString const& oth : otherControllers) {
            Choice2->Append(oth);
            Choice3->Append(oth);
        }
        Choice2->SetSelection(0);
        Choice3->SetSelection(0);

        HinkControllerSizer->Add(Choice2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        HinkControllerSizer->Add(Choice3, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        if (otherControllers.size() == 0) {
            Choice2->Enable(false);
            Choice3->Enable(false);
        } else if (otherControllers.size() == 1) {
            Choice3->Enable(false);
        }

        row++;
    }

    if (m_hixControllers.empty()) {
        wxStaticText* label = new wxStaticText(HinkControllerList, wxID_ANY, "No HinksPix Found", wxDefaultPosition, wxDefaultSize, 0, _T("ID_IPADDRESS_0"));
        HinkControllerSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 1);
    }

    Connect(wxID_ANY, wxEVT_CHOICE, (wxObjectEventFunction)&HinksPixExportDialog::OnChoiceSelected);

    HinkControllerList->FitInside();
    HinkControllerList->SetScrollRate(10, 10);
    HinkControllerList->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
    HinkControllerList->Thaw();
}

void HinksPixExportDialog::OnPopup(wxCommandEvent& event) {
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

void HinksPixExportDialog::OnPopupGrid(wxCommandEvent& event)
{
    int id = event.GetId();
    int col = GridSchedule->GetGridCursorCol();
    wxString name = GridSchedule->GetColLabelValue(col);
    int min = 0;
    int max = 59;
    if (id == ID_MNU_SETALL) {
        if (col <= static_cast<int>(ScheduleColumn::EndMin) &&
            col >= static_cast<int>(ScheduleColumn::StartHour)) {
            switch (static_cast<ScheduleColumn>(col)) {
            case ScheduleColumn::StartHour:
            case ScheduleColumn::EndHour:
                max = 23;
                break;
            default:
                break;
            }
            wxNumberEntryDialog dlg(this, "", "Set " + name, "Set " + name, 0, min, max);
            if (dlg.ShowModal() == wxID_OK) {
                for (int i = 0; i < GridSchedule->GetNumberRows(); ++i) {
                    GridSchedule->SetCellValue(i, col, wxString::Format("%i", dlg.GetValue()));
                }
            }
        } else if (col <= static_cast<int>(ScheduleColumn::Enabled)) {
            wxTextEntryDialog dlg(this, "Set " + name, "Set " + name, "X");
            if (dlg.ShowModal() == wxID_OK) {
                for (int i = 0; i < GridSchedule->GetNumberRows(); ++i) {
                    GridSchedule->SetCellValue(i, col, dlg.GetValue());
                }
            }
        }
    } else if (id == ID_MNU_SETALLPLAY) {
        int row = GridSchedule->GetGridCursorRow();
        auto playList = GridSchedule->GetCellValue(row, static_cast<int>(ScheduleColumn::PlayList));
        if (col <= static_cast<int>(ScheduleColumn::EndMin) &&
            col >= static_cast<int>(ScheduleColumn::StartHour)) {
            switch (static_cast<ScheduleColumn>(col)) {
            case ScheduleColumn::StartHour:
            case ScheduleColumn::EndHour:
                max = 23;
                break;
            default:
                break;
            }
            wxNumberEntryDialog dlg(this, "", "Set " + name, "Set " + name, 0, min, max);
            if (dlg.ShowModal() == wxID_OK) {
                for (int i = 0; i < GridSchedule->GetNumberRows(); ++i) {
                    auto row_playList = GridSchedule->GetCellValue(i, static_cast<int>(ScheduleColumn::PlayList));
                    if (row_playList == playList) {
                        GridSchedule->SetCellValue(i, col, wxString::Format("%i", dlg.GetValue()));
                    }
                }
            }
        } else if (col <= static_cast<int>(ScheduleColumn::Enabled)) {
            wxTextEntryDialog dlg(this, "Set " + name, "Set " + name, "X");
            if (dlg.ShowModal() == wxID_OK) {
                for (int i = 0; i < GridSchedule->GetNumberRows(); ++i) {
                    auto row_playList = GridSchedule->GetCellValue(i, static_cast<int>(ScheduleColumn::PlayList));
                    if (row_playList == playList) {
                        GridSchedule->SetCellValue(i, col, dlg.GetValue());
                    }
                }
            }
        }
    }
}

void HinksPixExportDialog::LoadSequencesFromFolder(wxString dir) const {
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning folder for sequences for FPP upload: %s", (const char*)dir.c_str());

    const wxString fseqDir = xLightsFrame::FseqDir;

    wxDir directory;
    directory.Open(dir);

    wxArrayString files;
    GetAllFilesInDir(dir, files, "*.x*");
    static const int BUFFER_SIZE = 1024 * 12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer
    for (auto &filename : files) {
        wxFileName fn(filename);
        wxString file =fn.GetFullName();
        if (file != XLIGHTS_RGBEFFECTS_FILE && file != OutputManager::GetNetworksFileName() && file != XLIGHTS_KEYBINDING_FILE && (file.Lower().EndsWith("xml") || file.Lower().EndsWith("xsq"))
            && FileExists(filename)) {
            // this could be a sequence file ... lets open it and check
            //just check if <xsequence" is in the first 512 bytes, parsing every XML is way too expensive
            wxFile doc(filename);
            SP_XmlPullParser* parser = new SP_XmlPullParser();
            size_t read = doc.Read(&buf[0], BUFFER_SIZE);
            parser->append(&buf[0], read);
            SP_XmlPullEvent* event = parser->getNext();
            bool done = false;
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
                        SP_XmlStartTagEvent* stagEvent = (SP_XmlStartTagEvent*)event;
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
                    } break;
                    case SP_XmlPullEvent::eCData:
                        if (isMedia) {
                            SP_XmlCDataEvent* stagEvent = (SP_XmlCDataEvent*)event;
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
                if (!FileExists(fseqName)) {
                    fseqName = dir + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
                }
                if (!wxFile::Exists(fseqName)) {
                    isSequence = false;
                }
            }
            if (!mediaName.empty()) {
                if (!FileExists(mediaName)) {
                    wxFileName fn(mediaName);
                    for (auto const& md : frame->GetMediaFolders()) {
                        std::string tmn = md + wxFileName::GetPathSeparator() + fn.GetFullName();
                        if (FileExists(tmn)) {
                            mediaName = tmn;
                            break;
                        }
                    }
                    if (!FileExists(mediaName)) {
                        const std::string fixedMN = FixFile(frame->CurrentDir, mediaName);
                        if (!FileExists(fixedMN)) {
                            logger_base.info("Could not find media: %s", mediaName.c_str());
                            mediaName = "";
                        } else {
                            mediaName = fixedMN;
                        }
                    }
                }
            }
            logger_base.debug("XML:  %s   IsSeq:  %d    FSEQ:  %s   Media:  %s", (const char*)file.c_str(), isSequence, (const char*)fseqName.c_str(), (const char*)mediaName.c_str());
            if (isSequence) {
                long index = CheckListBox_Sequences->GetItemCount();
                CheckListBox_Sequences->InsertItem(index, fseqName);
                if (!mediaName.empty()) {
                    CheckListBox_Sequences->SetItem(index, 1, mediaName);
                }
            }
        }
    }
    if (ChoiceFilter->GetSelection() == 0) {
        wxString file;
        bool fcont = directory.GetFirst(&file, wxEmptyString, wxDIR_DIRS);
        while (fcont) {
            if (file != "Backup") {
                LoadSequencesFromFolder(dir + wxFileName::GetPathSeparator() + file);
            }
            fcont = directory.GetNext(&file);
        }
    }
}

void HinksPixExportDialog::LoadSequences() {
    CheckListBox_Sequences->ClearAll();
    CheckListBox_Sequences->AppendColumn("Sequence");
    CheckListBox_Sequences->AppendColumn("Media");

    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxString freqDir = frame->GetFseqDirectory();

    if (ChoiceFolder->GetSelection() == 0) {
        LoadSequencesFromFolder(xLightsFrame::CurrentDir);
    } else {
        const wxString folder = ChoiceFolder->GetString(ChoiceFolder->GetSelection());
        LoadSequencesFromFolder(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder);
        freqDir = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder;
    }

    CheckListBox_Sequences->SetColumnWidth(0, wxLIST_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

void HinksPixExportDialog::CreateDriveList() {
    m_drives.Clear();

#ifdef __WXMSW__
    #ifdef _DEBUG
    wxArrayString ud = wxFSVolume::GetVolumes();
    #else
    wxArrayString ud = wxFSVolume::GetVolumes( wxFS_VOL_REMOVABLE | wxFS_VOL_MOUNTED, 0);
    #endif
    for (const auto& a : ud) {
        m_drives.push_back(a);
    }
#elif defined(__WXOSX__)
    wxDir d;
    d.Open("/Volumes");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont) {
        m_drives.push_back("/Volumes/" + dir);
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
        while (fcont2) {
            m_drives.push_back("/media/" + dir + "/" + dir2);
            fcont2 = d2.GetNext(&dir2);
        }
        fcont = d.GetNext(&dir);
    }
#endif
    m_drives.push_back(SEL_DISK);
}

void HinksPixExportDialog::SaveSettings()
{
    if (!m_selectedPlayList.empty()) {
        StoreToObjectPlayList(m_selectedPlayList);
    }
    StoreToObjectSchedule();
    wxJSONValue data;
    data["filter"] = ChoiceFilter->GetSelection();
    data["folder"] = ChoiceFolder->GetString(ChoiceFolder->GetSelection());
    data["tab"] = NotebookExportItems->GetSelection();

    int row = 0;
    for (const auto& inst : m_hixControllers) {
        std::string rowStr = std::to_string(row);
        wxJSONValue controller;
        controller["ip"] = wxString(inst->GetIP());
        controller["enabled"] = GetCheckValue(CHECK_COL + rowStr);
        controller["mode"] = GetChoiceValue(MODE_COL + rowStr);
        controller["slave1"] = GetChoiceValue(SLAVE1_COL + rowStr);
        controller["slave2"] = GetChoiceValue(SLAVE2_COL + rowStr);
        controller["disk"] = GetChoiceValue(DISK_COL + rowStr);
        data["controllers"].Append(controller);
        row++;
    }

    for (const auto& schedule : m_schedules) {
        data["schedules"].Append(schedule.asJSON());
    }

    for (const auto& playlist : m_playLists) {
        data["playlists"].Append(playlist.asJSON());
    }
    wxFileOutputStream hinksfile(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "hinks_export.json");
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(data, hinksfile);
    hinksfile.Close();
}

void HinksPixExportDialog::LoadSettings()
{
    auto path = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "hinks_export.json";
    bool loaded{false};
    if (FileExists(path)) {
        wxJSONValue data;
        wxJSONReader reader;
        wxFileInputStream f(path);
        int errors = reader.Parse(f, &data);
        if (!errors) {
            wxString folderSelect = data.Get("folder", wxJSONValue(wxString())).AsString();
            int const filterSelect = data.Get("filter", wxJSONValue(-1)).AsInt();

            if (filterSelect != wxNOT_FOUND) {
                ChoiceFilter->SetSelection(filterSelect);
            }
            int ifoldSelect = ChoiceFolder->FindString(folderSelect);
            if (ifoldSelect != wxNOT_FOUND) {
                ChoiceFolder->SetSelection(ifoldSelect);
            }else {
                ChoiceFolder->SetSelection(0);
            }
            auto tab = data.Get("tab", wxJSONValue(0)).AsInt();
            NotebookExportItems->SetSelection(tab);
            LoadSequences();

            auto schedules = data["schedules"].AsArray();
            if (schedules) {
                for (int i = 0; i < schedules->Count(); ++i) {
                    m_schedules.emplace_back(schedules->Item(i));
                }
            }
            auto playlists = data["playlists"].AsArray();
            if (playlists) {
                for (int i = 0; i < playlists->Count(); ++i) {
                    auto play = m_playLists.emplace_back(playlists->Item(i));
                    ChoicePlaylists->AppendString(play.Name);
                }
            }
            loaded = true;
            ApplySavedSettings(data["controllers"]);
        }
    }

    if (m_schedules.empty()) {
        for (auto const& day : DAYS) {
            m_schedules.emplace_back(day);
        }
    }

    if (!loaded) {
        ChoiceFolder->SetSelection(0);
        LoadSequences();
        RedrawPlayList(wxString());
    } else if (ChoicePlaylists->GetStrings().size() > 0) {
        ChoicePlaylists->SetSelection(0);
        RedrawPlayList(ChoicePlaylists->GetStrings()[0], false);
    }
}

void HinksPixExportDialog::ApplySavedSettings(wxJSONValue json)
{
    /*
    static const std::string CHECK_COL = "ID_UPLOAD_";
    static const std::string MODE_COL = "ID_MODE_";
    static const std::string SLAVE1_COL = "ID_SLAVE1_";
    static const std::string SLAVE2_COL = "ID_SLAVE2_";
    static const std::string DISK_COL = "ID_DISK_";
     */

    auto controllers = json.AsArray();
    if (!controllers) {
        return;
    }
    int row = 0;
    for (const auto& hix : m_hixControllers) {
        std::string rowStr = std::to_string(row);

        for (int i = 0; i < controllers->Count(); ++i) {
            auto controller = controllers->Item(i);
            if (hix->GetIP() == controller.ItemAt("ip").AsString()) {
                SetCheckValue(CHECK_COL + rowStr, controller.ItemAt("enabled").AsBool());
                SetChoiceValue(MODE_COL + rowStr, controller.ItemAt("mode").AsString());
                SetChoiceValue(SLAVE1_COL + rowStr, controller.ItemAt("slave1").AsString());
                SetChoiceValue(SLAVE2_COL + rowStr, controller.ItemAt("slave2").AsString());
                SetChoiceValue(DISK_COL + rowStr, controller.ItemAt("disk").AsString());
                break;
            }
        }
        row++;
    }
}

void HinksPixExportDialog::OnClose(wxCloseEvent& /*event*/) {
    SaveSettings();
    EndDialog(0);
}

void HinksPixExportDialog::OnGridScheduleCellChanged(wxGridEvent& event)
{
    CheckSchedules();
}

void HinksPixExportDialog::OnGridScheduleCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_SETALL, "Set All");
    mnu.Append(ID_MNU_SETALLPLAY, "Set All - PlayList");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&HinksPixExportDialog::OnPopupGrid, nullptr, this);
    PopupMenu(&mnu);
}

void HinksPixExportDialog::SequenceListPopup(wxListEvent& /*event*/) {
    wxMenu mnu;
    mnu.Append(ID_MNU_SELECTALL, "Select All");
    mnu.Append(ID_MNU_SELECTNONE, "Clear Selections");
    mnu.Append(ID_MNU_SELECTHIGH, "Select Highlighted");
    mnu.Append(ID_MNU_DESELECTHIGH, "Deselect Highlighted");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&HinksPixExportDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void HinksPixExportDialog::GetFolderList(wxString const& folder) {
    ChoiceFolder->Append("--Show Folder--");
    wxArrayString subfolders;
    wxDir dir(folder);
    if (!dir.IsOpened()) {
        return;
    }
    wxString strFile;

    if (dir.GetFirst(&strFile, "*", wxDIR_HIDDEN | wxDIR_DIRS)) {
        subfolders.Add(strFile);
    }

    while (dir.GetNext(&strFile)) {
        subfolders.Add(strFile);
    }
    subfolders.Sort();
    for (const auto& subfolder : subfolders) {
        if (subfolder.StartsWith("Backup")) {
            continue;
        }
        if (subfolder.StartsWith(".")) {
            continue;
        }
        ChoiceFolder->Append(subfolder);
    }
}

void HinksPixExportDialog::OnAddRefreshButtonClick(wxCommandEvent& /*event*/) {
    CreateDriveList();
    for (int i = 0; i < m_hixControllers.size(); i++) {
        std::string rowStr = std::to_string(i);
        SetDropDownItems(DISK_COL + rowStr, m_drives);
    }
}

void HinksPixExportDialog::OnButton_ExportClick(wxCommandEvent& /*event*/) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!m_selectedPlayList.empty()) {
        StoreToObjectPlayList(m_selectedPlayList);
    }
    StoreToObjectSchedule();

    if (m_playLists.empty()) {
        DisplayError("No Playlists found\nPlease Add a Playlist");
        return;
    }
    int totalProgress{ 0 };
    std::for_each(m_playLists.begin(), m_playLists.end(), [&totalProgress](auto const& pl) { totalProgress += pl.Items.size(); });
    totalProgress *= m_hixControllers.size();
    totalProgress += 2;

    wxProgressDialog prgs(
        "Generating HinksPix Files", "Generating HinksPix Files", totalProgress, this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prgs.Show();

    std::map<wxString, wxString> shortNames;
    std::vector<wxString> names;
    for (auto& playlist : m_playLists) {
        for (auto& play : playlist.Items) {
            if (shortNames.find( play.FSEQ) == shortNames.end()) {
                wxString const shortName = createUniqueShortName(play.FSEQ, names);
                shortNames.insert({ play.FSEQ, shortName });
                names.push_back(shortName);
            }
        }
    }

    bool error = false;
    wxString errorMsg;
    int count = 0;
    int row = 0;
    for (auto* hix : m_hixControllers) {
        std::string const rowStr = std::to_string(row);
        ++row;
        bool upload = GetCheckValue(CHECK_COL + rowStr);
        if (!upload) {
            ++count;
            continue;
        }
        wxString const ip = hix->GetIP();

        prgs.Update(++count, wxString::Format("Generating HinksPix Files for '%s'", hix->GetName()));

        wxString slaveName1 = GetChoiceValue(SLAVE1_COL + rowStr);
        wxString slaveName2 = GetChoiceValue(SLAVE2_COL + rowStr);

        if (!slaveName1.IsEmpty() && slaveName1 == slaveName2) {
            error = true;
            errorMsg = wxString::Format("Slave Controller 1 and 2 cannot not be the same: '%s'", slaveName1);
            continue;
        }

        auto* slave1 = getSlaveController(slaveName1);
        auto* slave2 = getSlaveController(slaveName2);

        if (!slave1 && slave2) {
            std::swap(slave1, slave2);
        }

        if (slave1 && slave2) {
            if (slave2->GetOutputCount() > slave1->GetOutputCount()) {
                std::swap(slave1, slave2);
            }
        }

        if (slave1 || slave2) {
            if (!CheckSlaveControllerSizes(hix, slave1, slave2)) {
                error = true;
                errorMsg = wxString::Format("Too Many Slave Controller Universes for '%s'", hix->GetName());
                continue;
            }
        }

        wxString drive = GetChoiceValue(DISK_COL + rowStr);

        //try to fix path
        wxFileName dirname( drive, "" );
        drive = dirname.GetPath();

        if (drive.IsEmpty() || drive == SEL_DISK) {
            wxDirDialog dlg(this, "Select SD Directory for " + hix->GetName(), drive, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK) {
                drive = dlg.GetPath();
            } else {
                error = true;
                errorMsg = wxString::Format("No USB Drive Set for '%s'", hix->GetName());
                continue;
            }
        }
        if (!ObtainAccessToURL(drive) || !createTestFile(drive))
        {
            wxDirDialog dlg(this, "Select SD Directory for " + hix->GetName(), drive, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK) {
                drive = dlg.GetPath();
            }
            if (!ObtainAccessToURL(drive)) {
                errorMsg = wxString::Format("Could not obtain write access for '%s'", drive);
                continue;
            }
        }
        std::vector<wxString> filesDone;
        for (auto & playlist : m_playLists) {
            bool worked {true};
            for (auto& play : playlist.Items) {
                wxString const shortName = shortNames.at(play.FSEQ);
                wxString const shortHseqName = shortName + ".hseq";
                prgs.Update(++count, "Generating HSEQ File " + shortHseqName);

                if (std::find(filesDone.begin(), filesDone.end(), play.FSEQ) == filesDone.end()) {
                    worked &= Create_HinksPix_HSEQ_File(play.FSEQ, drive + wxFileName::GetPathSeparator() + shortHseqName, hix, slave1, slave2, errorMsg);
                    filesDone.push_back(play.FSEQ);
                }
                if (worked && !play.Audio.IsEmpty()) {
                    wxString auName = shortName + ".au";
                    prgs.Update(count, "Generating AU File " + auName);
                    if (std::find(filesDone.begin(), filesDone.end(), auName) == filesDone.end()) {
                        AudioLoader audioLoader(play.Audio.ToStdString(), true);
                        worked &= audioLoader.loadAudioData();

                        if (worked) {
                            worked &= Make_AU_From_ProcessedAudio(audioLoader.processedAudio(), drive + wxFileName::GetPathSeparator() + auName, errorMsg);
                            filesDone.push_back(auName);
                        } else {
                            AudioLoader::State loaderState = audioLoader.state();
                            AudioReaderDecoderInitState decoderInitState = AudioReaderDecoderInitState::NoInit;
                            audioLoader.readerDecoderInitState(decoderInitState);
                            AudioResamplerInitState resamplerInitState = AudioResamplerInitState::NoInit;
                            audioLoader.resamplerInitState(resamplerInitState);
                            logger_base.error("HinksPixExportDialog export - loading audio fails - %d : %d : %d", int(loaderState), int(decoderInitState), int(resamplerInitState));
                        }
                    }
                    play.AU = auName;
                }
                play.HSEQ = shortHseqName;
            }
            if (worked) {
                playlist.saveAsFile(drive);
            } else {
                error = true;
            }
        }
        prgs.Update(++count, "Generating Schedule File");
        for (const auto& schedule : m_schedules)
        {
            schedule.saveAsFile(drive);
        }
        createModeFile(drive, GetChoiceValueIndex(MODE_COL + rowStr));
    }

    SaveSettings();
    prgs.Hide();
    if (error) {
        DisplayError("HinksPix File Generation Error\n" + errorMsg);
    } else {
        wxMessageBox("HinksPix File Generation Complete");
    }
    // EndDialog(wxID_CLOSE);
}

void HinksPixExportDialog::OnBitmapButtonMoveDownClick(wxCommandEvent& /*event*/) {
    for (int i = CheckListBox_Sequences->GetItemCount() - 1; i >= 0; --i) {
        if (CheckListBox_Sequences->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED) {
            if (i == CheckListBox_Sequences->GetItemCount() - 1) {
                return;
            }
            moveSequenceItem(i + 1, i);
        }
    }
}

void HinksPixExportDialog::OnBitmapButtonMoveUpClick(wxCommandEvent& /*event*/) {
    for (int i = 0; i < CheckListBox_Sequences->GetItemCount(); ++i) {
        if (CheckListBox_Sequences->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED) {
            if (i == 0) {
                return;
            }
            moveSequenceItem(i - 1, i);
        }
    }
}

void HinksPixExportDialog::OnButtonAddPlaylistClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Playlist Name", "Enter Playlist Name", "MAIN");
    if (dlg.ShowModal() == wxID_OK) {
        wxString n = dlg.GetValue();
        if (auto playlistRef = GetPlayList(n); playlistRef) {
            DisplayError("Playlist Name Already Exists", this);
            return;
        }
        ChoicePlaylists->AppendString(n);
        m_playLists.emplace_back(n);

        ChoicePlaylists->SetStringSelection(n);
        RedrawPlayList(n);
    }
}

void HinksPixExportDialog::OnButtonRemoveClick(wxCommandEvent& event)
{
    int selected = ChoicePlaylists->GetSelection();
    auto selectedTxt = ChoicePlaylists->GetStringSelection();
    ChoicePlaylists->Delete(selected);
    DeletePlayList(selectedTxt);
    if (ChoicePlaylists->GetStrings().size() > 0) {
        RedrawPlayList(ChoicePlaylists->GetStrings().Last());
    } else {
        RedrawPlayList(wxString());
    }
}

void HinksPixExportDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    SaveSettings();
    EndDialog(wxID_CLOSE);
}

void HinksPixExportDialog::OnChoiceSelected(wxCommandEvent& event) {

    wxString const text = event.GetString();
    if (text.IsEmpty()) {
        return;
    }

    auto* item = event.GetEventObject();
    if (item) {
        wxChoice* cb = dynamic_cast<wxChoice*>(item);
        if (cb) {
            if (cb == ChoiceFilter || cb == ChoiceFolder) {
                LoadSequences();
                return;
            }

            if (cb == ChoicePlaylists) {
                RedrawPlayList(text);
                return;
            }

            auto name = cb->GetName();
            if (name.Contains(SLAVE1_COL) || name.Contains(SLAVE2_COL)) {
                int row = 0;
                for (auto* hix : m_hixControllers) {
                    std::string const rowStr = std::to_string(row);
                    ++row;
                    wxString const slaveName1 = GetChoiceValue(SLAVE1_COL + rowStr);
                    wxString const slaveName2 = GetChoiceValue(SLAVE2_COL + rowStr);
                    if (name == SLAVE1_COL + rowStr || name == SLAVE2_COL + rowStr) {
                        if (!CheckSlaveControllerSizes(hix, getSlaveController(slaveName1), getSlaveController(slaveName2))) {
                            cb->SetSelection(0);
                            event.Skip();
                            return;
                        }
                        if (name == SLAVE1_COL + rowStr && slaveName2 == text) {
                            DisplayError(wxString::Format("Cannot Set Slave Controller 1 and 2 as the same '%s' ", text));
                            cb->SetSelection(0);
                            event.Skip();
                            return;
                        }
                        if (name == SLAVE2_COL + rowStr && slaveName1 == text) {
                            DisplayError(wxString::Format("Cannot Set Slave Controller 1 and 2 as the same '%s' ", text));
                            cb->SetSelection(0);
                            event.Skip();
                            return;
                        }
                        continue;
                    }

                    if (text == slaveName1 || text == slaveName2) {
                        DisplayError(wxString::Format("Cannot use the Same Slave Controller across multiple HinksPix Controllers '%s' ", text));
                        cb->SetSelection(0);
                        event.Skip();
                        return;
                    }
                }
            }
        }
    }
}

void HinksPixExportDialog::moveSequenceItem(int to, int from, bool select) {
    bool const checked = CheckListBox_Sequences->IsItemChecked(to);
    wxString const fseq = CheckListBox_Sequences->GetItemText(to);
    wxString const audio = CheckListBox_Sequences->GetItemText(to, 1);
    CheckListBox_Sequences->CheckItem(to, CheckListBox_Sequences->IsItemChecked(from));
    CheckListBox_Sequences->SetItem(to, 0, CheckListBox_Sequences->GetItemText(from));
    CheckListBox_Sequences->SetItem(to, 1, CheckListBox_Sequences->GetItemText(from, 1));
    CheckListBox_Sequences->CheckItem(from, checked);
    CheckListBox_Sequences->SetItem(from, 0, fseq);
    CheckListBox_Sequences->SetItem(from, 1, audio);
    if (select) {
        CheckListBox_Sequences->Select(to);
        CheckListBox_Sequences->Select(from, false);
    }
}

void HinksPixExportDialog::createModeFile(wxString const& drive, int mode) const {
    //0=Master, 1=Slave
    wxFile f;
    wxString const filename = drive + wxFileName::GetPathSeparator() + "SD_StandAlone.sys";

    if (mode == 2) //2=Skip, delete file that sets mode
    {
        wxRemoveFile(filename);
        return;
    }

    f.Open(filename, wxFile::write);

    if (f.IsOpened()) {
        auto const line = wxString::Format("M  %d\r\n", mode);
        f.Write(line);
        f.Write("  \r\n");
        f.Close();
    }
}

bool HinksPixExportDialog::Create_HinksPix_HSEQ_File(wxString const& fseqFile, wxString const& shortHSEQName, ControllerEthernet* hix, ControllerEthernet* slave1, ControllerEthernet* slave2, wxString& errorMsg) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug(wxString::Format("HinksPix HSEQ Creation from %s", fseqFile));

    std::unique_ptr<FSEQFile> xf(FSEQFile::openFSEQFile(fseqFile));
    if (!xf) {
        errorMsg = wxString::Format("HinksPix Failed opening FSEQ %s", fseqFile);
        logger_base.error(errorMsg);
        return false;
    }

    uint32_t const ogNumber_of_Frames = xf->getNumFrames();
    uint32_t const ogNumChannels = xf->getChannelCount();
    int const ogFrame_Rate = xf->getStepTime();
    if (ogFrame_Rate != 50) {
        errorMsg = wxString::Format("HinksPix Failed Framerate must be 50Ms FSEQ %s", fseqFile);
        logger_base.error(errorMsg);
        return false;
    }

    std::vector<std::pair<uint32_t, uint32_t>> rng;
    rng.emplace_back(std::pair<uint32_t, uint32_t>(0, ogNumChannels));
    xf->prepareRead(rng);

    //Get Map of HinksPix Port channel locations to xLights channel locations
    int32_t hix_Channels;
    std::vector<HinksChannelMap> modelStarts;
    if (hix->IsUniversePerString()) {
        modelStarts = getModelChannelMap(hix, hix_Channels);
    } else {
        hix_Channels = hix->GetChannels();
        auto const& hmm = modelStarts.emplace_back(hix->GetStartChannel(), hix->GetChannels(), 1);
        hmm.Dump();
    }

    int32_t ef_Num_Channel_To_Write = hix_Channels;

    if (slave1) {
        ef_Num_Channel_To_Write += slave1->GetChannels();
    }

    if (slave2) {
        ef_Num_Channel_To_Write += slave2->GetChannels();
    }

    // read file ready -- do write file
    std::unique_ptr<FSEQFile> ef(new HSEQFile(shortHSEQName, hix, slave1, slave2, ogNumChannels));
    if (!ef) {
        errorMsg = wxString::Format("HinksPix Failed Write opening FSEQ %s", shortHSEQName);
        logger_base.error(errorMsg);
        return false;
    }

    ef->setChannelCount(ef_Num_Channel_To_Write);
    ef->setStepTime(ogFrame_Rate);
    ef->setNumFrames(ogNumber_of_Frames);

    ef->writeHeader(); // ready for frame data

    uint8_t* WriteBuf = new uint8_t[ef_Num_Channel_To_Write];

    // read buff
    uint8_t* tmpBuf = new uint8_t[ogNumChannels];

    uint32_t frame = 0;

    //mostly copied from Joe's pull request(#1441) in Jan 2019
    while (frame < ogNumber_of_Frames) {
        FSEQFile::FrameData* data = xf->getFrame(frame);

        data->readFrame(tmpBuf, ogNumChannels); // we have a read frame

        uint8_t* dest = WriteBuf;

        //Loop through models channels
        for (auto const& modelChan : modelStarts) {
            uint8_t* tempSrc = tmpBuf + modelChan.OrgStartChannel - 1;
            uint8_t* tempDest = WriteBuf + modelChan.HinksStartChannel - 1;
            memmove(tempDest, tempSrc, modelChan.ChannelCount);
            dest += modelChan.ChannelCount;
        }

        if (slave1) {
            uint8_t* src = tmpBuf + slave1->GetStartChannel() - 1;
            memmove(dest, src, slave1->GetChannels());
            dest += slave1->GetChannels();
        }

        if (slave2) {
            uint8_t* src = tmpBuf + slave2->GetStartChannel() - 1;
            memmove(dest, src, slave2->GetChannels());
            dest += slave2->GetChannels();
        }

        ef->addFrame(frame, WriteBuf);

        delete data;
        frame++;
    }

    ef->finalize();

    delete[] tmpBuf;
    delete[] WriteBuf;

    logger_base.debug(wxString::Format("HinksPix Completed HSEQ %s", shortHSEQName));
    return true;
}

wxString HinksPixExportDialog::createUniqueShortName(wxString const& fseqName, std::vector<wxString> const& names) {
    //max Length is 20 according to HSA
    wxFileName fn(fseqName);
    wxString newfseqName = fn.GetName().Upper();
    newfseqName.erase(std::remove_if(newfseqName.begin(), newfseqName.end(), [](unsigned char c) { return !std::isalnum(c); }), newfseqName.end());
    if (newfseqName.Length() > 20) {
        newfseqName = newfseqName.Left(20);
    }

    //make sure name is unique
    int index = 1;
    while (std::find_if(names.begin(), names.end(), [newfseqName](auto const& e) { return newfseqName == e; }) != names.end()) {
        //if not, add number to end
        auto const& suffix = wxString::Format("%d", index);
        if ((newfseqName.Length() + suffix.Length()) > 20) {
            newfseqName = newfseqName.Left(20 - suffix.Length());
        }
        newfseqName = newfseqName + suffix;
    }
    return newfseqName;
}

bool HinksPixExportDialog::Make_AU_From_ProcessedAudio(const std::vector<int16_t>& processedAudio, wxString const& AU_File, wxString& errorMsg) {
    //this format was copied from Joe's HSA 2.0 JavaScript sourcecode
    uint8_t header[24];
    ::memset(header, 0, 24);

    uint32_t sz = processedAudio.size() * sizeof(int16_t);

    uint32_t magic = (0x2e << 24) | (0x73 << 16) | (0x6e << 8) | 0x64;
    write4ByteUInt(&header[0], magic);
    write4ByteUInt(&header[4], 24);

    write4ByteUInt(&header[8], sz);

    write4ByteUInt(&header[12], 3);
    write4ByteUInt(&header[16], 44100); //bitrate
    write4ByteUInt(&header[20], 2);     //channels?

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxFile fo;
    fo.Open(AU_File, wxFile::write);
    if (!fo.IsOpened()) {
        errorMsg = wxString::Format("Error Creating the AU Audio file %s", AU_File);
        logger_base.error(errorMsg);
        return false;
    }

    fo.Write(&header, sizeof(header));
    fo.Write(processedAudio.data(), sz);
    fo.Close();

    return true;
}

bool HinksPixExportDialog::CheckSlaveControllerSizes(ControllerEthernet* controller, ControllerEthernet* slave1, ControllerEthernet* slave2) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int slaveUni{ 0 };
    int slaveUni2{ 0 };
    slaveUni = slaveUni2 = getMaxSlaveControllerUniverses(controller);

    if (slave1) {
        if (slave1->GetOutputCount() > 32) {
            logger_base.error("HinksPixExportDialog export - Slave Controller '%s' has too many Universes, Max is 32 Currently Used is %d", slave1->GetName().c_str(), slave1->GetOutputCount());

            DisplayError(wxString::Format("Slave Controller '%s' has too many Universes, Max is 32 Currently Used is %d", slave1->GetName().c_str(), slave1->GetOutputCount()));
            return false;
        }
        slaveUni -= slave1->GetOutputCount();
    }

    if (slave2) {
        if (slave2->GetOutputCount() > 16) {
            logger_base.error("HinksPixExportDialog export - Slave Controller '%s' has too many Universes, Max is 16 Currently Used is %d", slave2->GetName().c_str(), slave2->GetOutputCount());

            DisplayError(wxString::Format("Slave Controller '%s' has too many Universes, Max is 16 Currently Used is %d", slave2->GetName().c_str(), slave2->GetOutputCount()));
            return false;
        }
        slaveUni -= slave2->GetOutputCount();
    }

    if (slaveUni >= 0) {
        return true;
    }

    logger_base.error("HinksPixExportDialog export - too many Slave Controller Universes - '%s' : Max %d Used %d", controller->GetName().c_str(), slaveUni2, (slaveUni2 - slaveUni));

    DisplayError(wxString::Format("Too Many Slave Controller Universes off '%s': Max %d Used %d\n", controller->GetName().c_str(), slaveUni2, (slaveUni2 - slaveUni)));

    return false;
}

ControllerEthernet* HinksPixExportDialog::getSlaveController(std::string const& name) {
    auto contrl = std::find_if(m_otherControllers.begin(), m_otherControllers.end(), [&name](auto po) { return po->GetName() == name; });
    if (contrl != m_otherControllers.end()) {
        return *contrl;
    }
    return nullptr;
}

std::vector<HinksChannelMap> HinksPixExportDialog::getModelChannelMap(ControllerEthernet* hinks, int32_t& chanCount) const {
    std::vector<HinksChannelMap> _modelMap;

    UDController cud(hinks, m_outputManager, m_modelManager, false);
    int32_t hinkstartChan = 1;

    wxASSERT(hinks->GetControllerCaps()->DMXAfterPixels());

    //pixels first
    for (int port = 1; port <= hinks->GetControllerCaps()->GetMaxPixelPort(); port++) {
        if (cud.HasPixelPort(port)) {
            UDControllerPort* portData = cud.GetControllerPixelPort(port);
            for (auto const& m : portData->GetModels()) {
                auto sizeofchan = m->Channels();
                auto startChan = m->GetStartChannel();
                auto const& hmm = _modelMap.emplace_back(startChan, sizeofchan, hinkstartChan);
                hmm.Dump();
                hinkstartChan += sizeofchan;
            }
        }
    }

    // serial second
    for (int port = 1; port <= hinks->GetControllerCaps()->GetMaxSerialPort(); port++) {
        if (cud.HasSerialPort(port)) {
            UDControllerPort* portData = cud.GetControllerSerialPort(port);
            for (auto const& m : portData->GetModels()) {
                auto sizeofchan = m->Channels();
                auto startChan = m->GetStartChannel();
                auto const& hmm = _modelMap.emplace_back(startChan, sizeofchan, hinkstartChan);
                hmm.Dump();
                hinkstartChan += sizeofchan;
            }
        }
    }
    chanCount = hinkstartChan - 1;
    return _modelMap;
}

int HinksPixExportDialog::getMaxSlaveControllerUniverses(ControllerEthernet* controller) const {
    if (controller->GetModel().find("PRO") != std::string::npos) // PRO
    {
        if (controller->GetOutputCount() <= ((3 * 16) + 1)) {
            return (33 + 17);
        }
        if (controller->GetOutputCount() <= ((5 * 16) + 1)) {
            return (33);
        }
        if (controller->GetOutputCount() <= ((6 * 16) + 1)) {
            return (17);
        }
    }

    if (controller->GetModel().find("EasyLights") != std::string::npos) // easy
    {
        if (controller->GetOutputCount() <= ((1 * 16) + 1)) {
            return (33);

        }
        if (controller->GetOutputCount() <= ((2 * 16) + 1)) {
            return (17);
        }
    }
    return 0;
}

void HinksPixExportDialog::AddInstanceHeader(wxString const& h) {
    wxPanel* Panel1 = new wxPanel(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER | wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    wxBoxSizer* BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* StaticText3 = new wxStaticText(Panel1, wxID_ANY, h, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    BoxSizer1->Add(StaticText3, 1, wxLEFT | wxRIGHT | wxEXPAND, 5);
    Panel1->SetSizer(BoxSizer1);
    BoxSizer1->Fit(Panel1);
    BoxSizer1->SetSizeHints(Panel1);
    HinkControllerSizer->Add(Panel1, 1, wxALL | wxEXPAND, 0);
}

bool HinksPixExportDialog::createTestFile(wxString const& drive) const {
#ifdef __WXOSX__
    wxFile f;
    wxString const filename = drive + wxFileName::GetPathSeparator() + "test.seh";

    f.Open(filename, wxFile::write);

    if (f.IsOpened()) {
        f.Write("test\r\n");
        f.Close();

        wxRemoveFile(filename);
        return true;
    }
    return false;
#else
    return true;
#endif
}

bool HinksPixExportDialog::GetCheckValue(wxString const& col) const {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxCheckBox* cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->GetValue();
        }
    }
    return false;
}

int HinksPixExportDialog::GetChoiceValueIndex(wxString const& col) const {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetSelection();
        }
    }
    return 0;
}

wxString HinksPixExportDialog::GetChoiceValue(wxString const& col) const {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetStringSelection();
        }
    }
    return "";
}

void HinksPixExportDialog::SetChoiceValueIndex(wxString const& col, int i) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->SetSelection(i);
        }
    }
}

void HinksPixExportDialog::SetChoiceValue(wxString const& col, std::string const& value) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            int find = cb->FindString(value);
            return cb->SetSelection(find);
        }

        wxComboBox* comboBox = dynamic_cast<wxComboBox*>(w);
        if (comboBox) {
            int find = comboBox->FindString(value);
            return comboBox->SetSelection(find);
        }
    }
}

void HinksPixExportDialog::SetCheckValue(wxString const& col, bool b) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxCheckBox* cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->SetValue(b);
        }
    }
}

void HinksPixExportDialog::SetDropDownItems(wxString const& col, wxArrayString const& items)
{
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->Set(items);
        }
    }
}

void HinksPixExportDialog::StoreToObjectPlayList(wxString const& playlist) {
    if (auto playlistRef = GetPlayList(playlist); playlistRef) {
        auto& playlist = playlistRef->get();
        playlist.Items.clear();
        for (int fs = 0; fs < CheckListBox_Sequences->GetItemCount(); fs++) {
            if (CheckListBox_Sequences->IsItemChecked(fs)) {
                wxString const fseq = CheckListBox_Sequences->GetItemText(fs);
                wxString const media = CheckListBox_Sequences->GetItemText(fs, 1);
                playlist.Items.emplace_back(fseq, media);
            }
        }
    }
}

void HinksPixExportDialog::RedrawPlayList(wxString const& new_playlist, bool save_old) {
    if (save_old) {
        if (!m_selectedPlayList.empty()) {
            StoreToObjectPlayList(m_selectedPlayList);
        }
        StoreToObjectSchedule();
    }

    for (int fs = 0; fs < CheckListBox_Sequences->GetItemCount(); fs++) {
        CheckListBox_Sequences->CheckItem(fs, false);
    }

    if (auto newplaylistRef = GetPlayList(new_playlist); newplaylistRef) {
        auto& nplaylist = newplaylistRef->get();
        int index = 0;
        for (auto const& [seq, media,h,au] : nplaylist.Items) {
            int const i = CheckListBox_Sequences->FindItem(0, seq);
            if (i != -1) {
                CheckListBox_Sequences->CheckItem(i, true);
                moveSequenceItem(index, i, false);
                index++;
            }
        }
    }
    if (!new_playlist.IsEmpty()) {
        m_selectedPlayList = new_playlist;
        CheckListBox_Sequences->Enable();
    } else {
        CheckListBox_Sequences->Disable();
        m_selectedPlayList.clear();
    }

    RedrawSchedules();
}

void HinksPixExportDialog::StoreToObjectSchedule()
{
    CheckSchedules();

    auto rows = GridSchedule->GetNumberRows();
    if (rows == 0) {
        return;
    }
    for (auto const& day : DAYS) {
        if (auto dayRef = GetSchedule(day); dayRef) {
            auto& sch = dayRef->get();
            sch.clearItems();
        }
    }

    auto GetCell = [&](int row, ScheduleColumn col) {
        return GridSchedule->GetCellValue(row, static_cast<int>(col));
    };

    for (int row = 0; row <rows; ++row)
    {
        auto day = GridSchedule->GetRowLabelValue(row);
        if (auto dayRef = GetSchedule(day); dayRef) {
            auto& sch = dayRef->get();
            auto use = GetCell(row, ScheduleColumn::Enabled);
            auto play = GetCell(row, ScheduleColumn::PlayList);
            auto st_hr = GetCell(row, ScheduleColumn::StartHour);
            auto st_min = GetCell(row, ScheduleColumn::StartMin);
            auto ed_hr = GetCell(row, ScheduleColumn::EndHour);
            auto ed_min = GetCell(row, ScheduleColumn::EndMin);

            ScheduleItem item(play);
            item.StartHour = wxAtoi(st_hr);
            item.StartMin = wxAtoi(st_min);
            item.EndHour = wxAtoi(ed_hr);
            item.EndMin = wxAtoi(ed_min);
            item.Enabled = !use.IsEmpty();
            sch.Items.push_back(item);
        }
    }
}

void HinksPixExportDialog::RedrawSchedules()
{
    int row = 0;
    auto currentRows = GridSchedule->GetNumberRows();

    auto CheckRows = [&]()
    {
        if (row >= currentRows) {
            GridSchedule->AppendRows(1);
        }
    };

    auto SetCell = [&](ScheduleColumn col, wxString const& val) {
        GridSchedule->SetCellValue(row, static_cast<int>(col), val);
    };

    auto DrawDefaultPlaylist = [&](wxString const& day, wxString const& play) {
        CheckRows();
        GridSchedule->SetRowLabelValue(row, day);
        SetCell(ScheduleColumn::PlayList, play);
        GridSchedule->SetReadOnly(row, 0, true);
        SetCell(ScheduleColumn::StartHour, "0");
        SetCell(ScheduleColumn::StartMin,  "0");
        SetCell(ScheduleColumn::EndHour,   "23");
        SetCell(ScheduleColumn::EndMin,    "59");
        SetCell(ScheduleColumn::Enabled,   "X");
        row++;
    };

    auto DrawDefaultDay = [&]( wxString const& day) {
        for (auto sch : ChoicePlaylists->GetStrings()) {
            DrawDefaultPlaylist(day,sch);
        }
    };

    auto DrawPlaylistItem = [&](wxString const& day, ScheduleItem const& item) {
        CheckRows();
        GridSchedule->SetRowLabelValue(row, day);
        SetCell(ScheduleColumn::PlayList, item.Playlist);
        GridSchedule->SetReadOnly(row, 0, true);
        SetCell(ScheduleColumn::StartHour, wxString::Format("%i", item.StartHour));
        SetCell(ScheduleColumn::StartMin , wxString::Format("%i", item.StartMin));
        SetCell(ScheduleColumn::EndHour  , wxString::Format("%i", item.EndHour));
        SetCell(ScheduleColumn::EndMin   , wxString::Format("%i", item.EndMin));
        SetCell(ScheduleColumn::Enabled  , item.Enabled ? "X" : "");
        row++;
    };

    for (auto day : DAYS) {
        if (auto dayRef = GetSchedule(day); dayRef) {
            auto const& sch = dayRef->get();
            for (auto play : ChoicePlaylists->GetStrings()) {
                bool found{false};
                for(auto const& item :sch.Items){
                    if (item.Playlist == play ){
                        DrawPlaylistItem(day, item);
                        found =true;
                    }
                }
                if (!found) {
                    DrawDefaultPlaylist(day, play);
                }
            }
        } else {
            DrawDefaultDay(day);
        }
    }

    if (row < currentRows) {
        GridSchedule->DeleteRows(row, currentRows - row);
    }
}

bool HinksPixExportDialog::CheckSchedules()
{
    auto rows = GridSchedule->GetNumberRows();

    auto IsNumber = [](const std::string& s) {
        return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
    };

    auto IsInt = [&](int row, ScheduleColumn col) {
        auto val = GridSchedule->GetCellValue(row, static_cast<int>(col));
        return IsNumber(val);
    };

    auto GetInt = [&](int row, ScheduleColumn col) {
        auto val = GridSchedule->GetCellValue(row, static_cast<int>(col));
        return wxAtoi(val);
    };

    auto IsValidRange = [&](int row, ScheduleColumn col, int start, int end) {
        auto val = GetInt(row, col);
        return (val >= start && val <= end);
    };

    auto GetCell = [&](int row, ScheduleColumn col) {
        return GridSchedule->GetCellValue(row, static_cast<int>(col));
    };

    auto CheckRow = [&](int row ) {
        if (!IsInt(row, ScheduleColumn::StartHour) || !IsInt(row, ScheduleColumn::StartMin) ||
            !IsInt(row, ScheduleColumn::EndHour) || !IsInt(row, ScheduleColumn::EndMin)) {
            return false;
        }
        if (!IsValidRange(row, ScheduleColumn::StartHour, 0, 23) ||
            !IsValidRange(row, ScheduleColumn::StartMin, 0, 59) ||
            !IsValidRange(row, ScheduleColumn::EndHour, 0, 23) ||
            !IsValidRange(row, ScheduleColumn::EndMin, 0, 59)) {
            return false;
        }
        return true;
    };

    for (int row = 0; row < rows; ++row) {
        auto day = GridSchedule->GetRowLabelValue(row);
        auto play = GetCell(row, ScheduleColumn::PlayList);
        if (!CheckRow(row)) {
            DisplayError(wxString::Format("Schedule %s '%s' Has Invalid Date Time", day, play));
            return false;
        }
    }
    //todo: add overlap time checking
    return true;
}

[[nodiscard]] std::optional<std::reference_wrapper<PlayList>> HinksPixExportDialog::GetPlayList(wxString const& name)
{
    if (auto const found{ std::find_if(m_playLists.begin(), m_playLists.end(),
                                       [&name](auto& c) { return c.Name.compare(name) == 0; }) };
        found != m_playLists.end()) {
        return *found;
    }
    return std::nullopt;
}

[[nodiscard]] std::optional<std::reference_wrapper<Schedule>> HinksPixExportDialog::GetSchedule(wxString const& day)
{
    if (auto const found{ std::find_if(m_schedules.begin(), m_schedules.end(),
                                       [&day](auto& c) { return c.Day.compare(day) == 0; }) };
        found != m_schedules.end()) {
        return *found;
    }
    return std::nullopt;
}

void HinksPixExportDialog::DeletePlayList(wxString const& name)
{
    if (auto const found{ std::find_if(m_playLists.begin(), m_playLists.end(),
                                       [&name](auto& c) { return c.Name.compare(name) == 0; }) };
        found != m_playLists.end()) {
        m_playLists.erase(found);
    }

    for(auto & day : m_schedules)
    {
        day.removePlaylist(name);
    }
}
