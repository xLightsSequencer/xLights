#include "PlayListItemColourOrgan.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemColourOrganPanel.h"
#include "../../xLights/outputs/OutputManager.h"
#include "../../xLights/AudioManager.h"

PlayListItemColourOrgan::PlayListItemColourOrgan(OutputManager* outputManager, wxXmlNode* node) : PlayListItem(node)
{
    _outputManager = outputManager;
    _sc = 0;
    _startChannel = "1";
    _pixels = 1;
    _duration = 60000;
    _colour = *wxWHITE;
    _mode = "Value 2";
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
    _startNote = 1;
    _endNote = 127;
    _lastValue = 0;
    _fadeFrames = 0;
    _fadePerFrame = 1.0;
    _threshold = 80;
    PlayListItemColourOrgan::Load(node);
}

PlayListItemColourOrgan::~PlayListItemColourOrgan()
{
}

void PlayListItemColourOrgan::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _mode = node->GetAttribute("Mode", "");
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _pixels = wxAtoi(node->GetAttribute("Pixels", "1"));
    _colour = wxColour(node->GetAttribute("Colour", "WHITE"));
    _duration = wxAtol(node->GetAttribute("Duration", "60000"));
    _blendMode = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", "1"));
    _startNote = wxAtoi(node->GetAttribute("StartNote", "1"));
    _endNote = wxAtoi(node->GetAttribute("EndNote", "127"));
    _fadeFrames = wxAtoi(node->GetAttribute("FadeFrames", "0"));
    _threshold = wxAtoi(node->GetAttribute("Threshold", "80"));
    if (_fadeFrames == 0) {
        _fadePerFrame = 10.0;
    }
    else {
        _fadePerFrame = 10.0 / _fadeFrames;
    }
}

PlayListItemColourOrgan::PlayListItemColourOrgan(OutputManager* outputManager) : PlayListItem()
{
    _type = "PLIColourOrgan";
    _outputManager = outputManager;
    _sc = 0;
    _startChannel = "1";
    _pixels = 1;
    _duration = 60000;
    _colour = *wxWHITE;
    _mode = "Value 2";
    _startNote = 1;
    _endNote = 127;
    _lastValue = 0;
    _fadeFrames = 0;
    _fadePerFrame = 1.0;
    _threshold = 80;
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
}

PlayListItem* PlayListItemColourOrgan::Copy() const
{
    PlayListItemColourOrgan* res = new PlayListItemColourOrgan(_outputManager);
    res->_outputManager = _outputManager;
    res->_startChannel = _startChannel;
    res->_pixels = _pixels;
    res->_duration = _duration;
    res->_colour = _colour;
    res->_mode = _mode;
    res->_blendMode = _blendMode;
    res->_startNote = _startNote;
    res->_endNote = _endNote;
    res->_lastValue = _lastValue;
    res->_fadeFrames = _fadeFrames;
    res->_fadePerFrame = _fadePerFrame;
    res->_threshold = _threshold;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemColourOrgan::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Mode", _mode);
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Pixels", wxString::Format(wxT("%i"), (long)_pixels));
    node->AddAttribute("Colour", _colour.GetAsString());
    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_blendMode));
    node->AddAttribute("StartNote", wxString::Format(wxT("%i"), (int)_startNote));
    node->AddAttribute("EndNote", wxString::Format(wxT("%i"), (int)_endNote));
    node->AddAttribute("FadeFrames", wxString::Format(wxT("%i"), (int)_fadeFrames));
    node->AddAttribute("Threshold", wxString::Format(wxT("%i"), (int)_threshold));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemColourOrgan::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemColourOrganPanel(notebook, _outputManager, this), GetTitle(), true);
}

size_t PlayListItemColourOrgan::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

std::string PlayListItemColourOrgan::GetTitle() const
{
    return "Colour Organ";
}

std::string PlayListItemColourOrgan::GetNameNoTime() const
{
    if (_name != "") return _name;

    return _mode;
}

void PlayListItemColourOrgan::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        long sc = GetStartChannelAsNumber();
        int toset = std::min(_pixels * 3, size - ((size_t)sc - 1));

        float value = 0;
        
        if (_mode == "Maximum")
        {
            auto res = AudioManager::GetSDL()->GetOutputSpectrum(framems);
            if (res.size() >= _endNote)
            {
                for (int i = _startNote; i <= _endNote; i++)
                {
                    value = std::max(res[i-1], value);
                }
            }
        }

        if (value < ((float)_threshold * 10.0) / 255.0)
        {
            value = 0;
        }

        if (_lastValue - _fadePerFrame > value)
        {
            value = _lastValue - _fadePerFrame;
        }
        if (value < 0) value = 0;

        _lastValue = value;

        wxASSERT(value <= 10);
        wxColour c((float)_colour.Red() * value / 10, (float)_colour.Green() * value / 10, (float)_colour.Blue() * value / 10);

        //wxASSERT(c.Red() <= _colour.Red());
        //wxASSERT(c.Green() <= _colour.Green());
        //wxASSERT(c.Blue() <= _colour.Blue());

        for (uint8_t* p = buffer + sc - 1; p < buffer + sc - 1 + toset; p+=3)
        {
            SetPixel(p, c.Red(), c.Green(), c.Blue(), _blendMode);
        }
    }
}

void PlayListItemColourOrgan::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);
}

void PlayListItemColourOrgan::Stop()
{
    PlayListItem::Stop();
}

void PlayListItemColourOrgan::SetPixel(uint8_t* p, uint8_t r, uint8_t g, uint8_t b, APPLYMETHOD blendMode)
{
    uint8_t rgb[3];
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;

    Blend(p, 3, rgb, 3, blendMode);
}
