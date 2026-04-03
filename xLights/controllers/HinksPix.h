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

#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "BaseController.h"
#include "ControllerUploadData.h"
//#include "UtilClasses.h"

#include <curl/curl.h>

class HinksPix;
class ControllerEthernet;

struct HinksPixOutput {
    HinksPixOutput(int output_ ,int defaultBrightness_) :
        output(output_),
        brightness(defaultBrightness_) {};
    const int output;
    int universe{ 1 };
    int startChannel{ 1 };
    int pixels{ 0 };
    int direction{ 0 };
    int protocol{ 0 };
    int nullPixel{ 0 };
    int colorOrder{ 0 };
    int brightness{100};
    int gamma{ 1 };
    bool used{ false };

    [[nodiscard]] int getControllerStartChannel() const { return controllerStartChannel; }
    [[nodiscard]] int getControllerEndChannel() const { return controllerEndChannel; }
    void setControllerChannels(const int startChan);

    void Dump() const;
    void SetConfig(const std::string& data);
    [[nodiscard]] nlohmann::json BuildCommand() const;
    [[nodiscard]] std::string BuildCommandEasyLights() const;

private:
    int controllerStartChannel{ 1 };
    int controllerEndChannel{ 0 };
};

struct HinksPixSerial {
    HinksPixSerial() = default;
    int e131Universe{ 1 };
    int e131StartChannel{ 1 };
    int e131NumOfChan{ 512 };
    bool e131Enabled{ false };
    int ddpDMXStartChannel{ 1 };
    int ddpDMXNumOfChan{ 512 };
    bool ddpDMXEnabled{ false };
    bool upload{ false };
    void Dump() const;
    void SetConfig(nlohmann::json const& data);

    [[nodiscard]] nlohmann::json BuildCommand() const;
    [[nodiscard]] std::string BuildCommandEasyLights(int mode) const;
};

struct HinksSmartOutput {
    HinksSmartOutput(int id_) :
        id(id_) {};
    int id{ 0 };
    int type{ 0 };
    std::array<int, 4> portStartPixel{ 0, 0, 0, 0 };

    bool operator==(const HinksSmartOutput& rhs) const {
        return id == rhs.id;
    }

    void Dump() const;
    void SetConfig(const std::string& data);
    [[nodiscard]] nlohmann::json BuildCommand() const;
};

struct HinksPixInputUniverse {
    HinksPixInputUniverse(int universe_, int numOfChan_) :
        universe(universe_),
        numOfChan(numOfChan_){};

    int index{ -1 };
    int universe{ 1 };
    int numOfChan{ 0 };
    int32_t hinksPixStartChannel{ 1 };
    bool operator==(const HinksPixInputUniverse& rhs) const {
        return universe == rhs.universe;
    }
    void Dump() const;
    [[nodiscard]] nlohmann::json BuildCommand() const;
    [[nodiscard]] std::string BuildCommandEasyLights() const;
};

struct HinksPixFileData {
    std::string FileName;
    uint16_t Date;
    uint16_t Time;
};

class UnPack
{
public:
    int32_t MyStart;
    int32_t MyEnd;
    int32_t NewStart;
    int32_t NewEnd;
    int32_t NumChans;
    int32_t Port;
    bool InActive{ false };
};

class HinksPix : public BaseController
{
    static constexpr int UN_PER = 6;
    static constexpr int OUT_SIZE = 16;
    static constexpr int EXP_PORTS = 5;
    static constexpr int REC_SIZE = 4;

    enum class EXPType {
        Not_Present,
        Local_SPI,
        Long_Range,
        Local_AC
    };

#pragma region Member Variables
    EXPType _EXP_Outputs[EXP_PORTS]{ EXPType::Not_Present, EXPType::Not_Present, EXPType::Not_Present, EXPType::Not_Present, EXPType::Not_Present };
    std::string _controllerType;
    CURL* _curl { nullptr };
    int _numberOfUniverses{ 0 };
    int _MCPU_Version{ 0 };
    bool _hardwareV3{false};

    std::vector<HinksPixOutput> _pixelOutputs;
    std::unique_ptr<HinksPixSerial> _serialOutput;
    std::vector<HinksSmartOutput> _smartOutputs[EXP_PORTS][REC_SIZE];
#pragma endregion

#pragma region Encode and Decode
    [[nodiscard]] int EncodeColorOrder(const std::string& colorOrder) const;
    [[nodiscard]] int EncodeDirection(const std::string& direction) const;
    [[nodiscard]] int EncodeStringPortProtocol(const std::string& protocol) const;
    [[nodiscard]] int EncodeBrightness(int brightness) const;
    [[nodiscard]] int EncodeGamma(int gamma) const;
    [[nodiscard]] EXPType DecodeExpansionType(const std::string& type) const;
#pragma endregion

#pragma region Private Functions
    bool InitControllerOutputData(bool fullControl, int defaultBrightness);
    void InitExpansionBoardData(int expansion, int startport, int length);
    [[nodiscard]] std::unique_ptr<HinksPixSerial> InitSerialData(bool fullControl);

    [[nodiscard]] bool UploadInputUniverses(Controller* controller, std::vector<HinksPixInputUniverse> const& inputUniverses) const;
    [[nodiscard]] bool UploadUnPack(bool& worked, std::vector<std::unique_ptr<UnPack>> const& UPA, bool dirty) const;
    
    [[nodiscard]] bool UploadInputUniversesEasyLights(Controller* controller, std::vector<HinksPixInputUniverse> const& inputUniverses) const;
    void UploadPixelOutputsEasyLights(bool& worked);
    [[nodiscard]] std::string GetControllerE131Data(int rowIndex) const;
    [[nodiscard]] std::string GetControllerData(int rowIndex, std::string const& data = std::string()) const;
    [[nodiscard]] std::string GetControllerRowData(int rowIndex, std::string const& url, std::string const& data) const;
    [[nodiscard]] std::map<std::string, std::string> StringToMap(std::string const& text) const;

    void UpdatePortData(HinksPixOutput& pd, UDControllerPort* stringData, int32_t hinkstartChan) const;
    void UpdateSerialData(HinksPixSerial& pd, UDControllerPort* serialData, int const mode, std::vector<HinksPixInputUniverse>& inputUniverses, int32_t& hinkstartChan, int& index, bool individualUniverse) const;
    void UploadPixelOutputs(bool& worked) const;
    void UpdateUniverseControllerChannels(UDControllerPort* stringData, std::vector<HinksPixInputUniverse>& inputUniverses, int32_t& hinkstartChan, int& index, bool individualUniverse);
    void UploadExpansionBoardData(int expansion, int startport, int length, bool& worked) const;
    void UploadSmartReceivers(bool& worked) const;
    void UploadSmartReceiverData(int expan, int bank, std::vector<HinksSmartOutput> const& receivers, bool& worked) const;
    void CalculateSmartReceivers(UDControllerPort* stringData);

    [[nodiscard]] std::string GetJSONControllerData(std::string const& url, std::string const& data) const;
    [[nodiscard]] bool GetControllerDataJSON(const std::string& url, nlohmann::json& val, std::string const& data) const;
    void PostToControllerNoResponse(std::string const& url, std::string const& data) const;
    [[nodiscard]] bool CheckPixelOutputs(std::string& message);
    [[nodiscard]] bool CheckSmartReceivers(std::string& message);

    static const std::string GetJSONPostURL() { return "/Xlights_PostData.cgi"; };
    static const std::string GetJSONInfoURL() { return "/XLights_BoardInfo.cgi"; };
    static const std::string GetJSONPortURL() { return "/Xlights_Board_Port_Config.cgi"; };
    static const std::string GetJSONUnPackURL() { return "/Xlights_UnPack_Config.cgi"; };
    static const std::string GetJSONModeURL() { return "/Xlights_Data_Mode.cgi"; };
    static const std::string GetE131URL() { return"/GetE131Data.cgi"; };
    static const std::string GetInfoURL() { return"/GetInfo.cgi"; };

#pragma endregion

public:
    [[nodiscard]] bool IsUnPackSupported_Hinks(ControllerEthernet* controller) const;

#pragma region Constructors and Destructors
    HinksPix(const std::string& ip, const std::string& fppProxy);
    virtual ~HinksPix();
#pragma endregion

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
    bool UploadFileToController(const std::string& localpathname, const std::string& remotepathname, std::function<bool(int, int, std::string)> progress_dlg, const std::chrono::system_clock::time_point& fileTime) const;
    bool UploadTimeToController() const;
    bool UploadModeToController(unsigned char mode) const;
    [[nodiscard]] std::vector<HinksPixFileData> GetFileInfoFromSDCard(uint8_t cmd) const;
    [[nodiscard]] int GetMPUVersion() const { return _MCPU_Version; }
    [[nodiscard]] bool IsHardwareV3() const { return _hardwareV3; }
    [[nodiscard]] bool FirmwareSupportsUpload() const;
    void SendRebootController(bool& worked) const;
#pragma endregion
};

