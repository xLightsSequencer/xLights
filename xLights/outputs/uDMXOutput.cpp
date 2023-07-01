
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "uDMXOutput.h"

#include "Output.h"

#include <wx/xml/xml.h>

#include <log4cpp/Category.hh>

#include <libusb.h>
#include <uchar.h>

#pragma region Constructors and Destructors
uDMXOutput::uDMXOutput(SerialOutput* output) : SerialOutput(output) {

    //memset(_lastSent, 0x00, sizeof(_lastSent));
    //memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}

uDMXOutput::uDMXOutput(wxXmlNode* node)  : SerialOutput(node) {
   // _baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

uDMXOutput::uDMXOutput() : SerialOutput() {
    //_baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

uDMXOutput::~uDMXOutput()
{
    Close();
}
#pragma endregion

#pragma region Start and Stop
bool uDMXOutput::Open() {

    if (!_enabled){ return true;}
    _datalen = _channels;
    m_ctx = nullptr;

    if (libusb_init(&m_ctx) != 0) {
         return false;
    }
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    libusb_device** devices = nullptr;
    ssize_t count = libusb_get_device_list(m_ctx, &devices);
    for (ssize_t i = 0; i < count; i++)
    {
        libusb_device* dev = devices[i];

        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0)
        {
            continue;
        }
        logger_base.debug("%04x:%04x (bus %d, device %d)",
		desc.idVendor, desc.idProduct,
		libusb_get_bus_number(dev), libusb_get_device_address(dev));

		if (isValidDevice(&desc)) {
           m_device = dev;
        }
    }

    if (nullptr != m_device && nullptr == m_handle )
    {
        int ret = libusb_open(m_device, &m_handle);
        if (ret < 0)
        {
            m_handle = nullptr;
            _ok = false;
        } else {
            _ok = true;
        }
    }

    if ( nullptr == m_handle) {
        _ok = false;
    }

    return _ok;
}

void uDMXOutput::Close()
{
    if (nullptr != m_device && nullptr != m_handle) {
        libusb_close(m_handle);
    }

    m_handle = nullptr;
}
#pragma endregion 

#pragma region Frame Handling
void uDMXOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || m_device == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        if (nullptr != m_device && nullptr != m_handle) {

            libusb_control_transfer(m_handle,
                            LIBUSB_REQUEST_TYPE_VENDOR |
                            LIBUSB_RECIPIENT_INTERFACE |
                            LIBUSB_ENDPOINT_OUT,
                            UDMX_SET_CHANNEL_RANGE,     /* Command */
                            _datalen,          /* Number of channels to set */
                            0,                          /* Starting index */
                            (unsigned char *)_data, /* Values to set */
                            _datalen,          /* Size of values */
                            500);                       /* Timeout 500ms */
        }
        FrameOutput();
    }
    else {
        SkipFrame();
    }
}
#pragma endregion

#pragma region Data Setting
void uDMXOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    if (_data[channel] != data) {
        _changed = true;
        _data[channel] = data;
    }
}

void uDMXOutput::SetManyChannels(int32_t channel, unsigned char data[], size_t size) {

    if (!_enabled) return;
    size_t chs = std::min(size, (size_t)(GetMaxChannels() - channel));
    
    if (memcmp(&_data[channel], data, chs) == 0) {
        // nothing changed
    }
    else {
        memcpy(&_data[channel], data, chs);
        _changed = true;
    }
}

void uDMXOutput::AllOff() {

    if (!_enabled) return;
    memset(&_data[0], 0x00, _channels);
    _changed = true;
}
#pragma endregion

#pragma region Getters and Setters
Output::PINGSTATE uDMXOutput::Ping() const {

    if (nullptr != m_device && nullptr != m_handle && _ok) {
        return Output::PINGSTATE::PING_OPEN;
    } else {
        Output::PINGSTATE res = Output::PINGSTATE::PING_ALLFAILED;
        if ( m_ctx != nullptr) {
            struct libusb_device_handle* handle{nullptr};
            libusb_device** devices = nullptr;
            ssize_t count = libusb_get_device_list(m_ctx, &devices);
            for (ssize_t i = 0; i < count; i++) {
                libusb_device* dev = devices[i];

                libusb_device_descriptor desc;
                int r = libusb_get_device_descriptor(dev, &desc);
                if (r < 0) {
                    continue;
                }
                if (isValidDevice(&desc)) {
                    if (nullptr != handle) {
                        int ret = libusb_open(dev, &handle);
                        if (ret < 0) {
                        } else {
                            res = Output::PINGSTATE::PING_OPENED;
                            libusb_close(handle);
                        }
                    }
                }
            }
        }

        return res;
    }
}

bool uDMXOutput::isValidDevice(const struct libusb_device_descriptor* desc) const
{
    if (nullptr == desc) {
        return false;
    }

    if (desc->idVendor != UDMX_SHARED_VENDOR &&
        desc->idVendor != UDMX_AVLDIY_D512_CLONE_VENDOR) {
        return false;
    }

    if (desc->idProduct != UDMX_SHARED_PRODUCT &&
        desc->idProduct != UDMX_AVLDIY_D512_CLONE_PRODUCT) {
        return false;
    }

    return true;
}
#pragma endregion