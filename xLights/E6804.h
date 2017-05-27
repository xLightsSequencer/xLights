#ifndef E6804_H
#define E6804_H

#include <wx/protocol/http.h>
#include <list>
#include <string>

class ModelManager;
class Output;
class OutputManager;

// I had to write this http class as wxHTTP is unable to handle the E6804
class SimpleHTTP : public wxHTTP
{
    bool MyBuildRequest(const wxString& path, const wxString& method, wxString& startResult);

public:
    SimpleHTTP() : wxHTTP() { }
    virtual ~SimpleHTTP() { }
    wxInputStream *GetInputStream(const wxString& path, wxString& startResult);
};


class E6804
{
	SimpleHTTP _http;
	std::string _ip;
    std::string _version;
    bool _connected;
    std::string GetURL(const std::string& url, bool logresult = false);
    char DecodeStringPortProtocol(std::string protocol);
    void UploadStringPort(const std::string& page, int output, char protocol, int portstartchannel, char universe, int pixels, const std::string& description, wxWindow* parent);
    void ResetStringOutputs();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;
    std::string ExtractFromPage(const std::string page, const std::string parameter, const std::string type, int start = 0);
    char EncodeUniverse(int universe, OutputManager* outputManager, std::list<int>& selected);

public:
    E6804(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~E6804();
    void SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    void SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
