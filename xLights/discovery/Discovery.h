#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <functional>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <curl/curl.h>

#include "outputs/ControllerEthernet.h"
#include "outputs/SocketAbstraction.h"
#include <nlohmann/json.hpp>

class OutputManager;
class Discovery;
class BonjourData;

// Interface for Discovery to call back into the UI/platform layer.
// Default implementations are no-ops so headless usage works without a subclass.
class DiscoveryDelegate {
public:
    virtual ~DiscoveryDelegate() = default;

    // Prompt user for credentials. Returns true if user provided them (not cancelled).
    virtual bool PromptForPassword(const std::string& host, std::string& username,
                                   std::string& password, bool& savePassword) { return false; }

    // Retrieve stored password for a service. Returns true if found.
    virtual bool GetStoredPassword(const std::string& service, std::string& user, std::string& pwd) { return false; }

    // Store (or delete if pwd is empty) password for a service. Returns true on success.
    virtual bool StorePassword(const std::string& service, const std::string& user, const std::string& pwd) { return false; }

    // Called periodically during discovery to yield to the UI event loop.
    // Note: not named Yield() to avoid conflict with the Windows Yield macro.
    virtual void YieldToUI() {}
};

class DiscoveredData {
public:
    DiscoveredData() {}
    DiscoveredData(ControllerEthernet *e);
    ~DiscoveredData();

    void SetModel(const std::string &m);
    void SetVendor(const std::string &v);
    void SetVariant(const std::string &v);
    void SetDescription(const std::string &s);
    void SetProxy(const std::string &p);


    ControllerEthernet *controller = nullptr;
    std::string description;
    std::string hostname;
    std::string vendor;
    std::string model;
    std::string variant;
    std::string mode = "bridge";

    std::string version;
    int minorVersion = 0;
    int majorVersion = 0;
    int patchVersion = 0;
    int typeId = 0;
    std::string uuid;

    std::string platform;
    std::string platformModel;
    std::string ranges;
    std::string pixelControllerType;
    std::string panelSize;

    std::string ip;
    std::string proxy;
    std::string username;
    std::string password;
    bool canZipUpload = false;

    nlohmann::json extraData;
};

class Discovery
{
public:
    Discovery(OutputManager* outputManager, DiscoveryDelegate* delegate = nullptr);
    virtual ~Discovery();
    void Close(bool wait);

    // parallel HTTP requests
    void AddCurl(const std::string &host, const std::string &url, std::function<bool(int rc, const std::string &buffer, const std::string &errorBuffer)>&& callback);

    // various protocols that use broadcast or multicast requests
    void AddBroadcast(int port, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)>&& callback);
    void AddMulticast(const std::string &mcAddr, int port, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)>&& callback);

    // bonjour discovery (on platforms that support it)
    void AddBonjour(const std::string &serviceName, std::function<void(const std::string &ipAddress)>&& callback);

    void SendBroadcastData(int port, uint8_t *buffer, int len);
    void SendData(int port, const std::string &host, uint8_t *buffer, int len);

    void Discover();

    DiscoveredData *AddController(ControllerEthernet *c);

    std::vector<DiscoveredData*> &GetResults() { return results; }
    OutputManager *GetOutputManager() const { return _outputManager; }



    DiscoveredData *DetectControllerType(const std::string &ip, const std::string &proxy, const std::string &htmlBuffer);

    DiscoveredData *FindByIp(const std::string &ip, const std::string &hostname = "", bool create = false);
    DiscoveredData *FindByUUID(const std::string &uuid, const std::string &ip);
private:
    bool HandleAuth(const std::string &host, CURL* curl, int authStatus);

    class DatagramData {
    public:
        DatagramData(int port, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)> & cb);
        DatagramData(const std::string &mc, int port, std::function<void(uint8_t *buffer, int len, const std::string &fromIP)> & cb);
        ~DatagramData();

        int port;
        std::list<sockets::UDPSocket *> sockets;
        std::list<std::function<void(uint8_t *buffer, int len, const std::string &fromIP)>> callbacks;

    private:
        void Init(const std::string &mc, int port);
    };
    class CurlData {
    public:
        CurlData() {}
        ~CurlData();
        int authStatus = 0;
        std::set<std::string> urls;
    };

    OutputManager *_outputManager;
    DiscoveryDelegate _defaultDelegate;
    DiscoveryDelegate *_delegate;

    std::map<std::string, CurlData*> https;
    std::list<DatagramData *> datagrams;
    std::list<BonjourData *> bonjours;
    std::vector<DiscoveredData*> results;
    bool finished = false;

    uint64_t discoveryFinishTime;
};
