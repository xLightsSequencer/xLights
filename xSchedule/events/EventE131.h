#ifndef EVENTE131_H
#define EVENTE131_H

#include "EventBase.h"

class EventE131: public EventBase
{
    int _universe;
    int _channel;

	public:
        virtual bool IsUniverseProcess() const override { return true; }
        EventE131();
		EventE131(wxXmlNode* node);
		virtual ~EventE131() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "E131"; }
        void SetUniverse(int universe) { if (_universe != universe) { _universe = universe; _changeCount++; } }
        void SetChannel(int channel) { if (_channel != channel) { _channel = channel; _changeCount++; } }
        int GetUniverse() const { return _universe; }
        int GetChannel() const { return _channel; }
        virtual void Process(int universe, uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

#endif
