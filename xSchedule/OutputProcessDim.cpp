#include "OutputProcessDim.h"
#include <wx/xml/xml.h>

OutputProcessDim::OutputProcessDim(wxXmlNode* node) : OutputProcess(node)
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

OutputProcessDim::OutputProcessDim() : OutputProcess()
{
    _lastDim = -1;
    _channels = 1;
    _dim = 100;
    BuildDimTable();
}

OutputProcessDim::OutputProcessDim(size_t _startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(_startChannel, description)
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

    size_t chs = std::min(_channels, size - (_startChannel - 1));

    if (_dim == 0)
    {
        memset(buffer + _startChannel - 1, 0x00, chs);
        return;
    }

    for (int i = 0; i < chs; i++)
    {
        *(buffer + i + _startChannel - 1) = _dimTable[*(buffer + i + _startChannel - 1)];
    }
}
