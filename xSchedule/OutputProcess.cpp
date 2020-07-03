/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputProcess.h"
#include <wx/xml/xml.h>
#include "OutputProcessDim.h"
#include "OutputProcessDimWhite.h"
#include "OutputProcessThreeToFour.h"
#include "OutputProcessSet.h"
#include "OutputProcessSustain.h"
#include "OutputProcessRemap.h"
#include "OutputProcessReverse.h"
#include "OutputProcessGamma.h"
#include "OutputProcessColourOrder.h"
#include "OutputProcessDeadChannel.h"
#include "../xLights/outputs/OutputManager.h"

OutputProcess::OutputProcess(OutputManager* outputManager, wxXmlNode* node)
{
    _sc = 0;
    _outputManager = outputManager;
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _description = node->GetAttribute("Description", "").ToStdString();
    _enabled = node->GetAttribute("Enabled", "TRUE") == "TRUE";
}

OutputProcess::OutputProcess(const OutputProcess& op)
{
    _sc = 0;
    _outputManager = op._outputManager;
    _description = op._description;
    _changeCount = op._changeCount;
    _enabled = op._enabled;
    _lastSavedChangeCount = op._lastSavedChangeCount;
    _startChannel = op._startChannel;
}

OutputProcess::OutputProcess(OutputManager* outputManager)
{
    _sc = 0;
    _outputManager = outputManager;
    _changeCount = 1;
    _lastSavedChangeCount = 0;
    _startChannel = 1;
    _description = "";
    _enabled = true;
}

OutputProcess::OutputProcess(OutputManager* outputManager, std::string startChannel, const std::string& description)
{
    _sc = 0;
    _outputManager = outputManager;
    _changeCount = 1;
    _lastSavedChangeCount = 0;
    _startChannel = startChannel;
    _description = description;
    _enabled = true;
}

size_t OutputProcess::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

void OutputProcess::Save(wxXmlNode* node)
{
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Description", _description);
    if (!_enabled)
    {
        node->AddAttribute("Enabled", "FALSE");
    }
}

OutputProcess* OutputProcess::CreateFromXml(OutputManager* outputManager, wxXmlNode* node)
{
    if (node->GetName() == "OPDim")
    {
        return new OutputProcessDim(outputManager, node);
    }
    else if (node->GetName() == "OPDimWhite")
    {
        return new OutputProcessDimWhite(outputManager, node);
    }
    else if (node->GetName() == "OP3To4")
    {
        return new OutputProcessThreeToFour(outputManager, node);
    }
    else if (node->GetName() == "OPSet")
    {
        return new OutputProcessSet(outputManager, node);
    }
    else if (node->GetName() == "OPSustain")
    {
        return new OutputProcessSustain(outputManager, node);
    }
    else if (node->GetName() == "OPRemap")
    {
        return new OutputProcessRemap(outputManager, node);
    }
    else if (node->GetName() == "OPReverse")
    {
        return new OutputProcessReverse(outputManager, node);
    }
    else if (node->GetName() == "OPColourOrder")
    {
        return new OutputProcessColourOrder(outputManager, node);
    }
    else if (node->GetName() == "OPGamma")
    {
        return new OutputProcessGamma(outputManager, node);
    }
    else if (node->GetName() == "OPDeadChannel")
    {
        return new OutputProcessDeadChannel(outputManager, node);
    }
    return nullptr;
}
