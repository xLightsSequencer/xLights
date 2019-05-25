#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <wx/file.h>

#include "../xScheduleApp.h"
#include "../xScheduleMain.h"
#include "PlayListItemPlugin.h"
#include "PlayListItemPluginPanel.h"
#include "../PluginManager.h"
#include "../ScheduleManager.h"

#include <log4cpp/Category.hh>

PlayListItemPlugin::PlayListItemPlugin(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _plugin = "";
	_action = "Start";
	_eventParm = "";
    PlayListItemPlugin::Load(node);
}

void PlayListItemPlugin::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _plugin = node->GetAttribute("Plugin", "");
    _action = node->GetAttribute("Action", "Start");
    _eventParm = node->GetAttribute("EventParm", "");
}

PlayListItemPlugin::PlayListItemPlugin() : PlayListItem()
{
    _started = false;
    _plugin = "";
    _action = "Start";
	_eventParm = "";
}

PlayListItem* PlayListItemPlugin::Copy() const
{
    PlayListItemPlugin* res = new PlayListItemPlugin();
    res->_plugin = _plugin;
    res->_action = _action;
	res->_eventParm = _eventParm;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemPlugin::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIPlugin");

    node->AddAttribute("Plugin", _plugin);
    node->AddAttribute("Action", _action);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemPlugin::GetTitle() const
{
    return "Plugin";
}

void PlayListItemPlugin::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemPluginPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemPlugin::GetNameNoTime() const
{
    if (_plugin != "") return _plugin + ":" + _action;

    return "Plugin";
}

void PlayListItemPlugin::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
	//static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	if (_plugin == "") return;
	
    if (ms >= _delay && !_started)
    {
        _started = true;

        std::string showDir = xScheduleFrame::GetScheduleManager()->GetShowDir();
        std::string webURL = ((xScheduleApp*)wxTheApp)->GetFrame()->GetOurURL();

		if (_action == "Start")
		{
			((xScheduleApp*)wxTheApp)->GetFrame()->GetPluginManager().StartPlugin(_plugin, showDir, webURL);
		}
		else if (_action == "Stop")
		{
            ((xScheduleApp*)wxTheApp)->GetFrame()->GetPluginManager().StopPlugin(_plugin);
		}
		else if (_action == "Event")
		{
            ((xScheduleApp*)wxTheApp)->GetFrame()->GetPluginManager().FirePluginEvent(_plugin, "PlayListPlugin", _eventParm);
		}
    }
}

void PlayListItemPlugin::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
