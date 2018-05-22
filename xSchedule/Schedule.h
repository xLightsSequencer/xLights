#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <wx/wx.h>
#include <string>

class wxWindow;
class wxXmlNode;

class Schedule
{
    static std::string __city;
    wxUint32 _id;
    std::string _name;
    std::string _dow;
    int _lastSavedChangeCount;
    int _changeCount;
	wxDateTime _startDate;
	wxDateTime _endDate;
	wxDateTime _startTime;
    std::string _startTimeString;
	wxDateTime _endTime;
    std::string _endTimeString;
	bool _loop;
	int _loops;
	bool _random;
    bool _everyYear;
    int _priority;
    bool _active;
    bool _enabled;
    int _nthDay;
    int _nthDayOffset;
    std::string _fireFrequency;

    void SetTime(wxDateTime& toset, std::string city, wxDateTime time, std::string timeString) const;
    bool IsOkDOW(const wxDateTime& date);
    bool IsOkNthDay(const wxDateTime& date);
    bool CheckActiveAt(const wxDateTime& now);

    public:

        static void Test();

        static void SetCity(std::string city) { __city = city; }
        wxUint32 GetId() const { return _id; }
        bool operator<(const Schedule& rhs) const { return _priority < rhs._priority; }
        bool operator==(const Schedule& rhs) const { return _id == rhs._id; }
        int GetPriority() const { return _priority; }
        void SetPriority(int priority) { if (_priority != priority) { _priority = priority; _changeCount++; } }
        std::string GetName() const { return _name; }
        void SetName(const std::string& name) { if (_name != name) { _name = name; _changeCount++; } }
        std::string GetStartTimeAsString() const { if (_startTimeString == "") return _startTime.FormatTime().ToStdString(); else return _startTimeString; }
        std::string GetEndTimeAsString() const { if (_endTimeString == "") return _endTime.FormatTime().ToStdString(); else return _endTimeString; }
        void SetStartTime(const std::string& start);
        void SetEndTime(const std::string& end);
        void SetLoops(int loops) { if (_loops != loops) { _loops = loops; _changeCount++; } }
        int GetLoops() const { return _loops; }
        void SetLoop(bool loop) { if (_loop != loop) { _loop = loop; _changeCount++; } }
        int GetNthDay() const { return _nthDay; }
        void SetNthDay(int nthDay) { if (_nthDay != nthDay) { _nthDay = nthDay; _changeCount++; } }
        int GetNthDayOffset() const { return _nthDayOffset; }
        void SetNthDayOffset(int nthDayOffset) { if (_nthDayOffset != nthDayOffset) { _nthDayOffset = nthDayOffset; _changeCount++; } }
        void SetEnabled(bool enabled) { if (_enabled != enabled) { _enabled = enabled; _changeCount++; } }
        bool GetLoop() const { return _loop; }
        bool GetEnabled() const { return _enabled; }
        void SetRandom(bool random) { if (_random != random) { _random = random; _changeCount++; } }
        bool GetRandom() const { return _random; }
        void SetFireFrequency(std::string fireFrequency) { if (_fireFrequency != fireFrequency) { _fireFrequency = fireFrequency; _changeCount++; } }
        std::string GetFireFrequency() const { return _fireFrequency; }
        void SetEveryYear(bool everyYear) { if (_everyYear != everyYear) { _everyYear = everyYear; _changeCount++; } }
        bool GetEveryYear() const { return _everyYear; }
        bool IsOnDOW(const std::string& dow) const;
        void SetDOW(bool mon, bool tue, bool wed, bool thu, bool fri, bool sat, bool sun);
        wxDateTime GetStartDate() const { return _startDate; }
        wxDateTime GetEndDate() const { return _endDate; }
        void SetStartDate(const wxDateTime& start) { if (_startDate != start) { _startDate = start; _changeCount++; } }
        void SetEndDate(const wxDateTime& end) { if (_endDate != end) { _endDate = end; _changeCount++; } }
        bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
        std::string GetJSON(const std::string& reference);
        Schedule();
        Schedule(wxXmlNode* node);
        Schedule(const Schedule& schedule, bool newid = false);
        virtual ~Schedule() {}
		void Load(wxXmlNode* node);
		wxXmlNode* Save();
		Schedule* Configure(wxWindow* parent);
        bool IsActive() const { return _active; }
        bool CheckActive();
        std::string GetNextTriggerTime();
        std::string GetNextEndTime();
        void AddMinsToEndTime(int mins);
        wxDateTime GetNextTriggerDateTime();
        static std::string GetNextNthDay(int nthDay, int nthDayOffset);
        bool ShouldFire() const;
};

#endif
