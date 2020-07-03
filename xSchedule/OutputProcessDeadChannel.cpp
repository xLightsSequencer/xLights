/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputProcessDeadChannel.h"
#include <wx/xml/xml.h>

OutputProcessDeadChannel::OutputProcessDeadChannel(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _channel = wxAtol(node->GetAttribute("Channel", "1"));
}

OutputProcessDeadChannel::OutputProcessDeadChannel(const OutputProcessDeadChannel& op) : OutputProcess(op)
{
    _channel = op._channel;
}

OutputProcessDeadChannel::OutputProcessDeadChannel(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _channel = 1;
}

OutputProcessDeadChannel::OutputProcessDeadChannel(OutputManager* outputManager, std::string startChannel, size_t p1, const std::string& description) : OutputProcess(outputManager, startChannel, description)
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

void OutputProcessDeadChannel::Frame(uint8_t* buffer, size_t size)
{
    if (!_enabled) return;

    size_t sc = GetStartChannelAsNumber();

    uint8_t* p = buffer + (sc - 1);

    // if our dead channel is not zero then blank the node
    // this ensures the pixel never looks the wrong colour
    if (*(p + _channel - 1) != 0)
    {
        *(p) = 0;
        *(p + 1) = 0;
        *(p + 2) = 0;
    }
}
