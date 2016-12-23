#include "DMXOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

DMXOutput::DMXOutput(SerialOutput* output) : SerialOutput(output)
{
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

wxXmlNode* DMXOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    SerialOutput::Save(node);

    return node;
}

DMXOutput::DMXOutput(wxXmlNode* node) : SerialOutput(node)
{
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

DMXOutput::DMXOutput() : SerialOutput()
{
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

bool DMXOutput::Open()
{
    _ok = SerialOutput::Open();

    int len = _channels < 512 ? 513 : _channels + 1;
    _datalen = len + 5;
    _data[0] = 0x7E;               // start of message
    _data[1] = 6;                  // dmx send
    _data[2] = len & 0xFF;         // length LSB
    _data[3] = (len >> 8) & 0xFF;  // length MSB
    _data[4] = 0;                  // DMX start
    _data[_datalen - 1] = 0xE7;       // end of message
#ifdef USECHANGEDETECTION
    changed = false;
#endif

    return _ok;
}

void DMXOutput::EndFrame()
{
    if (!_enabled || _serial == nullptr || !_ok) return;

#ifdef USECHANGEDETECTION
    if (changed)
    {
#endif
        _serial->Write((char *)_data, _datalen);
#ifdef USECHANGEDETECTION
        changed = false;
    }
#endif
}

void DMXOutput::SetOneChannel(int channel, unsigned char data)
{
    _data[channel + 5] = data;

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void DMXOutput::SetManyChannels(int channel, unsigned char data[], size_t size)
{
#ifdef _MSC_VER
    int chs = min((int)size, (int)(_channels - channel + 1));
#else
    int chs = std::min((int)size, (int)(GetMaxChannels() - channel + 1));
#endif
    memcpy(&_data[channel + 5], data, chs);

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void DMXOutput::AllOff()
{
    memset(&_data[6], 0x00, _channels);

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}
