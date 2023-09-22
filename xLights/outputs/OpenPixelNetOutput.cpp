
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OpenPixelNetOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructs and Destructors
OpenPixelNetOutput::OpenPixelNetOutput(const OpenPixelNetOutput& from) :
    SerialOutput(from)
{
    _baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

OpenPixelNetOutput::OpenPixelNetOutput(wxXmlNode* node) : SerialOutput(node) {

    _baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}

OpenPixelNetOutput::OpenPixelNetOutput() : SerialOutput() {

    _baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
    memset(_serialBuffer, 0x00, sizeof(_serialBuffer));
}
#pragma endregion 

#pragma region Start and Stop
bool OpenPixelNetOutput::Open() {

    if (!_enabled) return true;

    _serialConfig[2] = '2';
    _ok = SerialOutput::Open();

    _datalen = _channels;
    memset(_data, 0, sizeof(_data));

    _serialBuffer[0] = 0xAA;
    _serialBuffer[1] = 0x55;
    _serialBuffer[2] = 0x55;
    _serialBuffer[3] = 0xAA;
    _serialBuffer[4] = 0x15;
    _serialBuffer[5] = 0x5D;

    return _ok;
}
#pragma endregion 

#pragma region Frame Handling
void OpenPixelNetOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        if (_serial != nullptr) {
            if (_serial->WaitingToWrite() == 0) {
                memcpy(&_serialBuffer[6], _data, sizeof(_data));
                _serial->Write((char *)_serialBuffer, sizeof(_serialBuffer));
                FrameOutput();
            }
        }
    }
    else {
        SkipFrame();
    }
}
#pragma endregion 

#pragma region Data Setting
void OpenPixelNetOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;

    if (_data[channel] != (data == 170 ? 171 : data)) {
        _data[channel] = (data == 170 ? 171 : data);
        _changed = true;
    }
}

void OpenPixelNetOutput::AllOff() {
    if (!_enabled) return;
    memset(_data, 0, _channels);
    _changed = true;
}
#pragma endregion
