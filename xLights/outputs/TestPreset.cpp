/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TestPreset.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "Output.h"
#include "../UtilFunctions.h"

#pragma region Constructors and Destructors
TestPreset::TestPreset(const std::string& name)
{
	_name = name;
}

TestPreset::TestPreset(wxXmlNode* node)
{
	_name = UnXmlSafe(node->GetAttribute("name", ""));
	
	for (wxXmlNode* e = node->GetChildren(); e != nullptr; e = e->GetNext())
	{
		if (e->GetName() == "channel")
		{
			AddChannel(wxAtoi(e->GetAttribute("id", "")));
		}
		else if (e->GetName() == "channelr")
		{
			AddChannelRange(wxAtoi(e->GetAttribute("start", "")), wxAtoi(e->GetAttribute("end", "")));
		}
	}
}
#pragma endregion Constructors and Destructors

#pragma region Save
wxXmlNode* TestPreset::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "testpreset");
    node->AddAttribute("name", XmlSafe(_name));

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
                wxXmlNode* one = new wxXmlNode(wxXML_ELEMENT_NODE, "channel");
                one->AddAttribute("id", wxString::Format("%d", *it));
                node->AddChild(one);
            }
            else
            {
                wxXmlNode* range = new wxXmlNode(wxXML_ELEMENT_NODE, "channelr");
                range->AddAttribute("start", wxString::Format("%d", start));
                range->AddAttribute("end", wxString::Format("%d", *it));
                node->AddChild(range);
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

