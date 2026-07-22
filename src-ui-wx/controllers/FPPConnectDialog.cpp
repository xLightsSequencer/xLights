//(*InternalHeaders(FPPConnectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/regex.h>
#include <wx/volume.h>
#include <wx/progdlg.h>
#include <wx/dir.h>
#include <wx/hyperlink.h>
#include <wx/choicdlg.h>
#include <wx/dcclient.h>

#include "FPPConnectDialog.h"
#include "xLightsMain.h"
#include "settings/XLightsConfigAdapter.h"
#include "controllers/FPP.h"
#include "render/SequenceFile.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "outputs/ControllerEthernet.h"
#include "utils/CurlManager.h"
#include "controllers/ControllerCaps.h"
#include "utils/ip_utils.h"
#include "FPPUploadProgressDialog.h"
#include "layout/ModelPreview.h"

#include "nlohmann/json.hpp"

#include <log.h>

#include "utils/XsqFileScanner.h"
#include "render/FSEQFile.h"
#include "Parallel.h"
#include "discovery/Discovery.h"
#include "setup/DiscoveryAuthDialog.h"
#include "controllers/Falcon.h"
#include "controllers/Experience.h"
#include "controllers/PowerDMX.h"
#include <algorithm>
#include <memory>

//(*IdInit(FPPConnectDialog)
const wxWindowID FPPConnectDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_CHOICE_FILTER = wxNewId();
const wxWindowID FPPConnectDialog::ID_STATICTEXT2 = wxNewId();
const wxWindowID FPPConnectDialog::ID_CHOICE_FOLDER = wxNewId();
const wxWindowID FPPConnectDialog::ID_STATICTEXT3 = wxNewId();
const wxWindowID FPPConnectDialog::ID_STATICTEXT4 = wxNewId();
const wxWindowID FPPConnectDialog::ID_PANEL2 = wxNewId();
const wxWindowID FPPConnectDialog::ID_PANEL1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_SPLITTERWINDOW1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_BUTTON1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_BUTTON2 = wxNewId();
const wxWindowID FPPConnectDialog::ID_CHECKBOX1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_BUTTON_Upload = wxNewId();
//*)

static const long ID_FPP_INSTANCE_LIST = wxNewId();

static const long ID_POPUP_MNU_SORT_NAME = wxNewId();
static const long ID_POPUP_MNU_SORT_IP = wxNewId();
static const long ID_POPUP_MNU_SELECT_ALL = wxNewId();
static const long ID_POPUP_MNU_DESELECT_ALL = wxNewId();
static const long ID_POPUP_MNU_SORT_UPLOAD = wxNewId();
static const long ID_POPUP_MNU_CAPE_SELECT_ALL = wxNewId();
static const long ID_POPUP_MNU_CAPE_DESELECT_ALL = wxNewId();
static const long ID_POPUP_MNU_MEDIA_DESELECT_ALL = wxNewId();
static const long ID_POPUP_MNU_SELECT_HIGH = wxNewId();
static const long ID_POPUP_MNU_DESELECT_HIGH = wxNewId();
static const long ID_POPUP_MNU_SELECT_BATCH = wxNewId();
static const long ID_POPUP_MNU_SELECT_FAILED = wxNewId();
static const long ID_POPUP_MNU_SELECT_SUBNET = wxNewId();

wxString locationSortCol = "ip";

#define SORT_SEQ_NAME_COL 0
#define SORT_SEQ_TIME_COL 1
#define SORT_SEQ_MEDIA_COL 2

BEGIN_EVENT_TABLE(FPPConnectDialog,wxDialog)
	//(*EventTable(FPPConnectDialog)
	//*)
    EVT_TREELIST_ITEM_CONTEXT_MENU(wxID_ANY, FPPConnectDialog::SequenceListPopup)

END_EVENT_TABLE()


class SequencePathData : public wxClientData {
public:
    wxString fseq;
    wxString media;
};

static wxString GetSequencePath(wxTreeListCtrl* ctrl, const wxTreeListItem& item) {
    auto* d = dynamic_cast<SequencePathData*>(ctrl->GetItemData(item));
    return d != nullptr ? d->fseq : ctrl->GetItemText(item);
}

static wxString GetMediaPath(wxTreeListCtrl* ctrl, const wxTreeListItem& item) {
    auto* d = dynamic_cast<SequencePathData*>(ctrl->GetItemData(item));
    return d != nullptr ? d->media : ctrl->GetItemText(item, 2);
}

static wxColour InstanceRowShade(wxWindow* win) {
    return win->GetBackgroundColour().ChangeLightness(IsDarkMode() ? 115 : 93);
}

static const std::string CHECK_COL = "ID_UPLOAD_";
static const std::string FSEQ_COL = "ID_FSEQTYPE_";
static const std::string MEDIA_COL = "ID_MEDIA_";
static const std::string MODELS_COL = "ID_MODELS_";
static const std::string UDP_COL = "ID_UDPOUT_";
static const std::string PROXY_COL = "ID_PROXY_";
static const std::string PLAYLIST_COL = "ID_PLAYLIST_";
static const std::string UPLOAD_CONTROLLER_COL = "ID_CONTROLLER_";

FPPConnectDialog::FPPConnectDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;

	//(*Initialize(FPPConnectDialog)
	wxBoxSizer* BoxSizer1;
	wxButton* cancelButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticText* StaticText3;

	Create(parent, wxID_ANY, _("FPP Upload"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetMinSize(wxSize(800,-1));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_3DSASH|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinimumPaneSize(100);
	SplitterWindow1->SetSashGravity(0.5);
	FPPInstanceList = new wxScrolledWindow(SplitterWindow1, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FPPInstanceList->SetMinSize(wxDLG_UNIT(SplitterWindow1,wxSize(800,100)));
	FPPInstanceSizer = new wxFlexGridSizer(0, 12, 0, 0);
	FPPInstanceList->SetSizer(FPPInstanceSizer);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 6, 0, 0);
	FlexGridSizer3->AddGrowableCol(5);
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
	Selected_Label = new wxStaticText(Panel1, ID_STATICTEXT3, _("Selected: 0"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer3->Add(Selected_Label, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer1->Add(40,0,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	ShowDirLabel = new wxStaticText(Panel1, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	BoxSizer1->Add(ShowDirLabel, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(BoxSizer1, 1, wxEXPAND, 0);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxEXPAND, 0);
	CheckListBoxHolder = new wxPanel(Panel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	CheckListBoxHolder->SetMinSize(wxSize(-1,100));
	FlexGridSizer2->Add(CheckListBoxHolder, 1, wxALL|wxEXPAND, 0);
	Panel1->SetSizer(FlexGridSizer2);
	SplitterWindow1->SplitHorizontally(FPPInstanceList, Panel1);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 6, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	FlexGridSizer4->AddGrowableRow(0);
	AddFPPButton = new wxButton(this, ID_BUTTON1, _("Add FPP"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer4->Add(AddFPPButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ReDiscover = new wxButton(this, ID_BUTTON2, _("Re-Discover"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer4->Add(ReDiscover, 1, wxALL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxEXPAND, 5);
	KeepWinOpen = new wxCheckBox(this, ID_CHECKBOX1, _("Keep Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	KeepWinOpen->SetValue(false);
	FlexGridSizer4->Add(KeepWinOpen, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Upload = new wxButton(this, ID_BUTTON_Upload, _("Upload"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Upload"));
	FlexGridSizer4->Add(Button_Upload, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_CANCEL"));
	FlexGridSizer4->Add(cancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	Fit();

	Connect(ID_CHOICE_FILTER, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&FPPConnectDialog::OnChoiceFilterSelect);
	Connect(ID_CHOICE_FOLDER, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&FPPConnectDialog::OnChoiceFolderSelect);
	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPPConnectDialog::OnAddFPPButtonClick);
	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPPConnectDialog::OnFPPReDiscoverClick);
	Connect(ID_BUTTON_Upload, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPPConnectDialog::OnButton_UploadClick);
	Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&FPPConnectDialog::OnClose);
	//*)

    FPPInstanceList->Bind(wxEVT_PAINT, &FPPConnectDialog::OnInstanceListPaint, this);

    ShowDirLabel->SetFont(ShowDirLabel->GetFont().Bold());
    ShowDirLabel->SetLabel(xLightsFrame::CurrentDir);

    CheckListBox_Sequences = new wxTreeListCtrl(Panel1, wxID_ANY,
                                                wxDefaultPosition, wxDefaultSize,
                                                wxTL_CHECKBOX | wxTL_MULTIPLE, "ID_TREELISTVIEW_SEQUENCES");
    Connect(CheckListBox_Sequences->GetId(), wxEVT_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&FPPConnectDialog::OnSequenceListToggled);
    CheckListBox_Sequences->SetMinSize(wxSize(-1, 100));
    CheckListBox_Sequences->AppendColumn("Sequence", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Modified Date", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Media", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Channel Count", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);

    auto* config = GetXLightsConfig();
    auto seqSortCol = config->ReadLong("xLightsFPPConnectSequenceSortCol", SORT_SEQ_NAME_COL);
    auto seqSortOrder = config->ReadBool("xLightsFPPConnectSequenceSortOrder", true);
    CheckListBox_Sequences->SetSortColumn(seqSortCol, seqSortOrder);

    wxDataViewCtrl* seqDataView = CheckListBox_Sequences->GetDataView();
    seqDataView->SetWindowStyleFlag(seqDataView->GetWindowStyleFlag() | wxDV_ROW_LINES);
    seqDataView->SetAlternateRowColour(InstanceRowShade(CheckListBox_Sequences));
    wxItemAttr headerAttr;
    headerAttr.SetFont(CheckListBox_Sequences->GetFont().Bold());
    seqDataView->SetHeaderAttr(headerAttr);

    FlexGridSizer2->Replace(CheckListBoxHolder, CheckListBox_Sequences, true);

    CheckListBoxHolder->Destroy();

    FlexGridSizer2->Layout();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    wxProgressDialog prgs("Discovering FPP Instances",
                          "Discovering FPP Instances", 100, parent);
    prgs.Pulse("Discovering FPP Instances");
    prgs.Show();

    wxDiscoveryDelegate delegate(this);
    instances = static_cast<xLightsFrame*>(GetParent())->DiscoverFPPInstances(&delegate);

    wxPanel *p1 = AddInstanceHeader("Upload", "Enable to Upload Files/Configs to this FPP Device.");
    p1->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::UploadPopupMenu, nullptr, this);
    wxPanel *p2 = AddInstanceHeader("HostName/IP Address", "FPP's hostname and current IP Address.");
    p2->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::HostSortMenu, nullptr, this);
    AddInstanceHeader("Mode", "FPP Mode.");
    wxPanel* p4 = AddInstanceHeader("Media", "Enable to upload MP3, MP4 or WAV files to play your show music.*\n \nSuggested to only enable on the FPP device that plays your audio.\n \n* - Special use cases for sending video files to a Virtual Matrix.");
    p4->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&FPPConnectDialog::MediaPopupMenu, nullptr, this);
    AddInstanceHeader("UDP Out", "'None'- Device is not going to send Pixel data across the network.\n \n'All' This will send pixel data over your Show Network from FPP instance to all controllers marked as 'ACTIVE'.\n \n'Proxied' will send pixel data to only controllers that are set as proxies on this FPP device.");
    AddInstanceHeader("Add Proxies", "Automatically adds proxy settings to this FPP device");
    wxPanel *p5 = AddInstanceHeader("Upload Outputs", "When selected will upload latest inputs/outputs from xLights to your FPP device.\n \nThis may Restart FPPD and disrupt a running show.");
    p5->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::CapePopupMenu, nullptr, this);
    AddInstanceHeader("FSEQ Type", "FSEQ file version.\n \nV2 Sparse/zstd is recommended - Unless not supported by your controller");
    AddInstanceHeader("Models", "Upload models for selected controller to FPP for Display Testing.\n \nTypically not required and will Restart FPPD when selected.");
    AddInstanceHeader("Playlist","Select playlist to add uploaded sequences to.");
    AddInstanceHeader("Description", "Description of this FPP device from FPP Network settings.");
    AddInstanceHeader("Version", "FPP software version.");

    prgs.Pulse("Populating FPP instance list");
    if (config != nullptr) {
        config->Read("FPPConnectLocationSort", &locationSortCol);
    }
    if (locationSortCol == "name") {
        instances.sort(sortByName);
    } else {
        instances.sort([this](const FPP* a, const FPP* b) {
            std::vector<int> aComponents = SplitIP(a->ipAddress);
            std::vector<int> bComponents = SplitIP(b->ipAddress);
            for (size_t i = 0; i < aComponents.size() && i < bComponents.size(); ++i) {
                if (aComponents[i] != bComponents[i])
                    return aComponents[i] < bComponents[i];
            }
            return aComponents.size() < bComponents.size();
        });
    }
    PopulateFPPInstanceList(&prgs);
    prgs.Update(100);
    prgs.Hide();
    GetFolderList(xLightsFrame::CurrentDir);

    if (config != nullptr) {
        int filterSelect = -1;
        wxString folderSelect = "";
        config->Read("FPPConnectFilterSelection", &filterSelect);
        config->Read("FPPConnectFolderSelection", &folderSelect);
        if (filterSelect != wxNOT_FOUND)
        {
            ChoiceFilter->SetSelection(filterSelect);
        }
        int ifoldSelect = ChoiceFolder->FindString(folderSelect);
        if (ifoldSelect != wxNOT_FOUND) {
            ChoiceFolder->SetSelection(ifoldSelect);
        } else {
            ChoiceFolder->SetSelection(0);
        }
    }

    LoadSequences();

    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsFPPConnectPos", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 400)
            sz.SetWidth(400);
        if (sz.GetHeight() < 300)
            sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
    }
    int h = config->ReadLong("FPPConnectSashPosition", 0);
    if (h == 0) {
        SplitterWindow1->GetSize().GetHeight();
        h *= 33;
        h /= 100;
    }
    SplitterWindow1->SetSashPosition(h);
    EnsureWindowHeaderIsOnScreen(this);
    Layout();

    UpdateSeqCount();
}

void FPPConnectDialog::UpdateSeqCount()
{
    uint32_t items = 0;
    uint32_t selected = 0;

    auto item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        items++;
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED)
            selected++;
        item = CheckListBox_Sequences->GetNextItem(item);
    }

    Selected_Label->SetLabel(wxString::Format("Selected: %u/%u", selected, items));
    Panel1->Layout();
}

uint32_t FPPConnectDialog::GetSelectedSeqCount() {
    uint32_t selected = 0;

    auto item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED)
            selected++;
        item = CheckListBox_Sequences->GetNextItem(item);
    }

    return selected;
}

void FPPConnectDialog::OnSequenceListToggled(wxDataViewEvent& event)
{
    UpdateSeqCount();
}

std::vector<int> FPPConnectDialog::SplitIP(const wxString& ip) const {
    std::vector<int> components;
    wxString temp;
    for (size_t i = 0; i < ip.size(); ++i) {
        if (ip[i] == '.') {
            components.push_back(wxAtoi(temp));
            temp = "";
        } else {
            temp += ip[i];
        }
    }
    components.push_back(wxAtoi(temp)); // Add the last component
    return components;
}

void FPPConnectDialog::OnHostSortClick(wxCommandEvent& evt) {
    if (evt.GetId() == ID_POPUP_MNU_SORT_NAME || locationSortCol == "name") {
        SaveSettings(true);
        instances.sort(sortByName);
        locationSortCol = "name";
        PopulateFPPInstanceList();
    }
}

void FPPConnectDialog::OnIPSortClick(wxCommandEvent& evt) {
    if (evt.GetId() == ID_POPUP_MNU_SORT_IP || locationSortCol == "ip") {
        SaveSettings(true);
        //  Sort the instances ip IP using a number-aware comparison function (lambda)
        instances.sort([this](const FPP* a, const FPP* b) {
            std::vector<int> aComponents = SplitIP(a->ipAddress);
            std::vector<int> bComponents = SplitIP(b->ipAddress);
            for (size_t i = 0; i < aComponents.size() && i < bComponents.size(); ++i) {
                if (aComponents[i] != bComponents[i])
                    return aComponents[i] < bComponents[i];
            }
            return aComponents.size() < bComponents.size();
        });
        locationSortCol = "ip";
        PopulateFPPInstanceList();
    }
}

void FPPConnectDialog::HostSortMenu(wxContextMenuEvent& event) {
    wxMenu mnu;
    mnu.Append(ID_POPUP_MNU_SORT_NAME, "Sort by HostName");
    mnu.Append(ID_POPUP_MNU_SORT_IP, "Sort by IP Address");
    mnu.Connect(ID_POPUP_MNU_SORT_NAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnHostSortClick), NULL, this);
    mnu.Connect(ID_POPUP_MNU_SORT_IP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnIPSortClick), NULL, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::CapePopupMenu(wxContextMenuEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_POPUP_MNU_SELECT_ALL, "Select All");
    mnu.Append(ID_POPUP_MNU_DESELECT_ALL, "Deselect All");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnCapePopupClick), NULL, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::OnCapePopupClick(wxCommandEvent& event)
{
    int row = 0;
    for (const auto& inst : instances) {
        std::string l = inst->hostName + " - " + inst->ipAddress;
        if (inst->fppType == FPP_TYPE::FPP) {
            if (inst->supportedForFPPConnect()) {
                std::string rowStr = std::to_string(row);
                if (event.GetId() == ID_POPUP_MNU_SELECT_ALL) {
                    SetCheckValue(UPLOAD_CONTROLLER_COL + rowStr, true);
                } else if (event.GetId() == ID_POPUP_MNU_DESELECT_ALL) {
                    SetCheckValue(UPLOAD_CONTROLLER_COL + rowStr, false);
                }
            }
        }
        row++;
    }
}

void FPPConnectDialog::UploadPopupMenu(wxContextMenuEvent& event) {
    wxMenu mnu;
    mnu.Append(ID_POPUP_MNU_SELECT_ALL, "Select All");
    mnu.Append(ID_POPUP_MNU_DESELECT_ALL, "Deselect All");
    mnu.Append(ID_POPUP_MNU_SORT_UPLOAD, "Sort");
    mnu.Append(ID_POPUP_MNU_SELECT_SUBNET, "Select Subnet");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnUploadPopupClick), NULL, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::OnUploadPopupClick(wxCommandEvent& event) {
    if (ID_POPUP_MNU_SELECT_SUBNET == event.GetId()) {
        SelectIPsWithSubnet();
        return;
    }
    if (event.GetId() == ID_POPUP_MNU_SORT_UPLOAD) {
        instances.sort(sortByUpload);
        PopulateFPPInstanceList();
        return;
    }
      int row = 0;
      for (const auto& inst : instances) {
          std::string l = inst->hostName + " - " + inst->ipAddress;
          if (inst->fppType == FPP_TYPE::FPP) {
              if (inst->supportedForFPPConnect()) {
                  std::string rowStr = std::to_string(row);
                  if (event.GetId() == ID_POPUP_MNU_SELECT_ALL) {
                      SetCheckValue(CHECK_COL + rowStr, true);
                  } else if (event.GetId() == ID_POPUP_MNU_DESELECT_ALL) {
                      SetCheckValue(CHECK_COL + rowStr, false);
                  }
              }
          }
          row++;
      }
}

void FPPConnectDialog::SelectIPsWithSubnet() {
    wxArrayString subs;
    for (const auto& inst : instances) {

        wxString const& sub_net = BeforeLast(inst->ipAddress, '.') + ".0";
        if (wxNOT_FOUND == subs.Index(sub_net)) {
            subs.push_back(sub_net);
        }
    }
    wxString const& message = "Select Subnet";
    wxSingleChoiceDialog dlg(this, message, message, subs);

    if (dlg.ShowModal() != wxID_OK) {
       // return dlg.GetStringSelection();
        return;
    }

    std::string const& subnet = dlg.GetStringSelection();
    int row { 0 };
    for (const auto& inst : instances) {
        if (inst->fppType == FPP_TYPE::FPP) {
            if (inst->supportedForFPPConnect()) {
                std::string rowStr = std::to_string(row);
                if (ip_utils::IsInSameSubnet(subnet, inst->ipAddress)) {
                    SetCheckValue(CHECK_COL + rowStr, true);
                } else {
                    SetCheckValue(CHECK_COL + rowStr, false);
                }
            }
        } 
        row++;
    }
}

void FPPConnectDialog::MediaPopupMenu(wxContextMenuEvent& event) {
    wxMenu mnu;
    mnu.Append(ID_POPUP_MNU_DESELECT_ALL, "Deselect All");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnMediaPopupClick), NULL, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::OnMediaPopupClick(wxCommandEvent& event) {
    int row = 0;
    for (const auto& inst : instances) {
        std::string l = inst->hostName + " - " + inst->ipAddress;
        if (inst->fppType == FPP_TYPE::FPP) {
            if (inst->supportedForFPPConnect()) {
                std::string rowStr = std::to_string(row);
                if (event.GetId() == ID_POPUP_MNU_DESELECT_ALL) {
                    SetCheckValue(MEDIA_COL + rowStr, false);
                }
            }
        }
        row++;
    }
}

void FPPConnectDialog::PopulateFPPInstanceList(wxProgressDialog *prgs) {
    FPPInstanceList->Freeze();
    //remove all the children from the first upload checkbox on
    wxWindow *w = FPPInstanceList->FindWindow(CHECK_COL + "0");
    while (w) {
        wxWindow *tmp = w->GetNextSibling();
        w->Destroy();
        w = tmp;
    }
    while (FPPInstanceSizer->GetItemCount () > 12) {
        FPPInstanceSizer->Remove(12);
    }

    std::list<std::string> discoveredControllers;
    for (const auto& c : instances) discoveredControllers.push_back(c->ipAddress);
    for (const auto& ctrl : this->_outputManager->GetControllers()) {
        auto c = dynamic_cast<ControllerEthernet*>(ctrl);
        if (c != nullptr && c->GetActive() == Controller::ACTIVESTATE::ACTIVEINXLIGHTSONLY) {
            if (std::find(discoveredControllers.begin(), discoveredControllers.end(), c->GetResolvedIP()) == discoveredControllers.end()) {
                FPP* missing = new FPP(c->GetResolvedIP());
                missing->hostName = c->GetIP();
                instances.push_back(missing);
            }
        }
    }

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        wxCheckBox *doUploadCheckbox = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, CHECK_COL + rowStr);
        FPPInstanceSizer->Add(doUploadCheckbox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);

        std::string lhn = "http://" + inst->hostName;
        if (inst->hostName.find('.') == std::string::npos) {
            lhn += ".local";
        }
        std::string lip = "http://" + inst->ipAddress;
        if (!inst->proxy().empty()) {
            lip = "http://" + inst->proxy() + "/proxy/" + inst->ipAddress;
            lhn = lip;
        }
        wxWindow* hostIPWidget;
        if (inst->hostName.empty() || inst->hostName == inst->ipAddress || inst->ipAddress.empty()) {
            std::string label = inst->hostName.empty() ? inst->ipAddress : inst->hostName;
            auto link1 = new wxHyperlinkCtrl(FPPInstanceList, wxID_ANY, label, inst->ipAddress.empty() ? lhn : lip, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE, "ID_HOSTNAME_" + rowStr);
            link1->SetNormalColour(CyanOrBlue());
            link1->SetVisitedColour(CyanOrBlue());
            hostIPWidget = link1;
        } else {
            wxPanel* hostIPPanel = new wxPanel(FPPInstanceList, wxID_ANY);
            wxBoxSizer* hostIPSizer = new wxBoxSizer(wxHORIZONTAL);
            auto link1 = new wxHyperlinkCtrl(hostIPPanel, wxID_ANY, inst->hostName, lhn, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE, "ID_HOSTNAME_" + rowStr);
            link1->SetNormalColour(CyanOrBlue());
            link1->SetVisitedColour(CyanOrBlue());
            hostIPSizer->Add(link1, 0, wxALIGN_CENTER_VERTICAL, 0);
            hostIPSizer->Add(new wxStaticText(hostIPPanel, wxID_ANY, " / "), 0, wxALIGN_CENTER_VERTICAL, 0);
            auto link2 = new wxHyperlinkCtrl(hostIPPanel, wxID_ANY, inst->ipAddress, lip, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE, "ID_IP_" + rowStr);
            link2->SetNormalColour(CyanOrBlue());
            link2->SetVisitedColour(CyanOrBlue());
            hostIPSizer->Add(link2, 0, wxALIGN_CENTER_VERTICAL, 0);
            hostIPPanel->SetSizer(hostIPSizer);
            hostIPSizer->Fit(hostIPPanel);
            hostIPWidget = hostIPPanel;
        }
        hostIPWidget->SetMinSize(wxSize(hostIPWidget->GetBestSize().GetWidth() + 2 * hostIPWidget->GetCharWidth(), -1));
        FPPInstanceSizer->Add(hostIPWidget, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        auto descriptionLabel = new wxStaticText(FPPInstanceList, wxID_ANY, inst->description, wxDefaultPosition, wxDefaultSize, 0, "ID_DESCRIPTION_" + rowStr);
        descriptionLabel->SetMinSize(wxSize(descriptionLabel->GetBestSize().GetWidth() + 2 * descriptionLabel->GetCharWidth(), -1));

        auto mode = inst->mode;
        auto modeLabel = new wxStaticText(FPPInstanceList, wxID_ANY, mode, wxDefaultPosition, wxDefaultSize, 0, "ID_MODE_" + rowStr);
        FPPInstanceSizer->Add(modeLabel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        auto versionLabel = new wxStaticText(FPPInstanceList, wxID_ANY, inst->fullVersion, wxDefaultPosition, wxDefaultSize, 0, "ID_VERSION_" + rowStr);

        wxWindow* fseqWidget = nullptr;
        int fseqBorder = 1;
        //FSEQ Type listbox
        if (inst->fppType == FPP_TYPE::FPP) {
            if (!inst->supportedForFPPConnect()) {
                doUploadCheckbox->SetValue(false);
                doUploadCheckbox->Enable(false);

                if (!inst->fullVersion.empty()) {
                    fseqWidget = new wxStaticText(FPPInstanceList, wxID_ANY, "Unsupported", wxDefaultPosition, wxDefaultSize, 0, "ID_STATIC_TEXT_FS_" + rowStr);
                }else {
                    fseqWidget = new wxStaticText(FPPInstanceList, wxID_ANY, "Unavailable", wxDefaultPosition, wxDefaultSize, 0, "ID_STATIC_TEXT_FS_" + rowStr);
                }
            } else {
                wxChoice *Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, FSEQ_COL + rowStr);
                wxFont font = Choice1->GetFont();
                font.SetPointSize(font.GetPointSize() - 2);
                Choice1->SetFont(font);
                Choice1->Append(_("V1"));
                Choice1->Append(_("V2 zstd"));
                Choice1->Append(_("V2 Sparse/zstd"));
                Choice1->Append(_("V2 Sparse/Uncompressed"));
                Choice1->SetSelection(inst->mode == "master" ? 1 : 2);
                fseqWidget = Choice1;
                fseqBorder = 0;
            }
        } else if (inst->fppType == FPP_TYPE::FALCONV4V5 
            || inst->fppType == FPP_TYPE::POWERDMX) {
            wxChoice* Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, FSEQ_COL + rowStr);
            wxFont font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);
            Choice1->Append(_("V1"));
            Choice1->Append(_("V2 zlib"));
            Choice1->Append(_("V2 Sparse/zlib"));
            Choice1->Append(_("V2 Sparse/Uncompressed"));
            Choice1->Append(_("V2 Uncompressed"));
            Choice1->SetSelection(2);
            fseqWidget = Choice1;
            fseqBorder = 0;
        } else if (inst->fppType == FPP_TYPE::ESPIXELSTICK || inst->fppType == FPP_TYPE::GENIUS) {
            fseqWidget = new wxStaticText(FPPInstanceList, wxID_ANY, "V2 Sparse/Uncompressed", wxDefaultPosition, wxDefaultSize, 0, "ID_STATIC_TEXT_FS_" + rowStr);
        } else {
            fseqWidget = new wxStaticText(FPPInstanceList, wxID_ANY, "V1", wxDefaultPosition, wxDefaultSize, 0, "ID_STATIC_TEXT_FS_" + rowStr);
        }

        wxWindow* mediaWidget = nullptr;
        wxWindow* modelsWidget = nullptr;
        wxWindow* udpWidget = nullptr;
        wxWindow* proxyWidget = nullptr;
        wxWindow* playlistWidget = nullptr;
        bool playlistExpands = false;
        if (inst->fppType == FPP_TYPE::FPP && inst->supportedForFPPConnect()) {
            wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MEDIA_COL + rowStr);
            mediaWidget = CheckBox1;
            wxChoice* Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, MODELS_COL + rowStr);
            wxFont font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);

            Choice1->Append(_("None"));
            Choice1->Append(_("All"));
            Choice1->Append(_("Local"));
            Choice1->SetSelection(0);
            modelsWidget = Choice1;

            Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, UDP_COL + rowStr);
            font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);

            Choice1->Append(_("None"));
            Choice1->Append(_("All"));
            Choice1->Append(_("Proxied"));
            Choice1->SetSelection(0);
            udpWidget = Choice1;
            if (inst->solePlayer) {
				SetChoiceValueIndex(UDP_COL + rowStr, 1);
			} else if (inst->isaProxy) {
				SetChoiceValueIndex(UDP_COL + rowStr, 2);
			}
            wxCheckBox* CheckBoxProxy = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, PROXY_COL + rowStr);
            proxyWidget = CheckBoxProxy;
			CheckBoxProxy->SetValue(inst->isaProxy);

            //playlist combo box
            if (StartsWith(inst->mode, "player") || StartsWith(inst->mode, "master")) {
                wxComboBox* ComboBox1 = new wxComboBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxTE_PROCESS_ENTER, wxDefaultValidator, PLAYLIST_COL + rowStr);
                ComboBox1->Append(_(""));
                for (const auto& pl : inst->playlists) {
                    ComboBox1->Append(pl);
                }
                font = ComboBox1->GetFont();
                font.SetPointSize(font.GetPointSize() - 2);
                ComboBox1->SetFont(font);
                playlistWidget = ComboBox1;
                playlistExpands = true;
            } else {
                playlistWidget = new wxStaticText(FPPInstanceList, wxID_ANY, "");
            }

        } else if (inst->fppType == FPP_TYPE::FALCONV4V5) {
            // this probably needs to be moved as this is not really a zlib thing but only the falcons end up here today so I am going to put it here for now
            wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MEDIA_COL + rowStr);
            CheckBox1->SetValue(inst->mode != "remote");
            mediaWidget = CheckBox1;
        }

        auto c = _outputManager->GetControllers(inst->ipAddress);
        std::string m = FPP::GetModel(inst->pixelControllerType);
        if (c.size() == 1) {
            ControllerEthernet *controller = dynamic_cast<ControllerEthernet*>(c.front());
            const ControllerCaps *a = controller->GetControllerCaps();
            if (a) {
                m = a->GetModel();
            }
        }
        wxWindow* capeWidget = nullptr;
        if (m != "") {
            std::string desc = m;
            if (inst->panelSize != "") {
                desc += " - " + inst->panelSize;
            }
            wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, desc, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, UPLOAD_CONTROLLER_COL + rowStr);
            CheckBox1->SetValue(false);
            capeWidget = CheckBox1;
        }

        auto addCell = [this](wxWindow* w, int flags, int border) {
            if (w != nullptr) {
                FPPInstanceSizer->Add(w, 1, flags, border);
            } else {
                FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            }
        };
        addCell(mediaWidget, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        addCell(udpWidget, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        addCell(proxyWidget, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        addCell(capeWidget, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 1);
        addCell(fseqWidget, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, fseqBorder);
        addCell(modelsWidget, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        addCell(playlistWidget, playlistExpands ? (wxALL | wxEXPAND) : (wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL), playlistExpands ? 0 : 1);
        FPPInstanceSizer->Add(descriptionLabel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        FPPInstanceSizer->Add(versionLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);

        ++row;
    }
    ApplySavedHostSettings();

    const wxColour shade = InstanceRowShade(FPPInstanceList);
    const int cols = FPPInstanceSizer->GetCols();
    int idx = 0;
    for (auto* item : FPPInstanceSizer->GetChildren()) {
        int r = idx++ / cols - 1;
        if (r < 0 || (r % 2) == 0 || !item->IsWindow()) {
            continue;
        }
        wxWindow* win = item->GetWindow();
        if (dynamic_cast<wxStaticText*>(win) != nullptr || dynamic_cast<wxCheckBox*>(win) != nullptr || dynamic_cast<wxHyperlinkCtrl*>(win) != nullptr) {
            win->SetBackgroundColour(shade);
        } else if (dynamic_cast<wxPanel*>(win) != nullptr) {
            win->SetBackgroundColour(shade);
            for (auto* child : win->GetChildren()) {
                child->SetBackgroundColour(shade);
            }
        }
    }

    FPPInstanceList->FitInside();
    FPPInstanceList->SetScrollRate(100, 100);
    FPPInstanceList->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
    FPPInstanceList->Thaw();
    FPPInstanceList->Refresh();
}

wxPanel *FPPConnectDialog::AddInstanceHeader(const std::string &h, const std::string &tt) {
    wxPanel *Panel1 = new wxPanel(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    wxBoxSizer *BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *StaticText3 = new wxStaticText(Panel1, wxID_ANY, h, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    if(!tt.empty())
        StaticText3->SetToolTip(tt);
    BoxSizer1->Add(StaticText3, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);
    Panel1->SetSizer(BoxSizer1);
    BoxSizer1->Fit(Panel1);
    BoxSizer1->SetSizeHints(Panel1);
    FPPInstanceSizer->Add(Panel1, 1, wxALL|wxEXPAND, 0);
    return Panel1;
}

void FPPConnectDialog::AddInstanceRow(const FPP &inst) {

}

void FPPConnectDialog::OnInstanceListPaint(wxPaintEvent& event) {
    wxPaintDC dc(FPPInstanceList);

    const int cols = FPPInstanceSizer->GetCols();
    std::vector<wxRect> rows;
    int right = 0;
    int idx = 0;
    for (auto* item : FPPInstanceSizer->GetChildren()) {
        int r = idx++ / cols - 1;
        if (!item->IsWindow()) {
            continue;
        }
        wxWindow* win = item->GetWindow();
        if (!win->IsShown()) {
            continue;
        }
        right = std::max(right, win->GetRect().GetRight());
        if (r < 0) {
            continue;
        }
        if (r >= static_cast<int>(rows.size())) {
            rows.resize(r + 1);
        }
        if (rows[r].IsEmpty()) {
            rows[r] = win->GetRect();
        } else {
            rows[r].Union(win->GetRect());
        }
    }
    if (rows.size() < 2) {
        return;
    }

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(InstanceRowShade(FPPInstanceList)));
    for (size_t r = 1; r < rows.size(); r += 2) {
        if (!rows[r].IsEmpty()) {
            dc.DrawRectangle(0, rows[r].GetTop() - 1, right + 2, rows[r].GetHeight() + 2);
        }
    }
}

void FPPConnectDialog::OnPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    if (ID_POPUP_MNU_SELECT_BATCH == id) {
        SequenceSelector("BatchRendererItemList");
        return;
    }
    if (ID_POPUP_MNU_SELECT_FAILED == id) {
        SequenceSelector("FPPConnectFailedList");
        return;
    }
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
        bool isSelected = CheckListBox_Sequences->IsSelected(item);
        if (id == ID_POPUP_MNU_SELECT_ALL && !isChecked) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_POPUP_MNU_DESELECT_ALL && isChecked) {
            CheckListBox_Sequences->UncheckItem(item);
        } else if (id == ID_POPUP_MNU_SELECT_HIGH && isSelected) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_POPUP_MNU_DESELECT_HIGH && isSelected) {
            CheckListBox_Sequences->UncheckItem(item);
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
    UpdateSeqCount();
}

void FPPConnectDialog::SequenceSelector(const std::string regexKey) {

    auto* config = GetXLightsConfig();
    if (nullptr == config) {
        return;
    }
    wxString itcsv;
    config->Read(regexKey, &itcsv, "");

    if (!itcsv.IsEmpty()) {
        auto const& list = wxSplit(itcsv, ',');
        xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
        wxString const& showDirectory = frame->GetShowDirectory();
        wxString const& fseqDirectory = frame->GetFseqDirectory();

        if (list.empty()) {
            return;
        }
        auto uitem = CheckListBox_Sequences->GetFirstItem();
        while (uitem.IsOk()) {
            auto const& isChecked = CheckListBox_Sequences->GetCheckedState(uitem) == wxCHK_CHECKED;
            if (isChecked) {
                CheckListBox_Sequences->UncheckItem(uitem);
            } 
            uitem = CheckListBox_Sequences->GetNextItem(uitem);
        }
        for (auto const& seq : list) {
            auto const& xsqName = showDirectory + wxFileName::GetPathSeparator() + seq;
            wxFileName fseqFile(xsqName);
            fseqFile.SetExt("fseq");
            
            if (fseqDirectory != showDirectory) {
                fseqFile.SetPath(fseqDirectory);
            }
            auto const& fseqName = fseqFile.GetFullPath();
            auto item = CheckListBox_Sequences->GetFirstItem();
            while (item.IsOk()) {
                if (GetSequencePath(CheckListBox_Sequences, item) == fseqName) {
                    CheckListBox_Sequences->CheckItem(item);
                    break;
                }
                item = CheckListBox_Sequences->GetNextItem(item);
            }
        }
        UpdateSeqCount();
    }
}

FPPConnectDialog::~FPPConnectDialog()
{
    unsigned int sortCol = SORT_SEQ_NAME_COL;
    bool ascendingOrder = 1;
    auto* config = GetXLightsConfig();
    CheckListBox_Sequences->GetSortColumn(&sortCol, &ascendingOrder);
    config->Write("xLightsFPPConnectSequenceSortCol", sortCol);
    config->Write("xLightsFPPConnectSequenceSortOrder", ascendingOrder);

    int i = SplitterWindow1->GetSashPosition();
    config->Write("FPPConnectSashPosition", i);

    SaveWindowPosition("xLightsFPPConnectPos", this);

	//(*Destroy(FPPConnectDialog)
	//*)

    for (const auto& a : instances) {
        delete a;
    }
}

void FPPConnectDialog::LoadSequencesFromFolder(wxString const& dir) const
{
    std::set<wxString> knownPaths;
    for (auto item = CheckListBox_Sequences->GetFirstItem(); item.IsOk(); item = CheckListBox_Sequences->GetNextItem(item)) {
        knownPaths.insert(GetSequencePath(CheckListBox_Sequences, item));
    }
    LoadSequencesFromFolder(dir, knownPaths);
}

void FPPConnectDialog::AddSequenceListItem(const wxString& fseqPath, const std::string& media, std::set<wxString>& knownPaths) const
{
    wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(),
                                                             "  " + SequenceDisplayName(fseqPath) + "  ");
    auto* pathData = new SequencePathData();
    pathData->fseq = fseqPath;
    CheckListBox_Sequences->SetItemData(item, pathData);

    DisplayDateModified(fseqPath, item);

    std::string mediaName = media;
    if (FileExists(fseqPath)) {
        try {
            std::unique_ptr<FSEQFile> sf(FSEQFile::openFSEQFile(ToUTF8(fseqPath)));
            if (sf != nullptr) {
                auto ch = sf->getChannelCount();
                CheckListBox_Sequences->SetItemText(item, 3, wxString::Format("%llu", static_cast<unsigned long long>(ch)));
                if (mediaName.empty() && fseqPath.Lower().EndsWith(".fseq")) {
                    std::string m = sf->getMediaFilename();
                    if (!m.empty()) {
                        m = FileUtils::FixFile(std::string(""), m);
                        if (FileExists(m)) {
                            mediaName = m;
                        }
                    }
                }
            }
        } catch (...) {
            spdlog::warn("AddSequenceListItem: exception reading FSEQ file: {}", ToUTF8(fseqPath));
        }
    }

    if (!mediaName.empty()) {
        CheckListBox_Sequences->SetItemText(item, 2, wxFileName(ToWXString(mediaName)).GetFullName() + "  ");
        pathData->media = ToWXString(mediaName);
    }

    knownPaths.insert(fseqPath);
}

void FPPConnectDialog::LoadSequencesFromFolder(wxString const& dir, std::set<wxString>& knownPaths) const
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    spdlog::info("Scanning folder for sequences for FPP upload: {}", ToUTF8(dir));

    wxDir directory;
    if (!directory.Open(dir)) {
        spdlog::warn("LoadSequencesFromFolder: could not open directory: {}", ToUTF8(dir));
        return;
    }

    wxArrayString files;
    try {
    GetAllFilesInDir(dir, files, "*.x*");

    for (auto &filename : files) {
        wxFileName fn(filename);
        wxString file = fn.GetFullName();
        if (file != XLIGHTS_RGBEFFECTS_FILE
            && file != OutputManager::GetNetworksFileName()
            && file != XLIGHTS_KEYBINDING_FILE
            && (file.Lower().EndsWith("xml") || file.Lower().EndsWith("xsq"))
            && FileExists(filename)) {
            // Quick scan of first few KB to detect xLights sequence and media file
            XsqFileInfo info = ScanXsqFile(ToUTF8(filename));
            bool isSequence = info.isSequence;
            std::string mediaName = info.mediaFile;

            xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());

            // if fpp dir and show dir match then start with the fseq in the current dir ... only if that does not exist take the one from the show dir
            // this is consistent with the code in SaveSequence
            wxString fseqName = dir + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
            if (frame->GetFseqDirectory() != frame->GetShowDirectory() || !FileExists(fseqName)) {
                fseqName = frame->GetFseqDirectory() + GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
            }
            if (isSequence) {
                //need to check for existence of fseq
                if (!FileExists(fseqName)) {
                    isSequence = false;
                }
            }
            if (mediaName != "") {
                if (!FileExists(mediaName)) {
                    wxFileName fn(mediaName);
                    for (auto &md : frame->GetMediaFolders()) {
                        wxString tmn = md + GetPathSeparator() + fn.GetFullName();
                        if (FileExists(tmn)) {
                            mediaName = ToUTF8(tmn);
                            break;
                        }
                    }
                    if (!FileExists(mediaName)) {
                        std::string fixedMN = FileUtils::FixFile(ToUTF8(frame->CurrentDir), mediaName);
                        if (!FileExists(fixedMN)) {
                            spdlog::info("Could not find media: {} ", mediaName.c_str());
                            mediaName = "";
                        } else {
                            mediaName = fixedMN;
                        }
                    }
                }
            }
            spdlog::debug("XML:  {}   IsSeq:  {}    FSEQ:  {}   Media:  {}", (const char*)file.c_str(), isSequence, (const char*)fseqName.c_str(), (const char*)mediaName.c_str());
            if (isSequence) {

                // where you have show folders within show folders and sequences with the same name
                // such as when you have an imported subfolder this can create duplicates ... so lets first check
                // we dont already have the fseq file in the list

                if (knownPaths.find(fseqName) == knownPaths.end()) {
                    AddSequenceListItem(fseqName, mediaName, knownPaths);
                }
            }
        }
    }

    // we also need to load fseq/eseq files which may not have the same name as an xsq file
    files.clear();
    GetAllFilesInDir(dir, files, "*.?seq");
    for (auto& filename : files) {
        spdlog::debug("SEQ:  {}", ToUTF8(filename));

        if (knownPaths.find(filename) == knownPaths.end()) {
            AddSequenceListItem(filename, "", knownPaths);
        }
    }

    if (ChoiceFilter->GetSelection() == 0) {
        wxString file;
        bool fcont = directory.GetFirst(&file, wxEmptyString, wxDIR_DIRS);
        while (fcont) {
            if (file != "Backup") {
                LoadSequencesFromFolder(dir + wxFileName::GetPathSeparator() + file, knownPaths);
            }
            fcont = directory.GetNext(&file);
        }
    }
    } catch (...) {
        spdlog::warn("LoadSequencesFromFolder: exception scanning folder: {}", ToUTF8(dir));
    }
}

void FPPConnectDialog::LoadSequences()
{
    CheckListBox_Sequences->DeleteAllItems();
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxString fseqDir = frame->GetFseqDirectory();

    if (ChoiceFolder->GetSelection() == 0) {
        LoadSequencesFromFolder(fseqDir);
    }
    else {
        const wxString folder = ChoiceFolder->GetString(ChoiceFolder->GetSelection());
        LoadSequencesFromFolder(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder);
    }

    if (xLightsFrame::CurrentSeqXmlFile != nullptr) {
        wxString curSeq = xLightsFrame::CurrentSeqXmlFile->GetFullPath();
        if (!curSeq.StartsWith(xLightsFrame::CurrentDir)) {
            LoadSequencesFromFolder(curSeq);
        }
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        while (item.IsOk()) {
            if (curSeq == GetSequencePath(CheckListBox_Sequences, item)) {
                CheckListBox_Sequences->CheckItem(item);
                break;
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }
    }

    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        const wxString itcsv = wxString(config->Read("FPPConnectSelectedSequences"));

        if (!itcsv.IsEmpty()) {
            wxArrayString savedUploadItems = wxSplit(itcsv, ',');

            wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
            while (item.IsOk()) {
                if (savedUploadItems.Index(GetSequencePath(CheckListBox_Sequences, item)) != wxNOT_FOUND) {
                    CheckListBox_Sequences->CheckItem(item);
                }
                item = CheckListBox_Sequences->GetNextItem(item);
            }
        }
    }

    CheckListBox_Sequences->SetColumnWidth(2, wxCOL_WIDTH_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
}

void FPPConnectDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    Button_Upload->Enable(false);
    AddFPPButton->Enable(false);

    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        config->Write("FPPConnectFailedList", "");
    }
    config->Flush();
    for (const auto& inst : instances) {
		inst->faileduploads.clear();
	}

    std::vector<bool> doUpload(instances.size());
    int row = 0;
    int uploadCount = 0;
    for (row = 0; row < (int)doUpload.size(); ++row) {
        std::string rowStr = std::to_string(row);
        doUpload[row] = GetCheckValue(CHECK_COL + rowStr);
        ++uploadCount;
    }

    FPPUploadProgressDialog prgs(this);
    row = 0;
    for (const auto& inst : instances) {
        inst->_ui = static_cast<xLightsFrame*>(GetParent());
        // not in discovery so we can increase the timeouts to make sure things get transferred
        inst->defaultConnectTimeout = 5000;
        inst->messages.clear();
        std::string rowStr = std::to_string(row);
        if (doUpload[row]) {
            std::string l = inst->hostName + " - " + inst->ipAddress;
            wxGauge* gauge = prgs.addGauge(l);
            inst->setProgress({
                [gauge](int val) { gauge->SetValue(val); },
                [&prgs]() -> bool { return prgs.isCancelled(); },
                []() { wxYield(); }
            });
        } else {
            inst->setProgress({});
        }
        ++row;
    }
    if (uploadCount) {
        //prgs.SetSize(450, 400);
        prgs.scrolledWindow->SetSizer(prgs.scrolledWindowSizer);
        prgs.scrolledWindow->FitInside();
        prgs.scrolledWindow->SetScrollRate(5, 5);
        prgs.SetSizeHints(350, 400);
        prgs.Fit();
        prgs.setActionLabel("Preparing Configuration");

        CallAfter(&FPPConnectDialog::doUpload, &prgs, doUpload);
        int c = prgs.ShowModal();

        if (!c) {
            SaveSettings();
            auto kwo = this->KeepWinOpen->Get3StateValue();
            if (kwo != wxCHK_CHECKED) {
    #ifndef _DEBUG
            EndDialog(wxID_CLOSE);
    #endif
            }
        }
        Button_Upload->Enable(true);
        AddFPPButton->Enable(true);
    }
}

void FPPConnectDialog::doUpload(FPPUploadProgressDialog *prgs, std::vector<bool> doUpload) {
    
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    int pw, ph;
    frame->GetLayoutPreview()->GetVirtualCanvasSize(pw, ph);
    std::map<std::string, std::string> virtualDisplayData;
    FPP::CreateVirtualDisplayMap(frame->AllModels, frame->AllObjects, pw, ph, virtualDisplayData);
    bool cancelled = false;

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        if (!cancelled && doUpload[row]) {
            auto controller = _outputManager->GetControllers(inst->ipAddress);
            if (controller.size() == 1 && inst->ranges == "") {
                ControllerEthernet *ipcontroller = dynamic_cast<ControllerEthernet*>(controller.front());
                if (ipcontroller) {
                    uint32_t sc = ipcontroller->GetStartChannel() - 1;
                    inst->ranges = std::to_string(sc) + "-" + std::to_string(sc + ipcontroller->GetChannels() - 1);
                }
            }
            if (inst->fppType == FPP_TYPE::FPP) {
                std::string playlist = GetChoiceValue(PLAYLIST_COL + rowStr);
                if (playlist != "") {
                    cancelled |= inst->UploadPlaylist(playlist);
                }
                if (GetCheckValue(PROXY_COL + rowStr)) {
                    cancelled |= inst->UploadControllerProxies(_outputManager);
                }
                if (GetChoiceValueIndex(UDP_COL + rowStr) == 1) {
                    std::map<int, int> udpRanges;
                    auto outputs = inst->CreateUniverseFile(_outputManager->GetControllers(), false, &udpRanges);
                    cancelled |= inst->UploadUDPOut(outputs);
                    //add the UDP ranges into the list of ranges
                    std::map<int, int> rngs(udpRanges);
                    inst->FillRanges(rngs);
                    inst->SetNewRanges(rngs);
                    inst->SetRestartFlag();
                } else if (GetChoiceValueIndex(UDP_COL + rowStr) == 2) {
                    cancelled |= inst->UploadUDPOutputsForProxy(_outputManager);
                    inst->SetRestartFlag();
                }
                if (GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr)) {
                    auto c = _outputManager->GetControllers(inst->ipAddress);
                    if (c.size() == 1) {
                        bool controllerCancelled = false;
                        controllerCancelled |= inst->UploadPanelOutputs(&frame->AllModels, _outputManager, c.front());
                        controllerCancelled |= inst->UploadVirtualMatrixOutputs(&frame->AllModels, _outputManager, c.front());
                        controllerCancelled |= inst->UploadPixelOutputs(&frame->AllModels, _outputManager, c.front());
                        controllerCancelled |= inst->UploadSerialOutputs(&frame->AllModels, _outputManager, c.front());
                        controllerCancelled |= inst->SetInputUniversesBridge(c.front());
                        cancelled |= controllerCancelled;

                        if (!controllerCancelled) {
                            auto ts = FormatTimestamp();
                            auto* config = GetXLightsConfig();
                            auto ctrlName = c.front()->GetName();
                            config->Write(MakeControllerTimestampKey("LastInputUpload", ctrlName, frame->showDirectory), wxString::FromUTF8(ts.c_str()));
                            config->Write(MakeControllerTimestampKey("LastOutputUpload", ctrlName, frame->showDirectory), wxString::FromUTF8(ts.c_str()));
                            config->Flush();
                        }
                    }
                }
                if (GetChoiceValueIndex(MODELS_COL + rowStr) == 1) {
                    auto const& memoryMaps = inst->CreateModelMemoryMap(&frame->AllModels, 0, std::numeric_limits<int32_t>::max());
                    cancelled |= inst->UploadModels(memoryMaps);
                    cancelled |= inst->UploadDisplayMap(virtualDisplayData);
                    // model uploads currently still require a full restart
                    inst->SetRestartFlag(true);
                } else if (GetChoiceValueIndex(MODELS_COL + rowStr) == 2) {
                    auto c = _outputManager->GetControllers(inst->ipAddress);
                    if (c.size() == 1) {
                        auto const& memoryMaps = inst->CreateModelMemoryMap(&frame->AllModels, c.front()->GetStartChannel(), c.front()->GetEndChannel());
                        cancelled |= inst->UploadModels(memoryMaps);
                        // cancelled |= inst->UploadDisplayMap(displayMap);
                        inst->SetRestartFlag(true);
                    }
                }
                //if restart flag is now set, restart and recheck range
                inst->Restart(true);
            } else if (GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr) && controller.size() == 1) {
                BaseController* bc = BaseController::CreateBaseController(controller.front(), inst->ipAddress);
                if (bc->UploadForImmediateOutput(&frame->AllModels, _outputManager, controller.front(), frame)) {
                    auto ts = FormatTimestamp();
                    auto* config = GetXLightsConfig();
                    auto ctrlName = controller.front()->GetName();
                    config->Write(MakeControllerTimestampKey("LastInputUpload", ctrlName, frame->showDirectory), wxString::FromUTF8(ts.c_str()));
                    config->Write(MakeControllerTimestampKey("LastOutputUpload", ctrlName, frame->showDirectory), wxString::FromUTF8(ts.c_str()));
                    config->Flush();
                }
                delete bc;
            }
        }
        ++row;
    }
    row = 0;
    for (const auto& inst : instances) {
        if (!cancelled && doUpload[row]) {
            // update the channel ranges now that the config has been uploaded an fppd restarted
            inst->UpdateChannelRanges();
        }
        ++row;
    }
    row = 0;
    uint32_t seqCountToUpload = GetSelectedSeqCount();
    uint32_t seqCountUploaded = 0;
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED) {
            for (const auto& inst : instances) {
                inst->updateProgress(0, true);
            }
            seqCountUploaded++;
            wxString fseqRaw = GetSequencePath(CheckListBox_Sequences, item);
            std::string fseq = ToUTF8(fseqRaw);
            std::string media = ToUTF8(GetMediaPath(CheckListBox_Sequences, item));

            FSEQFile *seq = FSEQFile::openFSEQFile(fseq);
            if (seq) {
                // every frame is read in order below to build the upload
                seq->setReadPattern(FSEQFile::ReadPattern::Bulk);
                prgs->setActionLabel("Checking Media and FSEQ file for " + media + "/" + wxFileName(ToWXString(fseq)).GetFullName());
                row = 0;
                int uploadCount = 0;
                int prepareCount = 0;
                for (const auto& inst : instances) {
                    std::string rowStr = std::to_string(row);
                    if (!cancelled && doUpload[row]) {
                        std::string m2 = media;
                        if (!GetCheckValue(MEDIA_COL + rowStr)) {
                            m2 = "";
                        }

                        int fseqType = 0;
                        if (inst->fppType == FPP_TYPE::FPP) {
                            fseqType = GetChoiceValueIndex(FSEQ_COL + rowStr);
                        } else if (inst->fppType == FPP_TYPE::FALCONV4V5) {
                            fseqType = GetChoiceValueIndex(FSEQ_COL + rowStr);
                            // need to adjust so they are unique
                            if (fseqType == 1) fseqType = 5;
                            if (fseqType == 2) fseqType = 6;
                        } else {
                            fseqType = 3;
                        }
                        cancelled |= inst->PrepareUploadSequence(seq,
                                                                fseq, m2,
                                                                fseqType);
                    }
                    row++;
                }
                while (CurlManager::INSTANCE.processCurls()) {
                    wxYield();
                }
                row = 0;
                for (const auto& inst : instances) {
                    if (!cancelled && doUpload[row]) {
                        if (inst->WillUploadSequence()) {
                            uploadCount++;
                            if (inst->NeedCustomSequence()) {
                                prepareCount++;
                            }
                        } else {
                            inst->updateProgress(1000, true);
                        }
                    }
                    row++;
                }
                if (!cancelled && uploadCount) {
                    if (prepareCount) {
                        prgs->setActionLabel("Preparing FSEQ File for " + wxFileName(ToWXString(fseq)).GetFullName() + " (" + std::to_string(seqCountUploaded) + "/" + std::to_string(seqCountToUpload) + ")");
                        for (const auto& inst : instances) {
                            inst->updateProgress(0, false);
                        }
                        wxYield();
                        int lastDone = 0;
                        static const int FRAMES_TO_BUFFER = 50;
                        std::vector<std::vector<uint8_t>> frames(FRAMES_TO_BUFFER);
                        for (size_t x = 0; x < frames.size(); x++) {
                            frames[x].resize(seq->getMaxChannel() + 1);
                        }

                        for (size_t frame = 0; frame < seq->getNumFrames() && !cancelled; frame++) {
                            int donePct = frame * 1000 / seq->getNumFrames();
                            if (donePct != lastDone) {
                                lastDone = donePct;
                                for (const auto& inst : instances) {
                                    inst->updateProgress(donePct, false);
                                }
                                wxYield();
                            }

                            int lastBuffered = 0;
                            size_t startFrame = frame;
                            //Read a bunch of frames so each parallel thread has more info to work with before returning out here
                            while (lastBuffered < FRAMES_TO_BUFFER && frame < seq->getNumFrames()) {
                                FSEQFile::FrameData *f = seq->getFrame(frame);
                                if (f != nullptr) {
                                    if (!f->readFrame(&frames[lastBuffered][0], frames[lastBuffered].size())) {
                                        spdlog::error("FPPConnect FSEQ file corrupt.");
                                    }
                                    delete f;
                                }
                                lastBuffered++;
                                frame++;
                            }
                            frame--;
                            std::function<void(FPP * &, int)> func = [startFrame, lastBuffered, &frames, &doUpload](FPP* &inst, int row) {
                                if (doUpload[row]) {
                                    for (int x = 0; x < lastBuffered; x++) {
                                        inst->AddFrameToUpload(startFrame + x, &frames[x][0]);
                                    }
                                }
                            };
                            parallel_for(instances, func);
                        }
                    }
                    row = 0;
                    prgs->setActionLabel("Uploading " + wxFileName(ToWXString(fseq)).GetFullName() + " (" + std::to_string(seqCountUploaded) + "/" + std::to_string(seqCountToUpload) + ")");
                    for (const auto& inst : instances) {
                        inst->updateProgress(0, false);
                    }
                    wxYield();

                    for (const auto &inst : instances) {
                        if (!cancelled && doUpload[row]) {
                            cancelled |= inst->FinalizeUploadSequence();

                            if (inst->fppType == FPP_TYPE::FALCONV4V5) {
                                // a falcon
                                std::string proxy = "";
                                auto c = _outputManager->GetControllers(inst->ipAddress);
                                if (c.size() == 1) proxy = c.front()->GetFPPProxy();
                                Falcon falcon(inst->ipAddress, proxy);

                                if (falcon.IsConnected()) {
                                    std::string m2 = media;
                                    std::string rowStr = std::to_string(row);
                                    if (!GetCheckValue(MEDIA_COL + rowStr)) {
                                        if (m2 != "") {
                                            spdlog::debug("Media file {} not uploaded because media checkbox not selected.", (const char*)m2.c_str());
                                        }
                                        m2 = "";
                                    }
                                    std::function<bool(int, std::string)> updateProg = [&prgs, inst](int val, std::string msg)
                                    {
                                        prgs->setActionLabel(msg);
                                        inst->updateProgress(val, true);
                                        return true;
                                    };
                                    cancelled |= !falcon.UploadSequence(inst->GetTempFile(), fseq, inst->mode == "remote" ? "" : m2, updateProg);
                                }
                                else {
                                    spdlog::debug("Upload failed as FxxV4 is not connected.");
                                    cancelled = true;
                                }
                                inst->ClearTempFile();
                            } else if (inst->fppType == FPP_TYPE::GENIUS) {
                                // a Genius
                                std::string proxy;
                                auto c = _outputManager->GetControllers(inst->ipAddress);
                                if (c.size() == 1) {
                                    proxy = c.front()->GetFPPProxy();
                                }
                                Experience genius(inst->ipAddress, proxy);
                                if (genius.IsConnected()) {
                                        std::function<bool(int, std::string)> updateProg = [&prgs, inst](int val, std::string msg) {
                                        prgs->setActionLabel(msg);
                                        inst->updateProgress(val, true);
                                        return true;
                                    };
                                    cancelled |= !genius.UploadSequence(inst->GetTempFile(), fseq, updateProg);
                                } else {
                                    spdlog::debug("Upload failed as Genius is not connected.");
                                    cancelled = true;
                                }
                                inst->ClearTempFile();
                            } else if (inst->fppType == FPP_TYPE::POWERDMX) {
                                // a PowerDMX
                                std::string proxy;
                                auto c = _outputManager->GetControllers(inst->ipAddress);
                                if (c.size() == 1) {
                                    proxy = c.front()->GetFPPProxy();
                                }
                                PowerDMX powerMax(inst->ipAddress, proxy);
                                if (powerMax.IsConnected()) {
                                    std::function<bool(int, std::string)> updateProg = [&prgs, inst](int val, std::string msg) {
                                        prgs->setActionLabel(msg);
                                        inst->updateProgress(val, true);
                                        return true;
                                    };
                                    cancelled |= !powerMax.UploadSequence(inst->GetTempFile(), fseq, updateProg);
                                } else {
                                    spdlog::debug("Upload failed as PowerDMX is not connected.");
                                    cancelled = true;
                                }
                                inst->ClearTempFile();
                            }
                        }
                        row++;
                    }
                    while (CurlManager::INSTANCE.processCurls()) {
                        wxYield();
                    }
                    cancelled |= prgs->isCancelled();
                }
            }
            delete seq;
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
    row = 0;


    std::string messages;
    wxString failedUploadsList;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        if (inst->fppType == FPP_TYPE::FPP) {
            if (!cancelled && doUpload[row]) {
                std::string playlist = GetChoiceValue(PLAYLIST_COL + rowStr);
                if (playlist != "") {
                    cancelled |= inst->UploadPlaylist(playlist);
                }
                inst->Restart(true);
            }
        }
        if (!inst->messages.empty()) {
            messages += inst->ipAddress;
            if (inst->hostName != "" && inst->hostName != inst->ipAddress) {
                messages += "/";
                messages += inst->hostName;
            }
            messages += ":\n";
            for (auto &m : inst->messages) {
                messages += "    ";
                messages += m;
                messages += "\n";
            }
        }
        if (!inst->faileduploads.empty()) {
            for (auto& m : inst->faileduploads) {
                if (failedUploadsList.empty()) {
                    failedUploadsList = m;
                } else {
                    failedUploadsList += ",";
                    failedUploadsList += m;
                }
			}
		}
        row++;
    }
    if (!failedUploadsList.empty()) {
        auto* config = GetXLightsConfig();
        if (config != nullptr) {
            config->Write("FPPConnectFailedList", failedUploadsList);
        }
        config->Flush();
    };
    xLightsFrame* xlframe = static_cast<xLightsFrame*>(GetParent());
    if (messages != "") {
        xlframe->SetStatusText("FPP Connect Upload had errors or warnings", 0);
        wxMessageBox(messages, "Problems Uploading", wxOK | wxCENTRE, this);
        spdlog::warn("FPP Connect Upload had errors or warnings:\n" + messages);
        prgs->EndModal(2);
    } else {
        if (cancelled) {
            xlframe->SetStatusText("FPP Connect Upload Cancelled", 0);
            prgs->EndModal(1);
        } else {
            xlframe->SetStatusText("FPP Connect Upload Complete", 0);
            prgs->EndModal(0);
        }
    };
}

bool FPPConnectDialog::GetCheckValue(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxCheckBox *cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->GetValue();
        }
    }
    return false;
}
int FPPConnectDialog::GetChoiceValueIndex(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetSelection();
        }
    }
    return 0;
}

std::string FPPConnectDialog::GetChoiceValue(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxComboBox *comboBox = dynamic_cast<wxComboBox*>(w);
        if (comboBox) {
            return ToStdString(comboBox->GetValue());
        }

        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return ToStdString(cb->GetStringSelection());
        }
    }
    return "";
}

void FPPConnectDialog::SetChoiceValueIndex(const std::string &col, int i) {
    wxWindow *w = FPPInstanceList->FindWindow(ToWXString(col));
    if (w) {
        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            cb->SetSelection(i);
        }
    }
}
void FPPConnectDialog::SetCheckValue(const std::string &col, bool b) {
    wxWindow *w = FPPInstanceList->FindWindow(ToWXString(col));
    if (w) {
        wxCheckBox *cb = dynamic_cast<wxCheckBox*>(w);
        if (cb && cb->IsEnabled()) {
            cb->SetValue(b);
        }
    }

}

void FPPConnectDialog::SaveSettings(bool onlyInsts)
{
    auto* config = GetXLightsConfig();
    if (!onlyInsts) {
        wxString selected = "";
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        while (item.IsOk()) {
            bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
            if (isChecked) {
                if (selected != "") {
                    selected += ",";
                }
                selected += GetSequencePath(CheckListBox_Sequences, item);
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }

        config->Write("FPPConnectSelectedSequences", selected);
        config->Write("FPPConnectFilterSelection", ChoiceFilter->GetSelection());
        config->Write("FPPConnectFolderSelection", ChoiceFolder->GetString(ChoiceFolder->GetSelection()));
        config->Write("FPPConnectLocationSort", locationSortCol);
    }

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        wxString keyPostfx = (inst->uuid.empty() ? inst->ipAddress : inst->uuid);
        keyPostfx = Fixitup(keyPostfx);
        bool bval;
        int lval;
        // only save the settings if they are different from defaults, or if previously changed and saved - this will help new users with auto setting up UPD & proxy
        if (GetCheckValue(CHECK_COL + rowStr) != false || config->Read("FPPConnectUpload_" + Fixitup(inst->uuid), &bval)) {
            config->Write("FPPConnectUpload_" + keyPostfx, GetCheckValue(CHECK_COL + rowStr));
        }
        if (GetCheckValue(MEDIA_COL + rowStr) != false || config->Read("FPPConnectUploadMedia_" + Fixitup(inst->uuid), &bval)) {
            config->Write("FPPConnectUploadMedia_" + keyPostfx, GetCheckValue(MEDIA_COL + rowStr));
        }
        if (inst->fppType == FPP_TYPE::FPP && inst->supportedForFPPConnect()) {
            if (GetChoiceValueIndex(FSEQ_COL + rowStr) != 2 || config->Read("FPPConnectUploadFSEQType_" + Fixitup(inst->uuid), &lval)) {
                config->Write("FPPConnectUploadFSEQType_" + keyPostfx, GetChoiceValueIndex(FSEQ_COL + rowStr));
            }
        } else if (inst->fppType == FPP_TYPE::FALCONV4V5) {
            if (GetChoiceValueIndex(FSEQ_COL + rowStr) != 2 || config->Read("FPPConnectUploadFSEQType_" + Fixitup(inst->uuid), &lval)) {
                config->Write("FPPConnectUploadFSEQType_" + keyPostfx, GetChoiceValueIndex(FSEQ_COL + rowStr));
            }
        }
        if (GetChoiceValueIndex(MODELS_COL + rowStr) != 0 || config->Read("FPPConnectUploadModels_" + Fixitup(inst->uuid), &lval)) {
            config->Write("FPPConnectUploadModels_" + keyPostfx, GetChoiceValueIndex(MODELS_COL + rowStr));
        }
        if (GetChoiceValueIndex(UDP_COL + rowStr) > 0 || config->Read("FPPConnectUploadUDPOut_" + Fixitup(inst->uuid), &lval)) {
            config->Write("FPPConnectUploadUDPOut_" + keyPostfx, GetChoiceValueIndex(UDP_COL + rowStr));
        }
        if (GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr) != false || config->Read("FPPConnectUploadPixelOut_" + Fixitup(inst->uuid), &bval)) {
            config->Write("FPPConnectUploadPixelOut_" + keyPostfx, GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr));
        }
        if (GetCheckValue(PROXY_COL + rowStr) != false || config->Read("FPPConnectUploadProxy_" + Fixitup(inst->uuid), &bval)) {
            config->Write("FPPConnectUploadProxy_" + keyPostfx, GetCheckValue(PROXY_COL + rowStr));
        }
        row++;
    }
    config->Flush();
}

wxString FPPConnectDialog::Fixitup(wxString val) {
    val.Replace(":", "_");
    val.Replace("/", "_");
    val.Replace("\\", "_");
    val.Replace(".", "_");
    return val;
}

void FPPConnectDialog::ApplySavedHostSettings()
{
    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        int row = 0;
        for (const auto& inst : instances) {
            std::string rowStr = std::to_string(row);

            bool bval;
            int lval;
            if (config->Read("FPPConnectUpload_" + Fixitup(inst->uuid), &bval)) {
                SetCheckValue(CHECK_COL + rowStr, bval);
                inst->upload = bval;
            } else if (config->Read("FPPConnectUpload_" + Fixitup(inst->ipAddress), &bval)) {
                SetCheckValue(CHECK_COL + rowStr, bval);
                inst->upload = bval;
            }
            if (config->Read("FPPConnectUploadFSEQType_" + Fixitup(inst->uuid), &lval)) {
                SetChoiceValueIndex(FSEQ_COL + rowStr, lval);
            } else if (config->Read("FPPConnectUploadFSEQType_" + Fixitup(inst->ipAddress), &lval)) {
                SetCheckValue(FSEQ_COL + rowStr, lval);
            }
            if (config->Read("FPPConnectUploadMedia_" + Fixitup(inst->uuid), &bval)) {
                SetCheckValue(MEDIA_COL + rowStr, bval);
            } else if (config->Read("FPPConnectUploadMedia_" + Fixitup(inst->ipAddress), &bval)) {
                SetCheckValue(MEDIA_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadModels_" + Fixitup(inst->uuid), &lval)) {
                SetChoiceValueIndex(MODELS_COL + rowStr, lval);
            } else if (config->Read("FPPConnectUploadModels_" + Fixitup(inst->ipAddress), &lval)) {
                SetCheckValue(MODELS_COL + rowStr, lval);
            }
            if (config->Read("FPPConnectUploadUDPOut_" + Fixitup(inst->uuid), &lval)) {
                SetChoiceValueIndex(UDP_COL + rowStr, lval);
            } else if (config->Read("FPPConnectUploadUDPOut_" + Fixitup(inst->ipAddress), &lval)) {
                SetCheckValue(UDP_COL + rowStr, lval);
            }
            if (config->Read("FPPConnectUploadPixelOut_" + Fixitup(inst->uuid), &bval)) {
                SetCheckValue(UPLOAD_CONTROLLER_COL + rowStr, bval);
            } else if (config->Read("FPPConnectUploadPixelOut_" + Fixitup(inst->ipAddress), &bval)) {
                SetCheckValue(UPLOAD_CONTROLLER_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadProxy_" + Fixitup(inst->uuid), &bval)) {
                SetCheckValue(PROXY_COL + rowStr, bval);
            } else if (config->Read("FPPConnectUploadProxy_" + Fixitup(inst->ipAddress), &bval)) {
                SetCheckValue(PROXY_COL + rowStr, bval);
            }
            row++;
        }
    }
}

void FPPConnectDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(0);
}

void FPPConnectDialog::SequenceListPopup(wxTreeListEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_POPUP_MNU_SELECT_ALL, "Select All");
    mnu.Append(ID_POPUP_MNU_DESELECT_ALL, "Clear Selections");
    mnu.Append(ID_POPUP_MNU_SELECT_HIGH, "Select Highlighted");
    mnu.Append(ID_POPUP_MNU_DESELECT_HIGH, "Deselect Highlighted");
    mnu.Append(ID_POPUP_MNU_SELECT_BATCH, "Select Batch Render");
    mnu.Append(ID_POPUP_MNU_SELECT_FAILED, "Select Failed Uploads");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&FPPConnectDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::OnFPPReDiscoverClick(wxCommandEvent& event) {
    int curSize = instances.size();
    std::list<std::string> add;
  
    wxProgressDialog prgs("Discovering FPP Instances",
                          "Discovering FPP Instances", 100, this);
    prgs.Pulse("Discovering FPP Instances");

    std::string fppConnectIP = "";
    prgs.Show();
    wxDiscoveryDelegate delegate(this);
    std::list<FPP*> newInstances = static_cast<xLightsFrame*>(GetParent())->DiscoverFPPInstances(&delegate);
    
    for (FPP* fpp : newInstances) {
        bool found = false;
        
        for (auto f : instances) {
            if (f->ipAddress == fpp->ipAddress) {
                found = true;
            }
        }
        
        if (!found) {
            prgs.Pulse("Gathering configuration for " + fpp->hostName + " - " + fpp->ipAddress);
            fpp->AuthenticateAndUpdateVersions();
            fpp->probePixelControllerType();
            instances.push_back(fpp);
        } else {
            delete fpp;
        }
    }
    if ((size_t)curSize < instances.size()) {
        instances.sort(sortByIP);
        // it worked, we found some new instances, record this
        auto* config = GetXLightsConfig();
        wxString ip;
        config->Read("FPPConnectForcedIPs", &ip);
        if (locationSortCol == "name") {
            instances.sort(sortByName);
        } else {
            instances.sort([this](const FPP* a, const FPP* b) {
                std::vector<int> aComponents = SplitIP(a->ipAddress);
                std::vector<int> bComponents = SplitIP(b->ipAddress);
                for (size_t i = 0; i < aComponents.size() && i < bComponents.size(); ++i) {
                    if (aComponents[i] != bComponents[i])
                        return aComponents[i] < bComponents[i];
                }
                return aComponents.size() < bComponents.size();
            });
        }
        PopulateFPPInstanceList();
    }
    prgs.Hide();
}

void FPPConnectDialog::OnAddFPPButtonClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Find FPP Instance", "Enter IP address or hostname for FPP Instance");
    if (dlg.ShowModal() == wxID_OK && ip_utils::IsIPValidOrHostname(ToStdString(dlg.GetValue()))) {
        std::string ipAd = ToStdString(dlg.GetValue());
        int curSize = instances.size();

        wxProgressDialog prgs("Gathering configuration for " + ipAd,
                              "Gathering configuration for " + ipAd, 100, this);
        prgs.Pulse("Gathering configuration for " + ipAd);
        prgs.Show();

        std::list<std::string> add;
        add.push_back(ipAd);

        wxDiscoveryDelegate delegate(this);
        Discovery discovery(_outputManager, &delegate);
        FPP::PrepareDiscovery(discovery, add, false);
        discovery.Discover();
        FPP::MapToFPPInstances(discovery, instances, _outputManager);

        if ((size_t)curSize < instances.size()) {
            int cur = 0;
            for (const auto &fpp : instances) {
                if (cur >= curSize) {
                    prgs.Pulse("Gathering configuration for " + fpp->hostName + " - " + fpp->ipAddress);
                    fpp->AuthenticateAndUpdateVersions();
                    fpp->probePixelControllerType();
                }
                cur++;
            }

            instances.sort(sortByIP);
            //it worked, we found some new instances, record this
            auto* config = GetXLightsConfig();
            wxString ip;
            config->Read("FPPConnectForcedIPs", &ip);
            if (!ip.Contains(dlg.GetValue())) {
                if (ip != "") {
                    ip += "|";
                }
                ip += dlg.GetValue();
                config->Write("FPPConnectForcedIPs", ip);
                config->Flush();
            }
            PopulateFPPInstanceList();
        }

        prgs.Hide();
    }
}

void FPPConnectDialog::OnChoiceFolderSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void FPPConnectDialog::OnChoiceFilterSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void FPPConnectDialog::GetFolderList(const wxString& folder)
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

wxString FPPConnectDialog::SequenceDisplayName(const wxString& filePath) const
{
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    const wxString bases[2] = { xLightsFrame::CurrentDir, ToWXString(frame->GetFseqDirectory()) };
    for (wxString base : bases) {
        if (base.empty()) {
            continue;
        }
        if (!base.EndsWith("\\") && !base.EndsWith("/")) {
            base += wxFileName::GetPathSeparator();
        }
#ifdef __WXMSW__
        const bool underBase = filePath.Lower().StartsWith(base.Lower());
#else
        const bool underBase = filePath.StartsWith(base);
#endif
        if (underBase) {
            wxString rel = filePath.Mid(base.length());
            rel.Replace("\\", "/");
            return rel;
        }
    }
    return wxFileName(filePath).GetFullName();
}

void FPPConnectDialog::DisplayDateModified(const wxString& filePath, wxTreeListItem &item) const
{
    if (FileExists(filePath)) {
        wxDateTime last_modified_time(wxFileModificationTime(filePath));
        CheckListBox_Sequences->SetItemText(item, 1, last_modified_time.Format(wxT("%Y-%m-%d %H:%M:%S")) + "  ");
    }
}

