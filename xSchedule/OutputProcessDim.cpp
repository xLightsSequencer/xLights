#include "OutputProcessDim.h"
#include <wx/xml/xml.h>

OutputProcessDim::OutputProcessDim(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _lastDim = -1;
    _channels = wxAtol(node->GetAttribute("Channels", "1"));
    _dim = wxAtol(node->GetAttribute("Dim", "100"));
    BuildDimTable();
}

OutputProcessDim::OutputProcessDim(const OutputProcessDim& op) : OutputProcess(op)
{
    _channels = op._channels;
    _dim = op._dim;
    BuildDimTable();
}

void OutputProcessDim::BuildDimTable()
{
    if (_lastDim != _dim)
    {
        for (int i = 0; i < 256; i++)
        {
            _dimTable[i] = i * _dim / 100;
        }
    }
}

OutputProcessDim::OutputProcessDim(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _lastDim = -1;
    _channels = 1;
    _dim = 100;
    BuildDimTable();
}

OutputProcessDim::OutputProcessDim(OutputManager* outputManager, std::string startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(outputManager, startChannel, description)
{
    _lastDim = -1;
    _channels = p1;
    _dim = p2;
    BuildDimTable();
}

wxXmlNode* OutputProcessDim::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPDim");

    res->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));
    res->AddAttribute("Dim", wxString::Format(wxT("%d"), _dim));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessDim::Frame(wxByte* buffer, size_t size)
{
    if (!_enabled) return;
    if (_dim == 100) return;

    size_t sc = GetStartChannelAsNumber();

    size_t chs = std::min(_channels, size - (sc - 1));

    if (_dim == 0)
    {
        memset(buffer + sc - 1, 0x00, chs);
        return;
    }

    for (int i = 0; i < chs; i++)
    {
        *(buffer + i + sc - 1) = _dimTable[*(buffer + i + sc - 1)];
    }
}
