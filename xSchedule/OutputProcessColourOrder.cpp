#include "OutputProcessColourOrder.h"
#include <wx/xml/xml.h>

OutputProcessColourOrder::OutputProcessColourOrder(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _nodes = wxAtol(node->GetAttribute("Nodes", "1"));
    _colourOrder = wxAtol(node->GetAttribute("ColourOrder", "312"));
}

OutputProcessColourOrder::OutputProcessColourOrder(const OutputProcessColourOrder& op) : OutputProcess(op)
{
    _nodes = op._nodes;
    _colourOrder = op._colourOrder;
}

OutputProcessColourOrder::OutputProcessColourOrder(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _nodes = 1;
    _colourOrder = 312;
}

OutputProcessColourOrder::OutputProcessColourOrder(OutputManager* outputManager, std::string startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(outputManager, startChannel, description)
{
    _nodes = p1;
    _colourOrder = p2;
}

wxXmlNode* OutputProcessColourOrder::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPColourOrder");

    res->AddAttribute("Nodes", wxString::Format(wxT("%ld"), (long)_nodes));
    res->AddAttribute("ColourOrder", wxString::Format(wxT("%ld"), (long)_colourOrder));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessColourOrder::Frame(wxByte* buffer, size_t size)
{
    if (!_enabled) return;
    if (_colourOrder == 123) return;

    size_t sc = GetStartChannelAsNumber();

    size_t nodes = std::min(_nodes, (size - (sc - 1)) / 3);

    for (int i = 0; i < nodes; i++)
    {
        wxByte* p = buffer + (sc - 1) + (i * 3);
		wxByte r = *p;
		wxByte g = *(p+1);
		wxByte b = *(p+2);

		switch(_colourOrder)
		{
			case 132:
				*(p+1) = b;
				*(p+2) = g;
				break;
			case 213:
				*(p) = g;
				*(p+1) = r;
				break;
			case 231:
				*(p) = g;
				*(p+1) = b;
				*(p+2) = r;
				break;
			case 312:
				*(p) = b;
				*(p+1) = r;
				*(p+2) = g;
				break;
			case 321:
				*(p) = b;
				*(p+1) = g;
				*(p+2) = r;
				break;
		}
    }
}
