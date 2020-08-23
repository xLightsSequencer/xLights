#include "GenericSerialOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

#pragma region Constructors and Destructors
GenericSerialOutput::GenericSerialOutput(SerialOutput* output) : SerialOutput(output)
{
    _datalen = 0;
    _data = std::vector<uint8_t>(GENERICSERIAL_MAX_CHANNELS);
}

GenericSerialOutput::GenericSerialOutput(wxXmlNode* node) : SerialOutput(node)
{
    _datalen = 0;
    _data = std::vector<uint8_t>(GENERICSERIAL_MAX_CHANNELS);
}

GenericSerialOutput::GenericSerialOutput() : SerialOutput()
{
    _datalen = 0;
    _data = std::vector<uint8_t>(GENERICSERIAL_MAX_CHANNELS);
}
#pragma endregion Constructors and Destructors

#pragma region Start and Stop
bool GenericSerialOutput::Open()
{
    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    _datalen = _channels;

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void GenericSerialOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames))
    {
        if (_serial != nullptr)
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
    if (_data[channel] != data) {
        _data[channel] = data;
        _changed = true;
    }
}

void GenericSerialOutput::AllOff()
{
    if (!_enabled) return;
    memset(&_data[0], 0x00, _datalen);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
//std::string GenericSerialOutput::GetSetupHelp() const {

//    return "Generic Serial controllers connected to a serial port or \na USB dongle with virtual comm port.";
//}
#pragma endregion Getters and Setters
