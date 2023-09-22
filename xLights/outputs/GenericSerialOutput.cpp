#include "GenericSerialOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

#define MAX_PREFIX_POSTFIX 256

#pragma region Constructors and Destructors
GenericSerialOutput::GenericSerialOutput(const GenericSerialOutput& from) : SerialOutput(from)
{
    _datalen = 0;
    _data = std::vector<uint8_t>(GENERICSERIAL_MAX_CHANNELS + MAX_PREFIX_POSTFIX);
}

GenericSerialOutput::GenericSerialOutput(wxXmlNode* node) : SerialOutput(node)
{
    _datalen = 0;
    _data = std::vector<uint8_t>(GENERICSERIAL_MAX_CHANNELS + MAX_PREFIX_POSTFIX);
}

GenericSerialOutput::GenericSerialOutput() : SerialOutput()
{
    _datalen = 0;
    _data = std::vector<uint8_t>(GENERICSERIAL_MAX_CHANNELS + MAX_PREFIX_POSTFIX);
}
#pragma endregion Constructors and Destructors

#pragma region Start and Stop
bool GenericSerialOutput::Open()
{
    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    _datalen = _channels + _prefix.size() + _postfix.size();

    if (_datalen > GENERICSERIAL_MAX_CHANNELS + MAX_PREFIX_POSTFIX) {
        return false;
    }

    if (_prefix.size() > 0) {
        memcpy(_data.data(), _prefix.data(), _prefix.size());
    }

    if (_postfix.size() > 0) {
        memcpy(_data.data() + _channels + _prefix.size(), _postfix.data(), _postfix.size());
    }

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void GenericSerialOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames))
    {
        if (_serial != nullptr && _datalen > 0)
        {
            _serial->Write((char *)&_data[0], _datalen);
            FrameOutput();
        }
    }
    else
    {
        SkipFrame();
    }
}
#pragma endregion Frame Handling

#pragma region Data Setting
void GenericSerialOutput::SetOneChannel(int32_t channel, unsigned char data)
{
 if (!_enabled) return;
    if (_data[channel + _prefix.size()] != data) {
        _data[channel + _prefix.size()] = data;
        _changed = true;
    }
}

void GenericSerialOutput::AllOff()
{
    if (!_enabled || _datalen == 0) return;
    memset(&_data[_prefix.size()], 0x00, _datalen - _prefix.size() - _postfix.size());
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
//std::string GenericSerialOutput::GetSetupHelp() const {

//    return "Generic Serial controllers connected to a serial port or \na USB dongle with virtual comm port.";
//}
#pragma endregion Getters and Setters
