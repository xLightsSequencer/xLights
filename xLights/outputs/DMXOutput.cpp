
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DMXOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructors and Destructors
DMXOutput::DMXOutput(const DMXOutput& from) : SerialOutput(from) {

    _baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

DMXOutput::DMXOutput(wxXmlNode* node) : SerialOutput(node) {
    _baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}

DMXOutput::DMXOutput() : SerialOutput() {
    _baudRate = GetDefaultBaudRate();
    _datalen = 0;
    memset(_data, 0x00, sizeof(_data));
}
#pragma endregion

#pragma region Start and Stop
bool DMXOutput::Open() {

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
#pragma endregion 

#pragma region Frame Handling
void DMXOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        if (_serial != nullptr) {
            _serial->Write((char *)_data, _datalen);
        }
        FrameOutput();
    }
    else {
        SkipFrame();
    }
}
#pragma endregion

#pragma region Data Setting
void DMXOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    if (_data[channel + 5] != data) {
        _changed = true;
        _data[channel + 5] = data;
    }
}

void DMXOutput::SetManyChannels(int32_t channel, unsigned char data[], size_t size) {

    if (!_enabled) return;
    size_t chs = std::min(size, (size_t)(GetMaxChannels() - channel));
    
    if (memcmp(&_data[channel + 5], data, chs) == 0) {
        // nothing changed
    }
    else {
        memcpy(&_data[channel + 5], data, chs);
        _changed = true;
    }
}

void DMXOutput::AllOff() {

    if (!_enabled) return;
    memset(&_data[5], 0x00, _channels);
    _changed = true;
}
#pragma endregion
