#include "OutputProcessRemap.h"
#include <wx/xml/xml.h>

OutputProcessRemap::OutputProcessRemap(wxXmlNode* node) : OutputProcess(node)
{
    _channels = wxAtol(node->GetAttribute("Channels", "1"));
    _to = wxAtol(node->GetAttribute("To", "1"));
}

OutputProcessRemap::OutputProcessRemap(const OutputProcessRemap& op) : OutputProcess(op)
{
    _channels = op._channels;
    _to = op._to;
}

OutputProcessRemap::OutputProcessRemap(size_t _startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(_startChannel, description)
{
    _to = p1;
    _channels = p2;
}

OutputProcessRemap::OutputProcessRemap() : OutputProcess()
{
    _channels = 1;
    _to = 1;
}

wxXmlNode* OutputProcessRemap::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPRemap");

    res->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));
    res->AddAttribute("To", wxString::Format(wxT("%ld"), (long)_to));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessRemap::Frame(wxByte* buffer, size_t size)
{
    if (_startChannel == _to) return;

    size_t chs1 = std::min(_channels, size - (_startChannel - 1));
    size_t chs2 = std::min(_channels, size - (_to - 1));
    size_t chs = std::min(chs1, chs2);

    memcpy(buffer + _startChannel - 1, buffer + _to - 1, chs);
}
