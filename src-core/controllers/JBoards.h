#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "BaseController.h"
#include "ControllerUploadData.h"

class Discovery;
class DiscoveredData;
class FPP;
class OutputManager;

class JBoards : public BaseController
{
    #pragma region Member Variables
    std::vector<int> _chainBases;
    int _chainSlots = 6;
    int _serialPorts = 0;
    int _pixelPorts = 0;
    bool _audio = false;
    int64_t _sequenceStartChannel = 0;
    #pragma endregion

    #pragma region Private Functions
    bool PostJSON(const std::string& url, const nlohmann::json& body, nlohmann::json& response);
    bool PutJSON(const std::string& url, const nlohmann::json& body, nlohmann::json& response);
    static std::string SkippedText(const nlohmann::json& response);
    [[nodiscard]] bool InChain(int port) const;
#ifndef DISCOVERYONLY
    bool CheckBoard(Controller* controller, ControllerCaps*& caps, std::string& error) const;
    static nlohmann::json StringJson(const UDVirtualString& vs,
                                     bool fullControl, int defaultBrightness);
    bool UploadChains(UDController& cud, Controller* controller, bool fullControl,
                      std::set<int>& chainPorts, std::string& error);
    bool UploadPixelPorts(UDController& cud, ControllerCaps* caps, Controller* controller,
                          bool fullControl, const std::set<int>& chainPorts,
                          nlohmann::json& sent, std::string& warning, std::string& error);
    bool UploadSerialPorts(UDController& cud, bool fullControl, std::string& error);
    std::string ReadBackDifferences(const nlohmann::json& sent);
    static bool InputConfig(Controller* controller, nlohmann::json& cfg, std::string& error);
    bool PostInputs(const nlohmann::json& cfg, std::string& error);
    static bool UploadFile(const std::string& ip, const std::string& proxy, const std::string& path,
                           const std::string& name, const std::string& dir,
                           const std::function<bool(int, std::string)>& progress, std::string& error);
    static bool SendSequence(const std::string& ip, const std::string& proxy, const std::string& seq,
                             const std::string& file, const std::string& media,
                             const std::function<bool(int, std::string)>& progress, std::string& error);
#endif
    static void ProcessDiscoveryInfo(Discovery& discovery, const std::string& ip, const std::string& body);
    static void AdoptDiscoveredBoard(Discovery& discovery, DiscoveredData* cd);
    #pragma endregion

public:
    #pragma region Constructors and Destructors
    JBoards(const std::string& ip, const std::string& fppProxy);
    virtual ~JBoards() override {}
    #pragma endregion

    #pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetInputUniverses(Controller* controller, UICallbacks* ui) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) override;
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) override;
#endif
    [[nodiscard]] virtual bool UsesHTTP() const override { return true; }
    #pragma endregion

    #pragma region Sequence Upload
#ifndef DISCOVERYONLY
    static bool UploadSequence(const std::string& ip, const std::string& proxy, const std::string& seq,
                               const std::string& file, const std::string& media,
                               std::function<bool(int, std::string)> progress, std::string& error);
    static void PrepareSequenceUpload(FPP* inst, OutputManager* outputManager);
#endif
    [[nodiscard]] static bool PlaysAudio(const std::string& ip);
    [[nodiscard]] static bool PrefersUncompressedFSEQ(const std::string& ip);
    #pragma endregion

    #pragma region Static Functions
    static void PrepareDiscovery(Discovery& discovery);
    static void ProcessFPPDiscovery(Discovery& discovery, DiscoveredData* cd);
    #pragma endregion
};
