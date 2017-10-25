#include "OutputProcess.h"
#include <wx/xml/xml.h>
#include <wx/wxcrt.h>
#include "OutputProcessDim.h"
#include "OutputProcessDimWhite.h"
#include "OutputProcessSet.h"
#include "OutputProcessRemap.h"
#include "OutputProcessReverse.h"
#include "OutputProcessGamma.h"
#include "OutputProcessColourOrder.h"
#include "OutputProcessDeadChannel.h"

OutputProcess::OutputProcess(wxXmlNode* node)
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _startChannel = wxAtol(node->GetAttribute("StartChannel", "1"));
    _description = node->GetAttribute("Description", "").ToStdString();
    _enabled = node->GetAttribute("Enabled", "TRUE") == "TRUE";
}

OutputProcess::OutputProcess(const OutputProcess& op)
{
    _description = op._description;
    _changeCount = op._changeCount;
    _enabled = op._enabled;
    _lastSavedChangeCount = op._lastSavedChangeCount;
    _startChannel = op._startChannel;
}

OutputProcess::OutputProcess()
{
    _changeCount = 1;
    _lastSavedChangeCount = 0;
    _startChannel = 1;
    _description = "";
    _enabled = true;
}

OutputProcess::OutputProcess(size_t startChannel, const std::string& description)
{
    _changeCount = 1;
    _lastSavedChangeCount = 0;
    _startChannel = startChannel;
    _description = description;
    _enabled = true;
}

void OutputProcess::Save(wxXmlNode* node)
{
    node->AddAttribute("StartChannel", wxString::Format(wxT("%ld"), (long)_startChannel));
    node->AddAttribute("Description", _description);
    if (!_enabled)
    {
        node->AddAttribute("Enabled", "FALSE");
    }
}

OutputProcess* OutputProcess::CreateFromXml(wxXmlNode* node)
{
    if (node->GetName() == "OPDim")
    {
        return new OutputProcessDim(node);
    }
    else if (node->GetName() == "OPDimWhite")
    {
        return new OutputProcessDimWhite(node);
    }
    else if (node->GetName() == "OPSet")
    {
        return new OutputProcessSet(node);
    }
    else if (node->GetName() == "OPRemap")
    {
        return new OutputProcessRemap(node);
    }
    else if (node->GetName() == "OPReverse")
    {
        return new OutputProcessReverse(node);
    }
    else if (node->GetName() == "OPColourOrder")
    {
        return new OutputProcessColourOrder(node);
    }
    else if (node->GetName() == "OPGamma")
    {
        return new OutputProcessGamma(node);
    }
    else if (node->GetName() == "OPDeadChannel")
    {
        return new OutputProcessDeadChannel(node);
    }
    return nullptr;
}
