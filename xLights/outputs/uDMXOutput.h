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

#include "SerialOutput.h"

// ***************************************************************************************
// * This class represents a single uDMX Device
// * Compatible with uDMX dongles
// * Methods should be called with: 0 <= chindex < 512
// ***************************************************************************************

#pragma region DMX Constants
#define UDMX_MAX_CHANNELS 512

#define UDMX_SHARED_VENDOR     0x16C0 /* VOTI */
#define UDMX_SHARED_PRODUCT    0x05DC /* Obdev's free shared PID */

#define UDMX_AVLDIY_D512_CLONE_VENDOR     0x03EB /* Atmel Corp. (Clone VID)*/
#define UDMX_AVLDIY_D512_CLONE_PRODUCT    0x8888 /* Clone PID */

#define UDMX_SET_CHANNEL_RANGE 0x0002 /* Command to set n channel values */
#pragma endregion DMX Constants

struct libusb_context;
struct libusb_device;
struct libusb_device_handle;
struct libusb_device_descriptor;

class uDMXOutput : public SerialOutput
{
    #pragma region Member Variables
    int _datalen{0};
    uint8_t _data[UDMX_MAX_CHANNELS + 1];

    struct libusb_context* m_ctx{nullptr};
    struct libusb_device* m_device{nullptr};
    //struct libusb_device_descriptor *m_descriptor{nullptr};
    struct libusb_device_handle* m_handle{nullptr};
    #pragma endregion 

    bool isValidDevice(const struct libusb_device_descriptor* desc) const;

public:

    #pragma region Constructors and Destructors
    uDMXOutput(wxXmlNode* node);
    uDMXOutput(SerialOutput* output);
    uDMXOutput();
    virtual ~uDMXOutput() override;
    #pragma endregion 

    #pragma region Getters and Setters
    std::string GetType() const override { return OUTPUT_UDMX; }
    virtual int GetMaxChannels() const override { return UDMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= UDMX_MAX_CHANNELS; }
    virtual bool AllowsBaudRateSetting() const override { return false; }
    virtual int GetDefaultBaudRate() const override { return 250000; }
    PINGSTATE Ping() const override;
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion 
    
    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion 
    
    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char data[], size_t size) override;
    virtual void AllOff() override;
    #pragma region 
};
