#include "Schedule.h"
#include "ScheduleDialog.h"
#include <wx/xml/xml.h>

Schedule::Schedule(wxXmlNode* node)
{
    _dirty = false;
    Schedule::Load(node);
}

void Schedule::Load(wxXmlNode* node)
{
    _dirty = false;

    _name = node->GetAttribute("Name", "<unnamed>");
    _dow = node->GetAttribute("DOW", "MonTueWedThuFriSatSun");
    _startDate.ParseDate(node->GetAttribute("StartDate", "01/01/2017"));
    _endDate.ParseDate(node->GetAttribute("EndDate", "01/01/2099"));
    _startTime.ParseTime(node->GetAttribute("StartTime", "19:00"));
    _endTime.ParseTime(node->GetAttribute("EndTime", "22:00"));
    _loop = node->GetAttribute("Loop", "FALSE") == "TRUE";
    _random = node->GetAttribute("Random", "FALSE") == "TRUE";
    _everyYear = node->GetAttribute("EveryYear", "FALSE") == "TRUE";
    _loops = wxAtoi(node->GetAttribute("Loops", "0"));
    _priority = wxAtoi(node->GetAttribute("Priority", "0"));
}

wxXmlNode* Schedule::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Schedule");

    node->AddAttribute("Name", _name);
    node->AddAttribute("DOW", _dow);
    node->AddAttribute("StartDate", _startDate.FormatDate());
    node->AddAttribute("EndDate", _endDate.FormatDate());
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
    if (_everyYear)
    {
        node->AddAttribute("EveryYear", "TRUE");
    }

    return node;
}

Schedule::Schedule()
{
    _dirty = false;
    _loop = false;
    _endDate.ParseDate("01/01/2099");
    _startDate.ParseDate("01/01/2017");
    _startTime.ParseTime("19:00");
    _endTime.ParseTime("22:00");
    _name = "<unnamed>";
    _random = false;
    _loops = 0;
    _everyYear = false;
    _priority = 5;
    _dow = "MonTueWedThuFriSatSun";
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

