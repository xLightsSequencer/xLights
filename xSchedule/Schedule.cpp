#include "Schedule.h"
#include "ScheduleDialog.h"
#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "City.h"

int __scheduleid = 0;
std::string Schedule::__city = "";

Schedule::Schedule(wxXmlNode* node)
{
    _id = __scheduleid++;
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    Schedule::Load(node);
}

void Schedule::Load(wxXmlNode* node)
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;

    _name = node->GetAttribute("Name", "<unnamed>");
    _dow = node->GetAttribute("DOW", "MonTueWedThuFriSatSun");
    _startDate.ParseDate(node->GetAttribute("StartDate", "01/01/2017"));
    _endDate.ParseDate(node->GetAttribute("EndDate", "01/01/2099"));
    _startTimeString = node->GetAttribute("StartTime", "19:00").Lower();
    if (_startTimeString == "sunrise" || _startTimeString == "sunset")
    {
        _startTime.ParseTime("0:00");
    }
    else
    {
        _startTime.ParseTime(_startTimeString);
        _startTimeString = "";
    }
    _endTimeString = node->GetAttribute("EndTime", "22:00").Lower();
    if (_endTimeString == "sunrise" || _endTimeString == "sunset")
    {
        _endTime.ParseTime("0:00");
    }
    else
    {
        _endTime.ParseTime(_endTimeString);
        _endTimeString = "";
    }
    _loop = node->GetAttribute("Loop", "FALSE") == "TRUE";
    _random = node->GetAttribute("Random", "FALSE") == "TRUE";
    _everyYear = node->GetAttribute("EveryYear", "FALSE") == "TRUE";
    _enabled = node->GetAttribute("Enabled", "FALSE") == "TRUE";
    _loops = wxAtoi(node->GetAttribute("Loops", "0"));
    _priority = wxAtoi(node->GetAttribute("Priority", "0"));
    _nthDay = wxAtoi(node->GetAttribute("NthDay", "1"));
    _nthDayOffset = wxAtoi(node->GetAttribute("NthDayOffset", "0"));
    _fireFrequency = node->GetAttribute("FireFrequency", "Fire once");
}

wxXmlNode* Schedule::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Schedule");

    node->AddAttribute("Name", _name);
    node->AddAttribute("DOW", _dow);
    node->AddAttribute("StartDate", _startDate.Format("%Y-%m-%d"));
    node->AddAttribute("EndDate", _endDate.Format("%Y-%m-%d"));
    node->AddAttribute("StartTime", GetStartTimeAsString());
    node->AddAttribute("EndTime", GetEndTimeAsString());
    node->AddAttribute("Priority", wxString::Format(wxT("%i"), _priority));
    node->AddAttribute("NthDay", wxString::Format(wxT("%i"), _nthDay));
    node->AddAttribute("NthDayOffset", wxString::Format(wxT("%i"), _nthDayOffset));
    node->AddAttribute("FireFrequency", _fireFrequency);
    if (_loop)
    {
        node->AddAttribute("Loop", "TRUE");
        node->AddAttribute("Loops", wxString::Format(wxT("%i"), _loops));
    }
    if (_random)
    {
        node->AddAttribute("Random", "TRUE");
    }
    if (_enabled)
    {
        node->AddAttribute("Enabled", "TRUE");
    }
    if (_everyYear)
    {
        node->AddAttribute("EveryYear", "TRUE");
    }

    return node;
}

Schedule::Schedule()
{
    _active = false;
    _enabled = true;
    _id = __scheduleid++;
    _changeCount = 1;
    _lastSavedChangeCount = 0;
    _loop = true;
    _endDate.ParseDate("2099-01-01");
    _startDate.ParseDate("2017-01-01");
    _startTime.ParseTime("19:00");
    _endTime.ParseTime("22:00");
    _startTimeString = "";
    _endTimeString = "";
    _name = "<unnamed>";
    _random = false;
    _loops = 0;
    _everyYear = false;
    _priority = 5;
    _dow = "MonTueWedThuFriSatSun";
    _nthDay = 1;
    _nthDayOffset = 0;
    _fireFrequency = "Fire once";
}

std::string Schedule::GetJSON(const std::string& reference)
{
    std::string res = "{\"name\":\"" + _name +
        "\",\"id\":\"" + wxString::Format(wxT("%i"), _id).ToStdString() +
        "\",\"enabled\":\"" + std::string(_enabled ? "TRUE" : "FALSE") +
        "\",\"active\":\"" + std::string(CheckActive() ? "TRUE" : "FALSE") +
        "\",\"looping\":\"" + std::string(_loop ? "TRUE" : "FALSE") +
        "\",\"loops\":\"" + wxString::Format(wxT("%i"), _loops).ToStdString() +
        "\",\"random\":\"" + std::string(_random ? "TRUE" : "FALSE") +
        "\",\"nextactive\":\"" + GetNextTriggerTime();
 
    if (reference != "")
    {
        res += "\",\"reference\":\"" + reference;
    }

    res += "\",\"scheduleend\":\"" + (CheckActive() ? GetNextEndTime() : "N/A") +
        "\"}";

    return res;
}

Schedule::Schedule(const Schedule& schedule, bool newid)
{
    _active = schedule._active;
    _enabled = schedule._enabled;
    if (newid)
    {
        _id = __scheduleid++;
    }
    else
    {
        _id = schedule._id;
    }
    _changeCount = schedule._changeCount;
    _lastSavedChangeCount = schedule._lastSavedChangeCount;
    _loop = schedule._loop;
    _endDate = schedule._endDate;
    _startDate = schedule._startDate;
    _startTimeString = schedule._startTimeString;
    _startTime = schedule._startTime;
    _endTime = schedule._endTime;
    _endTimeString = schedule._endTimeString;
    _name = schedule._name;
    _random = schedule._random;
    _loops = schedule._loops;
    _everyYear = schedule._everyYear;
    _priority = schedule._priority;
    _dow = schedule._dow;
    _nthDay = schedule._nthDay;
    _nthDayOffset = schedule._nthDayOffset;
    _fireFrequency = schedule._fireFrequency;
}

Schedule* Schedule::Configure(wxWindow* parent)
{
    ScheduleDialog dlg(parent, this);

    if (dlg.ShowModal() == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}

bool Schedule::IsOnDOW(const std::string& dow) const
{
    return wxString(_dow).Contains(dow);
}

void Schedule::SetDOW(bool mon, bool tue, bool wed, bool thu, bool fri, bool sat, bool sun)
{
    _dow = "";
    if (mon) _dow += "Mon";
    if (tue) _dow += "Tue";
    if (wed) _dow += "Wed";
    if (thu) _dow += "Thu";
    if (fri) _dow += "Fri";
    if (sat) _dow += "Sat";
    if (sun) _dow += "Sun";
}

void Schedule::SetTime(wxDateTime& toset, std::string city, wxDateTime time, std::string timeString) const
{
    if (timeString == "sunrise")
    {
        City* c = City::GetCity(city);
        wxDateTime sunrise = c->GetSunrise(toset);
        toset.SetHour(sunrise.GetHour());
        toset.SetMinute(sunrise.GetMinute());
    }
    else if (timeString == "sunset")
    {
        City* c = City::GetCity(city);
        wxDateTime sunset = c->GetSunset(toset);
        toset.SetHour(sunset.GetHour());
        toset.SetMinute(sunset.GetMinute());
    }
    else
    {
        toset.SetHour(time.GetHour());
        toset.SetMinute(time.GetMinute());
    }
}

void Schedule::Test()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.warn("Running Schedule tests.");

    Schedule s;

    s._startDate = wxDateTime(5, (wxDateTime::Month)11, 2016);
    s._endDate = wxDateTime(12, (wxDateTime::Month)0, 2017);
    s._startTime = wxDateTime((wxDateTime::wxDateTime_t)22);
    s._endTime = wxDateTime((wxDateTime::wxDateTime_t)2);
    s._dow = "MonTue";
    wxASSERT(s.IsOkDOW(wxDateTime(9, (wxDateTime::Month)0, 2017)));
    wxASSERT(s.IsOkDOW(wxDateTime(10, (wxDateTime::Month)0, 2017)));
    wxASSERT(!s.IsOkDOW(wxDateTime(11, (wxDateTime::Month)0, 2017)));
    wxASSERT(!s.IsOkDOW(wxDateTime(12, (wxDateTime::Month)0, 2017)));

    wxASSERT(!s.CheckActiveAt(wxDateTime(5, (wxDateTime::Month)11, 2016, 21, 0)));
    wxASSERT(s.CheckActiveAt(wxDateTime(5, (wxDateTime::Month)11, 2016, 22, 0)));
    wxASSERT(s.CheckActiveAt(wxDateTime(6, (wxDateTime::Month)11, 2016, 1, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(6, (wxDateTime::Month)11, 2016, 3, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(16, (wxDateTime::Month)0, 2017, 23, 0)));

    s._startDate = wxDateTime(26, (wxDateTime::Month)11, 2017);
    s._endDate = wxDateTime(1, (wxDateTime::Month)0, 2018);
    s._startTime = wxDateTime((wxDateTime::wxDateTime_t)19);
    s._endTime = wxDateTime((wxDateTime::wxDateTime_t)22);
    s._dow = "MonTueWedThuFriSatSun";
    wxASSERT(s.IsOkDOW(wxDateTime(1, (wxDateTime::Month)0, 2018)));
    wxASSERT(s.CheckActiveAt(wxDateTime(1, (wxDateTime::Month)0, 2018, 19, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(1, (wxDateTime::Month)0, 2019, 19, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(2, (wxDateTime::Month)0, 2018, 19, 0)));
    wxASSERT(s.CheckActiveAt(wxDateTime(26, (wxDateTime::Month)11, 2017, 19, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(26, (wxDateTime::Month)11, 2018, 19, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(25, (wxDateTime::Month)11, 2017, 19, 0)));

    s._startTime = wxDateTime((wxDateTime::wxDateTime_t)19);
    s._endTime = wxDateTime((wxDateTime::wxDateTime_t)19);
    wxASSERT(!s.CheckActiveAt(wxDateTime(26, (wxDateTime::Month)11, 2017, 16, 0)));
    wxASSERT(s.CheckActiveAt(wxDateTime(26, (wxDateTime::Month)11, 2017, 19, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(26, (wxDateTime::Month)11, 2018, 19, 0)));
    wxASSERT(s.CheckActiveAt(wxDateTime(27, (wxDateTime::Month)11, 2017, 1, 0)));
    wxASSERT(s.CheckActiveAt(wxDateTime(1, (wxDateTime::Month)0, 2018, 1, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(1, (wxDateTime::Month)0, 2019, 1, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(1, (wxDateTime::Month)0, 2018, 20, 0)));

    s._everyYear = true;
    wxASSERT(s.CheckActiveAt(wxDateTime(1, (wxDateTime::Month)0, 2019, 1, 0)));
    wxASSERT(s.CheckActiveAt(wxDateTime(26, (wxDateTime::Month)11, 2019, 19, 0)));
    wxASSERT(!s.CheckActiveAt(wxDateTime(2, (wxDateTime::Month)0, 2018, 19, 0)));

    logger_base.warn("    Schedule tests done.");
}

bool Schedule::IsOkDOW(const wxDateTime& date)
{
    return (_dow.find(wxDateTime::GetEnglishWeekDayName(date.GetWeekDay(), wxDateTime::Name_Abbr).ToStdString()) != std::string::npos);
}

bool Schedule::CheckActive()
{
    return CheckActiveAt(wxDateTime::Now());
}

bool Schedule::ShouldFire() const
{
    int minute = wxDateTime::Now().GetMinute();

    if (_fireFrequency == "Fire every hour")
    {
        if (minute == 0) return true;
        return false;
    }
    else if (_fireFrequency == "Fire every 30 minutes")
    {
        if (minute == 0 || minute == 30) return true;
        return false;
    }
    else if (_fireFrequency == "Fire every 20 minutes")
    {
        if (minute == 0 || minute == 20 || minute == 40) return true;
        return false;
    }
    else if (_fireFrequency == "Fire every 15 minutes")
    {
        if (minute == 0 || minute == 15 || minute == 30 || minute == 45) return true;
        return false;
    }
    else if (_fireFrequency == "Fire every 10 minutes")
    {
        if (minute == 0 || minute == 10 || minute == 20 || minute == 30 || minute == 40 || minute == 50) return true;
        return false;
    }

    return true;
}

//#define LOGCALCNEXTTRIGGERTIME

bool Schedule::IsOkNthDay(const wxDateTime& date)
{
    return ((date.GetDayOfYear() % _nthDay) - _nthDayOffset == 0);
}

void Schedule::SetStartTime(const std::string& start)
{
    wxString s = start;
    s = s.Lower();

    if (s != GetStartTimeAsString())
    {
        if (s == "sunrise" || s == "sunset")
        {
            _startTimeString = s;
        }
        else
        {
            _startTimeString = "";
            _startTime.ParseTime(s);
        }
        _changeCount++;
    }
}

void Schedule::SetEndTime(const std::string& end)
{
    wxString e = end;
    e = e.Lower();

    if (e != GetEndTimeAsString())
    {
        if (e == "sunrise" || e == "sunset")
        {
            _endTimeString = e;
        }
        else
        {
            _endTimeString = "";
            _endTime.ParseTime(e);
        }
        _changeCount++;
    }
}

bool Schedule::CheckActiveAt(const wxDateTime& now)
{
#ifdef LOGCALCNEXTTRIGGERTIME
    logger_base.debug("   Checking %s.", (const char *)now.Format("%Y-%m-%d %H:%M").c_str());
#endif

    if (!_enabled || !IsOkDOW(now) || !IsOkNthDay(now))
    {
#ifdef LOGCALCNEXTTRIGGERTIME
        logger_base.debug("       Disabled or wrong day of week.");
#endif

        _active = false;
        return _active;
    }

    wxDateTime start = _startDate;
    wxDateTime end = _endDate;

    if (_everyYear)
    {
        wxDateTime n = now;
        n.SetHour(0);
        n.SetMinute(0);
        n.SetSecond(0);

        start.SetYear(n.GetYear());
        end.SetYear(n.GetYear());

        if (start > n)
        {
            start.SetYear(n.GetYear() - 1);
            end.SetYear(n.GetYear() - 1);
        }

        if (start > end)
        {
            end = end.SetYear(end.GetYear() + 1);
        }
    }

    wxDateTime s = now;
    wxDateTime e = now;

    SetTime(s, __city, _startTime, _startTimeString);
    SetTime(e, __city, _endTime, _endTimeString);

    start.SetHour(s.GetHour());
    start.SetMinute(s.GetMinute());
    end.SetHour(e.GetHour());
    end.SetMinute(e.GetMinute());

#ifdef LOGCALCNEXTTRIGGERTIME
    logger_base.debug("       Now %s. Start %s. End %s", (const char *)now.Format("%Y-%m-%d %H:%M").c_str(), (const char *)start.Format("%Y-%m-%d %H:%M").c_str(), (const char *)end.Format("%Y-%m-%d %H:%M").c_str());
#endif

    if (e < s)
    {
        end.Add(wxTimeSpan(24));
    }

    // handle the 24 hours a day case
    if (s == e)
    {
        _active = now >= start && now <= end;

#ifdef LOGCALCNEXTTRIGGERTIME
        if (!_active) logger_base.debug("       24 hrs a day but not within dates. %s-%s", (const char *)start.Format("%Y-%m-%d %H:%M").c_str(), (const char *)end.Format("%Y-%m-%d %H:%M").c_str());
#endif

        return _active;
    }

    if (now >= start && now <= end)
    {
        // dates are ok ... now check the times
        start.SetYear(now.GetYear());
        start.SetMonth(now.GetMonth());
        start.SetDay(now.GetDay());
        end.SetYear(now.GetYear());
        end.SetMonth(now.GetMonth());
        end.SetDay(now.GetDay());

        if (e < s && now.FormatISOTime() < end.FormatISOTime())
        {
            start.Add(wxTimeSpan(-24));
        }
        else if (e < s)
        {
            end.Add(wxTimeSpan(24));
        }

        _active = now >= start && now <= end;

#ifdef LOGCALCNEXTTRIGGERTIME
        if (!_active) logger_base.debug("       Valid dates but not at this time %s-%s.", (const char *)start.Format("%Y-%m-%d %H:%M").c_str(), (const char *)end.Format("%Y-%m-%d %H:%M").c_str());
#endif
    }
    else
    {
#ifdef LOGCALCNEXTTRIGGERTIME
        logger_base.debug("       Not valid on this date %s-%s.", (const char *)start.Format("%Y-%m-%d %H:%M").c_str(), (const char *)end.Format("%Y-%m-%d %H:%M").c_str());
#endif
        // outside date range
        _active = false;
        return _active;
    }

    return _active;
}

wxDateTime Schedule::GetNextTriggerDateTime()
{
    wxDateTime end = _endDate;
    if (_everyYear) end.SetYear(wxDateTime::Now().GetYear() + 1);

    // deal with the simple cases
    if (CheckActive()) return wxDateTime::Now();
    if (end < wxDateTime::Now()) return wxDateTime((time_t)0);

    if (_startDate > wxDateTime::Now()) // tomorrow or later
    {
        // some time in the future
        wxDateTime next = _startDate;

        SetTime(next, __city, _startTime, _startTimeString);
        if (next > wxDateTime::Now() && CheckActiveAt(next))
        {
            return next;
        }

        for (int i = 0; i < 7; i++)
        {
            next += wxTimeSpan(24);
            if (next > wxDateTime::Now() && CheckActiveAt(next))
            {
                return next;
            }
        }

        return wxDateTime((time_t)0);
    }

    // so now is between _startDate and end

    // check if the right answer is the starttime today
    wxDateTime next = wxDateTime::Now();

    SetTime(next, __city, _startTime, _startTimeString);
    next.SetSecond(0);
    if (next > wxDateTime::Now() && CheckActiveAt(next))
    {
        return next;
    }

    for (int i = 0; i < 7; i++)
    {
        next += wxTimeSpan(24);
        if (next > wxDateTime::Now() && CheckActiveAt(next))
        {
            return next;
        }
    }

    // I give up
    return wxDateTime((time_t)0);
}

std::string Schedule::GetNextNthDay(int nthDay, int nthDayOffset)
{
    wxDateTime now = wxDateTime::Now();
    wxTimeSpan day(24);

    for (int i = 0; i < 15; i++)
    {
        if ((now.GetDayOfYear() % nthDay) - nthDayOffset == 0)
        {
            return now.FormatISODate().ToStdString();
        }
        now.Add(day);
    }

    return "Unknown";
}

std::string Schedule::GetNextTriggerTime()
{
#ifdef LOGCALCNEXTTRIGGERTIME
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#endif

    wxDateTime end = _endDate;
    if (_everyYear) end.SetYear(wxDateTime::Now().GetYear() + 1);

    // deal with the simple cases
    if (CheckActive()) return "NOW!";
    if (end < wxDateTime::Now()) return "Never";

    if (_startDate > wxDateTime::Now()) // tomorrow or later
    {
        // some time in the future
        wxDateTime next = _startDate;

        SetTime(next, __city, _startTime, _startTimeString);

#ifdef LOGCALCNEXTTRIGGERTIME
        logger_base.debug("Checking %s.", (const char *)next.Format("%Y-%m-%d %H:%M").c_str());
#endif

        if (next > wxDateTime::Now() && CheckActiveAt(next))
        {
            return next.Format("%Y-%m-%d %H:%M").ToStdString();
        }

        for (int i = 0; i < 7; i++)
        {
            next += wxTimeSpan(24);
#ifdef LOGCALCNEXTTRIGGERTIME
            logger_base.debug("Checking %s.", (const char *)next.Format("%Y-%m-%d %H:%M").c_str());
#endif
            if (next > wxDateTime::Now() && CheckActiveAt(next))
            {
                return next.Format("%Y-%m-%d %H:%M").ToStdString();
            }
        }

        return "Never";
    }

    // so now is between _startDate and end

    // check if the right answer is the starttime today
    wxDateTime next = wxDateTime::Now();

    SetTime(next, __city, _startTime, _startTimeString);

    next.SetSecond(0);
#ifdef LOGCALCNEXTTRIGGERTIME
    logger_base.debug("Checking %s.", (const char *)next.Format("%Y-%m-%d %H:%M").c_str());
#endif
    if (next > wxDateTime::Now() && CheckActiveAt(next))
    {
        return next.Format("%Y-%m-%d %H:%M").ToStdString();
    }

    for (int i = 0; i < 7; i++)
    {
        next += wxTimeSpan(24);
        SetTime(next, __city, _startTime, _startTimeString);
#ifdef LOGCALCNEXTTRIGGERTIME
        logger_base.debug("Checking %s.", (const char *)next.Format("%Y-%m-%d %H:%M").c_str());
#endif

        if (next > wxDateTime::Now() && CheckActiveAt(next))
        {
            return next.Format("%Y-%m-%d %H:%M").ToStdString();
        }
    }

    if (_everyYear)
    {
        next = _startDate;
        next.SetYear(wxDateTime::Now().GetYear());

        SetTime(next, __city, _startTime, _startTimeString);

        if (next < wxDateTime::Now())
        {
            next.SetYear(next.GetYear() + 1);
        }

#ifdef LOGCALCNEXTTRIGGERTIME
        logger_base.debug("Checking %s.", (const char *)next.Format("%Y-%m-%d %H:%M").c_str());
#endif

        if (CheckActiveAt(next))
        {
            return next.Format("%Y-%m-%d %H:%M").ToStdString();
        }
    }

    // I give up
    if (_everyYear)
    {
        return "A long time from now";
    }

    return "Never";
}

void Schedule::AddMinsToEndTime(int mins)
{
    _endTime += wxTimeSpan(0, mins);
}

std::string Schedule::GetNextEndTime()
{
    if (!_active) return "N/A";

    wxDateTime e1 = wxDateTime::Now();
    SetTime(e1, __city, _endTime, _endTimeString);
    wxDateTime s1 = wxDateTime::Now();
    SetTime(s1, __city, _startTime, _startTimeString);

    // when end and start are the same we play 24 hours a day
    if (s1 == e1)
    {
        wxDateTime end = _endDate;

        SetTime(end, __city, _endTime, _endTimeString);
        end.SetSecond(0);

        return end.Format("%Y-%m-%d %H:%M").ToStdString();
    }
    else
    {
        wxDateTime end = wxDateTime::Now();
        SetTime(end, __city, _endTime, _endTimeString);
        wxDateTime start = wxDateTime::Now();
        SetTime(start, __city, _startTime, _startTimeString);

        if (end < start)
        {
            end += wxTimeSpan(24);
        }

        SetTime(end, __city, _endTime, _endTimeString);
        end.SetSecond(0);

        return end.Format("%Y-%m-%d %H:%M").ToStdString();
    }
}
