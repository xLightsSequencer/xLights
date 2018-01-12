#ifndef EVENTARTNET_H
#define EVENTARTNET_H

#include "EventBase.h"

class EventARTNet: public EventBase
{
    int _universe;
    int _channel;

	public:
        virtual bool IsUniverseProcess() const override { return true; }
        EventARTNet();
		EventARTNet(wxXmlNode* node);
		virtual ~EventARTNet() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "ARTNet"; }
        void SetUniverse(int universe) { if (_universe != universe) { _universe = universe; _changeCount++; } }
        void SetChannel(int channel) { if (_channel != channel) { _channel = channel; _changeCount++; } }
        int GetUniverse() const { return _universe; }
        int GetChannel() const { return _channel; }
        virtual void Process(int universe, wxByte* buffer, long buffersize, ScheduleManager* scheduleManager) override;
};

#endif
