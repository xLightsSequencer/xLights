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

#include "IPOutput.h"
#include <array>

class wxJSONValue;
class wxDatagramSocket;
class Discovery;

class TwinklyOutput : public IPOutput
{
public:
#pragma region Constructors and Destructors
    TwinklyOutput(wxXmlNode* node);
    TwinklyOutput();
    virtual ~TwinklyOutput() override;
    virtual wxXmlNode* Save() override;
#pragma endregion

#pragma region Output overrides
    virtual std::string GetType() const override;
    virtual int GetMaxChannels() const override;
    virtual bool IsValidChannelCount(int32_t channelCount) const override;
    virtual std::string GetLongDescription() const override;
    int GetId() const
    {
        return _universe;
    }
    void SetId(int id)
    {
        _universe = id;
        _dirty = true;
    }
#pragma endregion

#pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
#pragma endregion

#pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
#pragma endregion

#ifndef EXCLUDEDISCOVERY
    static wxJSONValue Query(const std::string& ip, uint8_t type, const std::string& localIP);
    static void PrepareDiscovery(Discovery& discovery);
#endif

#pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
#pragma endregion

    bool GetLayout(std::vector<std::tuple<float, float, float>>& result);
    static bool GetLayout(const std::string& ip, std::vector<std::tuple<float, float, float>>& result);

private:
    // A single twinkly connection may have unlimited channels
    static const int MAX_CHANNELS = 4 * 2000;
    // A device on lac should respond quickly
    static const int HTTP_TIMEOUT = 5;
    static const int TOKEN_SIZE = 8;
    static const short UDP_PORT = 7777;
    static const short DISCOVERY_PORT = 5555;

    // make an http call and returns a json
    bool MakeCall(const std::string& method, const std::string& path, wxJSONValue& result, const char* body = nullptr);

    bool ReloadToken();

    std::string m_token;
    std::array<char, TOKEN_SIZE> m_decodedToken;
    std::vector<unsigned char> m_channelData;
    wxDatagramSocket* _datagram = nullptr;

    void OpenDatagram();
};