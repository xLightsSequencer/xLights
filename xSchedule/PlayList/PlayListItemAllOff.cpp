#include "PlayListItemAllOff.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemAllOffPanel.h"
#include "../../xLights/outputs/OutputManager.h"

PlayListItemAllOff::PlayListItemAllOff(OutputManager* outputManager, wxXmlNode* node) : PlayListItem(node)
{
    _outputManager = outputManager;
    _sc = 0;
    _channels = 0;
    _startChannel = "1";
    _duration = 50;
    _value = 0;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    PlayListItemAllOff::Load(node);
}

void PlayListItemAllOff::Load(wxXmlNode* node) 
{
    PlayListItem::Load(node);
    _duration = wxAtoi(node->GetAttribute("Duration", "50"));
    _value = wxAtoi(node->GetAttribute("Value", "0"));
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _channels = wxAtol(node->GetAttribute("Channels", "0"));
}

PlayListItemAllOff::PlayListItemAllOff(OutputManager* outputManager) : PlayListItem()
{
    _type = "PLIAllOff";
    _outputManager = outputManager;
    _sc = 0;
    _channels = 0;
    _startChannel = "1";
    _duration = 50;
    _value = 0;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    SetName("All Set");
}

PlayListItem* PlayListItemAllOff::Copy() const
{
    PlayListItemAllOff* res = new PlayListItemAllOff(_outputManager);
    res->_duration = _duration;
    res->_outputManager = _outputManager;
    res->_applyMethod = _applyMethod;
    res->_value = _value;
    res->_channels = _channels;
    res->_startChannel = _startChannel;
    PlayListItem::Copy(res);

    return res;
}

size_t PlayListItemAllOff::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

wxXmlNode* PlayListItemAllOff::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("Value", wxString::Format(wxT("%i"), _value));
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemAllOff::GetTitle() const
{
    return "All Set";
}

void PlayListItemAllOff::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemAllOffPanel(notebook, _outputManager, this), GetTitle(), true);
}

void PlayListItemAllOff::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        if (ms >= _delay && ms <= _delay + _duration)
        {
            long sc = GetStartChannelAsNumber();

            if (sc > size) return;

            size_t toset = _channels + sc - 1 < size ? _channels : size - sc + 1;
            if (_channels == 0)
            {
                toset = size;
            }

            if (toset > 0)
            {
                uint8_t* values = (uint8_t*)malloc(toset);
                if (values != nullptr)
                {
                    memset(values, _value, toset);
                    Blend(buffer, size, values, toset, _applyMethod, sc - 1);
                    free(values);
                }
            }
        }
    }
}

