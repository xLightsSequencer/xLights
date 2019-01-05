#include "OutputProcessReverse.h"
#include <wx/xml/xml.h>

OutputProcessReverse::OutputProcessReverse(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _nodes = wxAtol(node->GetAttribute("Nodes", "1"));
}

OutputProcessReverse::OutputProcessReverse(const OutputProcessReverse& op) : OutputProcess(op)
{
    _nodes = op._nodes;
}

OutputProcessReverse::OutputProcessReverse(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _nodes = 1;
}

OutputProcessReverse::OutputProcessReverse(OutputManager* outputManager, std::string startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(outputManager, startChannel, description)
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

void OutputProcessReverse::Frame(uint8_t* buffer, size_t size)
{
    if (_nodes < 2) return;

    size_t sc = GetStartChannelAsNumber();

    size_t nodes = std::min(_nodes, (size - (sc - 1)) / 3);
    uint8_t* p = buffer + (sc - 1);

	uint8_t rgb[3];
	uint8_t* from = p;
	uint8_t* to = p + (nodes - 1) * 3;
		
	for (int i = 0; i < nodes; i++)
	{
		memcpy(rgb, from, 3);
		memcpy(from, to, 3);
		memcpy(to, rgb, 3);

		from += 3;
		to -= 3;
    }
}
