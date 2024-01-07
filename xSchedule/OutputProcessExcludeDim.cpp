/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputProcessExcludeDim.h"
#include <wx/xml/xml.h>

OutputProcessExcludeDim::OutputProcessExcludeDim(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _channels = wxAtol(node->GetAttribute("Channels", "1"));
}

OutputProcessExcludeDim::OutputProcessExcludeDim(const OutputProcessExcludeDim& op) : OutputProcess(op)
{
    _channels = op._channels;
}

OutputProcessExcludeDim::OutputProcessExcludeDim(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _channels = 1;
}

OutputProcessExcludeDim::OutputProcessExcludeDim(OutputManager* outputManager, std::string startChannel, size_t p1, const std::string& description) : OutputProcess(outputManager, startChannel, description)
{
    _channels = p1;
}

wxXmlNode* OutputProcessExcludeDim::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPExcludeDim");

    res->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));

    OutputProcess::Save(res);

    return res;
}

bool OutputProcessExcludeDim::Contains(size_t sc, size_t ec)
{
    return (sc <= GetLastExcludeChannel() && ec >= GetFirstExcludeChannel() && sc <= ec);
}

