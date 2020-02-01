#ifndef J1SYS_H
#define J1SYS_H

#include <wx/protocol/http.h>
#include <list>
#include <string>

class ModelManager;
class Output;
class OutputManager;
class ControllerEthernet;

class J1Sys
{
    struct J1SysOutput
    {
        char port;
        bool active;
        char protocol;
        int speed;
        int universe;
        int startChannel;
        int pixels;
    };

    struct J1SysSerialOutput
    {
        char port;
        bool active;
        char protocol;
        int speed;
        int universe;
    };

    wxHTTP _http;
	std::string _ip = "";
    std::string _proxy = "";
    std::string _baseUrl = "";
    std::string _version = "";
    std::string _model = "";
    int _outputs = 0;
    bool _connected = false;
    std::string GetURL(const std::string& url, bool logresult = false);
    std::string PutURL(const std::string& url, const std::string& request, bool logresult = false);
    char EncodeStringPortProtocol(std::string protocol) const;
    char EncodeSerialPortProtocol(std::string protocol) const;
    int DecodeProtocolSpeed(std::string protocol) const;
    std::string BuildStringPort(bool active, int string, char protocol, int speed, int startChannel, int universe, int pixels, wxWindow* parent) const;
    std::string BuildSerialPort(bool active, int port, char protocol, int speed, int universe, wxWindow* parent) const;
    void ResetStringOutputs();
    void ResetSerialOutputs();
    void Reboot();
    void DumpConfig(const std::vector<J1SysOutput>& j) const;
    void DumpConfig(const std::vector<J1SysSerialOutput>& j) const;
    void ReadCurrentConfig(std::vector<J1SysOutput>& j);
    void ReadCurrentSerialConfig(std::vector<J1SysSerialOutput>& j);
    int GetBankSize() const;

public:
    J1Sys(const std::string& ip, const std::string &proxy);
    bool IsConnected() const { return _connected; };
    virtual ~J1Sys();
    bool SetInputUniverses(ControllerEthernet* controller, OutputManager* outputManager);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent);
    
    const std::string &GetVersion() const { return _version; }
    const std::string &GetModel() const { return _model; }
    std::string GetPixelControllerTypeString() const;
};

#endif
