//(*InternalHeaders(FPPConnectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/regex.h>
#include <wx/volume.h>
#include <wx/progdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/hyperlink.h>
#include <wx/choicdlg.h>

#include "FPPConnectDialog.h"
#include "xLightsMain.h"
#include "FPP.h"
#include "xLightsXmlFile.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "../outputs/ControllerEthernet.h"
#include "../utils/CurlManager.h"
#include "ControllerCaps.h"
#include "utils/ip_utils.h"
#include "FPPUploadProgressDialog.h"
#include "ModelPreview.h"

#include <log4cpp/Category.hh>
#include "../xSchedule/wxJSON/jsonreader.h"

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../FSEQFile.h"
#include "../Parallel.h"
#include "../Discovery.h"
#include "Falcon.h"
#include "Experience.h"
#include <algorithm>

//(*IdInit(FPPConnectDialog)
const wxWindowID FPPConnectDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID FPPConnectDialog::ID_CHOICE_FILTER = wxNewId();
const wxWindowID FPPConnectDialog::ID_STATICTEXT2 = wxNewId();
const wxWindowID FPPConnectDialog::ID_CHOICE_FOLDER = wxNewId();
const wxWindowID FPPConnectDialog::ID_STATICTEXT3 = wxNewId();
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
	FPPInstanceSizer = new wxFlexGridSizer(0, 13, 0, 0);
	FPPInstanceList->SetSizer(FPPInstanceSizer);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer3->AddGrowableCol(4);
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

    wxConfigBase* config = wxConfigBase::Get();
    auto seqSortCol = config->ReadLong("xLightsFPPConnectSequenceSortCol", SORT_SEQ_NAME_COL);
    auto seqSortOrder = config->ReadBool("xLightsFPPConnectSequenceSortOrder", true);
    CheckListBox_Sequences->SetSortColumn(seqSortCol, seqSortOrder);

    FlexGridSizer2->Replace(CheckListBoxHolder, CheckListBox_Sequences, true);

    CheckListBoxHolder->Destroy();

    FlexGridSizer2->Layout();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    wxProgressDialog prgs("Discovering FPP Instances",
                          "Discovering FPP Instances", 100, parent);
    prgs.Pulse("Discovering FPP Instances");
    prgs.Show();

    instances = FPP::GetInstances(this, outputManager);

    wxPanel *p1 = AddInstanceHeader("Upload", "Enable to Upload Files/Configs to this FPP Device.");
    p1->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::UploadPopupMenu, nullptr, this);
    wxPanel *p2 = AddInstanceHeader("HostName", "FPP'a hostname that can be accessed via [hostnmame]");
    p2->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::HostSortMenu, nullptr, this);
    wxPanel* p3 = AddInstanceHeader("IP Address", "Current IP Address.");
    p3->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::IPSortMenu, nullptr, this);
    AddInstanceHeader("Description");
    AddInstanceHeader("Mode", "FPP Mode.");
    AddInstanceHeader("Version", "FPP Software Version.");
    AddInstanceHeader("FSEQ Type", "FSEQ File Version. FPP 2.6 required for V2 formats.");
    wxPanel* p4 = AddInstanceHeader("Media", "Enable to Upload MP3, MP4 and WAV Media Files.\nGenerally only checked on the main player.\n \nSpecial cases for sending videos to VirtualMatrixes.");
    p4->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&FPPConnectDialog::MediaPopupMenu, nullptr, this);
    AddInstanceHeader("Models", "Enable to Upload Models for Display Testing.\nNot necessary for FPP V7+ since it creates its own model overlays from the channel data uploaded.");
    AddInstanceHeader("UDP Out", "'None'- Device is not going to send Pixel data across the network.\n \n'All' This will send pixel data over your Show Network from FPP instance to all controllers marked as 'ACTIVE'.\n \n'Proxied' will set UDP Out only for Controllers with a Proxy IP address set.");
    AddInstanceHeader("Add Proxies", "Upload Proxy IP Adresses to FPP.");
    AddInstanceHeader("Playlist","Select Playlist to Add Uploaded Sequences Too.");
    wxPanel *p5 = AddInstanceHeader("Pixel Hat/Cape", "Display Hat or Hat Attached to FPP Device, if Found.\n \nWhen this is checked, it will upload Inputs/Outputs before setting up and uploading sequence data.\n \nThis will *RESTART* FPPD and may distrupt a currently running show.");
    p5->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::CapePopupMenu, nullptr, this);

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
    mnu.Append(ID_POPUP_MNU_SORT_NAME, "Sort");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnHostSortClick), NULL, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::IPSortMenu(wxContextMenuEvent& event) {
    wxMenu mnu;
    mnu.Append(ID_POPUP_MNU_SORT_IP, "Sort");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnIPSortClick), NULL, this);
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
                if (IsInSameSubnet(subnet, inst->ipAddress)) {
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
    while (FPPInstanceSizer->GetItemCount () > 13) {
        FPPInstanceSizer->Remove(13);
    }

    std::list<std::string> discoveredControllers;
    for (const auto& c : instances) discoveredControllers.push_back(c->ipAddress);
    for (const auto& ctrl : this->_outputManager->GetControllers()) {
        auto c = dynamic_cast<ControllerEthernet*>(ctrl);
        if (c != nullptr && c->GetActive() == Controller::ACTIVESTATE::ACTIVEINXLIGHTSONLY) {
            if (std::find(discoveredControllers.begin(), discoveredControllers.end(), c->GetResolvedIP()) == discoveredControllers.end()) {
                FPP* missing = new FPP(c->GetResolvedIP());
                missing->hostName = c->GetResolvedIP();
                instances.push_back(missing);
            }
        }
    }

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        wxCheckBox *doUploadCheckbox = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, CHECK_COL + rowStr);
        FPPInstanceSizer->Add(doUploadCheckbox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);

        std::string l = inst->hostName + " - " + inst->ipAddress;
        std::string lhn = "http://" + inst->hostName;
        if (!inst->proxy.empty()) {
            lhn = "http://" + inst->proxy + "/proxy/" + inst->ipAddress;
        }
        std::string lip = "http://" + inst->ipAddress;
        if (!inst->proxy.empty()) {
            lip = "http://" + inst->proxy + "/proxy/" + inst->ipAddress;
        }
        auto link1 = new wxHyperlinkCtrl(FPPInstanceList, wxID_ANY, inst->hostName, lhn, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE, "ID_HOSTNAME_" + rowStr);
        link1->SetNormalColour(CyanOrBlue());
        link1->SetVisitedColour(CyanOrBlue());
        FPPInstanceSizer->Add(link1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        auto link2 = new wxHyperlinkCtrl(FPPInstanceList, wxID_ANY, inst->ipAddress, lip, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE, "ID_IP_" + rowStr);
        link2->SetNormalColour(CyanOrBlue());
        link2->SetVisitedColour(CyanOrBlue());
        FPPInstanceSizer->Add(link2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        auto label = new wxStaticText(FPPInstanceList, wxID_ANY, inst->description, wxDefaultPosition, wxDefaultSize, 0, "ID_DESCRIPTION_" + rowStr);
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        auto mode = inst->mode;
        label = new wxStaticText(FPPInstanceList, wxID_ANY, mode, wxDefaultPosition, wxDefaultSize, 0, "ID_MODE_" + rowStr);
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        label = new wxStaticText(FPPInstanceList, wxID_ANY, inst->fullVersion, wxDefaultPosition, wxDefaultSize, 0, "ID_VERSION_" + rowStr);
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);

        //FSEQ Type listbox
        if (inst->fppType == FPP_TYPE::FPP) {
            if (!inst->supportedForFPPConnect()) {
                doUploadCheckbox->SetValue(false);
                doUploadCheckbox->Enable(false);

                label = new wxStaticText(FPPInstanceList, wxID_ANY, "Unavailable/Unsupported", wxDefaultPosition, wxDefaultSize, 0, "ID_STATIC_TEXT_FS_" + rowStr);
                FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
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
                FPPInstanceSizer->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
            }
        } else if (inst->fppType == FPP_TYPE::FALCONV4V5) {
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
            FPPInstanceSizer->Add(Choice1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        } else if (inst->fppType == FPP_TYPE::ESPIXELSTICK || inst->fppType == FPP_TYPE::GENIUS) {
            label = new wxStaticText(FPPInstanceList, wxID_ANY, "V2 Sparse/Uncompressed", wxDefaultPosition, wxDefaultSize, 0, "ID_STATIC_TEXT_FS_" + rowStr);
            FPPInstanceSizer->Add(label, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        } else {
            label = new wxStaticText(FPPInstanceList, wxID_ANY, "V1", wxDefaultPosition, wxDefaultSize, 0, "ID_STATIC_TEXT_FS_" + rowStr);
            FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        if (inst->fppType == FPP_TYPE::FPP && inst->supportedForFPPConnect()) {
            if (prgs) {
                prgs->Pulse("Probing information from " + l);
            }
            wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MEDIA_COL + rowStr);
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            wxChoice* Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, MODELS_COL + rowStr);
            wxFont font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);

            Choice1->Append(_("None"));
            Choice1->Append(_("All"));
            Choice1->Append(_("Local"));
            Choice1->SetSelection(0);
            FPPInstanceSizer->Add(Choice1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);

            Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, UDP_COL + rowStr);
            font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);

            Choice1->Append(_("None"));
            Choice1->Append(_("All"));
            Choice1->Append(_("Proxied"));
            Choice1->SetSelection(0);
            FPPInstanceSizer->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            if (inst->solePlayer) {
				SetChoiceValueIndex(UDP_COL + rowStr, 1);
			} else if (inst->isaProxy) {
				SetChoiceValueIndex(UDP_COL + rowStr, 2);
			}
            wxCheckBox* CheckBoxProxy = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, PROXY_COL + rowStr);
            FPPInstanceSizer->Add(CheckBoxProxy, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
			CheckBoxProxy->SetValue(inst->isaProxy);

            //playlist combo box
            if (StartsWith(inst->mode, "player")) {
                wxComboBox* ComboBox1 = new wxComboBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxTE_PROCESS_ENTER, wxDefaultValidator, PLAYLIST_COL + rowStr);
                ComboBox1->Append(_(""));
                for (const auto& pl : inst->playlists) {
                    ComboBox1->Append(pl);
                }
                font = ComboBox1->GetFont();
                font.SetPointSize(font.GetPointSize() - 2);
                ComboBox1->SetFont(font);
                FPPInstanceSizer->Add(ComboBox1, 1, wxALL | wxEXPAND, 0);
            } else {
                wxStaticText* placeholder = new wxStaticText(FPPInstanceList, wxID_ANY, "");
                FPPInstanceSizer->Add(placeholder, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            }

        } else if (inst->fppType == FPP_TYPE::FALCONV4V5) {
            // this probably needs to be moved as this is not really a zlib thing but only the falcons end up here today so I am going to put it here for now
            wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MEDIA_COL + rowStr);
            CheckBox1->SetValue(inst->mode != "remote");
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);

            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        } else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
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
        if (m != "") {
            std::string desc = m;
            if (inst->panelSize != "") {
                desc += " - " + inst->panelSize;
            }
            wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, desc, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, UPLOAD_CONTROLLER_COL + rowStr);
            CheckBox1->SetValue(false);
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        } else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        ++row;
    }
    ApplySavedHostSettings();

    FPPInstanceList->FitInside();
    FPPInstanceList->SetScrollRate(100, 100);
    FPPInstanceList->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
    FPPInstanceList->Thaw();
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

    wxConfigBase* config = wxConfigBase::Get();
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
                if (CheckListBox_Sequences->GetItemText(item) == fseqName) {
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
    wxConfigBase* config = wxConfigBase::Get();
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

void FPPConnectDialog::LoadSequencesFromFolder(wxString dir) const
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning folder for sequences for FPP upload: %s", (const char *)dir.c_str());

    wxDir directory;
    directory.Open(dir);

    wxArrayString files;
    GetAllFilesInDir(dir, files, "*.x*");

    static const int BUFFER_SIZE = 1024*12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer
    for (auto &filename : files) {
        wxFileName fn(filename);
        wxString file = fn.GetFullName();
        if (file != XLIGHTS_RGBEFFECTS_FILE
            && file != OutputManager::GetNetworksFileName()
            && file != XLIGHTS_KEYBINDING_FILE
            && (file.Lower().EndsWith("xml") || file.Lower().EndsWith("xsq"))
            && FileExists(filename)) {
            wxFile doc(filename);
            // this could be a sequence file ... lets open it and check
            //just check if <xsequence" is in the first 512 bytes, parsing every XML is way too expensive
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
                                ++count;
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
                                mediaName = FromUTF8(stagEvent->getText());
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
                        const wxString fixedMN = FixFile(frame->CurrentDir, mediaName);
                        if (!FileExists(fixedMN)) {
                            logger_base.info("Could not find media: %s ", mediaName.c_str());
                            mediaName = "";
                        } else {
                            mediaName = ToUTF8(fixedMN);
                        }
                    }
                }
            }
            logger_base.debug("XML:  %s   IsSeq:  %d    FSEQ:  %s   Media:  %s", (const char*)file.c_str(), isSequence, (const char*)fseqName.c_str(), (const char*)mediaName.c_str());
            if (isSequence) {

                // where you have show folders within show folders and sequences with the same name
                // such as when you have an imported subfolder this can create duplicates ... so lets first check
                // we dont already have the fseq file in the list

                bool found = false;
                for (auto item = CheckListBox_Sequences->GetFirstItem(); !found && item.IsOk(); item = CheckListBox_Sequences->GetNextItem(item)) {
                    if (fseqName == CheckListBox_Sequences->GetItemText(item)) {
                        found = true;
                    }
                }

                if (!found) {
                    wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(),
                        fseqName);

                    DisplayDateModified(fseqName, item);
                    if (mediaName != "") {
                        CheckListBox_Sequences->SetItemText(item, 2, mediaName);
                    }
                }
            }
        }
    }

    // we also need to load eseq files which may not have the same name as an xsq file
    files.clear();
    GetAllFilesInDir(dir, files, "*.eseq");
    for (auto& filename : files) {
        wxFileName fn(filename);
        wxString file = fn.GetFullName();

        logger_base.debug("ESEQ:  %s", (const char*)file.c_str());

        // The eseq may already be in the list
        bool found = false;
        for (auto item = CheckListBox_Sequences->GetFirstItem(); !found && item.IsOk(); item = CheckListBox_Sequences->GetNextItem(item)) {
            if (filename == CheckListBox_Sequences->GetItemText(item)) {
                found = true;
            }
        }

        if (!found) {
            wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(),
                                                                        filename);

            DisplayDateModified(filename, item);
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

void FPPConnectDialog::LoadSequences()
{
    CheckListBox_Sequences->DeleteAllItems();
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxString freqDir = frame->GetFseqDirectory();

    if (ChoiceFolder->GetSelection() == 0) {
        LoadSequencesFromFolder(xLightsFrame::CurrentDir);
    }
    else {
        const wxString folder = ChoiceFolder->GetString(ChoiceFolder->GetSelection());
        LoadSequencesFromFolder(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder);
        freqDir = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder;
    }

    wxDir directory;
    directory.Open(freqDir);

    wxArrayString files;
    GetAllFilesInDir(freqDir, files, "*.?seq");
    for (auto &v : files) {
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        bool found = false;
        while (item.IsOk()) {
            if (v == CheckListBox_Sequences->GetItemText(item)) {
                found = true;
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }
        if (!found && FileExists(v)) {
            wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(), v);
            DisplayDateModified(v, item);
            FSEQFile *file = FSEQFile::openFSEQFile(v.ToStdString());
            if (file != nullptr) {
                for (auto& header : file->getVariableHeaders()) {
                    if (header.code[0] == 'm' && header.code[1] == 'f') {
                        wxString mediaName = (const char*)(&header.data[0]);
                        mediaName = FixFile("", mediaName);
                        if (FileExists(mediaName)) {
                            CheckListBox_Sequences->SetItemText(item, 2, mediaName);
                        }
                    }
                }
            }
        }
    }

    if (xLightsFrame::CurrentSeqXmlFile != nullptr) {
        wxString curSeq = xLightsFrame::CurrentSeqXmlFile->GetLongPath();
        if (!curSeq.StartsWith(xLightsFrame::CurrentDir)) {
            LoadSequencesFromFolder(curSeq);
        }
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        while (item.IsOk()) {
            if (curSeq == CheckListBox_Sequences->GetItemText(item)) {
                CheckListBox_Sequences->CheckItem(item);
                break;
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        const wxString itcsv = config->Read("FPPConnectSelectedSequences", wxEmptyString);

        if (!itcsv.IsEmpty()) {
            wxArrayString savedUploadItems = wxSplit(itcsv, ',');

            wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
            while (item.IsOk()) {
                if (savedUploadItems.Index(CheckListBox_Sequences->GetItemText(item)) != wxNOT_FOUND) {
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

    wxConfigBase* config = wxConfigBase::Get();
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
    for (row = 0; row < doUpload.size(); ++row) {
        std::string rowStr = std::to_string(row);
        doUpload[row] = GetCheckValue(CHECK_COL + rowStr);
        ++uploadCount;
    }

    FPPUploadProgressDialog prgs(this);
    row = 0;
    for (const auto& inst : instances) {
        inst->parent = this;
        // not in discovery so we can increase the timeouts to make sure things get transferred
        inst->defaultConnectTimeout = 5000;
        inst->messages.clear();
        std::string rowStr = std::to_string(row);
        if (doUpload[row]) {
            std::string l = inst->hostName + " - " + inst->ipAddress;
            inst->setProgress(&prgs, prgs.addGauge(l));
        } else {
            inst->setProgress(nullptr, nullptr);
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    std::map<int, int> udpRanges;
    wxJSONValue outputs = FPP::CreateUniverseFile(_outputManager->GetControllers(), false, &udpRanges);
    int pw, ph;
    frame->GetLayoutPreview()->GetVirtualCanvasSize(pw, ph);
    std::string displayMap = FPP::CreateVirtualDisplayMap(&frame->AllModels, pw, ph);
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
                    //auto vendor = FPP::GetVendor(inst->pixelControllerType);
                    //auto model = FPP::GetModel(inst->pixelControllerType);
                    //auto caps = ControllerCaps::GetControllerConfig(vendor, model, "");
                    auto c = _outputManager->GetControllers(inst->ipAddress);
                    if (c.size() == 1) {
                        cancelled |= inst->UploadPanelOutputs(&frame->AllModels, _outputManager, c.front());
                        cancelled |= inst->UploadVirtualMatrixOutputs(&frame->AllModels, _outputManager, c.front());
                        cancelled |= inst->UploadPixelOutputs(&frame->AllModels, _outputManager, c.front());
                        cancelled |= inst->UploadSerialOutputs(&frame->AllModels, _outputManager, c.front());
                    }
                }
                if (GetChoiceValueIndex(MODELS_COL + rowStr) == 1) {
                    wxJSONValue const& memoryMaps = inst->CreateModelMemoryMap(&frame->AllModels, 0, std::numeric_limits<int32_t>::max());
                    cancelled |= inst->UploadModels(memoryMaps);
                    cancelled |= inst->UploadDisplayMap(displayMap);
                    inst->SetRestartFlag();
                } else if (GetChoiceValueIndex(MODELS_COL + rowStr) == 2) {
                    auto c = _outputManager->GetControllers(inst->ipAddress);
                    if (c.size() == 1) {
                        wxJSONValue const& memoryMaps = inst->CreateModelMemoryMap(&frame->AllModels, c.front()->GetStartChannel(), c.front()->GetEndChannel());
                        cancelled |= inst->UploadModels(memoryMaps);
                        // cancelled |= inst->UploadDisplayMap(displayMap);
                        inst->SetRestartFlag();
                    }
                }
                //if restart flag is now set, restart and recheck range
                inst->Restart(true);
            } else if (GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr) && controller.size() == 1) {
                BaseController *bc = BaseController::CreateBaseController(controller.front(), inst->ipAddress);
                bc->UploadForImmediateOutput(&frame->AllModels, _outputManager, controller.front(), this);
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
            wxString fseqRaw = CheckListBox_Sequences->GetItemText(item);
            std::string fseq = ToUTF8(fseqRaw);
            std::string media = ToUTF8(CheckListBox_Sequences->GetItemText(item, 2));

            FSEQFile *seq = FSEQFile::openFSEQFile(fseqRaw.ToStdString());
            if (seq) {
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
                                        logger_base.error("FPPConnect FSEQ file corrupt.");
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
                                            logger_base.debug("Media file %s not uploaded because media checkbox not selected.", (const char*)m2.c_str());
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
                                    logger_base.debug("Upload failed as FxxV4 is not connected.");
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
                                    logger_base.debug("Upload failed as Genius is not connected.");
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
        wxConfigBase* config = wxConfigBase::Get();
        if (config != nullptr) {
            config->Write("FPPConnectFailedList", failedUploadsList);
        }
        config->Flush();
    };
    xLightsFrame* xlframe = static_cast<xLightsFrame*>(GetParent());
    if (messages != "") {
        xlframe->SetStatusText("FPP Connect Upload had errors or warnings", 0);
        wxMessageBox(messages, "Problems Uploading", wxOK | wxCENTRE, this);
        logger_base.warn("FPP Connect Upload had errors or warnings:\n" + messages);
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
    wxConfigBase* config = wxConfigBase::Get();
    if (!onlyInsts) {
        wxString selected = "";
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        while (item.IsOk()) {
            bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
            if (isChecked) {
                if (selected != "") {
                    selected += ",";
                }
                selected += CheckListBox_Sequences->GetItemText(item);
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
    wxConfigBase* config = wxConfigBase::Get();
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
    instances = FPP::GetInstances(this, _outputManager);
    
    if (curSize < instances.size()) {
        int cur = 0;
        for (const auto& fpp : instances) {
            if (cur >= curSize) {
                prgs.Pulse("Gathering configuration for " + fpp->hostName + " - " + fpp->ipAddress);
                fpp->AuthenticateAndUpdateVersions();
                fpp->probePixelControllerType();
            }
            cur++;
        }

        instances.sort(sortByIP);
        // it worked, we found some new instances, record this
        wxConfigBase* config = wxConfigBase::Get();
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
    if (dlg.ShowModal() == wxID_OK && ip_utils::IsIPValidOrHostname(dlg.GetValue().ToStdString())) {
        std::string ipAd = ToStdString(dlg.GetValue());
        int curSize = instances.size();

        wxProgressDialog prgs("Gathering configuration for " + ipAd,
                              "Gathering configuration for " + ipAd, 100, this);
        prgs.Pulse("Gathering configuration for " + ipAd);
        prgs.Show();

        std::list<std::string> add;
        add.push_back(ipAd);

        Discovery discovery(this, _outputManager);
        FPP::PrepareDiscovery(discovery, add, false);
        discovery.Discover();
        FPP::MapToFPPInstances(discovery, instances, _outputManager);

        if (curSize < instances.size()) {
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
            wxConfigBase* config = wxConfigBase::Get();
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

void FPPConnectDialog::DisplayDateModified(const wxString& filePath, wxTreeListItem &item) const
{
    if (FileExists(filePath)) {
        wxDateTime last_modified_time(wxFileModificationTime(filePath));
        CheckListBox_Sequences->SetItemText(item, 1, last_modified_time.Format(wxT("%Y-%m-%d %H:%M:%S")));
    }
}
