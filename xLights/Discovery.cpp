/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/socket.h>
#include <wx/secretstore.h>

#ifndef __WXMSW__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#endif

#include "Discovery.h"
#include "UtilFunctions.h"

// This stuff is excluded from xSchedule build and this #define is only present in non xLights builds
#ifndef EXCLUDENETWORKUI
#include "controllers/ControllerCaps.h"
#include "controllers/SanDevices.h"
#include "controllers/Falcon.h"
#endif

#include <log4cpp/Category.hh>

#include "utils/CurlManager.h"

#if __has_include(<dns_sd.h>)
// if the dns_sd include header is available, we can do the bonjour discovery
#include <dns_sd.h>
#endif

bool xlPasswordEntryDialog::Create(wxWindow *parent,
            const wxString& message,
            const wxString& caption,
            const wxString& value,
            long style,
            const wxPoint& pos) {
    bool b = wxPasswordEntryDialog::Create(parent, message, caption, value, style, pos);
    
    savePasswordCheckbox = new wxCheckBox(this, wxID_ANY, _("Save Password"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PWD"));
    savePasswordCheckbox->SetValue(false);
    
    wxSizerFlags flagsBorder2;
    flagsBorder2.DoubleBorder();

#if wxUSE_SECRETSTORE
    GetSizer()->Insert(2, savePasswordCheckbox, flagsBorder2);
    GetSizer()->SetSizeHints(this);
    GetSizer()->Fit(this);
#endif
    return b;
}
    
#if wxUSE_SECRETSTORE
static wxSecretStore pwdStore = wxSecretStore::GetDefault();
bool xlPasswordEntryDialog::GetStoredPasswordForService(const std::string &service, std::string &user, std::string &pwd) {
    if (pwdStore.IsOk()) {
        wxSecretValue password;
        wxString usr;
        if (pwdStore.Load("xLights/Discovery/" + service, usr, password)) {
            user = usr;
            pwd = password.GetAsString();
            return true;
        }
    }
    return false;
}

bool xlPasswordEntryDialog::StorePasswordForService(const std::string &service, const std::string &user, const std::string &pwd) {
    if (pwdStore.IsOk()) {
        wxSecretValue password(pwd);
        if (pwdStore.Save("xLights/Discovery/" + service, user, password)) {
            return true;
        }
    }
    return false;
}
#else
bool xlPasswordEntryDialog::GetStoredPasswordForService(const std::string &service, std::string &user, std::string &pwd) {
    return false;
}
bool xlPasswordEntryDialog::StorePasswordForService(const std::string &service, const std::string &user, const std::string &pwd) {
    return false;
}
#endif

DiscoveredData::DiscoveredData(ControllerEthernet *e) {
    controller = e;
    ip = e->GetResolvedIP();
    hostname = e->GetIP();
    description = e->GetDescription();
    vendor = e->GetVendor();
    model = e->GetModel();
    variant = e->GetVariant();
    proxy = e->GetFPPProxy();
}
void DiscoveredData::SetModel(const std::string &m) {
    model = m;
    if (controller) {
        controller->SetModel(m);
    }
}
void DiscoveredData::SetVendor(const std::string &v) {
    vendor = v;
    if (controller) {
        controller->SetVendor(v);
    }
}

void DiscoveredData::SetVariant(const std::string &v) {
    variant = v;
    if (controller) {
        controller->SetVariant(v);
    }
}

void DiscoveredData::SetDescription(const std::string &s) {
    description = s;
    if (controller) {
        controller->SetDescription(s);
    }
}

void DiscoveredData::SetProxy(const std::string &p) {
    proxy = p;
    if (controller) {
        controller->SetFPPProxy(p);
    }
}

DiscoveredData::~DiscoveredData() {
    if (controller) {
        delete controller;
    }
}

Discovery::DatagramData::DatagramData(int p, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)> & cb) : port(p) {
    callbacks.push_back(cb);
    Init("", p);
}

Discovery::DatagramData::DatagramData(const std::string &mc, int p, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)> & cb) : port(p) {
    callbacks.push_back(cb);
    Init(mc, p);
}

void Discovery::DatagramData::Init(const std::string &mc, int port) {
    wxIPV4address localaddr;
    localaddr.AnyAddress();
    localaddr.Service(port);
    wxDatagramSocket* mainSocket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_BLOCK); // dont use NOWAIT as it can result in dropped packets
    mainSocket->SetTimeout(1);
    mainSocket->Notify(false);
    if (mainSocket->IsOk()) {
        sockets.push_back(mainSocket);
    }
    auto localIPs = GetLocalIPs();
    for (const auto& ip : localIPs) {
        if (ip == "127.0.0.1") {
            continue;
        }
        wxIPV4address localaddr;
        localaddr.Hostname(ip);
        
        wxDatagramSocket* socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_BLOCK); // dont use NOWAIT as it can result in dropped packets
        socket->SetTimeout(1);
        socket->Notify(false);
        if (socket->IsOk()) {
            sockets.push_back(socket);
        } else {
            delete socket;
        }
    }
    if (mc != "") {
        struct ip_mreq mreq;
        unsigned int a = 0;
        if (inet_pton(AF_INET, mc.c_str(), &a)) {
            mreq.imr_multiaddr.s_addr = a;
            mreq.imr_interface.s_addr = INADDR_ANY;
            for (auto socket : sockets) {
                socket->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*)&mreq, sizeof(mreq));
            }
        }
    }
}

Discovery::DatagramData::~DatagramData() {
    for (const auto& socket : sockets) {
        socket->Close();
        delete socket;
    }
}


class BonjourData {
public:
    BonjourData(const std::string &n, std::function<void(const std::string &ipAddress)>& callback);
    ~BonjourData();
    
    void handleEvents();
    
    std::string serviceName;
    std::list<std::function<void(const std::string &ipAddress)>> callbacks;
    void invokeCallbacks(const std::string &ip);

#ifdef _DNS_SD_H
    std::list<DNSServiceRef> bonjourRefs;
#endif
};

#ifdef _DNS_SD_H
void bonjourDNSCallBack(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
    const char *hostname, const struct sockaddr *address, uint32_t ttl, void *context) {
    
    char buf[256];
    inet_ntop(AF_INET, &(((struct sockaddr_in *)address)->sin_addr),
                        buf, 256);
    std::string ip = buf;
    if (ip != "0.0.0.0") {
        BonjourData *bj = (BonjourData*)context;
        bj->invokeCallbacks(ip);
    }
}

static void bonjourReplyCallBack(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
    const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const unsigned char *txtRecord, void *context) {
    DNSServiceRef ipRef;
    DNSServiceGetAddrInfo(&ipRef, 0, interfaceIndex, 0, hosttarget, bonjourDNSCallBack, context);
    BonjourData *bj = (BonjourData*)context;
    bj->bonjourRefs.push_back(ipRef);
}
static void BonjourBrowseCallBack(DNSServiceRef service, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
                                  const char * name, const char * type, const char * domain, void * context) {
    DNSServiceRef serviceRef;
    DNSServiceResolve(&serviceRef, 0, interfaceIndex, name, type, domain, bonjourReplyCallBack, context);
    BonjourData *bj = (BonjourData*)context;
    bj->bonjourRefs.push_back(serviceRef);
}
#endif

BonjourData::BonjourData(const std::string &n, std::function<void(const std::string &ipAddress)>& callback) : serviceName(n) {
    callbacks.push_back(callback);
#ifdef _DNS_SD_H
    DNSServiceRef serviceRef;
    DNSServiceErrorType err = DNSServiceBrowse(&serviceRef, 0, 0, serviceName.c_str(), nullptr,
                                               &BonjourBrowseCallBack, this);
    if (kDNSServiceErr_NoError == err)  {
        bonjourRefs.push_back(serviceRef);
    }
#endif
}
BonjourData::~BonjourData() {
#ifdef _DNS_SD_H
    for (auto &ref : bonjourRefs) {
        DNSServiceRefDeallocate(ref);
    }
    bonjourRefs.clear();
#endif
}
void BonjourData::handleEvents() {
#ifdef _DNS_SD_H
    fd_set fds;
    FD_ZERO( &fds );
    
    for (auto &ref : bonjourRefs) {
        dnssd_sock_t sock = DNSServiceRefSockFD(ref);
        FD_SET(sock, &fds );
    }
    
    struct timeval tv;
         
    tv.tv_sec = 0;
    tv.tv_usec = 5000;
    if (select( FD_SETSIZE, &fds, 0, 0, &tv ) > 0 ) {
        int x = 0;
        for (auto &ref : bonjourRefs) {
            dnssd_sock_t sock = DNSServiceRefSockFD(ref);
            if (FD_ISSET( sock, &fds ) != 0) {
                DNSServiceProcessResult(ref);
            }
            x++;
        }
    }
#endif
}
void BonjourData::invokeCallbacks(const std::string &ip) {
    for (auto &cb : callbacks) {
        cb(ip);
    }
}



Discovery::Discovery(wxWindow* frame, OutputManager* outputManager) : _frame(frame), _outputManager(outputManager) {
}

Discovery::~Discovery() {
    while (CurlManager::INSTANCE.processCurls()) {
        wxYieldIfNeeded();
    }
    for (size_t x = 0; x < results.size(); ++x) {
        delete results[x];
    }
    for (auto &dg : datagrams) {
        delete dg;
    }
    for (auto &bj : bonjours) {
        delete bj;
    }
    for (auto &h : https ) {
        delete h.second;
    }
}

Discovery::CurlData::~CurlData() {
}

void Discovery::AddCurl(const std::string &host, const std::string &url, std::function<bool(int rc, const std::string &buffer, const std::string &errorBuffer)>&& callback) {
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));

    std::string furl = "http://" + host + url;
    Discovery::CurlData *data = https[host];
    if (data == nullptr) {
        https[host] = new Discovery::CurlData();
        data = https[host];
    }
    if (data->urls.find(furl) != data->urls.end()) {
        //already querying this URL, we don't need to do it again
        return;
    }
    data->urls.insert(furl);
    CURL *curl = CurlManager::INSTANCE.createCurl(furl);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 3000L);

    logger_curl.info("Discovery Adding CURL - URL: %s    Method: GET", furl.c_str());

    CurlManager::INSTANCE.addCURL(furl, curl, [this, url, furl, host, callback](CURL* c) {
        if (finished) {
            return;
        }
        CurlManager::CurlPrivateData* cpd = nullptr;
        long rc = 0;
        curl_easy_getinfo(c, CURLINFO_PRIVATE, &cpd);
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &rc);

        logger_curl.info("    Discovery CURL Callback - URL: %s  RC: %d    Size: %d", furl.c_str(), rc, cpd ? cpd->resp.size() : -1);
        if (rc == 401) {
            if (HandleAuth(host, c)) {
                AddCurl(host, url, [callback](int rc, const std::string &buffer, const std::string &errorBuffer) {
                    return callback(rc, buffer, errorBuffer);
                });
            } else {
                logger_curl.info("    Discovery CURL Callback - Unauthorized");
            }
        } else {
            std::string resp(reinterpret_cast<char*>(cpd->resp.data()), cpd->resp.size());
            callback(rc, resp, cpd->errorResp);
        }
    });
}

bool Discovery::HandleAuth(const std::string &host, CURL* curl) {
    // we need to authenticate and redo
    Discovery::CurlData *data = https[host];

    if (data->authStatus != 1) {
        bool handled = false;
        std::string ip = host;
        if (data->authStatus != 2) {
            std::string username;
            std::string password;
            if (xlPasswordEntryDialog::GetStoredPasswordForService(host, username, password)) {
                if (password != "") {
                    data->authStatus = 2;
                    CurlManager::INSTANCE.setHostUsernamePassword(host, username, password);
                    //redo
                    handled = true;
                }
            }
        }
        if (!handled) {
            xlPasswordEntryDialog dlg(nullptr, "Password needed to connect to " + ip, "Password Required");
            int rc = dlg.ShowModal();
            if (rc != wxID_CANCEL) {
                std::string username = "admin";
                std::string password = dlg.GetValue().ToStdString();
                if (dlg.shouldSavePassword()) {
                    xlPasswordEntryDialog::StorePasswordForService(ip, username, password);
                }
                CurlManager::INSTANCE.setHostUsernamePassword(host, username, password);
                handled = true;
            }
        }
        return handled;
    }
    return false;
}

void Discovery::AddMulticast(const std::string &mcAddr, int port, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)>&& callback) {
    //if port already exists, just add the callback
    for (auto &a : datagrams) {
        if (port == a->port) {
            a->callbacks.push_back(callback);
            return;
        }
    }
    // doesn't, need to create the sockets and such
    DatagramData *dg = new DatagramData(mcAddr, port, callback);
    datagrams.push_back(dg);
}

void Discovery::AddBroadcast(int port, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)>&& callback) {
    //if port already exists, just add the callback
    for (auto &a : datagrams) {
        if (port == a->port) {
            a->callbacks.push_back(callback);
            return;
        }
    }
    // doesn't, need to create the sockets and such
    DatagramData *dg = new DatagramData(port, callback);
    datagrams.push_back(dg);
}
void Discovery::AddBonjour(const std::string &serviceName, std::function<void(const std::string &ipAddress)>&& callback) {
    //if port already exists, just add the callback
    for (auto &a : bonjours) {
        if (serviceName == a->serviceName) {
            a->callbacks.push_back(callback);
            return;
        }
    }
    BonjourData *dg = new BonjourData(serviceName, callback);
    bonjours.push_back(dg);
}


void Discovery::SendBroadcastData(int port, uint8_t *buffer, int len) {
    wxIPV4address bcAddress;
    bcAddress.BroadcastAddress();
    bcAddress.Service(port);

    for (auto &a : datagrams) {
        if (port == a->port) {
            for (auto s : a->sockets) {
                s->SendTo(bcAddress, buffer, len);
            }
        }
    }
}

void Discovery::SendData(int port, const std::string &host, uint8_t *buffer, int len) {
    wxIPV4address remoteaddr;
    remoteaddr.Hostname(host);
    remoteaddr.Service(port);
    for (auto &a : datagrams) {
        if (port == a->port) {
            // only need to send on the first socket as it's routed via the hostname
            a->sockets.front()->SendTo(remoteaddr, buffer, len);
        }
    }
}
DiscoveredData *Discovery::AddController(ControllerEthernet *c) {
    DiscoveredData *dd = new DiscoveredData(c);
    results.push_back(dd);

    std::string ipaddr = c->GetResolvedIP();
    DiscoveredData *byIp = FindByIp(ipaddr);
    if (byIp == dd) {
        //first time, probe
        AddCurl(ipaddr, "/", [ipaddr, c, this] (int rc, const std::string &buffer, const std::string &errorBuffer) {
            if (rc == 200) {
                DetectControllerType(ipaddr, c->GetFPPProxy(), buffer);
            }
            return true;
        });
    }
    return dd;
}

DiscoveredData *Discovery::FindByIp(const std::string &ip, const std::string &hn, bool create) {
    std::string host = ((hn == "") ? ip : hn);
    
    //first check direct IP address match
    for (auto a : results) {
        if (a->ip == ip || a->hostname == ip) {
            return a;
        }
    }
    // now attempt hostname matching
    for (auto a : results) {
        if (a->hostname == host && (a->ip == "" || ip == "")) {
            return a;
        }
    }
    if (create) {
        DiscoveredData *dd = new DiscoveredData();
        dd->ip = ip;
        dd->hostname = hn;
        results.push_back(dd);
        return dd;
    }
    return nullptr;
}

DiscoveredData *Discovery::FindByUUID(const std::string &uuid, const std::string &ip) {
    if (uuid != "") {
        for (auto a : results) {
            if (a->uuid == uuid) {
                if (ip == "" || ip == a->ip) {
                    return a;
                }
            }
        }
    }
    return nullptr;
}


DiscoveredData* Discovery::DetectControllerType(const std::string &ip, const std::string &proxy, const std::string &htmlBuffer) {
#ifndef EXCLUDENETWORKUI
    if (htmlBuffer.find("SanDevices SACN") != std::string::npos && htmlBuffer.find("Pixel Controller") != std::string::npos) {
        //SanDevices
        DiscoveredData *cd = FindByIp(ip);
        ControllerEthernet *ce = cd ? cd->controller : nullptr;
        SanDevices sand(ip, proxy);
        if (sand.IsConnected()) {
            if (!ce) {
                ce = new ControllerEthernet(_outputManager, false);
                ce->SetProtocol(OUTPUT_E131);
                ce->SetIP(ip);
                ce->SetFPPProxy(proxy);
                if (!cd) {
                    cd = AddController(ce);
                }
            }
            cd->SetVendor("SanDevices");
            cd->SetModel(sand.GetModel());
            auto variants = ControllerCaps::GetVariants(CONTROLLER_ETHERNET, "SanDevices", sand.GetModel());
            if (variants.front() != "") {
                if (sand.GetVersion()[0] == '5') {
                    cd->SetVariant("5.x Firmware");
                } else {
                    cd->SetVariant("4.x Firmware");
                }
            }
            cd->version = sand.GetVersion();
            cd->platform = "SanDevices";
            return cd;
        }
    } else if (htmlBuffer.find("pixelcontroller.com") != std::string::npos
               || htmlBuffer.find("f16v2.js") != std::string::npos
               || htmlBuffer.find("css/falcon.css") != std::string::npos
               || htmlBuffer.find("js/cntrlr_") != std::string::npos) {
        DiscoveredData *cd = FindByIp(ip);
        ControllerEthernet *ce = cd ? cd->controller : nullptr;
        Falcon falc(ip, proxy);
        if (falc.IsConnected()) {
            if (!ce) {
                ce = new ControllerEthernet(_outputManager, false);
                ce->SetProtocol(OUTPUT_E131);
                ce->SetIP(ip);
                ce->SetFPPProxy(proxy);
                if (!cd) {
                    cd = AddController(ce);
                }
            }
            
            if (falc.GetName() != "") {
                cd->hostname = falc.GetName();
                cd->SetDescription(falc.GetName());
            }
            cd->SetVendor("Falcon");
            wxString model = falc.GetModel();
            model.UpperCase();
            cd->SetModel(model.ToStdString());
            cd->version = falc.GetFullName();
            cd->platform = "Falcon";
            int stringCount = falc.NumConfiguredStrings();
            auto variants = ControllerCaps::GetVariants(CONTROLLER_ETHERNET, "Falcon", model);
            for (auto const& a : variants) {
                ControllerCaps *caps = ControllerCaps::GetControllerConfig("Falcon", model, a);
                if (caps && stringCount <= caps->GetMaxPixelPort()) {
                    cd->SetVariant(a);
                }
            }
            std::string mode = falc.GetMode();
            if (("DDP" == mode || "Player" == mode || "Remote" == mode || "Master" == mode)
                && ce->GetProtocol() != OUTPUT_DDP){
                ce->SetProtocol(OUTPUT_DDP);
                ce->SetAutoSize(true, nullptr);
                ce->SetAutoLayout(true);
                ce->SetFullxLightsControl(true);
            } else if (mode.find("31") != std::string::npos && ce->GetProtocol() != OUTPUT_E131) {
                //E1.31 or E131
                ce->GetOutput(0)->SetChannels(510);
                ce->SetProtocol(OUTPUT_E131);
                ce->SetAutoSize(true, nullptr);
                ce->SetAutoLayout(true);
                ce->SetFullxLightsControl(true);
                if (ce->GetOutput(0)->GetUniverse() == 0) {
                    //universe 0 is not valid
                    ce->GetOutput(0)->SetUniverse(1);
                }
            }
            return cd;
        }
    } else {
        //others we can detect and decode?   J1Sys? Minleon? PixLite?  HinksPix?  etc?
    }
#endif
    return nullptr;
}


void Discovery::Discover() {
    auto endBroadcastTime = wxGetLocalTimeMillis().GetValue() + 1500l;
    auto maxTime = wxGetLocalTimeMillis().GetValue() + 10000L; // 10 seconds max
    bool running = CurlManager::INSTANCE.processCurls();
    uint8_t buffer[1500];
    finished = false;
    
    while ((running || (wxGetLocalTimeMillis().GetValue() < endBroadcastTime))
           && (wxGetLocalTimeMillis().GetValue() < maxTime)){
        memset(buffer, 0x00, sizeof(buffer));
        int readSize = 0;
        //first check to see if any of the socket have received data
        for (const auto& dg : datagrams) {
            for (const auto &socket : dg->sockets) {
                if (socket->IsOk() && socket->IsData()) {
                    socket->Read(&buffer[0], sizeof(buffer));
                    readSize = socket->GetLastIOReadSize();
                    if (readSize != 0) {
                        for (auto &cb : dg->callbacks) {
                            cb(socket, buffer, readSize);
                        }
                    }
                }
            }
        }
        //check for any bonjour results
        for (auto &bj : bonjours) {
            bj->handleEvents();
        }

        //now check the http/curls
        running = CurlManager::INSTANCE.processCurls();
        wxYieldIfNeeded();
    }
    finished = true;
}
