#include "PlayListItemOSC.h"
#include "PlayListItemOSCPanel.h"
#include "PlayList.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "PlayListStep.h"
#include "../RunningSchedule.h"
#include "../../xLights/AudioManager.h"

PlayListItemOSC::PlayListItemOSC(wxXmlNode* node) : PlayListItem(node)
{
	_ip = "";
	_path = "";
	_port = 0;
    _started = false;
	for (int i = 0; i < MAXOSCPARMS; i++)
	{
		_types[i] = OSCTYPE::OSCNONE;
		_values[i] = "";
	}
    PlayListItemOSC::Load(node);
}

void PlayListItemOSC::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _ip = node->GetAttribute("IP", "");
    _path = node->GetAttribute("Path", "");
    _port = wxAtoi(node->GetAttribute("Port", ""));
	for (int i = 0; i < MAXOSCPARMS; i++)
	{
		_types[i] = EncodeType(node->GetAttribute(wxString::Format("Type%d", i), "0").ToStdString());
		_values[i] = node->GetAttribute(wxString::Format("Value%d", i), "");
	}
}

PlayListItemOSC::PlayListItemOSC() : PlayListItem()
{
	_ip = "";
	_path = "";
	_port = 0;
    _started = false;
	for (int i = 0; i < MAXOSCPARMS; i++)
	{
		_types[i] = OSCTYPE::OSCNONE;
		_values[i] = "";
	}
}

PlayListItem* PlayListItemOSC::Copy() const
{
    PlayListItemOSC* res = new PlayListItemOSC();

	res->_ip = _ip;
	res->_path = _path;
	res->_port = _port;
    res->_started = false;
	for (int i = 0; i < MAXOSCPARMS; i++)
	{
		res->_types[i] = _types[i];
		res->_values[i] = _values[i];
	}
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemOSC::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIOSC");

    node->AddAttribute("Path", _path);
    node->AddAttribute("IP", _ip);
    node->AddAttribute("Port", wxString::Format("%d", _port));
	for (int i = 0; i < MAXOSCPARMS; i++)
	{
		node->AddAttribute(wxString::Format("Type%d", i), DecodeType(_types[i]));
		node->AddAttribute(wxString::Format("Value%d", i), _values[i]);
	}

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemOSC::GetTitle() const
{
    return "OSC";
}

void PlayListItemOSC::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemOSCPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemOSC::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "OSC";
}

std::string PlayListItemOSC::GetTooltip()
{
    return "Available variables:\n    %RUNNING_PLAYLIST% - current playlist\n    %RUNNING_PLAYLISTSTEP% - step name\n    %RUNNING_PLAYLISTSTEPMS% - Position in current step\n    %RUNNING_PLAYLISTSTEPMSLEFT% - Time left in current step\n    %RUNNING_SCHEDULE% - Name of schedule\n    %STEPNAME% - Current step\n    %ALBUM% - from mp3\n    %TITLE% - from mp3\n    %ARTIST% - from mp3\n    %SHOWDIR% - the current show directory";
}

OSCTYPE PlayListItemOSC::EncodeType(const std::string type) const
{
    if (type == "None")
    {
        return OSCTYPE::OSCNONE;
    }
    else if (type == "Integer")
    {
        return OSCTYPE::OSCINT;
    }
    else if (type == "Float")
    {
        return OSCTYPE::OSCFLOAT;
    }
    else if (type == "String")
    {
        return OSCTYPE::OSCSTRING;
    }

    return OSCTYPE::OSCNONE;
}

std::string PlayListItemOSC::DecodeType(OSCTYPE type) const
{
    switch(type)
    {
    case OSCTYPE::OSCNONE:
        return "None";
    case OSCTYPE::OSCINT:
        return "Integer";
    case OSCTYPE::OSCFLOAT:
        return "Float";
    case OSCTYPE::OSCSTRING:
        return "String";
    }

    return "None";
}

std::string PlayListItemOSC::SubstituteVariables(const std::string value)
{
    wxString res = value;

    if (res.Contains("%SHOWDIR%"))
    {
        res.Replace("%SHOWDIR%", xScheduleFrame::GetScheduleManager()->GetShowDir(), true);
    }

    PlayList* pl = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
    if (pl != nullptr)
    {
        if (res.Contains("%RUNNING_PLAYLIST%"))
        {
            res.Replace("%RUNNING_PLAYLIST%", pl->GetNameNoTime(), true);
        }
        PlayListStep* pls = pl->GetRunningStep();
        if (pls != nullptr)
        {
            if (res.Contains("%RUNNING_PLAYLISTSTEP%"))
            {
                res.Replace("%RUNNING_PLAYLISTSTEP%", pls->GetNameNoTime(), true);
            }
            if (res.Contains("%RUNNING_PLAYLISTSTEPMS%"))
            {
                res.Replace("%RUNNING_PLAYLISTSTEPMS%", wxString::Format(wxT("%i"), pls->GetLengthMS()), true);
            }
            if (res.Contains("%RUNNING_PLAYLISTSTEPMSLEFT%"))
            {
                res.Replace("%RUNNING_PLAYLISTSTEPMSLEFT%", wxString::Format(wxT("%i"), pls->GetLengthMS() - pls->GetPosition()), true);
            }
        }
    }
    if (res.Contains("%RUNNING_SCHEDULE%"))
    {
        RunningSchedule* rs = xScheduleFrame::GetScheduleManager()->GetRunningSchedule();
        if (rs != nullptr && rs->GetPlayList()->IsRunning())
        {
            res.Replace("%RUNNING_SCHEDULE%", rs->GetSchedule()->GetName(), true);
        }
    }

    auto step = xScheduleFrame::GetScheduleManager()->GetRunningPlayList()->GetRunningStep();

    if (step == nullptr)
    {
        step = xScheduleFrame::GetScheduleManager()->GetStepContainingPlayListItem(GetId());
    }

    if (step != nullptr)
    {
        res.Replace("%STEPNAME%", step->GetNameNoTime());

        AudioManager* audio = step->GetAudioManager();
        if (audio != nullptr)
        {
            res.Replace("%TITLE%", audio->Title());
            res.Replace("%ARTIST%", audio->Artist());
            res.Replace("%ALBUM%", audio->Album());
        }
    }

    return res.ToStdString();
}

void PlayListItemOSC::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

		std::string values[5];
        wxString path = SubstituteVariables(_path);
		for (int i = 0; i < MAXOSCPARMS; i++)
		{
			values[i] = SubstituteVariables(_values[i]);
		}

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Sending OSC Event to %s:%d %s", (const char *)_ip.c_str(), _port, (const char *)_path.c_str());

        OSCPacket packet(path.ToStdString());
        for (int i = 0; i < MAXOSCPARMS; i++)
        {
            if (_types[i] != OSCTYPE::OSCNONE)
            {
                packet.AddParameter(_types[i], values[i]);
            }
            else
            {
                break;
            }
        }

        packet.Send(_ip, _port);
		
        logger_base.info("OSC Sent.");
    }
}

void PlayListItemOSC::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);
    _started = false;
}
