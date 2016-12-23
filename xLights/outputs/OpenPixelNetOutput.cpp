#include "OpenPixelNetOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

OpenPixelNetOutput::OpenPixelNetOutput(SerialOutput* output) : SerialOutput(output)
{
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
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

wxXmlNode* OpenPixelNetOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    SerialOutput::Save(node);

    return node;
}

bool OpenPixelNetOutput::Open()
{
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

#ifdef USECHANGEDETECTION
    changed = false;
#endif

    return _ok;
}

void OpenPixelNetOutput::EndFrame()
{
    if (!_enabled || _serial == nullptr || !_ok) return;

#ifdef USECHANGEDETECTION
    if (changed)
    {
#endif
        if (_serial->WaitingToWrite() == 0)
        {
            memcpy(&_serialBuffer[6], _data, sizeof(_data));
            _serial->Write((char *)_serialBuffer, sizeof(_serialBuffer));
        }
#ifdef USECHANGEDETECTION
        changed = false;
    }
#endif
}

void OpenPixelNetOutput::SetOneChannel(int channel, unsigned char data)
{
    _data[channel] = data == 170 ? 171 : data;

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void OpenPixelNetOutput::AllOff()
{
    memset(_data, 0, _channels);
#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}
