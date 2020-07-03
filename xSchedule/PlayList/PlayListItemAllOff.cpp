/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
    _fadeToZero = false;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    _perFrame = 999;
    PlayListItemAllOff::Load(node);
}

void PlayListItemAllOff::Load(wxXmlNode* node) 
{
    PlayListItem::Load(node);
    _duration = wxAtoi(node->GetAttribute("Duration", "50"));
    _value = wxAtoi(node->GetAttribute("Value", "0"));
    _fadeToZero = (node->GetAttribute("FadeToZero", "FALSE") == "TRUE");
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
    _fadeToZero = false;
    _perFrame = 999;
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
    res->_fadeToZero = _fadeToZero;
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
    if (_fadeToZero) {
        node->AddAttribute("FadeToZero", "TRUE");
    }

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
        if (_perFrame == 999) {
            if (_fadeToZero) {
                _perFrame = (float)_value / ((float)_duration / (float)framems);
            }
            else                 {
                _perFrame = 0;
            }
        }

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
                uint8_t vv = _value;
                if (_perFrame > 0) {
                    int frame = (ms - _delay) / framems;
                    float v = (float)_value - (_perFrame * frame);
                    if (v < 0) v = 0;
                    vv = v;
                }

                uint8_t* values = (uint8_t*)malloc(toset);
                if (values != nullptr)
                {
                    memset(values, vv, toset);
                    Blend(buffer, size, values, toset, _applyMethod, sc - 1);
                    free(values);
                }
            }
        }
    }
}