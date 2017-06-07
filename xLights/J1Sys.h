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
	wxHTTP _http;
	std::string _ip;
    std::string _version;
    bool _connected;
    std::string GetURL(const std::string& url, bool logresult = false);
    std::string PutURL(const std::string& url, const std::string& request, bool logresult = false);
    char DecodeStringPortProtocol(std::string protocol);
    int DecodeProtocolSpeed(std::string protocol);
    std::string BuildStringPort(int string, char protocol, int speed, int startChannel, int universe, int pixels, wxWindow* parent);
    void ResetStringOutputs();
    void Reboot();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;

public:
    J1Sys(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~J1Sys();
    void SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    void SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
