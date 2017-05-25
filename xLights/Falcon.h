#ifndef FALCON_H
#define FALCON_H

#include <wx/protocol/http.h>
#include <list>

class Output;
class OutputManager;
class ModelManager;

class Falcon
{
	wxHTTP _http;
	std::string _ip;
    std::string _version;
    std::string _model;
    bool _connected;
    std::string GetURL(const std::string& url, bool logresult = false);
    std::string PutURL(const std::string& url, const std::string& request, bool logresult = false);
    int DecodeStringPortProtocol(std::string protocol);
    void UploadStringPort(const std::string& request, bool final);
    std::string BuildStringPort(const std::string& strings, int output, int protocol, int portstartchannel, int universe, int pixels, const std::string& description, wxWindow* parent);
    int DecodeSerialOutputProtocol(std::string protocol);
    void UploadSerialOutput(int output, int protocol, int portstart, wxWindow* parent);
    void ResetStringOutputs();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;

public:
    Falcon(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~Falcon();
    void SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    void SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
