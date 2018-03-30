#ifndef EVENTOSC_H
#define EVENTOSC_H

#include "EventBase.h"

class EventOSC: public EventBase
{
    std::string _path;
    std::string _oscParm1Type;
    std::string _oscParm2Type;
    std::string _oscParm3Type;

    public:
		EventOSC();
		EventOSC(wxXmlNode* node);
		virtual ~EventOSC() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "OSC"; }
        void SetPath(std::string path) { if (_path != path) { _path = path; _changeCount++; } }
        void SetOSCParm1Type(std::string oscParm1Type) { if (_oscParm1Type != oscParm1Type) { _oscParm1Type = oscParm1Type; _changeCount++; } }
        void SetOSCParm2Type(std::string oscParm2Type) { if (_oscParm2Type != oscParm2Type) { _oscParm2Type = oscParm2Type; _changeCount++; } }
        void SetOSCParm3Type(std::string oscParm3Type) { if (_oscParm3Type != oscParm3Type) { _oscParm3Type = oscParm3Type; _changeCount++; } }
        std::string GetPath() const { return _path; }
        std::string GetParm1Type() const { return _oscParm1Type; }
        std::string GetParm2Type() const { return _oscParm2Type; }
        std::string GetParm3Type() const { return _oscParm3Type; }
        virtual void Process(const std::string& path, const std::string& p1, const std::string& p2, const std::string& p3, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

#endif
