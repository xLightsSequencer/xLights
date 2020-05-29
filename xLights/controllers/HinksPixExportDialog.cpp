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
#include "AudioManager.h"
#include "../outputs/ControllerEthernet.h"

#include <log4cpp/Category.hh>

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"

static const std::string CHECK_COL = "ID_UPLOAD_";
static const std::string MODE_COL = "ID_MODE_";
static const std::string SLAVE1_COL = "ID_SLAVE1_";
static const std::string SLAVE2_COL = "ID_SLAVE2_";
static const std::string DISK_COL = "ID_DISK_";

//(*IdInit(HinksPixExportDialog)
const long HinksPixExportDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT1 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_FILTER = wxNewId();
const long HinksPixExportDialog::ID_STATICTEXT2 = wxNewId();
const long HinksPixExportDialog::ID_CHOICE_FOLDER = wxNewId();
const long HinksPixExportDialog::ID_BITMAPBUTTON_MOVE_UP = wxNewId();
const long HinksPixExportDialog::ID_BITMAPBUTTON_MOVE_DOWN = wxNewId();
const long HinksPixExportDialog::ID_LISTVIEW_Sequences = wxNewId();
const long HinksPixExportDialog::ID_PANEL1 = wxNewId();
const long HinksPixExportDialog::ID_SPLITTERWINDOW1 = wxNewId();
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

inline std::array<int, 4> getIPBytes(const wxString& ip)
{
    wxArrayString const ips = wxSplit(ip, '.');
    if (ips.size() != 4)
        return { 0,0,0,0 };
    return { wxAtoi(ips[0]),wxAtoi(ips[1]),wxAtoi(ips[2]),wxAtoi(ips[3]) };
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
    if (_slave1)
        num_of_Addition_Controllers++;
    if (_slave2)
        num_of_Addition_Controllers++;   

    header[9] = num_of_Addition_Controllers; //# of Slave Controllers

    write2ByteUInt(&header[16], ((44100 * m_seqStepTime) / 1000));//framerate
    write4ByteUInt(&header[20], m_seqNumFrames);//number of frames

    write4ByteUInt(&header[24], m_seqChannelCount);//total channel count of all controllers
    write2ByteUInt(&header[68], _hinx->GetChannels());//total channel count of master controllers

    strcpy((char*)&header[28], _hinx->GetIP().c_str());//IP of master controllers

    if (_slave1)
    {
        write2ByteUInt(&header[76], _slave1->GetChannels());

        auto const slave1IP = getIPBytes(_slave1->GetIP());
        header[72] = slave1IP[0];
        header[73] = slave1IP[1];
        header[74] = slave1IP[2];
        header[75] = slave1IP[3];
        int j = 0;
        for (auto const output : _slave1->GetOutputs())
        {
            write2ByteUInt(&header[80 + (j * 2)], output->GetUniverse());
            write2ByteUInt(&header[148 + (j * 2)], output->GetChannels());
            j++;
        }
    }
    if (_slave2)
    {
        write2ByteUInt(&header[220], _slave2->GetChannels());

        auto const slave2IP = getIPBytes(_slave2->GetIP());
        header[216] = slave2IP[0];
        header[217] = slave2IP[1];
        header[218] = slave2IP[2];
        header[219] = slave2IP[3];
        int j = 0;
        for (auto const output : _slave2->GetOutputs())
        {
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

    write2ByteUInt(&header[330], _orgChannelCount);

    write2ByteUInt(&header[334], m_seqNumFrames);

    write(header, 336);
    m_seqChanDataOffset = dataOffset;
    dumpInfo(false);
}

HinksPixExportDialog::HinksPixExportDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
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
	SplitterWindow1->SetMinimumPaneSize(100);
	SplitterWindow1->SetSashGravity(0.5);
	HinkControllerList = new wxScrolledWindow(SplitterWindow1, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	HinkControllerList->SetMinSize(wxDLG_UNIT(SplitterWindow1,wxSize(-1,150)));
	HinkControllerSizer = new wxFlexGridSizer(0, 8, 0, 0);
	HinkControllerList->SetSizer(HinkControllerSizer);
	HinkControllerSizer->Fit(HinkControllerList);
	HinkControllerSizer->SetSizeHints(HinkControllerList);
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
	SplitterWindow1->SplitHorizontally(HinkControllerList, Panel1);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	AddRefreshButton = new wxButton(this, ID_BUTTON_REFRESH, _("Refresh USB Drives"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REFRESH"));
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
	BoxSizer1->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

    wxConfigBase* config = wxConfigBase::Get();

    PopulateControllerList(outputManager);
    ApplySavedSettings();

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

HinksPixExportDialog::~HinksPixExportDialog()
{
    //(*Destroy(HinksPixExportDialog)
    //*)
}

void HinksPixExportDialog::PopulateControllerList(OutputManager* outputManager)
{
    HinkControllerList->Freeze();
    //remove all the children from the first upload checkbox on
    wxWindow* w = HinkControllerList->FindWindow(CHECK_COL + "0");
    while (w) {
        wxWindow* tmp = w->GetNextSibling();
        w->Destroy();
        tmp = w;
    }

    auto const controllers = outputManager->GetControllers();
    wxArrayString otherControllers;
    for (const auto& it : controllers)
    {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && eth->GetIP() != "MULTICAST" && eth->GetProtocol() != OUTPUT_ZCPP && eth->IsManaged())
        {
            if (eth->GetVendor() == "HinksPix")
            {
                _hixControllers.push_back(eth);
            }
            else if(eth->GetProtocol() == OUTPUT_E131)
            {
                _otherControllers.push_back(eth);
                otherControllers.push_back(eth->GetName());
            }
        }
    }
    int row = 0;
    for (const auto& it : _hixControllers)
    {
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
        if(it == _hixControllers.front())
            Choice1->SetSelection(0);
        else
            Choice1->SetSelection(1);
        HinkControllerSizer->Add(Choice1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);

        Choice1 = new wxChoice(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, DISK_COL + rowStr);
        Choice1->Append(_drives);

        HinkControllerSizer->Add(Choice1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);

        wxChoice* Choice2 = new wxChoice(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, SLAVE1_COL + rowStr);
        Choice2->Append(_(""));
        wxChoice* Choice3 = new wxChoice(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, SLAVE2_COL + rowStr);
        Choice3->Append(_(""));
        for (wxString const& oth : otherControllers)
        {
            Choice2->Append(oth);
            Choice3->Append(oth);
        }
        Choice2->SetSelection(0);
        Choice3->SetSelection(0);

        HinkControllerSizer->Add(Choice2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        HinkControllerSizer->Add(Choice3, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        if (otherControllers.size() == 0)
        {
            Choice2->Enable(false);
            Choice3->Enable(false);
        }
        else if (otherControllers.size() == 1) 
        {
            Choice3->Enable(false);
        }
 
        row++;
    }

    Connect(wxID_ANY, wxEVT_CHOICE, (wxObjectEventFunction)&HinksPixExportDialog::OnChoiceSelected);    

    HinkControllerList->FitInside();
    HinkControllerList->SetScrollRate(10, 10);
    HinkControllerList->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
    HinkControllerList->Thaw();
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

void HinksPixExportDialog::LoadSequencesFromFolder(wxString dir) const
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning folder for sequences for FPP upload: %s", (const char *)dir.c_str());

    const wxString fseqDir = xLightsFrame::FseqDir;

    wxDir directory;
    directory.Open(dir);

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.x*");
    static const int BUFFER_SIZE = 1024*12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer
    while (fcont) {
        if (file != "xlights_rgbeffects.xml" && file != OutputManager::GetNetworksFileName() && file != "xlights_keybindings.xml" &&
            (file.Lower().EndsWith("xml") || file.Lower().EndsWith("xsq"))) {
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
    _drives.Clear();

#ifdef __WXMSW__
    wxArrayString ud = wxFSVolume::GetVolumes(wxFS_VOL_REMOVABLE | wxFS_VOL_MOUNTED, 0);
    for (const auto &a : ud) {
        _drives.push_back(a);
    }
#elif defined(__WXOSX__)
    wxDir d;
    d.Open("/Volumes");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont)
    {
        _drives.push_back("/Volumes/" + dir + "/");
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
            _drives.push_back("/media/" + dir + "/" + dir2);
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

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("HinksPixExportSelectedSequences", selectedFSEQ);
    config->Write("HinksPixExportFilterSelection", ChoiceFilter->GetSelection());
    config->Write("HinksPixExportFolderSelection", ChoiceFolder->GetString(ChoiceFolder->GetSelection()));
    config->Write("HinksPixExportStartHour", SpinCtrlStartHour->GetValue());
    config->Write("HinksPixExportStartMin", SpinCtrlStartMin->GetValue());
    config->Write("HinksPixExportEndHour", SpinCtrlEndHour->GetValue());
    config->Write("HinksPixExportEndMin", SpinCtrlEndMin->GetValue());

    int row = 0;
    for (const auto& inst : _hixControllers) {
        std::string rowStr = std::to_string(row);
        wxString ipaddress = inst->GetIP();

        ipaddress.Replace(".", "_");
        config->Write("HinksPixExportExport_" + ipaddress, GetCheckValue(CHECK_COL + rowStr));
        config->Write("HinksPixExportMode_" + ipaddress, GetChoiceValue(MODE_COL + rowStr));
        config->Write("HinksPixExportSlave1_" + ipaddress, GetChoiceValue(SLAVE1_COL + rowStr));
        config->Write("HinksPixExportSlave2_" + ipaddress, GetChoiceValue(SLAVE2_COL + rowStr));
        config->Write("HinksPixExportDisk_" + ipaddress, GetChoiceValue(DISK_COL + rowStr));
        row++;
    }

    config->Flush();
}

void HinksPixExportDialog::ApplySavedSettings()
{
    /*
    static const std::string CHECK_COL = "ID_UPLOAD_";
    static const std::string MODE_COL = "ID_MODE_";
    static const std::string SLAVE1_COL = "ID_SLAVE1_";
    static const std::string SLAVE2_COL = "ID_SLAVE2_";
    static const std::string DISK_COL = "ID_DISK_";
     */

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) { 
     
        int row = 0;
        for (const auto& hix : _hixControllers) {
            std::string rowStr = std::to_string(row);
            wxString ipAddress = hix->GetIP();
            ipAddress.Replace(".", "_");

            bool bval;
            wxString sval;
            if (config->Read("HinksPixExportExport_" + ipAddress, &bval)) {
                SetCheckValue(CHECK_COL + rowStr, bval);
            }
            if (config->Read("HinksPixExportMode_" + ipAddress, &sval)) {
                SetChoiceValue(MODE_COL + rowStr, sval);
            }
            if (config->Read("HinksPixExportSlave1_" + ipAddress, &sval)) {
                SetChoiceValue(SLAVE1_COL + rowStr, sval);
            }
            if (config->Read("HinksPixExportSlave2_" + ipAddress, &sval)) {
                SetChoiceValue(SLAVE2_COL + rowStr, sval);
            }
            if (config->Read("HinksPixExportDisk_" + ipAddress, &sval)) {
                SetChoiceValue(DISK_COL + rowStr, sval);
            }
            row++;
        }
    }
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
    for (int i = 0; i< _hixControllers.size(); i++) {
        std::string rowStr = std::to_string(i);
        SetDropDownItems(DISK_COL + rowStr, _drives);
    }    
}

void HinksPixExportDialog::OnButton_ExportClick(wxCommandEvent& event)
{
    wxProgressDialog prgs("Generating HinksPix Files", "Generating HinksPix Files",
        _hixControllers.size() * (CheckListBox_Sequences->GetItemCount()*2) + (_hixControllers.size() * 2) +1,
        this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prgs.Show();

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool error = false;
    wxString errorMsg;
    int count = 0;
    int row = 0;
    for (auto hix : _hixControllers) 
    {
        std::string const rowStr = std::to_string(row);
        ++row;
        bool upload = GetCheckValue(CHECK_COL + rowStr);
        if (!upload)
        {
            ++count;
            continue;
        }

        wxString const ip = hix->GetIP();

        prgs.Update(++count, wxString::Format("Generating HinksPix Files for '%s'" , hix->GetName()));

        wxString slaveName1 = GetChoiceValue(SLAVE1_COL + rowStr);
        wxString slaveName2 = GetChoiceValue(SLAVE2_COL + rowStr);

        if (!slaveName1.IsEmpty() && slaveName1 == slaveName2)
        {
            error = true;
            errorMsg = wxString::Format("Slave Controller 1 and 2 cannot not be the same: '%s'", slaveName1);
            continue;
        }  

        auto slave1 = getSlaveController(slaveName1);
        auto slave2 = getSlaveController(slaveName2);

        if (!slave1 && slave2)
        {
            std::swap(slave1, slave2);
        }

        if (slave1 && slave2)
        {
            if (slave2->GetOutputCount() > slave1->GetOutputCount())
            {
                std::swap(slave1, slave2);
            }
        }

        if (slave1 || slave2)
        {
            if (!CheckSlaveControllerSizes(hix, slave1, slave2))
            {
                error = true;
                errorMsg = wxString::Format("Too Many Slave Controller Universes for '%s'", hix->GetName());
                continue;
            }
        } 

        wxString const drive = GetChoiceValue(DISK_COL + rowStr);

        if (drive.IsEmpty())
        {
            error = true; 
            errorMsg = wxString::Format("No USB Drive Set for '%s'", hix->GetName());
            continue;
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

                bool worked = Create_HinksPix_HSEQ_File(fseq, drive + wxFileName::GetPathSeparator() + shortHseqName, hix, slave1, slave2, errorMsg);

                wxString auName;
                if (worked && !media.IsEmpty())
                {
                    wxString tempFile;
                    auName = shortName + ".au";
                    prgs.Update(++count, "Generating AU File " + auName);

                    AudioLoader audioLoader( media.ToStdString(), true );
                    worked &= audioLoader.loadAudioData();

                    if ( worked )
                    {
                        worked &= Make_AU_From_ProcessedAudio( audioLoader.processedAudio(), drive + wxFileName::GetPathSeparator() + auName, errorMsg );
                    }
                    else
                    {
                        AudioLoader::State loaderState = audioLoader.state();

                        AudioReaderDecoderInitState decoderInitState = AudioReaderDecoderInitState::NoInit;
                        audioLoader.readerDecoderInitState( decoderInitState );

                        AudioResamplerInitState resamplerInitState = AudioResamplerInitState::NoInit;
                        audioLoader.resamplerInitState( resamplerInitState );

                        logger_base.error( "HinksPixExportDialog export - loading audio fails - %d : %d : %d",
                                           int( loaderState ), int( decoderInitState ), int (resamplerInitState ) );
                    }
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
        createPlayList(songs, drive);
        createSchedule(drive);
        createModeFile(drive, GetChoiceValueIndex(MODE_COL + rowStr));
    }

    SaveSettings();
    prgs.Hide();
    if(error)
        DisplayError("HinksPix File Generation Error\n" + errorMsg);
    else
        wxMessageBox("HinksPix File Generation Complete");
      // EndDialog(wxID_CLOSE);
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

void HinksPixExportDialog::OnChoiceSelected(wxCommandEvent& event)
{
    wxString const text = event.GetString();
    if (text.IsEmpty())
        return;

    auto item = event.GetEventObject();
    if (item) {
        wxChoice* cb = dynamic_cast<wxChoice*>(item);
        if (cb) {
            auto name = cb->GetName();
            if (name.Contains(SLAVE1_COL) || name.Contains(SLAVE2_COL))
            {
                int row = 0;
                for (auto hix : _hixControllers)
                {
                    std::string const rowStr = std::to_string(row);
                    ++row;
                    wxString const slaveName1 = GetChoiceValue(SLAVE1_COL + rowStr);
                    wxString const slaveName2 = GetChoiceValue(SLAVE2_COL + rowStr);
                    if (name == SLAVE1_COL + rowStr || name == SLAVE2_COL + rowStr)
                    {
                        if (!CheckSlaveControllerSizes(hix, getSlaveController(slaveName1), getSlaveController(slaveName2)))
                        {
                            cb->SetSelection(0);
                            event.Skip();
                            return;
                        }
                        if (name == SLAVE1_COL + rowStr && slaveName2 == text)
                        {
                            DisplayError(wxString::Format("Cannot Set Slave Controller 1 and 2 as the same '%s' ", text));
                            cb->SetSelection(0);
                            event.Skip();
                            return;
                        }
                        if (name == SLAVE2_COL + rowStr && slaveName1 == text)
                        {
                            DisplayError(wxString::Format("Cannot Set Slave Controller 1 and 2 as the same '%s' ", text));
                            cb->SetSelection(0);
                            event.Skip();
                            return;
                        }
                        continue;
                    }

                    if (text == slaveName1 || text == slaveName2)
                    {
                        DisplayError(wxString::Format("Cannot use the Same Slave Controller accross multiple HinksPix Controllers '%s' ", text));
                        cb->SetSelection(0);
                        event.Skip();
                        return;
                    }
                }
            }            
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

void HinksPixExportDialog::createModeFile(wxString const& drive, int mode)
{
    //0=Master, 1=Slave
    wxFile f;
    wxString const filename = drive + "SD_StandAlone.sys";

    if (mode == 2)//2=Skip, delete file that sets mode
    {
        wxRemoveFile(filename);
        return;
    }

    f.Open(filename, wxFile::write);

    if (f.IsOpened())
    {
        auto const line = wxString::Format("M  %d\r\n", mode);
        f.Write(line);
        f.Write("  \r\n");
        f.Close();
    }
}

bool HinksPixExportDialog::Create_HinksPix_HSEQ_File(wxString const& fseqFile, wxString const& shortHSEQName, ControllerEthernet* hix, ControllerEthernet* slave1, ControllerEthernet* slave2, wxString & errorMsg)
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

    uint32_t const ogNumber_of_Frames = xf->getNumFrames();
    uint32_t const ogNumChannels = xf->getChannelCount();
    int const ogFrame_Rate = xf->getStepTime();
    if (ogFrame_Rate != 50)
    {
        errorMsg = wxString::Format("HinksPix Failed Framerate must be 50Ms FSEQ %s", fseqFile);
        logger_base.error(errorMsg);
        return false;
    }

    std::vector<std::pair<uint32_t, uint32_t>> rng;
    rng.push_back(std::pair<uint32_t, uint32_t>(0, ogNumChannels));
    xf->prepareRead(rng);

    // acquire channel data on all controllers
    int32_t ef_Num_Channel_To_Write = hix->GetChannels();

    if (slave1)
        ef_Num_Channel_To_Write += slave1->GetChannels();

    if (slave2)
        ef_Num_Channel_To_Write += slave2->GetChannels();

    // read file ready -- do write file
    std::unique_ptr<FSEQFile> ef(new HSEQFile(shortHSEQName, hix, slave1, slave2, ogNumChannels));
    if (!ef)
    {
        errorMsg = wxString::Format("HinksPix Failed Write opening FSEQ %s", shortHSEQName);
        logger_base.error(errorMsg);
        return false;
    }
   
    uint8_t* src, * dest;

    ef->setChannelCount(ef_Num_Channel_To_Write);
    ef->setStepTime(ogFrame_Rate);
    ef->setNumFrames(ogNumber_of_Frames);

    ef->writeHeader();	// ready for frame data

    uint8_t* WriteBuf = new uint8_t[ef_Num_Channel_To_Write];

    // read buff
    uint8_t* tmpBuf = new uint8_t[ogNumChannels];

    uint32_t frame = 0;

    //mostly copied from Joe's pull request(#1441) in Jan 2019
    while (frame < ogNumber_of_Frames)
    {
        FSEQFile::FrameData* data = xf->getFrame(frame);

        data->readFrame(tmpBuf, ogNumChannels);	// we have a read frame

        // move wanted write channels into write frame buffer
        src = tmpBuf + hix->GetStartChannel() - 1;		 // start of my data with global channel array
        dest = WriteBuf;
        memmove(dest, src, hix->GetChannels());
        dest += hix->GetChannels();
        
        if (slave1)
        {
            src = tmpBuf + slave1->GetStartChannel() - 1;
            memmove(dest, src, slave1->GetChannels());
            dest += slave1->GetChannels();
        }

        if (slave2)
        {
            src = tmpBuf + slave2->GetStartChannel() - 1;
            memmove(dest, src, slave2->GetChannels());
            dest += slave2->GetChannels();
        }        

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

bool HinksPixExportDialog::Make_AU_From_ProcessedAudio( const std::vector<int16_t>&processedAudio, wxString const& AU_File, wxString& errorMsg )
{
    //this format was copied from Joe's HSA 2.0 JavaScript sourcecode
    uint8_t header[24];
    ::memset( header, 0, 24 );

    uint32_t sz = processedAudio.size() * sizeof(int16_t);

    uint32_t magic = (0x2e << 24) | (0x73 << 16) | (0x6e << 8) | 0x64;
    write4ByteUInt( &header[0], magic );
    write4ByteUInt( &header[4], 24 );

    write4ByteUInt( &header[8], sz );

    write4ByteUInt( &header[12], 3) ;
    write4ByteUInt( &header[16], 44100 );//bitrate
    write4ByteUInt( &header[20], 2 );//channels?

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxFile fo;
    fo.Open( AU_File, wxFile::write );
    if ( !fo.IsOpened() )
    {
        errorMsg = wxString::Format("Error Creating the AU Audio file %s", AU_File);
        logger_base.error(errorMsg);
        return false;
    }

    fo.Write(&header, sizeof(header));
    fo.Write( processedAudio.data(), sz );
    fo.Close();

    return true;
}

bool HinksPixExportDialog::CheckSlaveControllerSizes(ControllerEthernet* controller, ControllerEthernet* slave1, ControllerEthernet* slave2)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int slaveUni, slaveUni2;
    slaveUni = slaveUni2 = getMaxSlaveControllerUniverses(controller);

    if (slave1)
    {
        if (slave1->GetOutputCount()> 32)
        {
            logger_base.error("HinksPixExportDialog export - Slave Controller '%s' has too many Unverses, Max is 32 Currently Used is %d",
                slave1->GetName().c_str(), slave1->GetOutputCount());

            DisplayError(wxString::Format("Slave Controller '%s' has too many Unverses, Max is 32 Currently Used is %d",
                slave1->GetName().c_str(), slave1->GetOutputCount()));
            return false;
        }
        slaveUni -= slave1->GetOutputCount();
    }

    if (slave2)
    {
        if (slave2->GetOutputCount() > 16)
        {
            logger_base.error("HinksPixExportDialog export - Slave Controller '%s' has too many Unverses, Max is 16 Currently Used is %d",
                slave2->GetName().c_str(), slave2->GetOutputCount());

            DisplayError(wxString::Format("Slave Controller '%s' has too many Unverses, Max is 16 Currently Used is %d",
                slave2->GetName().c_str(), slave2->GetOutputCount()));
            return false;
        }
        slaveUni -= slave2->GetOutputCount();
    }

    if (slaveUni >= 0)
        return true;

    logger_base.error("HinksPixExportDialog export - too many Slave Controller Unverses - '%s' : Max %d Used %d",
        controller->GetName().c_str(), slaveUni2, (slaveUni2 - slaveUni));

    DisplayError(wxString::Format("Too Many Slave Controller Unverses off '%s': Max %d Used %d\n", controller->GetName().c_str(), slaveUni2, (slaveUni2 - slaveUni)));

    return false;
}

int HinksPixExportDialog::getMaxSlaveControllerUniverses(ControllerEthernet* controller)
{
    if (controller->GetModel().find("PRO") != std::string::npos)   // PRO
    {
        if (controller->GetOutputCount() <= ((3 * 16) + 1))
        {
            return (33 + 17);
        }
        else if (controller->GetOutputCount() <= ((5 * 16) + 1))
        {
            return (33);
        }
        else if (controller->GetOutputCount() <= ((6 * 16) + 1))
        {
            return (17);
        }
    }

    if (controller->GetModel().find("EasyLights") != std::string::npos)  // easy
    {
        if (controller->GetOutputCount() <= ((1 * 16) + 1))
        {
            return (33);

        }
        else if (controller->GetOutputCount() <= ((2 * 16) + 1))
        {
            return (17);
        }
    }
    return 0;
}

void HinksPixExportDialog::AddInstanceHeader(const wxString& h) {
    wxPanel* Panel1 = new wxPanel(HinkControllerList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER | wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    wxBoxSizer* BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* StaticText3 = new wxStaticText(Panel1, wxID_ANY, h, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    BoxSizer1->Add(StaticText3, 1, wxLEFT | wxRIGHT | wxEXPAND, 5);
    Panel1->SetSizer(BoxSizer1);
    BoxSizer1->Fit(Panel1);
    BoxSizer1->SetSizeHints(Panel1);
    HinkControllerSizer->Add(Panel1, 1, wxALL | wxEXPAND, 0);
}

bool HinksPixExportDialog::GetCheckValue(const wxString& col) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxCheckBox* cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->GetValue();
        }
    }
    return false;
}

int HinksPixExportDialog::GetChoiceValueIndex(const wxString& col) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetSelection();
        }
    }
    return 0;
}

wxString HinksPixExportDialog::GetChoiceValue(const wxString& col) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetStringSelection();
        }
    }
    return "";
}

void HinksPixExportDialog::SetChoiceValueIndex(const wxString& col, int i) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->SetSelection(i);
        }
    }
}

void HinksPixExportDialog::SetChoiceValue(const wxString& col, const std::string& value) {
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

void HinksPixExportDialog::SetCheckValue(const wxString& col, bool b) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxCheckBox* cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->SetValue(b);
        }
    }
}

void HinksPixExportDialog::SetDropDownItems(const wxString& col, const wxArrayString& items) {
    wxWindow* w = HinkControllerList->FindWindow(col);
    if (w) {
        wxItemContainer* cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->Set(items);
        }
    }
}
