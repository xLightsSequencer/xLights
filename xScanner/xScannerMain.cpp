/***************************************************************
 * Name:      xScannerMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#define ZERO 0
#define E131PORT 5568
#define ARTNETPORT 0x1936

#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/debugrpt.h>
#include <wx/protocol/http.h>
#include <wx/filedlg.h>
#include <wx/numdlg.h>
#include <wx/progdlg.h>
#include <wx/dataview.h>

#include "xScannerMain.h"
#include "../xLights/xLightsVersion.h"
#include "../xLights/outputs/ControllerEthernet.h"
#include "../xLights/UtilFunctions.h"
#include "../xLights/utils/ip_utils.h"
#include "../xLights/controllers/BaseController.h"

#include "../xLights/automation/automation.h"

#include <log4cpp/Category.hh>

#ifndef __WXMSW__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

//(*InternalHeaders(xScannerFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(xScannerFrame)
const long xScannerFrame::ID_STATUSBAR1 = wxNewId();
const long xScannerFrame::Network = wxNewId();
const long xScannerFrame::ID_TIMER1 = wxNewId();
//*)

const long xScannerFrame::ID_MNU_EXPORT = wxNewId();
const long xScannerFrame::ID_MNU_RESCAN = wxNewId();
const long xScannerFrame::ID_MNU_ADDTOXLIGHTS = wxNewId();

BEGIN_EVENT_TABLE(xScannerFrame,wxFrame)
    //(*EventTable(xScannerFrame)
    //*)
END_EVENT_TABLE()

class ControllerData : public wxClientData
{
    TITLE_PRIORITY _tp = TITLE_PRIORITY::TP_NONE;

public:
    ControllerData(TITLE_PRIORITY tp) :
    wxClientData()
    {
        _tp = tp;
    }
    ControllerData() :
        wxClientData()
    {}
    bool IsHigherPriority(TITLE_PRIORITY tp) const
    {
        return tp < _tp;
    }
    void SetTitlePriority(TITLE_PRIORITY tp)
    {
        _tp = tp;
    }
};

xScannerFrame::xScannerFrame(wxWindow* parent, bool singleThreaded, wxWindowID id)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //(*Initialize(xScannerFrame)
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, _("xLights Scanner"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    SetSizer(FlexGridSizer1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -10 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    MenuItemScan = new wxMenuItem((&Menu1), Network, _("Scan"), wxEmptyString, wxITEM_NORMAL);
    Menu1.Append(MenuItemScan);
    Timer1.SetOwner(this, ID_TIMER1);
    Timer1.Start(1000, false);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(Network,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScannerFrame::OnMenuItemScanSelected);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xScannerFrame::OnTimer1Trigger);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xScannerFrame::OnResize);
    //*)

    _tree = new wxTreeListCtrl(this, wxNewId());
    FlexGridSizer1->Add(_tree, 1, wxALL | wxEXPAND, 5);

    _tree->Connect(wxEVT_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&xScannerFrame::OnTreeItemActivated, nullptr, this);
    _tree->Connect(wxEVT_TREELIST_ITEM_CONTEXT_MENU, (wxObjectEventFunction)&xScannerFrame::OnTreeRClick, nullptr, this);

    _tree->AppendColumn("", 200);
    _tree->AppendColumn("", 450);
    _tree->AppendColumn("", 200);

    SetTitle("xLights Scanner " + GetDisplayVersionString());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    _workManager.SetSingleThreaded(singleThreaded);

    Scan();

    SetMinSize(wxSize(600, 300));
    SetSize(800, 600);

    ValidateWindow();
}

void xScannerFrame::Scan()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _tree->DeleteAllItems();

    logger_base.debug("Launching scan ...");

    _workManager.Start();
    _workManager.AddComputer();
}

std::string xScannerFrame::GetItem(std::list<std::pair<std::string, std::string>>& res, const std::string& label)
{
    for (auto& it : res) {
        if (it.first == label) return it.second;
    }

    return "";
}

std::string xScannerFrame::GetIPSubnet(const std::string& ip)
{
    auto comp = wxSplit(ip, '.');
    if (comp.size() == 4)         {
        return comp[0] + "." + comp[1] + "." + comp[2] + ".*";
    }
    return "";
}

wxTreeListItem xScannerFrame::GetSubnetItem(const std::string& subnet)
{
    for (auto a = _tree->GetFirstItem(); a.IsOk(); a = _tree->GetNextSibling(a)) {
        if (_tree->GetItemText(a, 0) == subnet) return a;
    }
    auto item = _tree->AppendItem(_tree->GetRootItem(), subnet);
    _tree->SetItemData(item, new ControllerData());
    return item;
}

wxTreeListItem xScannerFrame::GetIPItem(const std::string& ip, bool create)
{
    auto subnet = GetSubnetItem(GetIPSubnet(ip));
    if (!subnet.IsOk()) return {};

    auto ips = wxSplit(ip, '.');
    if (ips.size() < 4) return {};
    std::vector<int> ipsi;
    for (const auto& it : ips)         {
        ipsi.push_back(atoi(it.c_str()));
    }

    int count = 0;
    wxTreeListItem previous = wxTLI_FIRST;
    for (auto a = _tree->GetFirstChild(subnet); a.IsOk(); a = _tree->GetNextSibling(a)) {
        count++;
        if (_tree->GetItemText(a, 0) == ip) return a;
        auto i = _tree->GetItemText(a, 0);
        auto tips = wxSplit(i, '.');
        if (tips.size() == 4) {
            std::vector<int> tipsi;
            for (const auto& it : tips) {
                tipsi.push_back(atoi(it.c_str()));
            }
            bool greater = false;
            for (int ii = 0; ii < 4; ii++) {
                if (tipsi[ii] == ipsi[ii]) {
                }
                else if (ipsi[ii] > tipsi[ii]) {
                    previous = a;
                    break;
                }
                else {
                    break;
                }
            }
        }
    }

    if (create) {

        wxTreeListItem ti;
        if (previous.IsOk())             {
            ti = _tree->InsertItem(subnet, previous, ip);
            _tree->SetItemData(ti, new ControllerData());
        }
        else             {
            ti = _tree->AppendItem(subnet, ip);
            _tree->SetItemData(ti, new ControllerData());
        }
        if (count == 0) {
            _tree->Expand(subnet);
        }
        return ti;
    }

    return wxTreeListItem();
}

wxString xScannerFrame::GetItemUnderParent(wxTreeListItem& parent, const std::string& label) const
{
    for (auto a = _tree->GetFirstChild(parent); a.IsOk(); a = _tree->GetNextSibling(a)) {
        if (_tree->GetItemText(a, 0) == label) {
            return _tree->GetItemText(a, 1);
        }
    }
    return wxString();
}

wxTreeListItem xScannerFrame::AddItemUnderParent(wxTreeListItem& parent, const std::string& label, const std::string& value)
{
    // only add if there isnt something here already
    for (auto a = _tree->GetFirstChild(parent); a.IsOk(); a = _tree->GetNextSibling(a)) {
        if (_tree->GetItemText(a, 0) == label) return a;
    }
    auto ti = _tree->AppendItem(parent, label);
    _tree->SetItemText(ti, 1, value);
    _tree->SetItemData(ti, new ControllerData());
    return ti;
}

void xScannerFrame::UpdateDeviceTitle(wxTreeListCtrl* tree, wxTreeListItem& ti, TITLE_PRIORITY tp, const std::string& name)
{
    if (name != "") {
        auto cd = dynamic_cast<ControllerData*>(tree->GetItemData(ti));

        if (cd != nullptr && cd->IsHigherPriority(tp)) {
            tree->SetItemText(ti, 1, name);
            cd->SetTitlePriority(tp);
        }
    }
}

void xScannerFrame::ProcessComputerResult(std::list<std::pair<std::string, std::string>>& res)
{
    // Type
    // Computer Name
    // Force Local IP
    // xLights Show Folder
    // xLights Global FPP Proxy
    // xSchedule Show Folder
    // xSchedule Global FPP Proxy
    // Local IP %d
    // Static Route %d
    for (const auto& it : GetStartsWith(res, "Local IP ")) {

        auto item = GetIPItem(it);

        if (item.IsOk()) {
            UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_COMPUTER_NAME, GetItem(res, "Computer Name"));
            AddItemUnderParentIfNotBlank(item, "xLights Show Folder", GetItem(res, "xLights Show Folder"));
            AddItemUnderParentIfNotBlank(item, "xLights FPP Global Proxy", GetItem(res, "xLights Global FPP Proxy"));
            AddItemUnderParentIfNotBlank(item, "xSchedule Show Folder", GetItem(res, "xSchedule Show Folder"));
            AddItemUnderParentIfNotBlank(item, "xSchedule FPP Global Proxy", GetItem(res, "xSchedule Global FPP Proxy"));
            int i = 1;
            for (const auto& it2 : GetStartsWith(res, "Local IP ")) {
                AddItemUnderParentIfNotBlank(item, wxString::Format("IP %d", i++), it2);
            }
            i = 1;
            for (const auto& it2 : GetStartsWith(res, "Static Route ")) {
                AddItemUnderParentIfNotBlank(item, wxString::Format("Static Route %d", i++), it2);
            }
        }
    }
}

void xScannerFrame::ProcessPingResult(std::list<std::pair<std::string, std::string>>& res)
{
    // PING
    // IP
    // Type
    // Why
    // Network
    // Network Type

    auto ip = GetItem(res, "IP");
    auto subnet = GetSubnetItem(GetIPSubnet(ip));
    _tree->SetItemText(subnet, 1, GetItem(res, "Network Type"));

    auto ping = GetItem(res, "PING");
    auto item = GetIPItem(ip, ping == "OK");

    if (item.IsOk()) {
        if (ping == "FAILED")             {
            _tree->SetItemText(item, 2, "OFFLINE");
        }
        AddItemUnderParentIfNotBlank(item, "Scanned because", GetItem(res, "Why"));
    }
}

void xScannerFrame::ProcessControllerResult(std::list<std::pair<std::string, std::string>>& res)
{
    // Type
    // IP
    // Why
    // Vendor
    // Model
    // Variant
    // Active
    // Name
    // Description
    // Protocol
    // Universes/Id
    // Channels

    auto ip = GetItem(res, "IP");
    auto item = GetIPItem(ip);

    if (item.IsOk()) {
        auto vmv = GetItem(res, "Vendor") + ":" + GetItem(res, "Model") + ":" + GetItem(res, "Variant");
        if (vmv != "::") {
            UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_CONTROLLER_VMV, vmv);
            AddItemUnderParent(item, "Vendor/Model/Variant", vmv);
        }
        AddItemUnderParent(item, "Active", GetItem(res, "Active"));
        UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_CONTROLLER_NAME, GetItem(res, "Name"));
        AddItemUnderParentIfNotBlank(item, "Description", GetItem(res, "Description"));
        AddItemUnderParentIfNotBlank(item, "Protocol", GetItem(res, "Protocol"));
        AddItemUnderParentIfNotBlank(item, "Universes/Id", GetItem(res, "Universes/Id"));
        AddItemUnderParentIfNotBlank(item, "Channels", GetItem(res, "Channels"));
    }
}

void xScannerFrame::AddItemUnderParentIfNotBlank(wxTreeListItem & item, const std::string & label, const std::string & value)
{
    if (value != "") {
        AddItemUnderParent(item, label,value);
    }
}

void xScannerFrame::ProcessHTTPResult(std::list<std::pair<std::string, std::string>>& res)
{
    // IP
    // Type
    // Port
    // Web

    auto web = GetItem(res, "Web");

    auto ip = GetItem(res, "IP");
    auto item = GetIPItem(ip, web == "OK");

    if (item.IsOk()) {
        AddItemUnderParentIfNotBlank(item, "Web", GetItem(res, "Web"));
        auto controller = GetItem(res, "Controller");
        UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_HTTP_CONTROLLER, controller);
        auto title = GetItem(res, "Title");
        UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_HTTP_TITLE, title);
        AddItemUnderParentIfNotBlank(item, "Controller", controller);
        AddItemUnderParentIfNotBlank(item, "Web title", title);
        _tree->SetItemText(item, 2, ""); // it must be online so remove OFFLINE if it is there
    }
}

std::list<std::string> xScannerFrame::GetStartsWith(std::list<std::pair<std::string, std::string>>& res, const std::string& prefix)
{
    std::list<std::string> result;

    for (const auto& it : res) {
        if (StartsWith(it.first, prefix)) {
            result.push_back(it.second);
        }
    }

    return result;
}

void xScannerFrame::ProcessFPPResult(std::list<std::pair<std::string, std::string>>& res)
{
    for (const auto& it : GetStartsWith(res, "IP ")) {

        wxString ip = it;
        ip = ip.AfterFirst(':');
        ip = ip.AfterFirst(' ');
        ip = ip.BeforeFirst(' ');
        auto item = GetIPItem(ip);

        if (item.IsOk()) {
            UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_CONTROLLER_FPP, "FPP");
            AddItemUnderParentIfNotBlank(item, "Version", GetItem(res, "Version"));
            AddItemUnderParentIfNotBlank(item, "Mode", GetItem(res, "Mode"));
            AddItemUnderParentIfNotBlank(item, "Sending Data", GetItem(res, "Sending Data"));
            int i = 1;
            for (const auto& it2 : GetStartsWith(res, "Net ")) {
                AddItemUnderParentIfNotBlank(item, wxString::Format("Network %d", i++), it2);
            }
            //for (const auto& it2 : GetStartsWith(res, "IP ")) {
            //    AddItemUnderParentIfNotBlank(item, "IP", it2);
            //}
            i = 1;
            for (const auto& it2 : GetStartsWith(res, "Proxying ")) {
                AddItemUnderParentIfNotBlank(item, wxString::Format("Proxying %d", i++), it2);
            }
        }
    }
}

void xScannerFrame::ProcessFalconResult(std::list<std::pair<std::string, std::string>>& res)
{
    // IP
    // Type
    // Banks
    // Mode
    // WIFI IP
    // ETH IP
    // Model
    // Test Mode
    // Temp1
    // Temp2
    // Processor Temp
    // Fan Speed
    // V1
    // V2
    // Board Configuration

    std::vector<std::string> iplabels = { "WIFI IP", "ETH IP"};

    int count = 0;
    for (const auto& it : iplabels) {
        wxString ip = GetItem(res, it);
        ip = ip.AfterFirst(':');
        ip = ip.AfterFirst(' ');
        ip = ip.BeforeFirst(' ');
        if (ip != "") {
            count++;
            auto item = GetIPItem(ip);

            if (item.IsOk()) {

                auto name = GetItem(res, "Name");
                if (name == "") {
                    name = "Falcon";
                }
                else                     {
                    name = "Falcon " + name;
                }
                UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_CONTROLLER_FALCON, name);

                AddItemUnderParentIfNotBlank(item, "Name", GetItem(res, "Name"));
                AddItemUnderParentIfNotBlank(item, "Model", GetItem(res, "Model"));
                AddItemUnderParentIfNotBlank(item, "Firmware Version", GetItem(res, "Firmware Version"));
                AddItemUnderParentIfNotBlank(item, "Banks", GetItem(res, "Banks"));
                AddItemUnderParentIfNotBlank(item, "Mode", GetItem(res, "Mode"));
                if (GetItem(res, "WIFI IP") != "") {
                    auto w = wxString::Format("%s IP: %s Gateway:%s DNS: %s", GetItem(res, "WDHCP"), GetItem(res, "WIFI IP"), GetItem(res, "WiFi Gateway"), GetItem(res, "WiFi DNS"));
                    AddItemUnderParentIfNotBlank(item, "WiFi", w);
                }
                if (GetItem(res, "ETH IP") != "") {
                    auto e = wxString::Format("%s IP: %s Gateway:%s DNS: %s", GetItem(res, "DHCP"), GetItem(res, "ETH IP"), GetItem(res, "Gateway"), GetItem(res, "DNS"));
                    AddItemUnderParentIfNotBlank(item, "Ethernet", e);
                }
                AddItemUnderParentIfNotBlank(item, "Test Mode", GetItem(res, "Test Mode"));
                AddItemUnderParentIfNotBlank(item, "Temp 1", GetItem(res, "Temp1"));
                AddItemUnderParentIfNotBlank(item, "Temp 2", GetItem(res, "Temp2"));
                AddItemUnderParentIfNotBlank(item, "Processor Temp", GetItem(res, "Processor Temp"));
                AddItemUnderParentIfNotBlank(item, "Fan Speed", GetItem(res, "Fan Speed"));
                AddItemUnderParentIfNotBlank(item, "Voltage 1", GetItem(res, "V1"));
                AddItemUnderParentIfNotBlank(item, "Voltage 2", GetItem(res, "V2"));
                AddItemUnderParentIfNotBlank(item, "Board Configuration", GetItem(res, "Board Configuration"));
            }
        }
    }

    if (count == 0)         {
        auto ip = GetItem(res, "IP");
        if (ip != "") {
            auto item = GetIPItem(ip);

            if (item.IsOk()) {

                auto name = GetItem(res, "Name");
                if (name == "") {
                    name = "Falcon";
                }
                else {
                    name = "Falcon " + name;
                }
                UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_CONTROLLER_FALCON, name);

                AddItemUnderParentIfNotBlank(item, "Name", GetItem(res, "Name"));
                AddItemUnderParentIfNotBlank(item, "Model", GetItem(res, "Model"));
                AddItemUnderParentIfNotBlank(item, "Firmware Version", GetItem(res, "Firmware Version"));
                AddItemUnderParentIfNotBlank(item, "Banks", GetItem(res, "Banks"));
                AddItemUnderParentIfNotBlank(item, "Mode", GetItem(res, "Mode"));
                AddItemUnderParentIfNotBlank(item, "WiFi IP", GetItem(res, "WIFI IP"));
                AddItemUnderParentIfNotBlank(item, "Eth IP", GetItem(res, "ETH IP"));
                AddItemUnderParentIfNotBlank(item, "Test Mode", GetItem(res, "Test Mode"));
                AddItemUnderParentIfNotBlank(item, "Temp 1", GetItem(res, "Temp1"));
                AddItemUnderParentIfNotBlank(item, "Temp 2", GetItem(res, "Temp2"));
                AddItemUnderParentIfNotBlank(item, "Processor Temp", GetItem(res, "Processor Temp"));
                AddItemUnderParentIfNotBlank(item, "Fan Speed", GetItem(res, "Fan Speed"));
                AddItemUnderParentIfNotBlank(item, "Voltage 1", GetItem(res, "V1"));
                AddItemUnderParentIfNotBlank(item, "Voltage 2", GetItem(res, "V2"));
                AddItemUnderParentIfNotBlank(item, "Board Configuration", GetItem(res, "Board Configuration"));
            }
        }
    }
}

void xScannerFrame::ProcessMACResult(std::list<std::pair<std::string, std::string>>& res)
{
    // Type
    // IP
    // MAC
    // MAC Vendor

    auto ip = GetItem(res, "IP");
    auto item = GetIPItem(ip);

    if (item.IsOk())
    {
        auto vendor = GetItem(res, "MAC Vendor");
        if (vendor != "MAC Lookup Unavailable") {
            UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_MAC, vendor);
        }
        AddItemUnderParentIfNotBlank(item, "MAC", GetItem(res, "MAC"));
        AddItemUnderParentIfNotBlank(item, "MAC Vendor", vendor);
    }
}

void xScannerFrame::ProcessDiscoverResult(std::list<std::pair<std::string, std::string>>& res)
{
    // Type
    // IP
    // Discovered
    // Vendor
    // Model

    auto ip = GetItem(res, "IP");
    auto item = GetIPItem(ip);

    if (item.IsOk()) {
        auto name = GetItem(res, "Name");
        if (name == "") {
            name = GetItem(res, "Vendor") + ":" + GetItem(res, "Model");
        }
        if (name != ":") {
            UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_DISCOVER, name);
        }
        AddItemUnderParentIfNotBlank(item, "Name", GetItem(res, "Name"));
        AddItemUnderParentIfNotBlank(item, "Vendor", GetItem(res, "Vendor"));
        AddItemUnderParentIfNotBlank(item, "Model", GetItem(res, "Model"));
        AddItemUnderParentIfNotBlank(item, "Discovered", GetItem(res, "Discovered"));
        AddItemUnderParentIfNotBlank(item, "Platform", GetItem(res, "Platform"));
        AddItemUnderParentIfNotBlank(item, "Platform Model", GetItem(res, "Platform Model"));
        AddItemUnderParentIfNotBlank(item, "Version", GetItem(res, "Version"));
        AddItemUnderParentIfNotBlank(item, "Mode", GetItem(res, "Mode"));
        AddItemUnderParentIfNotBlank(item, "Pixels", GetItem(res, "Pixels"));
    }
}

void xScannerFrame::ProcessxScheduleResult(std::list<std::pair<std::string, std::string>>& res)
{
    // IP
    // Type
    // Port
    // Version

    auto ip = GetItem(res, "IP");
    auto item = GetIPItem(ip);

    if (item.IsOk()) {

        UpdateDeviceTitle(_tree, item, TITLE_PRIORITY::TP_XSCHEDULE, "xSchedule");

        AddItemUnderParentIfNotBlank(item, "xSchedule Port", GetItem(res, "Port"));
        AddItemUnderParentIfNotBlank(item, "xSchedule Version", GetItem(res, "Version"));
    }
}

void xScannerFrame::ProcessProxiedResult(std::list<std::pair<std::string, std::string>>& res)
{
    // IP
    // Type
    // Proxied By

    auto ip = GetItem(res, "IP");
    auto item = GetIPItem(ip);

    if (item.IsOk()) {
        AddItemUnderParentIfNotBlank(item, "Proxied By", GetItem(res, "Proxied By"));
    }
}

void xScannerFrame::ProcessScanResult(std::list<std::pair<std::string, std::string>>& res)
{
    auto type = GetItem(res, "Type");
    if (type == "Computer")         {
        ProcessComputerResult(res);
    }
    else if (type == "Ping") {
        ProcessPingResult(res);
    }
    else if (type == "HTTP") {
        ProcessHTTPResult(res);
    }
    else if (type == "FPP") {
        ProcessFPPResult(res);
    }
    else if (type == "Falcon") {
        ProcessFalconResult(res);
    }
    else if (type == "MAC") {
        ProcessMACResult(res);
    }
    else if (type == "Discover") {
        ProcessDiscoverResult(res);
    }
    else if (type == "xSchedule") {
        ProcessxScheduleResult(res);
    }
    else if (type == "Controller") {
        ProcessControllerResult(res);
    }
    else if (type == "Proxied") {
        ProcessProxiedResult(res);
    }
    else         {
        // unexpected type
        wxASSERT(false);
    }
}

void xScannerFrame::ProcessScanResults()
{
    while (true)         {
        auto res = _workManager.GetNextResult();
        if (res.size() == 0) break;

        ProcessScanResult(res);
    }
}
xScannerFrame::~xScannerFrame()
{
    //(*Destroy(xScannerFrame)
    //*)
}

void xScannerFrame::OnQuit(wxCommandEvent& event)
{
    _workManager.Stop();
    Close();
}

void xScannerFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xScanner v%s, the xLights configuration scanner."), GetDisplayVersionString());
    wxMessageBox(about, _("Welcome to..."));
}

void xScannerFrame::ValidateWindow()
{
}

void xScannerFrame::OnResize(wxSizeEvent& event)
{
    Layout();
}

void xScannerFrame::CreateDebugReport(xlCrashHandler* crashHandler)
{
    crashHandler->ProcessCrashReport(xlCrashHandler::SendReportOptions::ASK_USER_TO_SEND);
}

void xScannerFrame::OnKeyDown(wxKeyEvent& event)
{
    ValidateWindow();
}

void xScannerFrame::OnMenuItemScanSelected(wxCommandEvent& event)
{
    Scan();
}

void xScannerFrame::OnTreeItemActivated(wxTreeListEvent& event)
{
    auto iptxt = _tree->GetItemText(event.GetItem()).BeforeFirst(' ');
    if (ip_utils::IsIPValid(iptxt)) {
        ::wxLaunchDefaultBrowser(iptxt);
    }
}

void xScannerFrame::OnTreeRClick(wxTreeListEvent& event)
{
    _item = event.GetItem();
    wxMenu mnuLayer;
    mnuLayer.Append(ID_MNU_RESCAN, "Rescan");
    mnuLayer.Append(ID_MNU_EXPORT, "Export to CSV");
    if (ip_utils::IsIPValid(_tree->GetItemText(_item,0))) {
        mnuLayer.Append(ID_MNU_ADDTOXLIGHTS, "Import to xLights");
    }
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xScannerFrame::OnPopup, nullptr, this);
    PopupMenu(&mnuLayer);
}

void xScannerFrame::ExportItem(int skip, wxTreeListItem& item, wxFile& f)
{
    wxString out;
    for (int i = 0; i < skip; i++) out += ",";

    auto i1 = _tree->GetItemText(item, 0);
    auto i2 = _tree->GetItemText(item, 1);
    auto i3 = _tree->GetItemText(item, 2);
    auto i4 = _tree->GetItemText(item, 3);

    if (i4 != "") out += i1 + "," + i2 + "," + i3 + "," + i4;
    else if (i3 != "") out += i1 + "," + i2 + "," + i3;
    else if (i2 != "") out += i1 + "," + i2;
    else if (i1 != "") out += i1;

    out += "\r\n";

    f.Write(out);

    skip++;

    for (auto i = _tree->GetFirstChild(item); i.IsOk(); i = _tree->GetNextSibling(i)) {
        ExportItem(skip, i, f);
    }
}

void xScannerFrame::AddtoxLights(wxTreeListItem& item)
{
    auto const ip = _tree->GetItemText(item, 0);
    auto name = _tree->GetItemText(item, 1);
    if (name.empty()) {
        name = ip;
    }

    auto controllertype = GetItemUnderParent(item, "Vendor/Model/Variant");
    auto typeList = wxSplit(controllertype, ':');
    auto vendor = typeList.size() > 0 ? typeList[0] : wxString();
    auto model = typeList.size() > 1 ? typeList[1] : wxString();
    auto variant = typeList.size() > 3 ? typeList[2] : wxString();

    std::string const cmd = "{\"cmd\":\"addEthernetController\", \"ip\":\"" + ip + "\", \"name\":\"" + name +
                            "\", \"vendor\":\"" + vendor + "\", \"model\":\"" + model + "\", \"variant\":\"" + variant + "\"}";
    auto const stat = Automation(false, "127.0.0.1", 0, "", cmd, {}, "");
    if (stat != 0) {
        wxMessageBox("Unable to Add Controller to xLights.\nVerify xLights is Running and xFade Port A or B is set in File->Preferences->Output Tab", "Error", 5L, this);
    }
}

void xScannerFrame::OnPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_MNU_EXPORT) {
        wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "xScanner.csv", wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (filename.IsEmpty()) return;

        wxFile f(filename);

        if (!f.Create(filename, true) || !f.IsOpened()) {
            wxMessageBox(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString(), "Error", 5L, this);
            return;
        }

        auto item = _tree->GetRootItem();
        ExportItem(0, item, f);
    }
    else if (event.GetId() == ID_MNU_RESCAN) {

        // reset the work manager ... this cleans most current activities out
        _workManager.Restart();

        Scan();
    } else if (event.GetId() == ID_MNU_ADDTOXLIGHTS) {
        AddtoxLights(_item);
    }
}

void xScannerFrame::OnTimer1Trigger(wxTimerEvent& event)
{
    StatusBar1->SetLabelText(_workManager.GetPendingWork());
    ProcessScanResults();
}
