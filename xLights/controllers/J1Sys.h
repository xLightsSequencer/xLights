#ifndef J1SYS_H
#define J1SYS_H

#include <wx/protocol/http.h>
#include <list>
#include <string>

class ModelManager;
class Output;
class OutputManager;

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

	wxHTTP _http;
	std::string _ip = "";
    std::string _version = "";
    std::string _model = "";
    int _outputs = 0;
    bool _connected = false;
    std::string GetURL(const std::string& url, bool logresult = false);
    std::string PutURL(const std::string& url, const std::string& request, bool logresult = false);
    char DecodeStringPortProtocol(std::string protocol) const;
    char DecodeSerialPortProtocol(std::string protocol) const;
    int DecodeProtocolSpeed(std::string protocol) const;
    std::string BuildStringPort(bool active, int string, char protocol, int speed, int startChannel, int universe, int pixels, wxWindow* parent) const;
    std::string BuildSerialPort(int string, char protocol, int speed, int universe, wxWindow* parent) const;
    void ResetStringOutputs();
    void ResetSerialOutputs();
    void Reboot();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;
    void DumpConfig(const std::vector<J1SysOutput>& j) const;
    void ReadCurrentConfig(std::vector<J1SysOutput>& j);
    int GetBankSize() const;

public:
    J1Sys(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~J1Sys();
    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
