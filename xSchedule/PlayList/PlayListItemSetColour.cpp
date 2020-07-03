/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
    _fadeToBlack = false;
    _perFrame = 999;
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
    _fadeToBlack = (node->GetAttribute("FadeToBlack", "FALSE") == "TRUE");
}

PlayListItemSetColour::PlayListItemSetColour(OutputManager* outputManager) : PlayListItem()
{
    _type = "PLISetColour";
    _outputManager = outputManager;
    _sc = 0;
    _nodes = 0;
    _startChannel = "1";
    _duration = 50;
    _value = *wxBLACK;
    _fadeToBlack = false;
    _perFrame = 999;
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
    res->_fadeToBlack = _fadeToBlack;
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
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("Value", _value.GetAsString());
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Nodes", wxString::Format(wxT("%ld"), (long)_nodes));
    if (_fadeToBlack) node->AddAttribute("FadeToBlack", "TRUE");

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
        if (_perFrame == 999)             {
            if (_fadeToBlack) {
                _perFrame = 1 / ((float)_duration / (float)framems);
            }
            else {
                _perFrame = 0;
            }
        }

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

                if (_perFrame > 0) {
                    int frame = (ms - _delay) / framems;
                    float fade = _perFrame * frame;
                    float f0 = (float)data[0] - ((float)data[0] * fade);
                    float f1 = (float)data[1] - ((float)data[1] * fade);
                    float f2 = (float)data[2] - ((float)data[2] * fade);
                    if (f0 < 0) f0 = 0;
                    if (f1 < 0) f1 = 0;
                    if (f2 < 0) f2 = 0;
                    data[0] = f0;
                    data[1] = f1;
                    data[2] = f2;
                }

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
