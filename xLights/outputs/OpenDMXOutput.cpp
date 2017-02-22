#include "OpenDMXOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructors and Destructors
OpenDMXOutput::OpenDMXOutput(SerialOutput* output) : SerialOutput(output)
{
    _baudRate = 250000;
    memset(_data, 0x00, sizeof(_data));
}

OpenDMXOutput::OpenDMXOutput(wxXmlNode* node) : SerialOutput(node)
{
    _baudRate = 250000;
    memset(_data, 0x00, sizeof(_data));
}

OpenDMXOutput::OpenDMXOutput() : SerialOutput()
{
    _baudRate = 250000;
    memset(_data, 0x00, sizeof(_data));
}
#pragma endregion Constructors and Destructors

#pragma region Start and Stop
bool OpenDMXOutput::Open()
{
    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    _data[0] = 0;   // dmx start code
    _serialConfig[2] = '2'; // use 2 stop bits so padding chars are not required

#ifdef USECHANGEDETECTION
    changed = false;
#endif

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void OpenDMXOutput::EndFrame()
{
    if (!_enabled || _serial == nullptr || !_ok) return;

#ifdef USECHANGEDETECTION
    if (changed)
    {
#endif
        if (_serial != nullptr)
        {
            _serial->SendBreak();  // sends a 1 millisecond break
            wxMilliSleep(1);      // mark after break (MAB) - 1 millisecond is overkill (8 microseconds is the minimum dmx requirement)
            _serial->Write((char *)_data, 513);
        }

#ifdef USECHANGEDETECTION
        changed = false;
    }
#endif
}
#pragma endregion Frame Handling

#pragma region Data Setting
void OpenDMXOutput::SetOneChannel(long channel, unsigned char data)
{
    _data[channel + 1] = data;

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void OpenDMXOutput::SetManyChannels(long channel, unsigned char data[], long size)
{
    long chs = std::min(size, GetMaxChannels() - channel);
    memcpy(&_data[channel + 1], data, chs);

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void OpenDMXOutput::AllOff()
{
    memset(&_data[1], 0x00, _channels);

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}
#pragma endregion Data Setting

#pragma region Getters and Setters
std::string OpenDMXOutput::GetSetupHelp() const
{
    return "DMX controllers (or LOR or D-Light controllers in DMX mode)\nattached to an LOR dongle, D-Light dongle, HolidayCoro\nprogramming cable, or Entec Open DMX dongle";
}
#pragma endregion Getters and Setters
