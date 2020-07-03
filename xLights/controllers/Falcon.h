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

class FalconString;

class Falcon : public BaseController
{
    #pragma region Member Variables
    std::string _firmwareVersion;
    bool _usingAbsolute = false;
    int _versionnum = -1;
    std::string _modelString;
    int _modelnum = -1;
    std::string _name;
    #pragma endregion

    #pragma region Private Functions

    #pragma region Strings.xml Handling
    int CountStrings(const wxXmlDocument& stringsDoc) const;
    void ReadStringData(const wxXmlDocument& stringsDoc, std::vector<FalconString*>& stringData) const;
    int MaxPixels(const wxXmlDocument& stringsDoc, int board) const;
    #pragma endregion

    #pragma region FalconString Handling
    void InitialiseStrings(std::vector<FalconString*>& stringsData, int max, int minuniverse) const;
    std::string BuildStringPort(FalconString* string) const;
    FalconString* FindPort(const std::vector<FalconString*>& stringData, int port) const;
    int GetPixelCount(const std::vector<FalconString*>& stringData, int port) const;
    int GetMaxPixelPort(const std::vector<FalconString*>& stringData) const;
    void EnsureSmartStringExists(std::vector<FalconString*>& stringData, int port, int smartRemote, int minuniverse);
    void RemoveNonSmartRemote(std::vector<FalconString*>& stringData, int port);
    void DumpStringData(std::vector<FalconString*> stringData) const;
    #pragma endregion

    #pragma region Port Handling
    void ResetStringOutputs();
    void UploadStringPort(const std::string& request, bool final);
    void UploadStringPorts(std::vector<FalconString*>& stringData, int maxMain, int maxDaughter1, int maxDaughter2, int minuniverse);
    std::string GetSerialOutputURI(ControllerCaps* caps, int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent);
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
    bool IsV3() const { return _versionnum == 3; }
    bool IsV2() const { return _versionnum == 2; }

    int GetDaughter1Threshold() const { if (IsF16() || IsF48()) { return 16; } else { return 4; } }
    int GetBank1Threshold() const { if (IsF4()) return 6; else return GetDaughter1Threshold(); }
    int GetDaughter2Threshold() const { if (IsF16() || IsF48()) { return 32; } else { return 12; } }
    
    bool SupportsVariableExpansions() const { return IsV3() || IsEnhancedV2Firmware(); }
    
    bool IsEnhancedV2Firmware() const;
    
    int GetMaxPixels() const;     
    #pragma endregion

    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent, bool progress);

public:
    #pragma region Constructors and Destructors
    Falcon(const std::string& ip, const std::string &proxy);
    virtual ~Falcon() {}
    #pragma endregion

    #pragma region Static Functions
    static void DecodeModelVersion(int p, int& model, int& version);
    #pragma endregion

    #pragma region Getters and Setters
    virtual bool SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;

    
    std::string GetName() const { return _name; }
    virtual bool UsesHTTP() const override { return true; }
    #pragma endregion
};

