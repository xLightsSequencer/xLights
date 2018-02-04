#ifndef EVENTMIDI_H
#define EVENTMIDI_H

#include "EventBase.h"
#include <list>

class EventMIDI: public EventBase
{
    std::string _device;
    std::string _status;
    std::string _channel;
    std::string _data1;
    int _statusByte;
    int _channelByte;
    int _data1Byte;

    void DoSetStatus(std::string status);
    void DoSetChannel(std::string channel);
    void DoSetData1(std::string data1);

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
        void SetData1(std::string data1) { if (_data1 != data1) { DoSetData1(data1); _changeCount++; } }
        std::string GetData1() const { return _data1; }
        void SetChannel(std::string channel) { if (_channel != channel) { DoSetChannel(channel); _changeCount++; } }
        std::string GetChannel() const { return _channel; }
        virtual void Process(wxByte status, wxByte channel, wxByte data1, wxByte data2, ScheduleManager* scheduleManager);
        static std::list<std::string> GetDevices();
        int GetDeviceId() const;
        void ProcessMIDICommand(wxByte data1, wxByte data2, ScheduleManager* scheduleManager);
        static std::string GetParmToolTip();
        bool IsAnyChannel() const { return _channel == "ANY"; }
        bool IsAnyData1() const { return _data1 == "ANY"; }
        wxByte GetStatusByte() const { return _statusByte; }
        wxByte GetChannelByte() const { return _channelByte; }
        wxByte GetData1Byte() const { return _data1Byte; }
};

#endif
