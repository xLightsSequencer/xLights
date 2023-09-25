
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xxxSerialOutput.h"

#include <wx/xml/xml.h>

#pragma region Private Functions
void xxxSerialOutput::SendHeartbeat() const {

    if (!_enabled || _serial == nullptr || !_ok) return;

    uint8_t d[4];
    d[0] = 0X80;
    d[1] = 0x00;
    d[2] = 0x00;
    d[3] = 0x81;
    if (_serial != nullptr) {
        _serial->Write((char*)d, 4);
    }
}
#pragma endregion

#pragma region Constructors and Destructors
xxxSerialOutput::xxxSerialOutput(const xxxSerialOutput& from) : SerialOutput(from)
{
    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}

xxxSerialOutput::xxxSerialOutput(wxXmlNode* node) : SerialOutput(node) {
    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}

xxxSerialOutput::xxxSerialOutput() : SerialOutput() {
    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}
#pragma endregion

#pragma region Start and Stop
bool xxxSerialOutput::Open() {

    if (!_enabled) return true;

    _ok = SerialOutput::Open();

    // initialise to a known state of all off
    memset(_lastSent, 0xFF, sizeof(_lastSent));
    memset(_notSentCount, 0xF0, sizeof(_notSentCount));
    AllOff();

    return _ok;
}
#pragma endregion

#pragma region Frame Handling
void xxxSerialOutput::ResetFrame() {

    _lastheartbeat = -1;
}

void xxxSerialOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        if (_serial != nullptr) {
			int devices = (_channels - 1) / 8 + 1;
			for (int i = 0; i < devices; i++) {
				if (memcmp(&_data[i * 8], &_lastSent[i*8], 8) != 0) {
					// something changed
					std::list<uint8_t> values;
					for (int j = 0; j < 8; j++) {
						if (std::find(values.begin(), values.end(), _data[i*8+j]) == values.end()) {
							values.push_back(_data[i*8+j]);
						}
					}
					for (const auto& it : values) {
						uint8_t mask = 0;
						uint8_t m = 0x01;
						for (int j = 0; j < 8; j++) {
							if (_data[i*8+j]==it) {
                                if (_data[i * 8 + j] != _lastSent[i * 8 + j]) {
                                    mask += m;
                                }
							}
							m = m << 1;
						}
						
                        if (mask != 0) {
                            uint8_t d[6];
                            d[0] = 0x80;
                            d[1] = i + 1;
                            d[2] = 0x11; // on
                            d[3] = it;
                            d[4] = mask;
                            d[5] = 0x81;

                            _serial->Write((char*)d, 6);
                        }
					}			
				}
			}
        }
        memcpy(_lastSent, _data, GetMaxChannels());
        FrameOutput();
    }
    else {
        SkipFrame();
    }

    if (_timer_msec >= _lastheartbeat + 500 || _timer_msec < _lastheartbeat || _lastheartbeat < 0) {
        SendHeartbeat();
        _lastheartbeat = _timer_msec;
    }
}
#pragma endregion 

#pragma region Data Setting
void xxxSerialOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled || _serial == nullptr || !_ok) return;

    // because xxx sends the channel number in the packet we can skip sending data that hasnt changed ... I think
    // Copied this from the way FPP seems to handle it - KW
    if (_lastSent[channel] != data || _notSentCount[channel] > 200) {
        _notSentCount[channel] = 0;
		
		uint8_t device = channel / 8;
		uint8_t mask = 0x01 << (channel % 8);
		
        uint8_t d[6];
        d[0] = 0x80;
		d[1] = device + 1;
		d[2] = 0x11; // on
		d[3] = data;
		d[4] = mask;
		d[5] = 0x81;

        if (_serial != nullptr) {
            _serial->Write((char *)d, 6);
            _lastSent[channel] = data;
        }
    }
    else {
        _notSentCount[channel] = _notSentCount[channel] + 1;
    }
}

void xxxSerialOutput::SetManyChannels(int32_t channel, unsigned char data[], size_t size) {

    size_t chs = std::min(size, (size_t)(GetMaxChannels() - channel));
    
    if (memcmp(&_data[channel], data, chs) != 0) {
        memcpy(&_data[channel], data, chs);
        _changed = true;
    }
}

void xxxSerialOutput::AllOff() {

    // force everything to be sent
    memset(_notSentCount, 0xF0, sizeof(_notSentCount));
	memset(&_data[0], 0x00, GetMaxChannels());
	_changed = true;
}
#pragma endregion 
