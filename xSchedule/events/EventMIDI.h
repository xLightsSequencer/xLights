#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventBase.h"
#include <list>

class EventMIDI: public EventBase
{
    std::string _device;
    std::string _status;
    std::string _channel;
    int _statusByte;
    int _channelByte;

    std::string _testData1 = "Any";
    std::string _testData2 = "Any";
    int _data1 = 0;
    int _data2 = 0;
    uint8_t _lastData1 = -1;
    uint8_t _lastData2 = -1;

    void DoSetStatus(std::string status);
    void DoSetChannel(std::string channel);
    bool IsDataMatch(uint8_t value, const std::string& testData, int data, uint8_t lastValue);

	public:
        EventMIDI();
		EventMIDI(wxXmlNode* node);
		virtual ~EventMIDI() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "MIDI"; }

        void SetDevice(std::string device) { if (_device != device) { _device = device; _changeCount++; } }
        std::string GetDevice() const { return _device; }

        void SetStatus(std::string status) { if (_status != status) { DoSetStatus(status); _changeCount++; } }
        std::string GetStatus() const { return _status; }

        void SetTestData1(std::string testData) { if (_testData1 != testData) { _testData1 = testData; _changeCount++; } }
        void SetTestData2(std::string testData) { if (_testData2 != testData) { _testData2 = testData; _changeCount++; } }
        void SetData1(int data) { if (_data1 != data) { _data1 = data; _changeCount++; } }
        void SetData2(int data) { if (_data2 != data) { _data2 = data; _changeCount++; } }

        std::string GetTestData1() const { return _testData1; }
        std::string GetTestData2() const { return _testData2; }
        int GetData1() const { return _data1; }
        int GetData2() const { return _data2; }

        void SetChannel(std::string channel) { if (_channel != channel) { DoSetChannel(channel); _changeCount++; } }
        std::string GetChannel() const { return _channel; }

        void Process(uint8_t status, uint8_t channel, uint8_t data1, uint8_t data2, ScheduleManager* scheduleManager) override;

        static std::list<std::string> GetDevices();
        static std::list<std::string> GetOutputDevices();
        int GetDeviceId() const;
        void ProcessMIDICommand(uint8_t data1, uint8_t data2, ScheduleManager* scheduleManager);
        static std::string GetParmToolTip();
        bool IsAnyChannel() const { return _channel == "ANY"; }

        uint8_t GetStatusByte() const { return _statusByte; }
        uint8_t GetChannelByte() const { return _channelByte; }
};

