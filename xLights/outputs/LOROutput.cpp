
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LOROutput.h"

#include <wx/xml/xml.h>

#pragma region Constructors and Destructors
LOROutput::LOROutput(const LOROutput& from) :
    SerialOutput(from)
{
    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}

LOROutput::LOROutput(wxXmlNode* node) : SerialOutput(node) {

    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}

LOROutput::LOROutput() : SerialOutput() {

    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}
#pragma endregion 

#pragma region Start and Stop
bool LOROutput::Open() {

    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    for (size_t i = 0; i < LOR_PACKET_LEN; i++) {
        // encode brightness value according to LOR protocol
        // see: https://github.com/Cryptkeeper/lightorama-protocol/blob/master/PROTOCOL.md#brightness
        // 100% brightness  = 0x01
        // 0% brightness    = 0xF0
        // brightness range = 0xEF
        _data[i] = (i / 255.0F) * -0xEF + 0xF0;
    }

    // initialise to a known state of all off
    memset(_lastSent, 0xFF, sizeof(_lastSent));
    memset(_notSentCount, 0xF0, sizeof(_notSentCount));
    AllOff();

    return _ok;
}
#pragma endregion 

#pragma region Frame Handling
void LOROutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend) return;

    // send heartbeat
    if (_timer_msec > _lastheartbeat + 300 || _timer_msec < _lastheartbeat || _lastheartbeat < 0) {
        SendHeartbeat();
        _lastheartbeat = _timer_msec;
    }
}

void LOROutput::ResetFrame() {
    _lastheartbeat = -1;
}

void LOROutput::SendHeartbeat() const {

    if (!_enabled || _serial == nullptr || !_ok) return;

    uint8_t d[5];
    d[0] = 0;
    d[1] = 0xFF;
    d[2] = 0x81;
    d[3] = 0x56;
    d[4] = 0;
    if (_serial != nullptr) {
        _serial->Write((char*)d, 5);
    }
}
#pragma endregion 

#pragma region Data Setting
void LOROutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled || _serial == nullptr || !_ok) return;

    // because LOR sends the channel number in the packet we can skip sending data that hasnt changed ... I think
    // Copied this from the way FPP seems to handle it - KW
    if (_lastSent[channel] != data || _notSentCount[channel] > 200) {
        _notSentCount[channel] = 0;
        uint8_t d[6];
        d[0] = 0;
        d[1] = channel >> 4;
        if (d[1] < 0xF0) d[1]++;
        d[2] = 3;
        d[3] = _data[data];
        d[4] = 0x80 | (channel % 16);
        d[5] = 0;

        if (_serial != nullptr) {
            _serial->Write((char *)d, 6);
            _lastSent[channel] = data;
        }
    }
    else {
        _notSentCount[channel] = _notSentCount[channel] + 1;
    }
}

void LOROutput::AllOff() {

    if (!_enabled) return;

    // force everything to be sent
    memset(_notSentCount, 0xF0, sizeof(_notSentCount));

    for (int i = 0; i < _channels; i++) {
        SetOneChannel(i, 0x00);
    }
    SendHeartbeat();
    _lastheartbeat = _timer_msec;
    wxMilliSleep(50);
}
#pragma endregion 
