#include "PlayListItemTest.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemTestPanel.h"

PlayListItemTest::PlayListItemTest(wxXmlNode* node) : PlayListItem(node)
{
    _startChannel = 1;
    _channels = 100;
    _duration = 60000;
    _frameDuration = 500;
    _mode = "Value 2";
    _state = 0;
    _value1 = 0;
    _value2 = 255;
    PlayListItemTest::Load(node);
}

PlayListItemTest::~PlayListItemTest()
{
}


void PlayListItemTest::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _mode = node->GetAttribute("Mode", "");
    _startChannel = wxAtoi(node->GetAttribute("StartChannel", "1"));
    _channels = wxAtoi(node->GetAttribute("Channels", "1000"));
    _value1 = wxAtoi(node->GetAttribute("Value1", "0"));
    _value2 = wxAtoi(node->GetAttribute("Value2", "255"));
    _duration = wxAtol(node->GetAttribute("Duration", "60000"));
    _frameDuration = wxAtol(node->GetAttribute("FrameDuration", "500"));
}

PlayListItemTest::PlayListItemTest() : PlayListItem()
{
    _startChannel = 1;
    _channels = 100;
    _duration = 60000;
    _frameDuration = 500;
    _mode = "Value 2";
    _state = 0;
    _value1 = 0;
    _value2 = 255;
}

PlayListItem* PlayListItemTest::Copy() const
{
    PlayListItemTest* res = new PlayListItemTest();
    res->_startChannel = _startChannel;
    res->_channels = _channels;
    res->_duration = _duration;
    res->_frameDuration = _frameDuration;
    res->_mode = _mode;
    res->_value1 = _value1;
    res->_value2 = _value2;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemTest::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLITest");

    node->AddAttribute("Mode", _mode);
    node->AddAttribute("StartChannel", wxString::Format(wxT("%i"), (long)_startChannel));
    node->AddAttribute("Channels", wxString::Format(wxT("%i"), (long)_channels));
    node->AddAttribute("Value1", wxString::Format(wxT("%i"), _value1));
    node->AddAttribute("Value2", wxString::Format(wxT("%i"), _value2));
    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("FrameDuration", wxString::Format(wxT("%i"), (long)_frameDuration));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemTest::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemTestPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemTest::GetTitle() const
{
    return "Test";
}

std::string PlayListItemTest::GetNameNoTime() const
{
    if (_name != "") return _name;

    return _mode;
}

void PlayListItemTest::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        int toset = std::min(_channels, size - (_startChannel - 1));

        if (_mode == "Value1")
        {
            memset(buffer + _startChannel - 1, _value1, toset);
        }
        else if (_mode == "Value2")
        {
            memset(buffer + _startChannel - 1, _value2, toset);
        }
        else if (_mode == "Alternate")
        {
            for (int i = 0; i < toset; i++)
            {
                if ((_state + i) % 2 == 0)
                {
                    *(buffer + _startChannel - 1 + i) = _value1;
                }
                else
                {
                    *(buffer + _startChannel - 1 + i) = _value2;
                }
            }
        }
        else if (_mode == "A-B-C")
        {
            int state = _state % 3;
            for (int i = 0; i < toset; i++)
            {
                if (i % 3 == state)
                {
                    *(buffer + _startChannel - 1 + i) = 255;
                }
                else
                {
                    *(buffer + _startChannel - 1 + i) = 0;
                }
            }
        }
        else if (_mode == "A-B-C-All")
        {
            int state = _state % 4;

            for (int i = 0; i < toset; i++)
            {
                if (i % 4 == state)
                {
                    *(buffer + _startChannel - 1 + i) = 255;
                }
                else
                {
                    if (state == 3)
                    {
                        *(buffer + _startChannel - 1 + i) = 255;
                    }
                    else
                    {
                        *(buffer + _startChannel - 1 + i) = 0;
                    }
                }
            }
        }
        else if (_mode == "None-A-B-C")
        {
            int state = _state % 4;

            for (int i = 0; i < toset; i++)
            {
                if (i % 4 == state)
                {
                    if (state == 0)
                    {
                        *(buffer + _startChannel - 1 + i) = 0;
                    }
                    else
                    {
                        *(buffer + _startChannel - 1 + i) = 255;
                    }
                }
                else
                {
                        *(buffer + _startChannel - 1 + i) = 0;
                }
            }
        }
        else if (_mode == "Node-A-B-C-All")
        {
            int state = _state % 5;

            for (int i = 0; i < toset; i++)
            {
                if (i % 5 == state)
                {
                    if (state == 0)
                    {
                        *(buffer + _startChannel - 1 + i) = 0;
                    }
                    else
                    {
                        *(buffer + _startChannel - 1 + i) = 255;
                    }
                }
                else
                {
                    if (state == 4)
                    {
                        *(buffer + _startChannel - 1 + i) = 255;
                    }
                    else
                    {
                        *(buffer + _startChannel - 1 + i) = 0;
                    }
                }
            }
        }

        if (ms / _frameDuration > (ms - framems) / _frameDuration)
        {
            _state++;
        }
    }
}

void PlayListItemTest::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);
    _state = 0;
}

