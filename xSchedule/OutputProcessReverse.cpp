#include "OutputProcessReverse.h"
#include <wx/xml/xml.h>

OutputProcessReverse::OutputProcessReverse(wxXmlNode* node) : OutputProcess(node)
{
    _nodes = wxAtol(node->GetAttribute("Nodes", "1"));
}

OutputProcessReverse::OutputProcessReverse(const OutputProcessReverse& op) : OutputProcess(op)
{
    _nodes = op._nodes;
}

OutputProcessReverse::OutputProcessReverse() : OutputProcess()
{
    _nodes = 1;
}

OutputProcessReverse::OutputProcessReverse(size_t _startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(_startChannel, description)
{
    _nodes = p1;
}

wxXmlNode* OutputProcessReverse::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPReverse");

    res->AddAttribute("Nodes", wxString::Format(wxT("%ld"), (long)_nodes));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessReverse::Frame(wxByte* buffer, size_t size)
{
    if (_nodes < 2) return;

    size_t nodes = std::min(_nodes, (size - (_startChannel - 1)) / 3);
    wxByte* p = buffer + (_startChannel - 1);

	wxByte rgb[3];
	wxByte* from = p;
	wxByte* to = p + (nodes - 1) * 3;
		
	for (int i = 0; i < nodes; i++)
	{
		memcpy(rgb, from, 3);
		memcpy(from, to, 3);
		memcpy(to, rgb, 3);

		from += 3;
		to -= 3;
    }
}
