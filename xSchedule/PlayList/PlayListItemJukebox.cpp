/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/xml/xml.h>

#include <nlohmann/json.hpp>

#include "PlayListItemJukebox.h"
#include "PlayListItemJukeboxPanel.h"
#include "../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>

PlayListItemJukebox::PlayListItemJukebox(wxXmlNode* node) :
    PlayListItem(node)
{
    _started = false;
    _jukeboxButton = 1;
    _port = "A";
    SetEnumState(ENUMJUKEBOX::ENUM_STATE_DONE);
    PlayListItemJukebox::Load(node);
}

void PlayListItemJukebox::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _jukeboxButton = wxAtoi(node->GetAttribute("Button", "1"));
    _port = node->GetAttribute("Port", "A");
}

PlayListItemJukebox::PlayListItemJukebox() :
    PlayListItem()
{
    _type = "PLIJukebox";
    _started = false;
    _jukeboxButton = 1;
    _port = "A";
    SetEnumState(ENUMJUKEBOX::ENUM_STATE_DONE);
}

PlayListItem* PlayListItemJukebox::Copy(const bool isClone) const
{
    PlayListItemJukebox* res = new PlayListItemJukebox();
    res->_jukeboxButton = _jukeboxButton;
    res->_port = _port;
    res->_started = false;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemJukebox::Save()
{
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Button", wxString::Format("%d", _jukeboxButton));
    node->AddAttribute("Port", _port);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemJukebox::GetTitle() const
{
    return "Play xLights Jukebox Button";
}

void PlayListItemJukebox::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemJukeboxPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemJukebox::GetNameNoTime() const
{
    if (_name != "")
        return _name;

    return "Play xLights Jukebox Button";
}

void PlayListItemJukebox::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started) {
        _started = true;

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Launching xLights Jukebox Button %d.", _jukeboxButton);

        nlohmann::json result = xLightsRequest(GetPort(), "{\"cmd\":\"lightsOn\"}");
        if (result["res"].get<int>() != 200) {
            logger_base.error("Failed to turn on output to lights: %s", (const char*)result["msg"].get<std::string>().c_str());
        }

        result = xLightsRequest(GetPort(), wxString::Format("{\"cmd\":\"playJukebox\",\"button\":%d}", _jukeboxButton));
        if (result["res"].get<int>() != 200) {
            logger_base.error("Failed to send jukebox button press: %s", (const char*)result["msg"].get<std::string>().c_str());
        }
    }
}

void PlayListItemJukebox::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
