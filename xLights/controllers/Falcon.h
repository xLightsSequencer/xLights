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
    std::string BuildStringPort(const std::string& strings, int output, int protocol, int portstartchannel, int universe, int pixels, const std::string& description, wxWindow* parent, int mainPixels, int daughter1Pixels, int daughter2Pixels);
    int DecodeSerialOutputProtocol(std::string protocol);
    void UploadSerialOutput(int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent);
    void ResetStringOutputs();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;
    int CountStrings(const std::string& strings);
    int MaxPixels(const std::string& strings, int board);
    void InitialiseStrings(const std::string& prefix, int start, int end, int mainPixels, int daughter1Pixels, int daughter2Pixels);

public:
    Falcon(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~Falcon();
    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
