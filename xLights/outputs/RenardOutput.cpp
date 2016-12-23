#include "RenardOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

RenardOutput::RenardOutput(SerialOutput* output) : SerialOutput(output)
{
    _datalen = 0;
    _data = std::vector<wxByte>(RENARD_MAX_CHANNELS+9);
}

wxXmlNode* RenardOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    SerialOutput::Save(node);

    return node;
}

RenardOutput::RenardOutput(wxXmlNode* node) : SerialOutput(node)
{
    _datalen = 0;
    _data = std::vector<wxByte>(RENARD_MAX_CHANNELS+9);
}

RenardOutput::RenardOutput() : SerialOutput()
{
    _datalen = 0;
    _data = std::vector<wxByte>(RENARD_MAX_CHANNELS+9);
}

bool RenardOutput::Open()
{
    _ok = SerialOutput::Open();

    _datalen = _channels + 2;
    _data[0] = 0x7E;               // start of message
    _data[1] = 0x80;               // start address
    _serialConfig[2] = '2'; // use 2 stop bits so padding chars are not required
#ifdef USECHANGEDETECTION
    changed = false;
#endif

    return _ok;
}

void RenardOutput::EndFrame()
{
    if (!_enabled || _serial == nullptr || !_ok) return;

#ifdef USECHANGEDETECTION
    if (changed)
    {
#endif
        _serial->Write((char *)&_data[0], _datalen);
#ifdef USECHANGEDETECTION
        changed = false;
    }
#endif
}

void RenardOutput::SetOneChannel(int channel, unsigned char data)
{
    wxByte RenIntensity;

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
    _data[channel + 2] = RenIntensity;

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void RenardOutput::AllOff()
{
    for (int i = 0; i < _channels; i++)
    {
        SetOneChannel(i + 1, 0x00);
    }
}
