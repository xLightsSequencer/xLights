#ifndef EVENTBASE_H
#define EVENTBASE_H

// Artnet channel
// E131 channel
// DMX Serial channel
// OSC Path
// TCPIP
// UDP
// File
// FPP Event

#include <string>
#include <wx/wx.h>

class ScheduleManager;
class wxXmlNode;

class EventBase
{
    protected:
        static int __nextId;
        int _id;
        std::string _command;
        std::string _parm1;
        std::string _parm2;
        std::string _parm3;
        std::string _name;
        int _changeCount;
        int _lastSavedChangeCount;
        std::string _condition;
        int _threshold;
        wxByte _lastValue;
        bool _first;

	public:
        virtual bool IsFrameProcess() const { return false; } // True is this event should look at the frame data to decide if it should fire
        virtual bool IsUniverseProcess() const { return false; } // True is this event should look at an integer and compare it to something like a universe
        EventBase();
		EventBase(wxXmlNode* node);
		virtual ~EventBase() {}
		void Save(wxXmlNode* node);
		virtual wxXmlNode* Save() = 0;
        int GetId() const { return _id; }
        bool operator==(const EventBase& eb) const { return _id == eb._id; }
        bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
        std::string GetName() const { return _name; }
        virtual std::string GetType() const = 0;
        virtual int GetSubType() const { return 0; }
        std::string GetCommand() const { return _command; }
        std::string GetP1() const { return _parm1; }
        std::string GetP2() const { return _parm2; }
        std::string GetP3() const { return _parm3; }
        void SetCommand(std::string command) { if (command != _command) { _command = command; _changeCount++; } }
        void SetP1(std::string value) { if (value != _parm1) { _parm1 = value; _changeCount++; } }
        void SetP2(std::string value) { if (value != _parm2) { _parm2 = value; _changeCount++; } }
        void SetP3(std::string value) { if (value != _parm3) { _parm3 = value; _changeCount++; } }
        void SetName(std::string value) { if (value != _name) { _name = value; _changeCount++; } }
        void ProcessCommand(wxByte value, ScheduleManager* scheduleManager);
        virtual void Process(wxByte* buffer, long buffersize, ScheduleManager* scheduleManager) {}
        virtual void Process(int universe, wxByte* buffer, long buffersize, ScheduleManager* scheduleManager) {}
        virtual void Process(wxByte stats, wxByte channel, wxByte data1, wxByte data2, ScheduleManager* scheduleManager) {}
        virtual void Process(const std::string& commPort, wxByte* buffer, long buffersize, ScheduleManager* scheduleManager) {}
        virtual void Process(const std::string& id, ScheduleManager* scheduleManager) {}
        virtual void Process(const std::string& path, const std::string& p1, const std::string& p2, const std::string& p3, ScheduleManager* scheduleManager) {}
        virtual void Process(bool success, const std::string& ip, ScheduleManager* scheduleManager) {}
        void SetCondition(std::string condition) { if (_condition != condition) { _condition = condition; _changeCount++; } }
        void SetThreshold(int threshold) { if (_threshold != threshold) { _threshold = threshold; _changeCount++; } }
        std::string GetCondition() const { return _condition; }
        int GetThreshold() const { return _threshold; }
        bool EvaluateCondition(wxByte value);
        static std::string DefaultParmTooltip();
};
#endif
