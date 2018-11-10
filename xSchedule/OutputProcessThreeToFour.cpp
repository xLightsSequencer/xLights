#include "OutputProcessThreeToFour.h"
#include <wx/xml/xml.h>

OutputProcessThreeToFour::OutputProcessThreeToFour(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _nodes = wxAtol(node->GetAttribute("Nodes", "1"));
    _colourOrder = node->GetAttribute("ColourOrder", "RGBW").ToStdString();
}

OutputProcessThreeToFour::OutputProcessThreeToFour(const OutputProcessThreeToFour& op) : OutputProcess(op)
{
    _nodes = op._nodes;
    _colourOrder = op._colourOrder;
}

OutputProcessThreeToFour::OutputProcessThreeToFour(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _nodes = 1;
    _colourOrder = "RGBW";
}

OutputProcessThreeToFour::OutputProcessThreeToFour(OutputManager* outputManager, std::string startChannel, size_t p1, std::string colourOrder, const std::string& description) : OutputProcess(outputManager, startChannel, description)
{
    _nodes = p1;
    _colourOrder = colourOrder;
}

wxXmlNode* OutputProcessThreeToFour::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OP3To4");

    res->AddAttribute("Nodes", wxString::Format(wxT("%ld"), (long)_nodes));
    res->AddAttribute("ColourOrder", _colourOrder);

    OutputProcess::Save(res);

    return res;
}

void OutputProcessThreeToFour::Frame(uint8_t* buffer, size_t size)
{
    if (!_enabled) return;

    size_t sc = GetStartChannelAsNumber();
    size_t nodes = std::min(_nodes, (size - (sc - 1)) / 4); // divide by 4 as that is what we are expanding it to

	uint8_t* target = buffer + sc - 1 + (nodes - 1) * 4;
	uint8_t* source = buffer + sc - 1 + (nodes - 1) * 3;
	
    for (int i = 0; i < nodes; i++)
    {
		uint8_t white = 0;
		if (*source == *(source + 1) && *source == *(source + 2))
		{
			white = *source;
		}
		if (_colourOrder == "RGBW")
		{
			if (white != 0)
			{
				memset(target, 0x00, 3);
				*(target+3) = white;
			}
			else
			{
				memcpy(target, source, 3);
				*(target+3) = 0x00;
			}
		}
		else if (_colourOrder == "WRGB")
		{
			if (white != 0)
			{
				memset(target + 1, 0x00, 3);
				*target = white;
			}
			else
			{
				memcpy(target + 1, source, 3);
				*target = 0x00;
			}
		}
		else
		{
			wxASSERT(false);
		}
		
		source -= 3;
		target -= 4;
    }
}
