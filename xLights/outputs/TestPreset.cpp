#include "TestPreset.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "Output.h"

TestPreset::TestPreset(const std::string& name)
{
	_name = name;
}

TestPreset::TestPreset(wxXmlNode* node)
{
	_name = Output::UnXmlSafe(node->GetAttribute("name", "").ToStdString());
	
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

wxXmlNode* TestPreset::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "testpreset");
    node->AddAttribute("name", Output::XmlSafe(_name));

    _channels.sort();

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

void TestPreset::AddChannel(int ch)
{
    if (std::find(_channels.begin(), _channels.end(), ch) == _channels.end())
    {
        _channels.push_back(ch);
    }
}

void TestPreset::AddChannelRange(int start, int end)
{
    for (int i = start; i <= end; i++)
    {
        AddChannel(i);
    }
}

