#include "Schedule.h"
#include "ScheduleDialog.h"
#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

int __scheduleid = 0;

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
    _startTime.ParseTime(node->GetAttribute("StartTime", "19:00"));
    _endTime.ParseTime(node->GetAttribute("EndTime", "22:00"));
    _loop = node->GetAttribute("Loop", "FALSE") == "TRUE";
    _random = node->GetAttribute("Random", "FALSE") == "TRUE";
    _everyYear = node->GetAttribute("EveryYear", "FALSE") == "TRUE";
    _enabled = node->GetAttribute("Enabled", "FALSE") == "TRUE";
    _loops = wxAtoi(node->GetAttribute("Loops", "0"));
    _priority = wxAtoi(node->GetAttribute("Priority", "0"));
}

wxXmlNode* Schedule::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Schedule");

    node->AddAttribute("Name", _name);
    node->AddAttribute("DOW", _dow);
    node->AddAttribute("StartDate", _startDate.Format("%Y-%m-%d"));
    node->AddAttribute("EndDate", _endDate.Format("%Y-%m-%d"));
    node->AddAttribute("StartTime", _startTime.FormatTime());
    node->AddAttribute("EndTime", _endTime.FormatTime());
    node->AddAttribute("Priority", wxString::Format(wxT("%i"), _priority));
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
    _loop = false;
    _endDate.ParseDate("2099-01-01");
    _startDate.ParseDate("2017-01-01");
    _startTime.ParseTime("19:00");
    _endTime.ParseTime("22:00");
    _name = "<unnamed>";
    _random = false;
    _loops = 0;
    _everyYear = false;
    _priority = 5;
    _dow = "MonTueWedThuFriSatSun";
}

std::string Schedule::GetJSON(const std::string& reference)
{
    std::string res;

    res = "{\"name\":\"" + _name +
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
    _startTime = schedule._startTime;
    _endTime = schedule._endTime;
    _name = schedule._name;
    _random = schedule._random;
    _loops = schedule._loops;
    _everyYear = schedule._everyYear;
    _priority = schedule._priority;
    _dow = schedule._dow;
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

void Schedule::Test()
{
    _startDate = wxDateTime(5, (wxDateTime::Month)11, 2016);
    _endDate = wxDateTime(12, (wxDateTime::Month)0, 2017);
    _startTime = wxDateTime((wxDateTime::wxDateTime_t)22);
    _endTime = wxDateTime((wxDateTime::wxDateTime_t)2);
    _dow = "MonTue";
    wxASSERT(IsOkDOW(wxDateTime(9, (wxDateTime::Month)0, 2017)));
    wxASSERT(IsOkDOW(wxDateTime(10, (wxDateTime::Month)0, 2017)));
    wxASSERT(!IsOkDOW(wxDateTime(11, (wxDateTime::Month)0, 2017)));
    wxASSERT(!IsOkDOW(wxDateTime(12, (wxDateTime::Month)0, 2017)));

    wxASSERT(!CheckActiveAt(wxDateTime(5, (wxDateTime::Month)11, 2016, 21, 0)));
    wxASSERT(CheckActiveAt(wxDateTime(5, (wxDateTime::Month)11, 2016, 22, 0)));
    wxASSERT(CheckActiveAt(wxDateTime(6, (wxDateTime::Month)11, 2016, 1, 0)));
    wxASSERT(!CheckActiveAt(wxDateTime(6, (wxDateTime::Month)11, 2016, 3, 0)));
    wxASSERT(!CheckActiveAt(wxDateTime(16, (wxDateTime::Month)0, 2017, 23, 0)));
}

bool Schedule::IsOkDOW(const wxDateTime& date)
{
    return (_dow.find(wxDateTime::GetEnglishWeekDayName(date.GetWeekDay(), wxDateTime::Name_Abbr).ToStdString()) != std::string::npos);
}

bool Schedule::CheckActive()
{
    return CheckActiveAt(wxDateTime::Now());
}

bool Schedule::CheckActiveAt(const wxDateTime& now)
{
    if (!_enabled || !IsOkDOW(now))
    {
        _active = false;
        return _active;
    }

    wxDateTime start = _startDate;
    wxDateTime end = _endDate;

    if (_everyYear)
    {
        if (start < end)
        {
            end = end.SetYear(end.GetYear() + 1);
        }
    }

    start.SetHour(_startTime.GetHour());
    start.SetMinute(_startTime.GetMinute());
    end.SetHour(_endTime.GetHour());
    end.SetMinute(_endTime.GetMinute());

    if (_endTime < _startTime)
    {
        end.Add(wxTimeSpan(24));
    }

    start.SetYear(now.GetYear());
    end.SetYear(now.GetYear() + end.GetYear() - start.GetYear());

    if (now >= start && now <= end)
    {
        // dates are ok ... now check the times
        start.SetDay(now.GetDay());
        start.SetMonth(now.GetMonth());
        start.SetYear(now.GetYear());
        end = start;
        end.SetHour(_endTime.GetHour());
        end.SetMinute(_endTime.GetMinute());

        if (_endTime < _startTime && now.FormatISOTime() < end.FormatISOTime())
        {
            start.Add(wxTimeSpan(-24));
        }
        else if (_endTime < _startTime)
        {
            end.Add(wxTimeSpan(24));
        }

        _active = now >= start && now <= end;
    }
    else
    {
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
        next.SetHour(_startTime.GetHour());
        next.SetMinute(_startTime.GetMinute());

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
    next.SetHour(_startTime.GetHour());
    next.SetMinute(_startTime.GetMinute());
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

std::string Schedule::GetNextTriggerTime()
{
    wxDateTime end = _endDate;
    if (_everyYear) end.SetYear(wxDateTime::Now().GetYear() + 1);

    // deal with the simple cases
    if (CheckActive()) return "NOW!";
    if (end < wxDateTime::Now()) return "Never";

    if (_startDate > wxDateTime::Now()) // tomorrow or later
    {
        // some time in the future
        wxDateTime next = _startDate;
        next.SetHour(_startTime.GetHour());
        next.SetMinute(_startTime.GetMinute());

        if (next > wxDateTime::Now() && CheckActiveAt(next))
        {
            return next.Format("%Y-%m-%d %H:%M").ToStdString();
        }

        for (int i = 0; i < 7; i++)
        {
            next += wxTimeSpan(24);
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
    next.SetHour(_startTime.GetHour());
    next.SetMinute(_startTime.GetMinute());
    next.SetSecond(0);
    if (next > wxDateTime::Now() && CheckActiveAt(next))
    {
        return next.Format("%Y-%m-%d %H:%M").ToStdString();
    }

    for (int i =0; i < 7; i++)
    {
        next += wxTimeSpan(24);
        if (next > wxDateTime::Now() && CheckActiveAt(next))
        {
            return next.Format("%Y-%m-%d %H:%M").ToStdString();
        }
    }

    // I give up
    return "Never";
}

void Schedule::AddMinsToEndTime(int mins)
{
    _endTime += wxTimeSpan(0, mins);
}

std::string Schedule::GetNextEndTime()
{
    if (!_active) return "N/A";

    wxDateTime end = wxDateTime::Now();

    if (_endTime < _startTime)
    {
        end += wxTimeSpan(24);
    }

    end.SetHour(_endTime.GetHour());
    end.SetMinute(_endTime.GetMinute());
    end.SetSecond(0);

    return end.Format("%Y-%m-%d %H:%M").ToStdString();
}
