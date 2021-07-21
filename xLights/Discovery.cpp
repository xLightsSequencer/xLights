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
static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
    if (data == nullptr) return 0;
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}
Discovery::CurlData::CurlData(const std::string &h, const std::string &u, std::function<bool(int rc, const std::string &buffer, const std::string &errorBuffer)> & cb) : host(h), url(u), callback(cb) {
    errorBuffer = new char[CURL_ERROR_SIZE];
    curl = nullptr;
}
Discovery::CurlData::~CurlData() {
    curl_easy_cleanup(curl);
}

void Discovery::CurlData::SetupCurl() {
    if (curl != nullptr) {
        curl_easy_cleanup(curl);
    }
    std::string furl = "http://" + host + url;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_URL, furl.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 2000);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 30000);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    if (username != "") {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
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
    wxDatagramSocket *mainSocket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_NOWAIT);
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
        
        wxDatagramSocket *socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_NOWAIT);
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


Discovery::Discovery(wxWindow* frame, OutputManager* outputManager) : _frame(frame), _outputManager(outputManager) {
    curlMulti = curl_multi_init();
}

Discovery::~Discovery() {
    for (size_t x = 0; x < curls.size(); x++) {
        if (curls[x]) {
            curl_multi_remove_handle(curlMulti, curls[x]);
            delete curls[x];
            curls[x] = nullptr;
            numCurls--;
        }
    }
    curl_multi_cleanup(curlMulti);
    
    for (size_t x = 0; x < results.size(); x++) {
        delete results[x];
    }
    for (auto &dg : datagrams) {
        delete dg;
    }
}

void Discovery::AddCurl(const std::string &host, const std::string &url, std::function<bool(int rc, const std::string &buffer, const std::string &errorBuffer)>&& callback) {
    CurlData *curl = new CurlData(host, url, callback);
    //check if we've logged into the host before and reuse credentials
    for (auto c : curls) {
        if (c && c->host == curl->host && c->username != "") {
            curl->username = c->username;
            curl->password = c->password;
            curl->authStatus = c->authStatus;
        }
    }
    curl->SetupCurl();
    curls.push_back(curl);
    curl_multi_add_handle(curlMulti, curl->curl);
    numCurls++;
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
    } else if (htmlBuffer.find("pixelcontroller.com") != std::string::npos && htmlBuffer.find("Controller Information") != std::string::npos) {
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
            for (auto a : variants) {
                ControllerCaps *caps = ControllerCaps::GetControllerConfig("Falcon", model, a);
                if (caps && stringCount <= caps->GetMaxPixelPort()) {
                    cd->SetVariant(a);
                }
            }
            //find the mode the controller is set to
            int idx = htmlBuffer.find("name=\"m\" ");
            if (idx > 0) {
                idx = htmlBuffer.find("value=\"", idx);
                if (idx > 0) {
                    idx += 7;
                    if (htmlBuffer[idx] == '0') {
                        ce->GetOutput(0)->SetChannels(510);
                        ce->SetProtocol(OUTPUT_E131);
                        ce->SetAutoSize(true, nullptr);
                        ce->SetAutoLayout(true);
                        ce->SetFullxLightsControl(true);
                        ce->GetOutput(0)->SetChannels(512);
                    }
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

void Discovery::HandleAuth(int curlIdx) {
    // we need to authenticate and redo
    if (curls[curlIdx]->authStatus != 1) {
        bool handled = false;
        std::string ip = curls[curlIdx]->host;
        if (curls[curlIdx]->authStatus != 2) {
            if (xlPasswordEntryDialog::GetStoredPasswordForService(curls[curlIdx]->host, curls[curlIdx]->username, curls[curlIdx]->password)) {
                if (curls[curlIdx]->password != "") {
                    curl_multi_remove_handle(curlMulti, curls[curlIdx]->curl);
                    curls[curlIdx]->SetupCurl();
                    curl_multi_add_handle(curlMulti, curls[curlIdx]->curl);
                    curls[curlIdx]->authStatus = 2;
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
                curls[curlIdx]->username = username;
                curls[curlIdx]->password = password;
                curl_multi_remove_handle(curlMulti, curls[curlIdx]->curl);
                curls[curlIdx]->SetupCurl();
                curl_multi_add_handle(curlMulti, curls[curlIdx]->curl);
            }
        }
    }
}

void Discovery::Discover() {

    if (curlMulti == nullptr) return;

    auto endBroadcastTime = wxGetLocalTimeMillis().GetValue() + 1200l;
    int running = numCurls;
    uint8_t buffer[1500];
    
    while (running || (wxGetLocalTimeMillis().GetValue() < endBroadcastTime)) {
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
        //now check the http/curls
        int start = running;
        curl_multi_perform(curlMulti, &running);
        if (start != running) {
            struct CURLMsg *m;
            int msgq = 0;
            while ((m = curl_multi_info_read(curlMulti, &msgq))) {
                if (m->msg == CURLMSG_DONE) {
                    CURL *e = m->easy_handle;
                    long response_code = 0;
                    curl_easy_getinfo(e, CURLINFO_HTTP_CODE, &response_code);
                    
                    for (size_t x = 0; x < curls.size(); x++) {
                        if (curls[x] && curls[x]->curl == e) {
                            if (response_code == 401) {
                                HandleAuth(x);
                            } else if (curls[x]->callback(response_code, curls[x]->buffer, curls[x]->errorBuffer)) {
                                curl_multi_remove_handle(curlMulti, curls[x]->curl);
                                delete curls[x];
                                curls[x] = nullptr;
                                numCurls--;
                            }
                        }
                    }
                }
            }
        }
    }
}
