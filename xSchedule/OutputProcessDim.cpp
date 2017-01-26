#include "OutputProcessDim.h"
#include <wx/xml/xml.h>

OutputProcessDim::OutputProcessDim(wxXmlNode* node) : OutputProcess(node)
{
    _channels = wxAtol(node->GetAttribute("Channels", "1"));
    _dim = wxAtol(node->GetAttribute("Dim", "100"));
}

OutputProcessDim::OutputProcessDim(const OutputProcessDim& op) : OutputProcess(op)
{
    _channels = op._channels;
    _dim = op._dim;
}

OutputProcessDim::OutputProcessDim() : OutputProcess()
{
    _channels = 1;
    _dim = 100;
}

OutputProcessDim::OutputProcessDim(size_t _startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(_startChannel, description)
{
    _channels = p1;
    _dim = p2;
}

wxXmlNode* OutputProcessDim::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPDim");

    res->AddAttribute("Channels", wxString::Format(wxT("%i"), _channels));
    res->AddAttribute("Dim", wxString::Format(wxT("%i"), _dim));

    return res;
}

void OutputProcessDim::Frame(wxByte* buffer, size_t size)
{
    if (_dim == 100) return;

    size_t chs = std::min(_channels, size - (_startChannel - 1));

    if (_dim == 0)
    {
        memset(buffer + _startChannel - 1, 0x00, chs);
        return;
    }

    for (int i = 0; i < chs; i++)
    {
        *(buffer + i + _startChannel - 1) = (*(buffer + i + _startChannel - 1) * _dim) / 100;
    }
}
