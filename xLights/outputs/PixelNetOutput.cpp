/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PixelNetOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructors and Destructors
PixelNetOutput::PixelNetOutput(const PixelNetOutput& from) :
    SerialOutput(from)
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
#pragma endregion Constructors and Destructors

#pragma region Start and Stop
bool PixelNetOutput::Open()
{
    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    _datalen = _channels;
    memset(_data, 0, sizeof(_data));

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void PixelNetOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames))
    {
        if (_serial != nullptr)
        {
            if (_serial->WaitingToWrite() == 0)
            {
                memcpy(&_serialBuffer[1], _data, sizeof(_data));
                _serialBuffer[0] = 170;    // start of message
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
void PixelNetOutput::SetOneChannel(int32_t channel, unsigned char data)
{
    if (!_enabled) return;

    if (_data[channel] != (data == 170 ? 171 : data))
    {
        _data[channel] = (data == 170 ? 171 : data);
        _changed = true;
    }
}

void PixelNetOutput::AllOff()
{
    if (!_enabled) return;

    memset(_data, 0, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
//std::string PixelNetOutput::GetSetupHelp() const {

//    return "Pixelnet controllers attached to a USB Lynx Pixelnet\ndongle";
//}
#pragma endregion Getters and Setters
