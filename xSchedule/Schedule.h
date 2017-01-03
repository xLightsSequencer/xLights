#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <wx/wx.h>
#include <string>

class wxWindow;
class wxXmlNode;

class Schedule
{
    std::string _name;
    std::string _dow;
    bool _dirty;
	wxDateTime _startDate;
	wxDateTime _endDate;
	wxDateTime _startTime;
	wxDateTime _endTime;
	bool _loop;
	int _loops;
	bool _random;
    bool _everyYear;
    int _priority;
	
    public:

        int GetPriority() const { return _priority; }
        void SetPriority(int priority) { _priority = priority; _dirty = true; }
        std::string GetName() const { return _name; }
        void SetName(const std::string& name) { _name = name; _dirty = true; }
        std::string GetStartTimeAsString() const { return _startTime.FormatTime().ToStdString(); }
        std::string GetEndTimeAsString() const { return _endTime.FormatTime().ToStdString(); }
        void SetStartTime(const std::string& start) { _startTime.ParseTime(start); _dirty = true; }
        void SetEndTime(const std::string& end) { _endTime.ParseTime(end); _dirty = true; }
        void SetLoops(int loops) { _loops = loops; _dirty = true; }
        int GetLoops() const { return _loops; }
        void SetLoop(bool loop) { _loop = loop; _dirty = true; }
        bool GetLoop() const { return _loop; }
        void SetRandom(bool random) { _random = random; _dirty = true; }
        bool GetRandom() const { return _random; }
        void SetEveryYear(bool everyYear) { _everyYear = everyYear; _dirty = true; }
        bool GetEveryYear() const { return _everyYear; }
        bool IsOnDOW(const std::string& dow) const;
        void SetDOW(bool mon, bool tue, bool wed, bool thu, bool fri, bool sat, bool sun);
        wxDateTime GetStartDate() const { return _startDate; }
        wxDateTime GetEndDate() const { return _endDate; }
        void SetStartDate(const wxDateTime& start) { _startDate = start; _dirty = true; }
        void SetEndDate(const wxDateTime& end) { _endDate = end; _dirty = true; }
        bool IsDirty() const { return _dirty; }
        void ClearDirty() { _dirty = false; }
        Schedule();
        Schedule(wxXmlNode* node);
        virtual ~Schedule() {}
		void Load(wxXmlNode* node);
		wxXmlNode* Save();
		Schedule* Configure(wxWindow* parent);
};

#endif 