#ifndef FALCON_H
#define FALCON_H

#include <wx/protocol/http.h>
#include <wx/xml/xml.h>
#include <list>

class Output;
class OutputManager;
class ModelManager;

class FalconString
{
public:
    int protocol;
    int universe;
    int startChannel;
    int pixels;
    std::string description;
    int index;
    int port;
};

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
    void UploadStringPorts(const std::vector<FalconString*>& stringData, int maxMain, int maxDaughter1, int maxDaughter2, const std::vector<FalconString*>& virtualStringData);
    std::string BuildStringPort(FalconString* string) const;
    int DecodeSerialOutputProtocol(std::string protocol);
    void UploadSerialOutput(int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent);
    void ResetStringOutputs();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;
    int CountStrings(const wxXmlDocument& stringsDoc) const;
    int ReadStringData(const wxXmlDocument& stringsDoc, std::vector<FalconString*>& stringData, std::vector<FalconString*>& virtualStringData);
    int MaxPixels(const wxXmlDocument& stringsDoc, int board) const;
    void InitialiseStrings(std::vector<FalconString*>& stringsData, int max, int virtualStrings);
    std::string SafeDescription(const std::string description) const;
    FalconString* FindPort(const std::vector<FalconString*>& stringData, int port) const;
    bool IsF16() const { return (wxString(_model).StartsWith("F16")); }
    bool IsF4() const { return (wxString(_model).StartsWith("F4")); }
    bool IsV3() const { return (wxString(_model).EndsWith("V3"));}
    bool IsV2() const { return (wxString(_model).EndsWith("V2"));}
    int GetVirtualStringPixels(const std::vector<FalconString*> &virtualStringData, int port);
    int GetDaughter1Threshold() const { if (IsF16()) { return 16; } else { return 4; } }
    int GetDaughter2Threshold() const { if (IsF16()) { return 32; } else { return 12; } }
    bool SupportsVariableExpansions() const { return IsV3(); }

public:
    Falcon(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~Falcon();
    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
