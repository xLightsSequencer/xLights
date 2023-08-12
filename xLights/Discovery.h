#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <functional>
#include <list>
#include <vector>
#include <map>
#include <curl/curl.h>

#include "outputs/ControllerEthernet.h"
#include "../xSchedule/wxJSON/jsonval.h"
#include <wx/secretstore.h>

class wxWindow;
class wxDatagramSocket;

class OutputManager;
class Discovery;
class BonjourData;


class WXDLLIMPEXP_CORE xlPasswordEntryDialog : public wxPasswordEntryDialog
{
public:
    xlPasswordEntryDialog(wxWindow *parent,
                      const wxString& message,
                      const wxString& caption = wxGetPasswordFromUserPromptStr,
                      const wxString& value = wxEmptyString,
                      long style = wxTextEntryDialogStyle,
                      const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, value, style, pos);
    }

    bool Create(wxWindow *parent,
                const wxString& message,
                const wxString& caption = wxGetPasswordFromUserPromptStr,
                const wxString& value = wxEmptyString,
                long style = wxTextEntryDialogStyle,
                const wxPoint& pos = wxDefaultPosition);
    
    
    bool shouldSavePassword() const {
        return savePasswordCheckbox && savePasswordCheckbox->IsChecked();
    }

    static bool GetStoredPasswordForService(const std::string &service, std::string &user, std::string &pwd);
    static bool StorePasswordForService(const std::string &service, const std::string &user, const std::string &pwd);

private:
    wxCheckBox *savePasswordCheckbox = nullptr;    
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
    
    wxJSONValue extraData;
};

class Discovery
{
public:
    Discovery(wxWindow* frame, OutputManager* outputManager);
    virtual ~Discovery();

    // parallel HTTP requests
    void AddCurl(const std::string &host, const std::string &url, std::function<bool(int rc, const std::string &buffer, const std::string &errorBuffer)>&& callback);
    
    // various protocols that use broadcast or multicast requests
    void AddBroadcast(int port, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)>&& callback);
    void AddMulticast(const std::string &mcAddr, int port, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)>&& callback);
    
    // bonjour discovery (on platforms that support it)
    void AddBonjour(const std::string &serviceName, std::function<void(const std::string &ipAddress)>&& callback);

    void SendBroadcastData(int port, uint8_t *buffer, int len);
    void SendData(int port, const std::string &host, uint8_t *buffer, int len);

    void Discover();
    
    DiscoveredData *AddController(ControllerEthernet *c);
    
    std::vector<DiscoveredData*> &GetResults() { return results; }
    wxWindow *GetFrame() const { return _frame; }
    OutputManager *GetOutputManager() const { return _outputManager; }
    
    
    
    DiscoveredData *DetectControllerType(const std::string &ip, const std::string &proxy, const std::string &htmlBuffer);
    
    DiscoveredData *FindByIp(const std::string &ip, const std::string &hostname = "", bool create = false);
    DiscoveredData *FindByUUID(const std::string &uuid, const std::string &ip);
private:
    bool HandleAuth(const std::string &host, CURL* curl);

    class DatagramData {
    public:
        DatagramData(int port, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)> & cb);
        DatagramData(const std::string &mc, int port, std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)> & cb);
        ~DatagramData();
        
        int port;
        std::list<wxDatagramSocket *> sockets;
        std::list<std::function<void(wxDatagramSocket* socket, uint8_t *buffer, int len)>> callbacks;
        
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
    wxWindow *_frame;
    
    
    std::map<std::string, CurlData*> https;
    std::list<DatagramData *> datagrams;
    std::list<BonjourData *> bonjours;
    std::vector<DiscoveredData*> results;
    bool finished = false;
};
