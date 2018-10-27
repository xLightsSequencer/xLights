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
    std::string _firmwareVersion;
    int _version;
    std::string _modelString;
    int _model;
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
    bool IsF48() const { return _model == 48; }
    bool IsF16() const { return _model == 16; }
    bool IsF4() const { return _model == 4; }
    bool IsV3() const { return _version == 3; }
    bool IsV2() const { return _version == 2; }
    int GetVirtualStringPixels(const std::vector<FalconString*> &virtualStringData, int port);
    int GetDaughter1Threshold() const { if (IsF16() || IsF48()) { return 16; } else { return 4; } }
    int GetDaughter2Threshold() const { if (IsF16() || IsF48()) { return 32; } else { return 12; } }
    bool SupportsVariableExpansions() const { return IsV3(); }
    int GetMaxPixels() const;

public:
    Falcon(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~Falcon();
    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
    bool SetOutputsOld(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
    static void DecodeModelVersion(int p, int& model, int& version);
};

#endif
