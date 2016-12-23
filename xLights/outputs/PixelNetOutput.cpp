#include "PixelNetOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

PixelNetOutput::PixelNetOutput(SerialOutput* output) : SerialOutput(output)
{
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

PixelNetOutput::PixelNetOutput(wxXmlNode* node) : SerialOutput(node)
{
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

PixelNetOutput::PixelNetOutput() : SerialOutput()
{
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

wxXmlNode* PixelNetOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    SerialOutput::Save(node);

    return node;
}

bool PixelNetOutput::Open()
{
    _ok = SerialOutput::Open();

    _datalen = _channels;
    memset(_data, 0, sizeof(_data));

#ifdef USECHANGEDETECTION
    changed = false;
#endif

    return _ok;
}

void PixelNetOutput::EndFrame()
{
    if (!_enabled || _serial == nullptr || !_ok) return;

#ifdef USECHANGEDETECTION
    if (changed)
    {
#endif
        if (_serial->WaitingToWrite() == 0)
        {
            memcpy(&_serialBuffer[1], _data, sizeof(_data));
            _serialBuffer[0] = 170;    // start of message
            _serial->Write((char *)_serialBuffer, sizeof(_serialBuffer));
        }
#ifdef USECHANGEDETECTION
        changed = false;
    }
#endif
}

void PixelNetOutput::SetOneChannel(int channel, unsigned char data)
{
    _data[channel] = data == 170 ? 171 : data;

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void PixelNetOutput::AllOff()
{
    memset(_data, 0, _channels);
#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}
