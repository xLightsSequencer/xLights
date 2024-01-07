#include "RenardOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

#pragma region Constructors and Destructors
RenardOutput::RenardOutput(wxXmlNode* node) : SerialOutput(node)
{
    _datalen = 0;
    _data = std::vector<uint8_t>(RENARD_MAX_CHANNELS+9);
}

RenardOutput::RenardOutput(const RenardOutput& from) :
    SerialOutput(from)
{
    _datalen = 0;
    _data = std::vector<uint8_t>(RENARD_MAX_CHANNELS + 9);
}

RenardOutput::RenardOutput() : SerialOutput()
{
    _datalen = 0;
    _data = std::vector<uint8_t>(RENARD_MAX_CHANNELS+9);
}
#pragma endregion Constructors and Destructors

#pragma region Start and Stop
bool RenardOutput::Open()
{
    if (!_enabled) return true;

    _serialConfig[2] = '2'; // use 2 stop bits so padding chars are not required
    _ok = SerialOutput::Open();

    _datalen = _channels + 2;
    _data[0] = 0x7E;               // start of message
    _data[1] = 0x80;               // start address
    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void RenardOutput::EndFrame(int suppressFrames)
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
void RenardOutput::SetOneChannel(int32_t channel, unsigned char data)
{
    if (!_enabled) return;

    uint8_t RenIntensity;

    switch (data)
    {
    case 0x7D:
    case 0x7E:
        RenIntensity = 0x7C;
        break;
    case 0x7F:
        RenIntensity = 0x80;
        break;
    default:
        RenIntensity = data;
    }

    if (_data[channel + 2] != RenIntensity)
    {
        _data[channel + 2] = RenIntensity;
        _changed = true;
    }
}

void RenardOutput::AllOff()
{
    if (!_enabled) return;

    for (int i = 0; i < _channels; i++)
    {
        SetOneChannel(i + 2, 0x00);
    }
}
#pragma endregion Data Setting

#pragma region Getters and Setters
//std::string RenardOutput::GetSetupHelp() const {

//    return "Renard controllers connected to a serial port or \na USB dongle with virtual comm port. 2 stop bits\nare set automatically.\nMax of 42 channels at 9600 baud.\nMax of 260 channels at 57600 baud.";
//}
#pragma endregion Getters and Setters
