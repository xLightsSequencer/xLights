#include "OpenPixelNetOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructs and Destructors
OpenPixelNetOutput::OpenPixelNetOutput(SerialOutput* output) : SerialOutput(output)
{
    _baudRate = 1000000;
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

OpenPixelNetOutput::OpenPixelNetOutput(wxXmlNode* node) : SerialOutput(node)
{
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

OpenPixelNetOutput::OpenPixelNetOutput() : SerialOutput()
{
    _baudRate = 1000000;
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}
#pragma endregion Constructs and Destructors

#pragma region Start and Stop
bool OpenPixelNetOutput::Open()
{
    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    _datalen = _channels;
    memset(_data, 0, sizeof(_data));

    _serialConfig[2] = '2';

    _serialBuffer[0] = 0xAA;
    _serialBuffer[1] = 0x55;
    _serialBuffer[2] = 0x55;
    _serialBuffer[3] = 0xAA;
    _serialBuffer[4] = 0x15;
    _serialBuffer[5] = 0x5D;

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void OpenPixelNetOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames))
    {
        if (_serial != nullptr)
        {
            if (_serial->WaitingToWrite() == 0)
            {
                memcpy(&_serialBuffer[6], _data, sizeof(_data));
                _serial->Write((char *)_serialBuffer, sizeof(_serialBuffer));
                FrameOutput();
            }
        }
    }
    else
    {
        SkipFrame();
    }
}
#pragma endregion Frame Handling

#pragma region Data Setting
void OpenPixelNetOutput::SetOneChannel(int32_t channel, unsigned char data)
{
    if (_data[channel] != (data == 170 ? 171 : data))
    {
        _data[channel] = (data == 170 ? 171 : data);
        _changed = true;
    }
}

void OpenPixelNetOutput::AllOff()
{
    memset(_data, 0, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
std::string OpenPixelNetOutput::GetSetupHelp() const
{
    return "Pixelnet controllers attached to a generic USB\nto RS485 dongle with FTDI chipset and virtual comm port.";
}
#pragma endregion Getters and Setters
