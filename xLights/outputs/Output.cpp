
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>

#include "Output.h"
#include "E131Output.h"
#include "ZCPPOutput.h"
#include "ArtNetOutput.h"
#include "DDPOutput.h"
#include "NullOutput.h"
#include "LOROutput.h"
#include "LOROptimisedOutput.h"
#include "DLightOutput.h"
#include "DMXOutput.h"
#include "xxxSerialOutput.h"
#include "xxxEthernetOutput.h"
#include "OPCOutput.h"
#include "PixelNetOutput.h"
#include "RenardOutput.h"
#include "OpenPixelNetOutput.h"
#include "OpenDMXOutput.h"
#include "../UtilFunctions.h"
#include "OutputManager.h"

#include <log4cpp/Category.hh>

#pragma region Private Functions
void Output::Save(wxXmlNode* node) {

    node->AddAttribute("NetworkType", wxString(GetType().c_str()));

    node->AddAttribute("MaxChannels", wxString::Format("%d", _channels));

    node->DeleteAttribute("FPPProxy");
    if (IsUsingFPPProxy()) {
        node->AddAttribute("FPPProxy", _fppProxy);
    }

    _dirty = false;
}
#pragma endregion

#pragma region Constructors and Destructors
Output::Output(Output* output) {
    _ok = true;
    _dirty = output->IsDirty();
    _channels = output->GetChannels();
    _startChannel = output->GetStartChannel();
    _suppressDuplicateFrames = output->IsSuppressDuplicateFrames();
    _description_CONVERT = output->GetDescription_CONVERT();
    _autoSize_CONVERT = output->IsAutoSize_CONVERT();
    _fppProxy = output->GetFPPProxyIP();
    _globalFPPProxy = output->_globalFPPProxy;
    _enabled = output->IsEnabled();
}

Output::Output(wxXmlNode* node) {
    _ok = true;

    _channels = wxAtoi(node->GetAttribute("MaxChannels", "0"));

    // Only kept for conversion
    _autoSize_CONVERT = node->GetAttribute("AutoSize", "FALSE") == "TRUE";
    _fppProxy = UnXmlSafe(node->GetAttribute("FPPProxy"));
    _enabled = (node->GetAttribute("Enabled", "Yes") == "Yes");
    _suppressDuplicateFrames = (node->GetAttribute("SuppressDuplicates", "No") == "Yes");
    _description_CONVERT = UnXmlSafe(node->GetAttribute("Description"));

    _dirty = false;
}

Output::Output() {
    _dirty = true;
    _ok = true;
}

Output::~Output() {
    if (_fppProxyOutput != nullptr) {
        delete _fppProxyOutput;
    }
}

wxXmlNode* Output::Save() {
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);
    return node;
}
#pragma endregion 

#pragma region Static Functions
Output* Output::Create(Controller* c, wxXmlNode* node, std::string showDir) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string type = node->GetAttribute("NetworkType", "").ToStdString();
    if (type == OUTPUT_E131) {
        return new E131Output(node);
    }
    else if (type == OUTPUT_ZCPP) {
        return new ZCPPOutput(c, node, showDir);
    }
    else if (type == OUTPUT_NULL) {
        return new NullOutput(node);
    }
    else if (type == OUTPUT_ARTNET) {
        return new ArtNetOutput(node);
    }
    else if (type == OUTPUT_DDP) {
        return new DDPOutput(node);
    }
    else if (type == OUTPUT_DMX) {
        return new DMXOutput(node);
    }
    else if (type == OUTPUT_xxxSERIAL) {
        return new xxxSerialOutput(node);
    }
    else if (type == OUTPUT_xxxETHERNET) {
        return new xxxEthernetOutput(node);
    }
    else if (type == OUTPUT_OPC) {
        return new OPCOutput(node);
    }
    else if (type == OUTPUT_PIXELNET) {
        return new PixelNetOutput(node);
    }
    else if (type == OUTPUT_LOR) {
        return new LOROutput(node);
    }
    else if (type == OUTPUT_LOR_OPT) {
        return new LOROptimisedOutput(node);
    }
    else if (type == OUTPUT_DLIGHT) {
        return new DLightOutput(node);
    }
    else if (type == OUTPUT_RENARD) {
        return new RenardOutput(node);
    }
    else if (type == OUTPUT_OPENDMX) {
        return new OpenDMXOutput(node);
    }
    else if (type == OUTPUT_OPENPIXELNET) {
        return new OpenPixelNetOutput(node);
    }

    logger_base.warn("Unknown network type %s ignored.", (const char *)type.c_str());
    wxASSERT(false);
    return nullptr;
}
#pragma endregion

#pragma region Getters and Setters
int Output::GetBaudRate() const {
    if (_baudRate == 0) return 115200;
    return _baudRate;
}

void Output::SetIP(const std::string& ip) {
    auto i = CleanupIP(ip);
    if (i != _ip) {
        _ip = i;
        _resolvedIp = _ip;
        _dirty = true;
    }
}

void Output::SetTransientData(int32_t& startChannel, int nullnumber) {
    wxASSERT(startChannel != -1);
    _startChannel = startChannel;
    if (GetType() == OUTPUT_NULL) {
        _nullNumber = nullnumber++;
    }
    
    if (_fppProxyOutput) {
        _fppProxyOutput->SetTransientData(startChannel, nullnumber);
    }
    startChannel += GetChannels();
}
#pragma endregion

#pragma region Operators
bool Output::operator==(const Output& output) const {
    if (GetType() != output.GetType()) return false;

    if (IsIpOutput()) {
        return _universe == output.GetUniverse() && (_ip == output.GetIP() || _resolvedIp == output.GetIP() || _resolvedIp == output.GetResolvedIP());
    }
    else {
        return _commPort == output.GetCommPort();
    }
}
#pragma endregion 

#pragma region Start and Stop
bool Output::Open() {
    _changed = false;
    _skippedFrames = 9999;
    _lastOutputTime = 0;

    // We only proxy IP outputs
    if (IsIpOutput()) {
        std::string proxy = _fppProxy;
        if (proxy == "") {
            proxy = _globalFPPProxy;
        }

        if (proxy != "") {
            _fppProxyOutput = new DDPOutput();
            _fppProxyOutput->_ip = proxy;
            _fppProxyOutput->_startChannel = _startChannel;
            _fppProxyOutput->_channels = GetEndChannel() - _startChannel + 1;
            _fppProxyOutput->Open();
        }
    }
    return true;
}

void Output::Close() {
    if (_fppProxyOutput) {
        _fppProxyOutput->Close();
        delete _fppProxyOutput;
        _fppProxyOutput = nullptr;
    }
}
#pragma endregion 

#pragma region Data Setting
// channel here is 0 based
void Output::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (!_enabled) return;

    if (_fppProxyOutput) {
        _fppProxyOutput->SetManyChannels(channel, data, size);
        return;
    }
    size_t chs = (std::min)(size, (size_t)(GetMaxChannels() - channel));

    for (size_t i = 0; i < chs; i++) {
        SetOneChannel(channel + i, data[i]);
    }
}
#pragma endregion 

#pragma region Frame Handling
void Output::FrameOutput() {
    _lastOutputTime = wxGetUTCTimeMillis();
    _skippedFrames = 0;
    _changed = false;
    OutputManager::RegisterSentPacket();
}
#pragma endregion 
