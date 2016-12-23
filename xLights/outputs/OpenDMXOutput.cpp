#include "OpenDMXOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

OpenDMXOutput::OpenDMXOutput(SerialOutput* output) : SerialOutput(output)
{
    memset(_data, 0x00, sizeof(_data));
}

OpenDMXOutput::OpenDMXOutput(wxXmlNode* node) : SerialOutput(node)
{
    memset(_data, 0x00, sizeof(_data));
}

OpenDMXOutput::OpenDMXOutput() : SerialOutput()
{
    memset(_data, 0x00, sizeof(_data));
}

wxXmlNode* OpenDMXOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    SerialOutput::Save(node);

    return node;
}

bool OpenDMXOutput::Open()
{
    _ok = SerialOutput::Open();

    _data[0] = 0;   // dmx start code
    _serialConfig[2] = '2'; // use 2 stop bits so padding chars are not required

#ifdef USECHANGEDETECTION
    changed = false;
#endif

    return _ok;
}

void OpenDMXOutput::EndFrame()
{
    if (!_enabled || _serial == nullptr || !_ok) return;

#ifdef USECHANGEDETECTION
    if (changed)
    {
#endif
        _serial->SendBreak();  // sends a 1 millisecond break
        wxMilliSleep(1);      // mark after break (MAB) - 1 millisecond is overkill (8 microseconds is the minimum dmx requirement)
        _serial->Write((char *)_data, 513);
#ifdef USECHANGEDETECTION
        changed = false;
    }
#endif
}

void OpenDMXOutput::SetOneChannel(int channel, unsigned char data)
{
    _data[channel + 1] = data;

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void OpenDMXOutput::SetManyChannels(int channel, unsigned char data[], size_t size)
{
#ifdef _MSC_VER
    int chs = min((int)size, (int)(_channels - channel + 1));
#else
    int chs = std::min((int)size, (int)(GetMaxChannels() - channel + 1));
#endif
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
