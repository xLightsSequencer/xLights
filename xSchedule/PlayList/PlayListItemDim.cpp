#include "PlayListItemDim.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemDimPanel.h"
#include "../../xLights/outputs/OutputManager.h"
#include "../../xLights/Parallel.h"

PlayListItemDim::PlayListItemDim(OutputManager* outputManager, wxXmlNode* node) : PlayListItem(node)
{
    _outputManager = outputManager;
    _sc = 0;
    _startChannel = "1";
    _channels = 100;
    _duration = 60000;
    _dim = 100;
    PlayListItemDim::Load(node);
}

PlayListItemDim::~PlayListItemDim()
{
}

void PlayListItemDim::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _channels = wxAtoi(node->GetAttribute("Channels", "1000"));
    _dim = wxAtoi(node->GetAttribute("Dim", "100"));
    _duration = wxAtol(node->GetAttribute("Duration", "60000"));
}

PlayListItemDim::PlayListItemDim(OutputManager* outputManager) : PlayListItem()
{
    _type = "PLIDim";
    _outputManager = outputManager;
    _sc = 0;
    _startChannel = "1";
    _channels = 100;
    _duration = 60000;
    _dim = 100;
}

PlayListItem* PlayListItemDim::Copy() const
{
    PlayListItemDim* res = new PlayListItemDim(_outputManager);
    res->_outputManager = _outputManager;
    res->_startChannel = _startChannel;
    res->_channels = _channels;
    res->_duration = _duration;
    res->_dim = _dim;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemDim::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Channels", wxString::Format(wxT("%i"), (long)_channels));
    node->AddAttribute("Dim", wxString::Format(wxT("%i"), _dim));
    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemDim::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemDimPanel(notebook, _outputManager, this), GetTitle(), true);
}

size_t PlayListItemDim::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

std::string PlayListItemDim::GetTitle() const
{
    return "Dim";
}

std::string PlayListItemDim::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "Dim";
}

void PlayListItemDim::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        long sc = GetStartChannelAsNumber();

        int toset = std::min(_channels, size - ((size_t)sc - 1));

        parallel_for(0, toset, [buffer, sc, toset, this](int i) {
            *(buffer + sc - 1 + i) = *(buffer + sc - 1 + i) * _dim / 100;
        });
    }
}

void PlayListItemDim::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);
}

