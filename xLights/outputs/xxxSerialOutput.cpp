
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xxxSerialOutput.h"
#include "ControllerSerial.h"
#include "OutputModelManager.h"

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
    SetDeviceChannels(from._deviceChannels);
    _dirty = false;
}

xxxSerialOutput::xxxSerialOutput(wxXmlNode* node) : SerialOutput(node) {
    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
    SetDeviceChannels(node->GetAttribute("DeviceChannels", "8"));
    _dirty = false;
}

xxxSerialOutput::xxxSerialOutput() : SerialOutput() {
    memset(_lastSent, 0x00, sizeof(_lastSent));
    memset(_notSentCount, 0x00, sizeof(_notSentCount));
    memset(_data, 0, sizeof(_data));
}
#pragma endregion

wxXmlNode* xxxSerialOutput::Save() {
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");

    node->AddAttribute("DeviceChannels", _deviceChannels);

    SerialOutput::Save(node);

    return node;
}

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

uint8_t xxxSerialOutput::PopulateBuffer(uint8_t* buffer, int32_t channel, uint8_t value) const {
    uint8_t device = GetDeviceFromChannel(channel);
    uint8_t channelOnDevice = GetChannelOnDevice(channel);
    //uint8_t deviceChannels = GetDeviceChannels(device);
    //uint32_t mask = 0x01 << channelOnDevice;

    buffer[0] = 0x80;
    buffer[1] = device + 1;
    buffer[2] = 0x10;
    buffer[3] = value;
    buffer[4] = channelOnDevice;
    buffer[5] = 0x81;
    return 6;
}

void xxxSerialOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _serial == nullptr || !_ok) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        if (_serial != nullptr) {
            for (int32_t i = 0; i < _channels; ++i)
            {
                if (_lastSent[i] != _data[i] || _notSentCount[i] > 200) {
                    _notSentCount[i] = 0;
                    uint8_t d[16];
                    uint8_t used = PopulateBuffer(d, i, _data[i]);
                    _serial->Write((char*)d, used);
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

uint8_t xxxSerialOutput::GetDeviceFromChannel(uint32_t channel) const {
    uint8_t device = 0;
    auto deviceChannels = Split(_deviceChannels, ',');
    for (const auto& it : deviceChannels) {
        int ch = std::stoi(it);
        if (channel < ch) {
            break;
        }
        channel -= ch;
        ++device;
    }
	return device;
}

uint8_t xxxSerialOutput::GetChannelOnDevice(uint32_t channel) const {
    auto deviceChannels = Split(_deviceChannels, ',');
    for (const auto& it : deviceChannels) {
        int ch = std::stoi(it);
        if (channel < ch) {
            break;
        }
        channel -= ch;
    }
    return channel;
}

uint8_t xxxSerialOutput::GetDeviceChannels(uint8_t device) const {
	auto deviceChannels = Split(_deviceChannels, ',');
    if (device < deviceChannels.size()) {
        return std::stoi(deviceChannels[device]);
	}
	return 0;
}

void xxxSerialOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled || _serial == nullptr || !_ok) return;

    // because xxx sends the channel number in the packet we can skip sending data that hasnt changed ... I think
    // Copied this from the way FPP seems to handle it - KW
    if (_lastSent[channel] != data || _notSentCount[channel] > 200) {
        _data[channel] = data;
        _notSentCount[channel] = 0;
		
        uint8_t d[16];
        uint8_t used = PopulateBuffer(d, channel, data);

        //DumpBinary(d, used);

        if (_serial != nullptr) {
            _serial->Write((char*)d, used);
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


#pragma region UI
#ifndef EXCLUDENETWORKUI
#include "ControllerEthernet.h"
#include "../models/ModelManager.h"

void xxxSerialOutput::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    SerialOutput::UpdateProperties(propertyGrid, c, modelManager, expandProperties);
    ControllerSerial* ce = dynamic_cast<ControllerSerial*>(c);

    auto p = propertyGrid->GetProperty("DeviceChannels");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(((xxxSerialOutput*)ce->GetOutputs().front())->GetDeviceChannels());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
}

void xxxSerialOutput::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    SerialOutput::AddProperties(propertyGrid, before, c, allSameSize, expandProperties);

    propertyGrid->Insert(before, new wxStringProperty("Device Channels", "DeviceChannels", GetDeviceChannels()));
}

void xxxSerialOutput::RemoveProperties(wxPropertyGrid* propertyGrid) {
    SerialOutput::RemoveProperties(propertyGrid);
    propertyGrid->DeleteProperty("DeviceChannels");
}

bool xxxSerialOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    if (SerialOutput::HandlePropertyEvent(event, outputModelManager, c)) {
        return true;
    }
    wxString const name = event.GetPropertyName();
    if (name == "DeviceChannels") {
        SetDeviceChannels(event.GetValue().GetString().ToStdString());
        wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());
        grid->GetProperty("Channels")->SetValueFromString(wxString::Format("%d", GetChannels()));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "xxxSerialOutput::HandlePropertyEvent::DeviceChannels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "xxxSerialOutput::HandlePropertyEvent::DeviceChannels", nullptr);
        return true;
    }
    return false;
}

#endif
#pragma endregion
