#ifndef SanDevices_H
#define SanDevices_H

#include <wx/protocol/http.h>
#include <list>
#include <string>

class ModelManager;
class Output;
class OutputManager;

// I had to write this http class as wxHTTP is unable to handle the SanDevices
class SimpleHTTP : public wxHTTP
{
    bool MyBuildRequest(const wxString& path, const wxString& method, wxString& startResult);

public:
    SimpleHTTP() : wxHTTP() { }
    virtual ~SimpleHTTP() { }
    wxInputStream *GetInputStream(const wxString& path, wxString& startResult);
};


class SanDevices
{
	SimpleHTTP _http;
	std::string _ip;
    std::string _version;
    std::string _model;
    bool _connected;
    std::string GetURL(const std::string& url, bool logresult = false);
    char DecodeStringPortProtocol(std::string protocol);
    bool UploadStringPort(const std::string& page, int output, int outputsUsed, char protocol, int portstartchannel, char universe, int pixels, const std::string& description, wxWindow* parent);
    void ResetStringOutputs();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;
    int GetOutputsPerPort() const;
    std::string ExtractFromPage(const std::string page, const std::string parameter, const std::string type, int start = 0);
    char EncodeUniverse(int universe, OutputManager* outputManager, std::list<int>& selected);

public:
    SanDevices(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~SanDevices();
    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
