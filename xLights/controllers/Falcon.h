#ifndef FALCON_H
#define FALCON_H

#include <wx/protocol/http.h>
#include <wx/xml/xml.h>
#include <list>
#include "ControllerUploadData.h"

class Output;
class OutputManager;
class ModelManager;
class ControllerEthernet;

class FalconString
{
public:
    int virtualStringIndex;
    int protocol;
    int universe;
    int startChannel;
    int pixels;
    std::string description;
    int index;
    int port;
    float gamma;
    int groupCount;
    int smartRemote;
    int nullPixels;
    std::string colourOrder;
    std::string direction;
    int brightness;
    void Dump() const;
};

class Falcon
{
	wxHTTP _http;
    std::string _baseUrl;
    std::string _fppProxy;
	std::string _ip;
    std::string _firmwareVersion;
    bool _usingAbsolute;
    int _version;
    std::string _modelString;
    int _model;
    std::string _name;
    bool _connected;
    int GetMaxPixelPort(const std::vector<FalconString*>& stringData) const;
    std::string GetURL(const std::string& url, bool logresult = false);
    std::string PutURL(const std::string& url, const std::string& request, bool logresult = false);
    int DecodeStringPortProtocol(std::string protocol) const;
    void UploadStringPort(const std::string& request, bool final);
    void UploadStringPorts(std::vector<FalconString*>& stringData, int maxMain, int maxDaughter1, int maxDaughter2);
    void EnsureSmartStringExists(std::vector<FalconString*>& stringData, int port, int smartRemote);
    std::string BuildStringPort(FalconString* string) const;
    int DecodeSerialOutputProtocol(std::string protocol) const;
    void UploadSerialOutput(int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent);
    void ResetStringOutputs();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;
    int CountStrings(const wxXmlDocument& stringsDoc) const;
    void ReadStringData(const wxXmlDocument& stringsDoc, std::vector<FalconString*>& stringData) const;
    int MaxPixels(const wxXmlDocument& stringsDoc, int board) const;
    void InitialiseStrings(std::vector<FalconString*>& stringsData, int max) const;
    std::string SafeDescription(const std::string description) const;
    FalconString* FindPort(const std::vector<FalconString*>& stringData, int port) const;
    bool IsF48() const { return _model == 48; }
    bool IsF16() const { return _model == 16; }
    bool IsF4() const { return _model == 4; }
    bool IsV3() const { return _version == 3; }
    bool IsV2() const { return _version == 2; }
    int GetPixelCount(const std::vector<FalconString*> &stringData, int port) const;
    void DumpStringData(std::vector<FalconString*> stringData) const;
    int GetDaughter1Threshold() const { if (IsF16() || IsF48()) { return 16; } else { return 4; } }
    int GetBank1Threshold() const { if (IsF4()) return 6; else return GetDaughter1Threshold(); }
    int GetDaughter2Threshold() const { if (IsF16() || IsF48()) { return 32; } else { return 12; } }
    bool SupportsVariableExpansions() const { return IsV3() || IsEnhancedV2Firmware(); }
    bool IsEnhancedV2Firmware() const;
    int GetMaxPixels() const;
    int DecodeBrightness(int brightnessCode) const;
    float DecodeGamma(int gammaCode) const;
    std::string DecodeColourOrder(int colourOrderCode) const;
    std::string DecodeDirection(int directionCode) const;
    int EncodeBrightness(int brightness) const;
    int EncodeGamma(float gamma) const;
    int EncodeColourOrder(const std::string& colourOrder) const;
    int EncodeDirection(const std::string& direction) const;

public:
    Falcon(const std::string& ip, const std::string &proxy);
    bool IsConnected() const { return _connected; };
    virtual ~Falcon();
    bool SetInputUniverses(ControllerEthernet* controller);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent);
    static void DecodeModelVersion(int p, int& model, int& version);
    
    const std::string &GetModel() const { return _modelString; }
    const std::string &GetFirmwareVersion() const { return _firmwareVersion; }
    const std::string &GetName() const { return _name; }
};

#endif
