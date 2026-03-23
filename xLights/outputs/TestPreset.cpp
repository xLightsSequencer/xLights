/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TestPreset.h"

#include "Output.h"
#include "../UtilFunctions.h"

#include <log.h>

#pragma region Constructors and Destructors
TestPreset::TestPreset(const std::string& name)
{
	_name = name;
}

TestPreset::TestPreset(pugi::xml_node node)
{
    _name = UnXmlSafe(node.attribute("name").as_string(""));

    for (pugi::xml_node e = node.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "channel") {
            AddChannel(e.attribute("id").as_int(0));
        } else if (std::string_view(e.name()) == "channelr") {
            AddChannelRange(e.attribute("start").as_int(0), e.attribute("end").as_int(0));
        }
    }
}
#pragma endregion Constructors and Destructors

#pragma region Save
pugi::xml_node TestPreset::Save(pugi::xml_node parent)
{
    pugi::xml_node node = parent.append_child("testpreset");
    node.append_attribute("name") = XmlSafe(_name);

    std::sort(_channels.begin(), _channels.end());

    int start = -1;
    int last = -1;

    for (auto it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (start == -1)
        {
            start = *it;
            last = start;
        }

        auto next = it;
        ++next;

        if (next == _channels.end() || *next != last + 1)
        {
            if (*it - start == 0)
            {
                pugi::xml_node one = node.append_child("channel");
                one.append_attribute("id") = *it;
            }
            else
            {
                pugi::xml_node range = node.append_child("channelr");
                range.append_attribute("start") = start;
                range.append_attribute("end") = *it;
            }
            start = -1;
            last = -1;
        }
        else
        {
            last = *it + 1;
        }
    }

    return node;
}
#pragma endregion Save

#pragma region Getters and Setters
void TestPreset::AddChannel(int ch)
{
    if (std::find(_channels.begin(), _channels.end(), ch) == _channels.end())
    {
        _channels.push_back(ch);
    }
}

void TestPreset::AddChannelRange(int start, int end)
{
    if (_channels.empty()) {
        _channels.reserve(end - start + 1);
        for (int i = start; i <= end; i++)
        {
            _channels.push_back(i);
        }
        return;
    }
    _channels.reserve(_channels.size() + end - start + 1);
    for (int i = start; i <= end; i++)
    {
        AddChannel(i);
    }
}
#pragma endregion Getters and Setters

