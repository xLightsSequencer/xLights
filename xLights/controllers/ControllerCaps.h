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

#include <list>
#include <vector>
#include <map>
#include <string>

#include <wx/xml/xml.h>

class Controller;
class BaseController;

class ControllerCaps
{
    #pragma region Member Variables
    std::string _vendor;
    std::string _model;
    wxXmlNode* _config = nullptr;
    #pragma endregion

    #pragma region Static Variables
    static std::map<std::string, std::map<std::string, std::list<ControllerCaps*>>> __controllers;
    #pragma endregion

    bool SupportsPixelPortCommonSettings() const;

public:

    #pragma region Constructors and Destructors
    ControllerCaps(const std::string& v, const std::string& m, wxXmlNode* n) : _vendor(v), _model(m) { _config = new wxXmlNode(*n); }
    virtual ~ControllerCaps() { if (_config != nullptr) { delete _config; } }
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    static void LoadControllers();
    static void UnloadControllers();

    static std::list<std::string> GetVendors(const std::string& type);
    static std::list<std::string> GetModels(const std::string& type, const std::string& vendor);
    static std::list<std::string> GetVariants(const std::string& type, const std::string& vendor, const std::string& model);

    static ControllerCaps* GetControllerConfig(const std::string& vendor, const std::string& model, const std::string& variant);
    static ControllerCaps* GetControllerConfig(const Controller* const controller);
    static ControllerCaps* GetControllerConfigByID(const std::string& ID);
    static ControllerCaps* GetControllerConfigByModel(const std::string& model, const std::string& variant);
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    bool SupportsUpload() const;
    bool SupportsFullxLightsControl() const;
    bool SupportsInputOnlyUpload() const;
    bool NeedsDDPInputUpload() const;
    bool SupportsLEDPanelMatrix() const;
    bool SupportsVirtualMatrix() const;
    bool SupportsVirtualStrings() const;
    bool SupportsSmartRemotes() const;
    bool SupportsRemotes() const;
    bool SupportsAutoLayout() const;
    bool SupportsAutoUpload() const;
    bool SupportsUniversePerString() const;
    bool SupportsMultipleSimultaneousOutputProtocols() const;
    bool SupportsMultipleSimultaneousInputProtocols() const;
    bool MergeConsecutiveVirtualStrings() const;
    bool AllInputUniversesMustBeSameSize() const;
    bool AllInputUniversesMustBe510() const;
    bool UniversesMustBeInNumericalOrder() const;
    bool UniversesMustBeSequential() const;
    bool NoWebUI() const;
    bool SupportsPixelPortBrightness() const;
    bool SupportsPixelPortGamma() const;
    bool SupportsDefaultGamma() const;
    bool SupportsDefaultBrightness() const;
    bool SupportsPixelPortNullPixels() const;
    bool SupportsPixelPortEndNullPixels() const;
    bool SupportsPixelPortGrouping() const;
    bool SupportsPixelZigZag() const;
    bool SupportsTs() const;
    bool SupportsPixelPortDirection() const;
    bool SupportsPixelPortColourOrder() const;
    bool SupportsEthernetInputProtols() const;
    bool SupportsSerialInputProtols() const;
    bool IsPlayerOnly() const;
    bool NeedsFullUniverseForDMX() const;
    bool AllSmartRemoteTypesPerPortMustBeSame() const;
    bool DMXAfterPixels() const;

    int GetSmartRemoteCount() const;
    int GetMaxInputE131Universes() const;
    int GetMaxPixelPort() const;
    int GetMaxSerialPort() const;
    int GetMaxVirtualMatrixPort() const;
    int GetMaxLEDPanelMatrixPort() const;
    int GetMaxPixelPortChannels() const;
    int GetMaxSerialPortChannels() const;
    int GetMaxInputUniverseChannels() const;
    int GetMinInputUniverseChannels() const;
    int GetNumberOfBanks() const;
    int GetBankSize() const;
    int GetMaxStartNullPixels() const;
    int GetMaxEndNullPixels() const;
    int GetMaxGroupPixels() const;
    int GetMinGroupPixels() const;
    int GetMaxZigZagPixels() const;

    bool IsValidPixelProtocol(const std::string& protocol) const;
    bool IsValidSerialProtocol(const std::string& protocol) const;
    bool IsValidInputProtocol(const std::string& protocol) const;
    bool IsSerialController() const;

    const std::string &GetVendor() const { return _vendor; }
    const std::string &GetModel() const { return _model; }

    std::string GetVariantName() const;
    std::string GetID() const;

    std::string GetPreferredInputProtocol() const;

    std::vector<std::string> GetInputProtocols() const;
    std::vector<std::string> GetPixelProtocols() const;
    std::vector<std::string> GetSerialProtocols() const;
    std::vector<std::string> GetAllProtocols() const;
    std::vector<std::string> GetSmartRemoteTypes() const;

    std::string GetCustomPropertyByPath(const std::string path, const std::string& def = "") const;
    
    
    std::string GetConfigDriver() const;

    void Dump() const;
    #pragma endregion
};
