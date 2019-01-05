#include "PlayListItemSetColour.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemSetColourPanel.h"
#include "../../xLights/outputs/OutputManager.h"

PlayListItemSetColour::PlayListItemSetColour(OutputManager* outputManager, wxXmlNode* node) : PlayListItem(node)
{
    _outputManager = outputManager;
    _sc = 0;
    _nodes = 0;
    _startChannel = "1";
    _duration = 50;
    _value = *wxBLACK;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    PlayListItemSetColour::Load(node);
}

void PlayListItemSetColour::Load(wxXmlNode* node) 
{
    PlayListItem::Load(node);
    _duration = wxAtoi(node->GetAttribute("Duration", "50"));
    _value = wxColor(node->GetAttribute("Value", "Black"));
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _nodes = wxAtol(node->GetAttribute("Nodes", "0"));
}

PlayListItemSetColour::PlayListItemSetColour(OutputManager* outputManager) : PlayListItem()
{
    _outputManager = outputManager;
    _sc = 0;
    _nodes = 0;
    _startChannel = "1";
    _duration = 50;
    _value = *wxBLACK;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    SetName("Set Colour");
}

PlayListItem* PlayListItemSetColour::Copy() const
{
    PlayListItemSetColour* res = new PlayListItemSetColour(_outputManager);
    res->_duration = _duration;
    res->_outputManager = _outputManager;
    res->_applyMethod = _applyMethod;
    res->_value = _value;
    res->_nodes = _nodes;
    res->_startChannel = _startChannel;
    PlayListItem::Copy(res);

    return res;
}

size_t PlayListItemSetColour::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

wxXmlNode* PlayListItemSetColour::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLISetColour");

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("Value", _value.GetAsString());
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Nodes", wxString::Format(wxT("%ld"), (long)_nodes));

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemSetColour::GetTitle() const
{
    return "Set Colour";
}

void PlayListItemSetColour::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemSetColourPanel(notebook, _outputManager, this), GetTitle(), true);
}

void PlayListItemSetColour::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        if (ms >= _delay && ms <= _delay + _duration)
        {
            long sc = GetStartChannelAsNumber();

            if (sc > size) return;

            size_t toset = _nodes * 3 + sc - 1 < size ? _nodes : (size - sc + 1) / 3;
            if (_nodes == 0)
            {
                toset = size / 3;
            }

            if (toset > 0)
            {
                uint8_t data[3];
                data[0] = _value.Red();
                data[1] = _value.Green();
                data[2] = _value.Blue();
                uint8_t* values = (uint8_t*)malloc(toset * 3);
                if (values != nullptr)
                {
                    for (int i = 0; i < toset; i++)
                    {
                        memcpy(values + i * 3, data, sizeof(data));
                    }
                    Blend(buffer, size, values, toset * 3, _applyMethod, sc - 1);
                    free(values);
                }
            }
        }
    }
}
