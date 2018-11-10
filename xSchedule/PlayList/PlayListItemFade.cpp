#include "PlayListItemFade.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemFadePanel.h"
#include "../../xLights/outputs/OutputManager.h"

PlayListItemFade::PlayListItemFade(OutputManager* outputManager, wxXmlNode* node) : PlayListItem(node)
{
    _outputManager = outputManager;
	_fadeDirection = FADEDIRECTION::FADE_OUT;
    _sc = 0;
    _channels = 0;
    _startChannel = "1";
    _duration = 50;
    PlayListItemFade::Load(node);
}

void PlayListItemFade::Load(wxXmlNode* node) 
{
    PlayListItem::Load(node);
    _duration = wxAtoi(node->GetAttribute("Duration", "50"));
    _fadeDirection = (FADEDIRECTION)wxAtoi(node->GetAttribute("FadeMode", "1"));
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _channels = wxAtol(node->GetAttribute("Channels", "0"));
}

PlayListItemFade::PlayListItemFade(OutputManager* outputManager) : PlayListItem()
{
    _type = "PLIFade";
    _outputManager = outputManager;
    _sc = 0;
    _channels = 0;
    _startChannel = "1";
    _duration = 50;
    _fadeDirection = FADEDIRECTION::FADE_OUT;
    SetName("Fade");
}

PlayListItem* PlayListItemFade::Copy() const
{
    PlayListItemFade* res = new PlayListItemFade(_outputManager);
    res->_duration = _duration;
    res->_outputManager = _outputManager;
    res->_fadeDirection = _fadeDirection;
    res->_channels = _channels;
    res->_startChannel = _startChannel;
    PlayListItem::Copy(res);

    return res;
}

size_t PlayListItemFade::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

wxXmlNode* PlayListItemFade::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));
    node->AddAttribute("FadeMode", wxString::Format(wxT("%d"), (int)_fadeDirection));

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFade::GetTitle() const
{
    return "Fade";
}

void PlayListItemFade::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFadePanel(notebook, _outputManager, this), GetTitle(), true);
}

void PlayListItemFade::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    long duration = _duration;

    if (_restOfStep)
    {
        duration = _stepLengthMS - _delay;
    }

	if (_duration == 0) return;
	
    if (outputframe)
    {
        if (ms >= _delay && ms <= _delay + duration)
        {
            long sc = GetStartChannelAsNumber();

            if (sc > size) return;

            size_t toset = _channels + sc - 1 < size ? _channels : (size - sc + 1);
            if (_channels == 0)
            {
                toset = size;
            }

			int adjust = ((ms - _delay) * 255) / duration;
			
            if (_fadeDirection == FADEDIRECTION::FADE_OUT)
            {
                adjust = 255 - adjust;
            }

            if (toset > 0)
            {
				for (long i = 0; i < toset; i++)
				{
					buffer[i] = ((int)buffer[i] * adjust) / 255;
				}
            }
        }
    }
}
