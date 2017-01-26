#include "OutputProcessDimWhite.h"
#include <wx/xml/xml.h>

OutputProcessDimWhite::OutputProcessDimWhite(wxXmlNode* node) : OutputProcess(node)
{
    _nodes = wxAtol(node->GetAttribute("Nodes", "1"));
    _dim = wxAtol(node->GetAttribute("Dim", "100"));
}

OutputProcessDimWhite::OutputProcessDimWhite(const OutputProcessDimWhite& op) : OutputProcess(op)
{
    _nodes = op._nodes;
    _dim = op._dim;
}

OutputProcessDimWhite::OutputProcessDimWhite() : OutputProcess()
{
    _nodes = 1;
    _dim = 100;
}

OutputProcessDimWhite::OutputProcessDimWhite(size_t _startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(_startChannel, description)
{
    _nodes = p1;
    _dim = p2;
}

wxXmlNode* OutputProcessDimWhite::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPDimWhite");

    res->AddAttribute("Nodes", wxString::Format(wxT("%i"), _nodes));
    res->AddAttribute("Dim", wxString::Format(wxT("%i"), _dim));

    return res;
}

void OutputProcessDimWhite::Frame(wxByte* buffer, size_t size)
{
    if (_dim == 100) return;

    size_t nodes = std::min(_nodes, (size - (_startChannel - 1)) / 3);

    for (int i = 0; i < nodes; i++)
    {
        wxByte* p = buffer + (_startChannel - 1) + (i * 3);

        if (*p == *(p+1) == *(p+2))
        {
            *p = *p * _dim / 100;
            *(p + 1) = *p;
            *(p + 2) = *p;
        }
    }
}
