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

#include <wx/socket.h>

class wxXmlNode;
class wxWindow;
class OutputManager;
class ModelManager;

class xxxEthernetOutput : public IPOutput
{
protected:

    #pragma region xxx Constants
    static const unsigned int xxxETHERNET_PACKET_HEADERLEN = 7;
    static const unsigned int xxxETHERNET_PACKET_FOOTERLEN = 1;
    static const unsigned int xxxETHERNET_MAX_CHANNELS = 4096*3;
    static const unsigned int xxx_PORT = 11000;
    static const unsigned int xxx_HEARTBEATINTERVAL = 25;
    #pragma endregion 

    #pragma region Member Variables
    uint8_t _packet[1208];
    uint8_t* _data = nullptr;
    wxIPV4address _remoteAddr;
    wxDatagramSocket* _datagram = nullptr;
    int _port = 0;
    #pragma endregion 

    #pragma region Private Functions
    static void Heartbeat(int mode);
    void OpenDatagram();
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    xxxEthernetOutput(wxXmlNode* node);
    xxxEthernetOutput(xxxEthernetOutput* output);
    xxxEthernetOutput();
    virtual ~xxxEthernetOutput() override;
    virtual wxXmlNode* Save() override;
    #pragma endregion 

    #pragma region Getters and Setters
    int GetPort() const { return _port; }
    void SetPort(int port) { _port = port; _dirty = true; }

    virtual std::string GetType() const override { return OUTPUT_xxxETHERNET; }

    virtual int32_t GetMaxChannels() const override { return xxxETHERNET_MAX_CHANNELS; }
    static int GetMaxxxxChannels() { return xxxETHERNET_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= xxxETHERNET_MAX_CHANNELS; }

    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }

    virtual std::string GetLongDescription() const override;

    virtual std::string GetExport() const override;
    virtual std::string GetUniverseString() const override { return wxString::Format(wxT("%i"), GetPort()).ToStdString(); }
    #pragma endregion

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion 

    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char data[], size_t size) override;
    virtual void AllOff() override;
    #pragma endregion 
};
