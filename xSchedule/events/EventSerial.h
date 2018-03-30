#ifndef EVENTSERIAL_H
#define EVENTSERIAL_H

#include "EventBase.h"

class EventSerial: public EventBase
{
    protected:
        std::string _protocol;
        std::string _commPort;
        int _speed;
        int _channel;

	public:
		EventSerial();
		EventSerial(wxXmlNode* node);
		virtual ~EventSerial() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "Serial"; }
        void SetProtocol(std::string protocol) { if (_protocol != protocol) { _protocol = protocol; _changeCount++; } }
        void SetCommPort(std::string commPort) { if (_commPort != commPort) { _commPort = commPort; _changeCount++; } }
        void SetSpeed(int speed) { if (_speed != speed) { _speed = speed; _changeCount++; } }
        void SetChannel(int channel) { if (_channel != channel) { _channel = channel; _changeCount++; } }
        std::string GetProtocol() const { return _protocol; }
        std::string GetCommPort() const { return _commPort; }
        std::string GetSerialConfig() const;
        int GetSpeed() const { return _speed; }
        int GetChannel() const { return _channel; }
        virtual void Process(const std::string& commPort, wxByte* buffer, long buffersize, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

#endif
