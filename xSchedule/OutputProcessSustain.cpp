/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputProcessSustain.h"
#include <wx/xml/xml.h>

OutputProcessSustain::OutputProcessSustain(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _save = nullptr;
    _channels = wxAtol(node->GetAttribute("Channels", "1"));
}

OutputProcessSustain::OutputProcessSustain(const OutputProcessSustain& op) : OutputProcess(op)
{
    _save = nullptr;
    _channels = op._channels;
}

OutputProcessSustain::OutputProcessSustain(OutputManager* outputManager, std::string startChannel, size_t p1, const std::string& description) : OutputProcess(outputManager, startChannel, description)
{
    _save = nullptr;
    _channels = p1;
}

OutputProcessSustain::~OutputProcessSustain()
{
    if (_save != nullptr) free(_save);
}

OutputProcessSustain::OutputProcessSustain(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _save = nullptr;
    _channels = 1;
}

wxXmlNode* OutputProcessSustain::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPSustain");

    res->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessSustain::Frame(uint8_t* buffer, size_t size)
{
    size_t sc = GetStartChannelAsNumber();
    size_t chs = std::min(_channels, size - (sc - 1));

    if (_save == nullptr)
    {
        _save = (uint8_t*)malloc(chs);
        memset(_save, 0x00, chs);
    }

    // copy from save wherever the value is zero
    for (long i = 0; i < chs; i++)
    {
        if (*(buffer + sc - 1 + i) == 0)
        {
            *(buffer + sc - 1 + i) = *(_save + i);
        }
    }

    // back everything up for next time
    memcpy(_save, buffer + sc - 1, chs);
}
