#include "DMXOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructors and Destructors
DMXOutput::DMXOutput(SerialOutput* output) : SerialOutput(output)
{
    _baudRate = 250000;
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

DMXOutput::DMXOutput(wxXmlNode* node) : SerialOutput(node)
{
    _baudRate = 250000;
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

DMXOutput::DMXOutput() : SerialOutput()
{
    _baudRate = 250000;
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}
#pragma endregion Constructors and Destructors

#pragma region Start and Stop
bool DMXOutput::Open()
{
    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    int len = _channels < 512 ? 513 : _channels + 1;
    _datalen = len + 5;
    _data[0] = 0x7E;               // start of message
    _data[1] = 6;                  // dmx send
    _data[2] = len & 0xFF;         // length LSB
    _data[3] = (len >> 8) & 0xFF;  // length MSB
    _data[4] = 0;                  // DMX start
    _data[_datalen - 1] = 0xE7;       // end of message

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void DMXOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames))
    {
        if (_serial != nullptr)
        {
            _serial->Write((char *)_data, _datalen);
        }
        FrameOutput();
    }
    else
    {
        SkipFrame();
    }
}
#pragma endregion Frame Handling

#pragma region Data Setting
void DMXOutput::SetOneChannel(long channel, unsigned char data)
{
    if (_data[channel + 5] != data)
    {
        _changed = true;
        _data[channel + 5] = data;
    }
}

void DMXOutput::SetManyChannels(long channel, unsigned char data[], long size)
{
    long chs = std::min(size, GetMaxChannels() - channel);
    
    if (memcmp(&_data[channel + 5], data, chs) == 0)
    {
        // nothing changed
    }
    else
    {
        memcpy(&_data[channel + 5], data, chs);
        _changed = true;
    }
}

void DMXOutput::AllOff()
{
    memset(&_data[5], 0x00, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region  Getters and Setters
std::string DMXOutput::GetSetupHelp() const
{
    return "DMX controllers (or LOR or D-Light controllers in DMX mode)\nattached to an Entec DMX USB Pro, Lynx DMX dongle,\nDIYC RPM, DMXking.com, or DIY Blinky dongle.\n\nLast Channel should be 512 or less, unless you are using\na DIY Blinky dongle (in which case it can be up to 3036).";
}
#pragma endregion  Getters and Setters
