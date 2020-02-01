#pragma once

#include <list>
#include <map>
#include <string>

#include <wx/xml/xml.h>

class Controller;

class ControllerCaps
{
    std::string _vendor;
    std::string _model;
    std::string _firmware;
    wxXmlNode* _config = nullptr;

#pragma region Member Variables
    static std::map<std::string, std::map<std::string, std::map<std::string, ControllerCaps*>>> __controllers;
#pragma endregion

public:

    #pragma region Constructors and Destructors
    ControllerCaps(wxXmlNode* n) { _config = new wxXmlNode(*n); }
    virtual ~ControllerCaps() { if (_config != nullptr) { delete _config; } }
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    static void LoadControllers();
    static void UnloadControllers();
    static std::list<std::string> GetVendors();
    static std::list<std::string> GetModels(const std::string& vendor);
    static std::list<std::string> GetFirmwareVersions(const std::string& vendor, const std::string& model);
    static ControllerCaps* GetControllerConfig(const std::string& vendor, const std::string& model, const std::string& firmwareVersion);
    static ControllerCaps* GetControllerConfig(const Controller* const controller);
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    bool SupportsUpload() const;
    bool SupportsInputOnlyUpload() const;
    int GetMaxInputE131Universes() const;
    int GetMaxPixelPort() const;
    int GetMaxSerialPort() const;
    bool SupportsLEDPanelMatrix() const;
    int GetMaxPixelPortChannels() const;
    int GetMaxSerialPortChannels() const;
    int GetMaxInputUniverseChannels() const;
    bool SupportsVirtualStrings() const;
    bool SupportsSmartRemotes() const;
    bool IsValidPixelProtocol(const std::string& protocol) const;
    bool IsValidSerialProtocol(const std::string& protocol) const;
    bool SupportsMultipleSimultaneousOutputProtocols() const;
    bool SupportsMultipleSimultaneousInputProtocols() const;
    bool MergeConsecutiveVirtualStrings() const;
    bool AllInputUniversesMustBeSameSize() const;
    bool IsValidInputProtocol(const std::string& protocol) const;
    std::list<std::string> GetInputProtocols() const;
    std::list<std::string> GetPixelProtocols() const;
    std::list<std::string> GetSerialProtocols() const;
    bool UniversesMustBeInNumericalOrder() const;
    bool UniversesMustBeSequential() const;
    std::string GetCustomPropertyByPath(const std::string path, const std::string& default = "") const;
    void Dump() const;
    #pragma endregion 
};
