#ifndef FALCON_H
#define FALCON_H

#include <wx/protocol/http.h>
#include <wx/xml/xml.h>
#include <list>
#include "ControllerUploadData.h"

class Output;
class OutputManager;
class ModelManager;

class FalconControllerRules : public ControllerRules
{
    int _type;
    int _version;
    int _expansions;

public:
    FalconControllerRules(int type, int version) : ControllerRules()
    {
        _type = type;
        _version = version;
        _expansions = 0;
    }
    FalconControllerRules(int model) : ControllerRules()
    {
        _type = 16;
        _version = 3;
        _expansions = 0;

        switch (model)
        {
        case 1:
        case 3:
            _type = 16;
            _version = 2;
            break;
        case 4:
            _type = 4;
            _version = 2;
            break;
        case 5:
            _type = 16;
            _version = 3;
            break;
        case 6:
            _type = 4;
            _version = 3;
            break;
        case 7:
            _type = 48;
            _version = 3;
            break;
        default:
            wxASSERT(false);
            break;
        }
    }
    virtual ~FalconControllerRules() {}
    virtual int GetMaxPixelPortChannels() const override
    {
        if (_version == 2)
        {
            return 680 * 3;
        }
        else
        {
            return 1024 * 3;
        }
    }
    virtual int GetMaxPixelPort() const override
    {
        if (_type == 4)
        {
            return 12;
        }
        else if (_type == 16)
        {
            return 48;
        }
        else if (_type == 48)
        {
            return 48;
        }

        return 48;
    }
    virtual int GetMaxSerialPortChannels() const override
    {
        return 512;
    }
    virtual int GetMaxSerialPort() const override
    {
        if (_type == 4)
        {
            return 1;
        }
        else
        {
            return 4;
        }
    }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        if (protocol == "ws2811") return true;
        if (protocol == "tm18xx") return true;
        if (protocol == "lx1203") return true;
        if (protocol == "ws2801") return true;
        if (protocol == "tls3001") return true;
        if (protocol == "lpd6803") return true;
        if (protocol == "gece") return true;

        return false;
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        if (protocol == "dmx") return true;
        if (protocol == "pixelnet") return true;
        if (protocol == "renard") return true;

        return false;
    }
    virtual bool SupportsMultipleProtocols() const override
    {
        return true;
    }
    virtual bool AllUniversesSameSize() const override
    {
        return false;
    }
    virtual std::list<std::string> GetSupportedInputProtocols() const override
    {
        std::list<std::string> res;
        res.push_back("E131");
        res.push_back("ARTNET");
        res.push_back("ZCPP");
        return res;
    }
    virtual bool UniversesMustBeSequential() const override
    {
        return false;
    }
};

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
    int GetMaxPixels() const { if (IsV2()) return 680; else return 1024; }

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
