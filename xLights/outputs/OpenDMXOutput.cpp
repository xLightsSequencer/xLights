#include "OpenDMXOutput.h"

#include <wx/xml/xml.h>

#pragma region Constructors and Destructors
OpenDMXOutput::OpenDMXOutput(SerialOutput* output) : SerialOutput(output) {
    _baudRate = 250000;
    memset(_data, 0x00, sizeof(_data));
}

OpenDMXOutput::OpenDMXOutput(wxXmlNode* node) : SerialOutput(node) {
    _baudRate = 250000;
    memset(_data, 0x00, sizeof(_data));
}

OpenDMXOutput::OpenDMXOutput() : SerialOutput() {
    _baudRate = 250000;
    memset(_data, 0x00, sizeof(_data));
}
#pragma endregion 

#pragma region Start and Stop
bool OpenDMXOutput::Open() {

    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    _data[0] = 0;   // dmx start code
    _serialConfig[2] = '2'; // use 2 stop bits so padding chars are not required

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

    if (_data[channel + 1] != data) {
        _data[channel + 1] = data;
        _changed = true;
    }
}

void OpenDMXOutput::SetManyChannels(int32_t channel, unsigned char data[], size_t size) {

    size_t chs = std::min(size, (size_t)(GetMaxChannels() - channel));
    if (memcmp(&_data[channel + 1], data, chs) != 0) {
        memcpy(&_data[channel + 1], data, chs);
        _changed = true;
    }
}

void OpenDMXOutput::AllOff() {
    memset(&_data[1], 0x00, _channels);
    _changed = true;
}
#pragma endregion
