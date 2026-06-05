/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#else
#include <WS2tcpip.h>
#endif

#include "Discovery.h"
#include "UtilFunctions.h"

// This stuff is excluded from xSchedule build and this #define is only present in non xLights builds
#ifndef EXCLUDENETWORKUI
#include "controllers/ControllerCaps.h"
#include "controllers/SanDevices.h"
#include "controllers/Falcon.h"
#include "controllers/PowerDMX.h"
#endif

#include <log.h>

#include "utils/CurlManager.h"

#include "utils/ip_utils.h"

#if __has_include(<dns_sd.h>)
// if the dns_sd include header is available, we can do the bonjour discovery
#include <dns_sd.h>
#endif

// On Windows (where Apple's dns_sd.h is normally absent) use the native
// DNS-SD / mDNS API in <windns.h> instead. macOS uses dns_sd.h above and
// Linux uses it too (via the avahi-compat-libdns_sd shim), so this path is
// Windows-only.
#if defined(_WIN32) && !defined(_DNS_SD_H)
#define XL_USE_WINDNS 1
#include <windns.h>
#include <mutex>
#include <memory>
#endif

#include <algorithm>
#include <cctype>

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

Discovery::DatagramData::DatagramData(int p, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)> & cb) : port(p) {
    callbacks.push_back(cb);
    Init("", p);
}

Discovery::DatagramData::DatagramData(const std::string &mc, int p, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)> & cb) : port(p) {
    callbacks.push_back(cb);
    Init(mc, p);
}

void Discovery::DatagramData::Init(const std::string &mc, int port) {
    // Main socket bound to ANY:port for receiving
    auto* mainSocket = new sockets::UDPSocket();
    if (mainSocket->Open(true) && mainSocket->Bind("", port, true)) {
        sockets.push_back(mainSocket);
    } else {
        delete mainSocket;
    }

    // Per-interface sockets for sending broadcasts out each NIC
    auto const localIPs = ip_utils::GetLocalIPs();
    for (const auto& ip : localIPs) {
        if (ip == "127.0.0.1") {
            continue;
        }
        auto* sock = new sockets::UDPSocket();
        if (sock->Open(true) && sock->Bind(ip, 0, false)) {
            sockets.push_back(sock);
        } else {
            delete sock;
        }
    }

    if (!mc.empty()) {
        for (auto* sock : sockets) {
            sock->JoinMulticast(mc);
        }
    }
}

Discovery::DatagramData::~DatagramData() {
    for (auto* sock : sockets) {
        sock->Close();
        delete sock;
    }
}


#ifdef XL_USE_WINDNS
struct WinMDNSState;
#endif

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
#elif defined(XL_USE_WINDNS)
    std::shared_ptr<WinMDNSState> state;
    std::shared_ptr<WinMDNSState>* browseCtx = nullptr;
    std::wstring browseQuery; // backing storage for DNS_SERVICE_BROWSE_REQUEST::QueryName
    DNS_SERVICE_CANCEL browseCancel{};
    bool browsing = false;
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

#ifdef XL_USE_WINDNS
namespace {
typedef DNS_STATUS (WINAPI *PFN_DnsServiceBrowse)(PDNS_SERVICE_BROWSE_REQUEST, PDNS_SERVICE_CANCEL);
typedef DNS_STATUS (WINAPI *PFN_DnsServiceBrowseCancel)(PDNS_SERVICE_CANCEL);
typedef DNS_STATUS (WINAPI *PFN_DnsServiceResolve)(PDNS_SERVICE_RESOLVE_REQUEST, PDNS_SERVICE_CANCEL);
typedef VOID (WINAPI *PFN_DnsServiceFreeInstance)(PDNS_SERVICE_INSTANCE);
typedef VOID (WINAPI *PFN_DnsRecordListFree)(PDNS_RECORD, DNS_FREE_TYPE);

struct WinDnsApi {
    PFN_DnsServiceBrowse browse = nullptr;
    PFN_DnsServiceBrowseCancel browseCancel = nullptr;
    PFN_DnsServiceResolve resolve = nullptr;
    PFN_DnsServiceFreeInstance freeInstance = nullptr;
    PFN_DnsRecordListFree recordListFree = nullptr;
    bool ok = false;
};

// The DnsService* DNS-SD functions only exist on Windows 10 1703+. Resolve
// them dynamically so xLights still launches (just without mDNS discovery) on
// older Windows where a static import would fail to load the whole process.
const WinDnsApi& GetWinDnsApi() {
    static const WinDnsApi api = []() {
        WinDnsApi a;
        if (HMODULE h = LoadLibraryW(L"dnsapi.dll")) {
            a.browse = (PFN_DnsServiceBrowse)GetProcAddress(h, "DnsServiceBrowse");
            a.browseCancel = (PFN_DnsServiceBrowseCancel)GetProcAddress(h, "DnsServiceBrowseCancel");
            a.resolve = (PFN_DnsServiceResolve)GetProcAddress(h, "DnsServiceResolve");
            a.freeInstance = (PFN_DnsServiceFreeInstance)GetProcAddress(h, "DnsServiceFreeInstance");
            a.recordListFree = (PFN_DnsRecordListFree)GetProcAddress(h, "DnsRecordListFree");
            a.ok = a.browse && a.browseCancel && a.resolve && a.freeInstance && a.recordListFree;
        }
        return a;
    }();
    return api;
}
}

// Shared state between a BonjourData and its in-flight async DnsService
// callbacks (which run on OS thread-pool threads). Held via shared_ptr by both
// sides so it outlives the BonjourData until every outstanding resolve callback
// has fired; `active` is cleared on teardown so those late callbacks become
// no-ops instead of touching a destroyed object.
struct WinMDNSState {
    std::mutex mtx;
    bool active = true;
    std::vector<std::string> pendingIps;
    std::set<std::wstring> seenInstances;
};

namespace {
struct WinResolveCtx {
    std::shared_ptr<WinMDNSState> state;
    std::wstring name; // backing storage for DNS_SERVICE_RESOLVE_REQUEST::QueryName
    DNS_SERVICE_CANCEL cancel{};
};

// Called once per DnsServiceResolve, on a thread-pool thread.
void WinResolveCallback(DWORD status, PVOID context, PDNS_SERVICE_INSTANCE instance) {
    std::unique_ptr<WinResolveCtx> ctx(static_cast<WinResolveCtx*>(context));
    if (instance) {
        if (status == ERROR_SUCCESS && instance->ip4Address) {
            in_addr addr;
            addr.S_un.S_addr = *instance->ip4Address;
            char buf[INET_ADDRSTRLEN] = { 0 };
            if (inet_ntop(AF_INET, &addr, buf, sizeof(buf)) != nullptr && strcmp(buf, "0.0.0.0") != 0) {
                std::lock_guard<std::mutex> lk(ctx->state->mtx);
                if (ctx->state->active) {
                    ctx->state->pendingIps.emplace_back(buf);
                }
            }
        }
        GetWinDnsApi().freeInstance(instance);
    }
}

// Called (possibly repeatedly) as DnsServiceBrowse finds service instances.
void WinBrowseCallback(DWORD status, PVOID context, PDNS_RECORD records) {
    auto state = *static_cast<std::shared_ptr<WinMDNSState>*>(context);
    if (status == ERROR_SUCCESS) {
        // DnsService* APIs are wide-only, so the records are always DNS_RECORDW
        // regardless of the build's UNICODE setting.
        for (auto* rec = (DNS_RECORDW*)records; rec != nullptr; rec = rec->pNext) {
            if (rec->wType != DNS_TYPE_PTR || rec->Data.PTR.pNameHost == nullptr) {
                continue;
            }
            std::wstring instance = rec->Data.PTR.pNameHost;
            {
                std::lock_guard<std::mutex> lk(state->mtx);
                if (!state->active) {
                    break;
                }
                if (!state->seenInstances.insert(instance).second) {
                    continue; // already resolving/resolved this instance
                }
            }
            auto* ctx = new WinResolveCtx();
            ctx->state = state;
            ctx->name = instance;
            DNS_SERVICE_RESOLVE_REQUEST req{};
            req.Version = DNS_QUERY_REQUEST_VERSION1;
            req.QueryName = const_cast<PWSTR>(ctx->name.c_str());
            req.pResolveCompletionCallback = WinResolveCallback;
            req.pQueryContext = ctx;
            DNS_STATUS s = GetWinDnsApi().resolve(&req, &ctx->cancel);
            if (s != DNS_REQUEST_PENDING && s != ERROR_SUCCESS) {
                delete ctx; // completion callback will not fire
            }
        }
    }
    if (records) {
        GetWinDnsApi().recordListFree(records, DnsFreeRecordList);
    }
}
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
#elif defined(XL_USE_WINDNS)
    const WinDnsApi& api = GetWinDnsApi();
    if (!api.ok) {
        return; // pre-1703 Windows: no mDNS, broadcast/multicast discovery still runs
    }
    state = std::make_shared<WinMDNSState>();
    // mDNS browse wants the fully-qualified service type, e.g. "_fppd._udp.local"
    browseQuery.assign(serviceName.begin(), serviceName.end()); // serviceName is ASCII
    if (browseQuery.find(L".local") == std::wstring::npos) {
        browseQuery += L".local";
    }
    browseCtx = new std::shared_ptr<WinMDNSState>(state);
    DNS_SERVICE_BROWSE_REQUEST req{};
    req.Version = DNS_QUERY_REQUEST_VERSION1;
    req.pBrowseCallback = WinBrowseCallback;
    req.pQueryContext = browseCtx;
    req.QueryName = const_cast<PWSTR>(browseQuery.c_str());
    DNS_STATUS s = api.browse(&req, &browseCancel);
    browsing = (s == DNS_REQUEST_PENDING || s == ERROR_SUCCESS);
    if (!browsing) {
        delete browseCtx;
        browseCtx = nullptr;
    }
#endif
}
BonjourData::~BonjourData() {
#ifdef _DNS_SD_H
    for (auto &ref : bonjourRefs) {
        DNSServiceRefDeallocate(ref);
    }
    bonjourRefs.clear();
#elif defined(XL_USE_WINDNS)
    if (state) {
        {
            std::lock_guard<std::mutex> lk(state->mtx);
            state->active = false; // make any late resolve callbacks no-ops
        }
        if (browsing) {
            // After this returns the browse callback is no longer invoked, so it's
            // safe to release the context holding our shared_ptr to the state.
            GetWinDnsApi().browseCancel(&browseCancel);
        }
    }
    delete browseCtx;
    browseCtx = nullptr;
    // Outstanding resolves still hold their own shared_ptr<WinMDNSState> via
    // WinResolveCtx; the state stays alive until each completion callback fires
    // (and frees its context), at which point it sees active==false and does nothing.
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
        for (auto &ref : bonjourRefs) {
            dnssd_sock_t sock = DNSServiceRefSockFD(ref);
            if (FD_ISSET( sock, &fds ) != 0) {
                DNSServiceProcessResult(ref);
            }
        }
    }
#elif defined(XL_USE_WINDNS)
    // The Windows DnsService* APIs deliver results asynchronously on their own
    // threads; here we just drain whatever IPs they've found and dispatch them
    // on the discovery thread.
    if (!state) {
        return;
    }
    std::vector<std::string> ips;
    {
        std::lock_guard<std::mutex> lk(state->mtx);
        ips.swap(state->pendingIps);
    }
    for (const auto& ip : ips) {
        invokeCallbacks(ip);
    }
#endif
}
void BonjourData::invokeCallbacks(const std::string &ip) {
    for (auto &cb : callbacks) {
        cb(ip);
    }
}



Discovery::Discovery(OutputManager* outputManager, DiscoveryDelegate* delegate)
    : _outputManager(outputManager), _delegate(delegate ? delegate : &_defaultDelegate) {
}

Discovery::~Discovery() {
    Close(true);
    for (size_t x = 0; x < results.size(); ++x) {
        delete results[x];
    }
}

void Discovery::Close(bool wait) {
    if (wait) {
        while (CurlManager::INSTANCE.processCurls()) {
            _delegate->YieldToUI();
        }
    }
    for (auto &dg : datagrams) {
        delete dg;
    }
    datagrams.clear();
    for (auto &bj : bonjours) {
        delete bj;
    }
    bonjours.clear();
    if (!CurlManager::INSTANCE.processCurls()) {
        for (auto &h : https ) {
            delete h.second;
        }
        https.clear();
    }
}


Discovery::CurlData::~CurlData() {
}

void Discovery::AddCurl(const std::string &host, const std::string &url, std::function<bool(int rc, const std::string &buffer, const std::string &errorBuffer)>&& callback) {
    auto logger = spdlog::get("curl");
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
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 3000L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);

    logger->info("Discovery Adding CURL - URL: {}    Method: GET", furl.c_str());

    int authStatus = data->authStatus;
    CurlManager::INSTANCE.addCURL(furl, curl, [this, url, furl, host, callback, authStatus, data, logger](CURL* c) {
        if (finished) {
            return;
        }
        CurlManager::CurlPrivateData* cpd = nullptr;
        long rc = 0;
        curl_easy_getinfo(c, CURLINFO_PRIVATE, &cpd);
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &rc);

        logger->info("    Discovery CURL Callback - URL: {}  RC: {}    Size: {}", furl.c_str(), rc, cpd ? cpd->resp.size() : -1);
        if (rc == 401) {
            if (HandleAuth(host, c, authStatus) || authStatus != data->authStatus) {
                data->urls.erase(furl);
                AddCurl(host, url, [callback, furl](int rc, const std::string &buffer, const std::string &errorBuffer) {
                    return callback(rc, buffer, errorBuffer);
                });
            } else {
                logger->info("    Discovery CURL Callback - Unauthorized");
            }
        } else {
            std::string resp(reinterpret_cast<char*>(cpd->resp.data()), cpd->resp.size());
            callback(rc, resp, cpd->errorResp);
        }
    });
}

bool Discovery::HandleAuth(const std::string &host, CURL* curl, int origAuthStatus) {
    // we need to authenticate and redo
    Discovery::CurlData *data = https[host];
    if (origAuthStatus != 0) {
        // still 401 so password must not be correct, remove it and re-prompt
        data->authStatus = 0;
        _delegate->StorePassword(host, "admin", "");
    }
    if (data->authStatus == 0) {
        bool handled = false;
        std::string ip = host;
        CurlManager::INSTANCE.setProcessCallbacks(false);
        if (data->authStatus != 2) {
            std::string username;
            std::string password;
            if (_delegate->GetStoredPassword(host, username, password)) {
                if (!password.empty()) {
                    data->authStatus = 2;
                    CurlManager::INSTANCE.setHostUsernamePassword(host, username, password);
                    //redo
                    handled = true;
                }
            }
        }
        if (!handled) {
            std::string username = "admin";
            std::string password;
            bool savePassword = false;
            if (_delegate->PromptForPassword(ip, username, password, savePassword)) {
                if (savePassword) {
                    _delegate->StorePassword(ip, username, password);
                }
                CurlManager::INSTANCE.setHostUsernamePassword(host, username, password);
                handled = true;
                discoveryFinishTime = GetCurrentTimeMillis() + 10000L;
                data->authStatus = 1;
            }
        }
        CurlManager::INSTANCE.setProcessCallbacks(true);
        return handled;
    }
    return false;
}

void Discovery::AddMulticast(const std::string &mcAddr, int port, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)>&& callback) {
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

void Discovery::AddBroadcast(int port, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)>&& callback) {
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
    for (auto &a : datagrams) {
        if (port == a->port) {
            for (auto* s : a->sockets) {
                s->SendTo("255.255.255.255", port, buffer, len);
            }
        }
    }
}

void Discovery::SendData(int port, const std::string &host, uint8_t *buffer, int len) {
    for (auto &a : datagrams) {
        if (port == a->port) {
            // only need to send on the first socket as it's routed via the hostname
            a->sockets.front()->SendTo(host, port, buffer, len);
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
            std::string falcModel = falc.GetModel();
            std::transform(falcModel.begin(), falcModel.end(), falcModel.begin(),
                           [](unsigned char c) { return std::toupper(c); });
            cd->SetModel(falcModel);
            cd->version = falc.GetFullName();
            cd->platform = "Falcon";
            std::string mode = falc.GetMode();
            if (mode == "Player") {
                cd->mode = "player";
            } else if (mode == "Remote") {
                cd->mode = "remote";
            } else if (mode == "Master") {
                cd->mode = "player w/multisync";
            } else {
                cd->mode = "bridge";
            }
            int stringCount = falc.NumConfiguredStrings();
            auto variants = ControllerCaps::GetVariants(CONTROLLER_ETHERNET, "Falcon", falcModel);
            for (auto const& a : variants) {
                ControllerCaps *caps = ControllerCaps::GetControllerConfig("Falcon", falcModel, a);
                if (caps && stringCount <= caps->GetMaxPixelPort()) {
                    cd->SetVariant(a);
                }
            }
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
        // illuminous PowerDMX
    } else if (htmlBuffer.find("PowerDMX") != std::string::npos) {
        DiscoveredData* cd = FindByIp(ip);
        ControllerEthernet* ce = cd ? cd->controller : nullptr;
        PowerDMX falc(ip, proxy);
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
            cd->SetVendor("PowerDMX");
            std::string model = falc.GetModel();
            model = Upper(model);

            cd->SetModel(model);
            cd->pixelControllerType = model;
            cd->typeId = 0xD0;
            // cd->version = falc.GetFullName();
            cd->platform = model;
            cd->mode = "player";            
            ce->SetProtocol(OUTPUT_DDP);
            ce->SetAutoSize(true, nullptr);
            ce->SetAutoLayout(true);
            ce->SetFullxLightsControl(true);
            
            return cd;
        }
    } else {
        //others we can detect and decode?   J1Sys? Minleon? PixLite?  HinksPix?  etc?
    }
#endif
    return nullptr;
}


void Discovery::Discover() {
    uint64_t curMs = GetCurrentTimeMillis();
    auto endBroadcastTime = curMs + 1000l;
    discoveryFinishTime = curMs + 5000L; // Start with 5 seconds max
    bool running = CurlManager::INSTANCE.processCurls();
    uint8_t buffer[1500];
    finished = false;
    uint32_t instanceCount = 0;
    while ((running || (curMs < endBroadcastTime))
           && (curMs < discoveryFinishTime)){
        memset(buffer, 0x00, sizeof(buffer));
        int readSize = 0;
        //first check to see if any of the socket have received data
        for (const auto& dg : datagrams) {
            for (auto* sock : dg->sockets) {
                while (sock->WaitForData(0)) {
                    std::string fromIP;
                    readSize = sock->ReceiveFrom(buffer, sizeof(buffer), &fromIP);
                    if (readSize <= 0) break;
                    for (auto &cb : dg->callbacks) {
                        cb(buffer, readSize, fromIP);
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
        _delegate->YieldToUI();
        curMs = GetCurrentTimeMillis();

        // If discovery is finding new instances, we'll increase
        // the timeouts so if there are a LOT of instances where
        // we start hitting curl resource limits and such, we
        // we can still discover everything.  If all the curls
        // finish, this bails immediately anyway so this
        // only increases the time if it's actually able to
        // find new instances
        if (instanceCount != results.size()) {
            instanceCount = results.size();
            auto nt = curMs + 1000;
            if (nt > discoveryFinishTime) {
                discoveryFinishTime = nt;
            }
        }
    }
    finished = true;
}
