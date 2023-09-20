
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OpenDMXOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructors and Destructors
OpenDMXOutput::OpenDMXOutput(wxXmlNode* node) : SerialOutput(node) {
    _baudRate = GetDefaultBaudRate();
    memset(_data, 0x00, sizeof(_data));
}

OpenDMXOutput::OpenDMXOutput() : SerialOutput() {
    _baudRate = GetDefaultBaudRate();
    memset(_data, 0x00, sizeof(_data));
}
OpenDMXOutput::OpenDMXOutput(const OpenDMXOutput& from) :
    SerialOutput(from)
{
    _baudRate = GetDefaultBaudRate();
    memset(_data, 0x00, sizeof(_data));
}
#pragma endregion 

#pragma region Start and Stop
bool OpenDMXOutput::Open() {

    if (!_enabled) return true;

    _serialConfig[2] = '2'; // use 2 stop bits so padding chars are not required
    _ok = SerialOutput::Open();
    _data[0] = 0;   // dmx start code

    return _ok;
}
#pragma endregion 

#pragma region Frame Handling
void OpenDMXOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        if (_serial != nullptr) {
            _serial->SendBreak();  // sends a 1 millisecond break
            wxMilliSleep(1);      // mark after break (MAB) - 1 millisecond is overkill (8 microseconds is the minimum dmx requirement)
            _serial->Write((char *)_data, 513);
            FrameOutput();
        }
    }
    else {
        SkipFrame();
    }
}
#pragma endregion 

#pragma region Data Setting
void OpenDMXOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    if (_data[channel + 1] != data) {
        _data[channel + 1] = data;
        _changed = true;
    }
}

void OpenDMXOutput::SetManyChannels(int32_t channel, unsigned char data[], size_t size) {

    if (!_enabled) return;
    size_t chs = std::min(size, (size_t)(GetMaxChannels() - channel));
    if (memcmp(&_data[channel + 1], data, chs) != 0) {
        memcpy(&_data[channel + 1], data, chs);
        _changed = true;
    }
}

void OpenDMXOutput::AllOff() {
    if (!_enabled) return;
    memset(&_data[1], 0x00, _channels);
    _changed = true;
}
#pragma endregion
