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
#include "UtilClasses.h"

#include <curl/curl.h>

class HinksPix;
class ControllerEthernet;

struct HinksPixOutput {
    HinksPixOutput(int output_ ,int defaultBrightness_) :
        output(output_),
        universe(1),
        startChannel(1),
        pixels(0),
        direction(0),
        protocol(0),
        nullPixel(0),
        colorOrder(0),
        brightness(defaultBrightness_),
        gamma(1),
        used(false),
        controllerStartChannel(1),
        controllerEndChannel(0){};
    const int output;
    int universe;
    int startChannel;
    int pixels;
    int direction;
    int protocol;
    int nullPixel;
    int colorOrder;
    int brightness;
    int gamma;
    bool used;

    int getControllerStartChannel() const { return controllerStartChannel; }
    int getControllerEndChannel() const { return controllerEndChannel; }
    void setControllerChannels(const int startChan);

    void Dump() const;
    void SetConfig(const std::string& data);
    std::string BuildCommand() const;
    std::string BuildCommandEasyLights() const;

private:
    int controllerStartChannel;
    int controllerEndChannel;
};

struct HinksPixSerial {
    HinksPixSerial() :
        e131Universe(1),
        e131StartChannel(1),
        e131NumOfChan(512),
        e131Enabled(false),
        ddpDMXStartChannel(1),
        ddpDMXNumOfChan(512),
        ddpDMXEnabled(false),
        upload(false){};
    int e131Universe;
    int e131StartChannel;
    int e131NumOfChan;
    bool e131Enabled;
    int ddpDMXStartChannel;
    int ddpDMXNumOfChan;
    bool ddpDMXEnabled;
    bool upload;
    void Dump() const;
    void SetConfig(nlohmann::json const& data);

    std::string BuildCommand() const;
    std::string BuildCommandEasyLights(int mode) const;
};

struct HinksSmartOutput {
    HinksSmartOutput(int id_) :
        id(id_), 
        type(0), 
        portStartPixel{ 0, 0, 0, 0 } {};
    int id;
    int type;
    std::array<int, 4> portStartPixel;

    bool operator==(const HinksSmartOutput& rhs) const {
        return id == rhs.id;
    }

    void Dump() const;
    void SetConfig(const std::string& data);
    std::string BuildCommand() const;
};

struct HinksPixInputUniverse {
    HinksPixInputUniverse(int universe_, int numOfChan_) :
        index(-1),
        universe(universe_),
        numOfChan(numOfChan_),
        hinksPixStartChannel(1){};
    int index;
    int universe;
    int numOfChan;
    int32_t hinksPixStartChannel;

    bool operator==(const HinksPixInputUniverse& rhs) const {
        return universe == rhs.universe;
    }

    void Dump() const;
    std::string BuildCommand() const;
    std::string BuildCommandEasyLights() const;
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
    bool InActive;
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
    EXPType _EXP_Outputs[EXP_PORTS];
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
    int EncodeColorOrder(const std::string& colorOrder) const;
    int EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;
    int EncodeBrightness(int brightness) const;
    int EncodeGamma(int gamma) const;
    EXPType DecodeExpansionType(const std::string& type) const;
#pragma endregion

#pragma region Private Functions
    bool InitControllerOutputData(bool fullControl, int defaultBrightness);
    void InitExpansionBoardData(int expansion, int startport, int length);
    std::unique_ptr<HinksPixSerial> InitSerialData(bool fullControl);

    bool UploadInputUniverses(Controller* controller, std::vector<HinksPixInputUniverse> const& inputUniverses) const;
    bool UploadUnPack(bool &worked, Controller *controller, std::vector<UnPack *> const &UPA, bool dirty) const;
    
    bool UploadInputUniversesEasyLights(Controller* controller, std::vector<HinksPixInputUniverse> const& inputUniverses) const;
    void UploadPixelOutputsEasyLights(bool& worked);
    std::string GetControllerE131Data(int rowIndex) const;
    std::string GetControllerData(int rowIndex, const std::string& data = std::string()) const;
    std::string GetControllerRowData(int rowIndex, const std::string& url, const std::string& data) const;
    std::map<std::string, std::string> StringToMap(const std::string& text) const;
    
    void UpdatePortData(HinksPixOutput& pd, UDControllerPort* stringData, int32_t hinkstartChan) const;
    void UpdateSerialData(HinksPixSerial& pd, UDControllerPort* serialData, int const mode, std::vector<HinksPixInputUniverse>& inputUniverses, int32_t& hinkstartChan, int& index, bool individualUniverse) const;
    void UploadPixelOutputs(bool& worked) const;
    void UpdateUniverseControllerChannels(UDControllerPort* stringData, std::vector<HinksPixInputUniverse>& inputUniverses, int32_t& hinkstartChan, int& index, bool individualUniverse);
    void UploadExpansionBoardData(int expansion, int startport, int length, bool& worked) const;
    void UploadSmartReceivers(bool& worked) const;
    void UploadSmartReceiverData(int expan, int bank, std::vector<HinksSmartOutput> const& receivers, bool& worked) const;
    void CalculateSmartReceivers(UDControllerPort* stringData);

    std::string GetJSONControllerData(std::string const& url, std::string const& data) const;
    bool GetControllerDataJSON(const std::string& url, nlohmann::json& val, std::string const& data) const;
    void PostToControllerNoResponse(std::string const& url, std::string const& data) const;
    bool CheckPixelOutputs(std::string & message);
    bool CheckSmartReceivers(std::string & message);


    static const std::string GetJSONPostURL() { return "/Xlights_PostData.cgi"; };
    static const std::string GetJSONInfoURL() { return "/XLights_BoardInfo.cgi"; };
    static const std::string GetJSONPortURL() { return "/Xlights_Board_Port_Config.cgi"; };
    static const std::string GetJSONUnPackURL() { return "/Xlights_UnPack_Config.cgi"; };
    static const std::string GetJSONModeURL() { return "/Xlights_Data_Mode.cgi"; };
    static const std::string GetE131URL() { return"/GetE131Data.cgi"; };
    static const std::string GetInfoURL() { return"/GetInfo.cgi"; };

#pragma endregion

public:

    bool IsUnPackSupported_Hinks(ControllerEthernet *controller);

#pragma region Constructors and Destructors
    HinksPix(const std::string& ip, const std::string& fppProxy);
    virtual ~HinksPix();
#pragma endregion

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
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

