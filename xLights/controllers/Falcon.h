#pragma once 

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>

#include <list>

#include "BaseController.h"
#include "ControllerUploadData.h"
#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

class FalconString;
class wxProgressDialog;

class Falcon : public BaseController
{
    #pragma region Member Variables
    std::string _firmwareVersion;
    int _majorFirmwareVersion;
    int _minorFirmwareVersion;
    bool _usingAbsolute = false;
    int _versionnum = -1;
    std::string _modelString;
    int _modelnum = -1;
    wxJSONValue _v4status;
    wxJSONValue _status;
    std::string _name;
    #pragma endregion

#pragma region V4
    
    typedef struct FALCON_V4_INPUTS
    {
        int universe;
        int channels;
        int universeCount;
        int protocol;
    } FALCON_V4_INPUTS;

    typedef struct FALCON_V4_STRING
    {
        int port; // p
        int string; // s
        int smartRemote; // r
        int universe; // u
        std::string name; // nm
        bool blank; // bl
        int gamma; // g
        int brightness; // b
        int zigcount; // z
        int endNulls; // ne
        int startNulls; // ns
        int colourOrder; // o
        int direction; // v
        int group; // gp
        int pixels; // n
        int protocol; // l
        unsigned long startChannel; // sc
    } FALCON_V4_STRING;

    bool V4_IsPortSmartRemoteEnabled(int boardMode, int port);
    int V4_GetBoardPorts(int boardMode);
    int V4_GetMaxPortPixels(int boardMode, int protocol);
    int V4_EncodeInputProtocol(const std::string& protocol);
    void V4_GetStartChannel(int modelUniverse, int modelUniverseStartChannel, unsigned long modelStartChannel, int& universe, unsigned long& startChannel, bool oneBased, uint32_t controllerFirstChannel);
    int V4_EncodeColourOrder(const std::string co) const;
    int V4_GetStringFirstIndex(const std::vector<FALCON_V4_STRING>& falconStrings, const int p, const int sr);
    std::string V4_DecodePixelProtocol(int protocol);
    int V4_EncodePixelProtocol(const std::string& protocol);
    void V4_DumpStrings(const std::vector<FALCON_V4_STRING>& str);
    int V4_GetRebootSecs();
    void V4_WaitForReboot(const std::string& name, wxWindow* parent);
    std::string SendToFalconV4(std::string msg);
    std::vector<std::string> V4_GetMediaFiles();
    bool V4_IsFileUploading();
    int V4_GetConversionProgress();
    int CallFalconV4API(const std::string& type, const std::string& method, int inbatch, int expected, int index, const wxJSONValue& params, bool& finalCall, int& outbatch, bool& reboot, wxJSONValue& result);
    bool V4_GetInputs(std::vector<FALCON_V4_INPUTS>& res);
    bool V4_SendInputs(std::vector<FALCON_V4_INPUTS>& res, bool& reboot);
    bool V4_SendBoardMode(int boardMode, int controllerMode, unsigned long startChannel, bool& reboot);
    bool V4_SetSerialConfig(int protocol, int universe, int startChannel, int rate);
    bool V4_GetStrings(std::vector<FALCON_V4_STRING>& res);
    bool V4_IsValidStartChannel(Controller* controller, int universe, long startChannel);
    bool V4_SendOutputs(std::vector<FALCON_V4_STRING>& res, int addressingMode, unsigned long startChannel, bool& reboot);
    bool V4_GetStatus(wxJSONValue& res);
    bool V4_SetInputUniverses(Controller* controller, wxWindow* parent);
    bool V4_SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent, bool doProgress);
    int V4_ValidBrightness(int b) const;
    int V4_ValidGamma(int g) const;
    bool V4_SetInputMode(Controller* controller, wxWindow* parent);
    bool V4_ValidateWAV(const std::string& media);

#ifndef DISCOVERYONLY
    bool V4_PopulateStrings(std::vector<FALCON_V4_STRING>& uploadStrings, const std::vector<FALCON_V4_STRING>& falconStrings, UDController& cud, ControllerCaps* caps, int defaultBrightness, std::string& error, bool oneBased, uint32_t firstControllerChannel);
    void V4_MakeStringsValid(Controller* controlle, UDController& cud, std::vector<FALCON_V4_STRING>& falconStrings, int addressingMode);
#endif

#pragma endregion

    #pragma region Private Functions

    bool IsFirmwareEqualOrGreaterThan(int major, int minor)
    {
        return _majorFirmwareVersion > major || (_majorFirmwareVersion == major && _minorFirmwareVersion >= minor);
    }
    bool ValidateBoard(Controller* controller);

    #pragma region Strings.xml Handling
    int CountStrings(const wxXmlDocument& stringsDoc) const;
    void ReadStringData(const wxXmlDocument& stringsDoc, std::vector<FalconString*>& stringData, int defaultBrightness) const;
    int MaxPixels(const wxXmlDocument& stringsDoc, int board) const;
    #pragma endregion

    #pragma region FalconString Handling
    void InitialiseStrings(std::vector<FalconString*>& stringsData, int max, int minuniverse, int defaultBrightness, int32_t firstchannel) const;
    std::string BuildStringPort(FalconString* string) const;
    FalconString* FindPort(const std::vector<FalconString*>& stringData, int port) const;
    int GetPixelCount(const std::vector<FalconString*>& stringData, int port) const;
    int GetMaxPixelPort(const std::vector<FalconString*>& stringData) const;
    void EnsureSmartStringExists(std::vector<FalconString*>& stringData, int port, int smartRemote, int minuniverse, int defaultBrightness, int32_t firstchannel);
    void RemoveNonSmartRemote(std::vector<FalconString*>& stringData, int port);
    void DumpStringData(std::vector<FalconString*> stringData) const;
    #pragma endregion

    #pragma region Port Handling
#ifndef DISCOVERYONLY
    void ResetStringOutputs();
    void UploadStringPort(const std::string& request, bool final);
    void UploadStringPorts(std::vector<FalconString*>& stringData, int maxMain, int maxDaughter1, int maxDaughter2, int minuniverse, int defaultBrightness, int32_t firstchannel);
    std::string GetSerialOutputURI(ControllerCaps* caps, int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent);
#endif
    #pragma endregion

    #pragma region Encode and Decode
    int DecodeSerialOutputProtocol(std::string protocol) const;
    int DecodeStringPortProtocol(std::string protocol) const;
    int DecodeBrightness(int brightnessCode) const;
    int EncodeBrightness(int brightness) const;
    float DecodeGamma(int gammaCode) const;
    int EncodeGamma(float gamma) const;
    std::string DecodeColourOrder(int colourOrderCode) const;
    int EncodeColourOrder(const std::string& colourOrder) const;
    std::string DecodeDirection(int directionCode) const;
    int EncodeDirection(const std::string& direction) const;
    #pragma endregion

    std::string SafeDescription(const std::string description) const;

    bool IsF48() const { return _modelnum == 48; }
    bool IsF16() const { return _modelnum == 16; }
    bool IsF4() const { return _modelnum == 4; }
    bool IsV4() const { return _versionnum == 4; }
    bool IsV3() const { return _versionnum == 3; }
    bool IsV2() const { return _versionnum == 2; }

    int GetDaughter1Threshold() const { if (IsF16() || IsF48()) { return 16; } else { return 4; } }
    int GetBank1Threshold() const { if (IsF4()) return 6; else return GetDaughter1Threshold(); }
    int GetDaughter2Threshold() const { if (IsF16() || IsF48()) { return 32; } else { return 12; } }
    
    bool SupportsVariableExpansions() const { return IsV3() || IsEnhancedV2Firmware(); }
    
    bool IsEnhancedV2Firmware() const;
    
    int GetMaxPixels() const;     
    #pragma endregion

#ifndef DISCOVERYONLY
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent, bool progress);
#endif

public:
    #pragma region Constructors and Destructors
    Falcon(const std::string& ip, const std::string &proxy);
    virtual ~Falcon() {}
    #pragma endregion

    #pragma region Static Functions
    static void DecodeModelVersion(int p, int& model, int& version);
    static std::string DecodeMode(int mode);
    #pragma endregion

    #pragma region Getters and Setters
    std::string GetMode();
    wxJSONValue V4_GetStatus() const { return _v4status; }
    std::string V4_DecodeBoardConfiguration(int config) const;
    std::string V4_DecodeMode(int mode) const;
    #ifndef DISCOVERYONLY
    virtual bool SetInputUniverses(Controller* controller, wxWindow* parent) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
    bool UploadSequence(const std::string& seq, const std::string& file, const std::string& media, std::function<bool(int, std::string)> progress);
    #endif
    
    std::string GetName() const { return _name; }
    virtual bool UsesHTTP() const override { return true; }
    #pragma endregion
    
    int NumConfiguredStrings();
    
};

