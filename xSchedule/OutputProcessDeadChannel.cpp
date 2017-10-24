#include "OutputProcessDeadChannel.h"
#include <wx/xml/xml.h>

OutputProcessDeadChannel::OutputProcessDeadChannel(wxXmlNode* node) : OutputProcess(node)
{
    _channel = wxAtol(node->GetAttribute("Channel", "1"));
}

OutputProcessDeadChannel::OutputProcessDeadChannel(const OutputProcessDeadChannel& op) : OutputProcess(op)
{
    _channel = op._channel;
}

OutputProcessDeadChannel::OutputProcessDeadChannel() : OutputProcess()
{
    _channel = 1;
}

OutputProcessDeadChannel::OutputProcessDeadChannel(size_t _startChannel, size_t p1, const std::string& description) : OutputProcess(_startChannel, description)
{
    _channel = p1;
}

wxXmlNode* OutputProcessDeadChannel::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPDeadChannel");

    res->AddAttribute("Channel", wxString::Format(wxT("%ld"), (long)_channel));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessDeadChannel::Frame(wxByte* buffer, size_t size)
{
    wxByte* p = buffer + (_startChannel - 1);

    // if our dead channel is not zero then blank the node
    // this ensures the pixel never looks the wrong colour
    if (*(p + _channel - 1) != 0)
    {
        *(p) = 0;
        *(p + 1) = 0;
        *(p + 2) = 0;
    }
}
