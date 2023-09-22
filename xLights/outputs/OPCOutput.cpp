
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OPCOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "../utils/ip_utils.h"

#include <wx/xml/xml.h>
#include <wx/process.h>

#include <log4cpp/Category.hh>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#pragma region Private Functions
void OPCOutput::OpenSocket() {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr) return;

    _socket = new wxSocketClient();
    if (_socket == nullptr) {
        logger_base.error("OPCOutput: Error opening socket to connect to %s.", (const char*)_remoteAddr.IPAddress().c_str());
    }
    else
    {
        wxIPV4address localaddr;
        if (GetForceLocalIPToUse() == "") {
            localaddr.AnyAddress();
        }
        else {
            localaddr.Hostname(GetForceLocalIPToUse());
        }

        _socket->SetFlags(wxSOCKET_NOWAIT);
        _socket->SetLocal(localaddr);
        _socket->Connect(_remoteAddr, true);
        if (!_socket->IsOk()) {
            logger_base.error("OPCOutput: %s Error connecting to socket. Network may not be connected? OK : FALSE", (const char*)_remoteAddr.IPAddress().c_str());
            delete _socket;
            _socket = nullptr;
        }
        else if (_socket->Error()) {
            logger_base.error("OPCOutput: %s Error connecting OPC socket => %d : %s.", (const char*)_remoteAddr.IPAddress().c_str(), _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str());
            delete _socket;
            _socket = nullptr;
        }
        else
        {
            logger_base.error("OPCOutput: OPC socket connected to %s.", (const char*)_remoteAddr.IPAddress().c_str());
            #ifdef __linux__
            // The pixels sent is timing sensitive, TCP_NODELAY disables
            // Nagle algorithm to delay sending out TCP packets to combine
            // with later data.  This needs to not delay or combine writes.
            int optval = 1;
            if(setsockopt(_socket->GetSocket(), IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1)
                logger_base.error("OPCOutput: failed to set TCP_NODELAY");
            #endif
        }
    }
}
#pragma endregion

#pragma region Constructors and Destructors
OPCOutput::OPCOutput(wxXmlNode* node, bool isActive) : IPOutput(node, isActive) {

    if (_channels > GetMaxChannels()) SetChannels(GetMaxChannels());
    _socket = nullptr;
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

OPCOutput::OPCOutput() : IPOutput() {

    _channels = 510;
    _universe = 1;
    _socket = nullptr;
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

OPCOutput::OPCOutput(const OPCOutput& from) :
    IPOutput(from)
{
    _channels = from._channels;
    _universe = from._universe;
    _socket = nullptr;
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

OPCOutput::~OPCOutput()
{
    if (_socket != nullptr) delete _socket;
    if (_data != nullptr) delete _data;
}

wxXmlNode* OPCOutput::Save() {

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    IPOutput::Save(node);

    return node;
}
#pragma endregion

#pragma region Getters and Setters
std::string OPCOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "OPC {" + wxString::Format(wxT("%i"), _universe).ToStdString() + "} ";
    res += "[1-" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ")";

    return res;
}

void OPCOutput::SetChannels(int32_t channels)
{
    IPOutput::SetChannels(channels);
    if (_data != nullptr)
    {
        free(_data);
        _data = nullptr;
    }
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

std::string OPCOutput::GetExport() const {

    return wxString::Format(",%ld,%ld,,%s,%s,,,,%d,%i",
        GetStartChannel(),
        GetEndChannel(),
        GetType(),
        GetIP(),
        GetUniverse(),
        GetChannels());
}

void OPCOutput::SetTransientData(int32_t& startChannel, int nullnumber) {

    //if (_fppProxyOutput) {
    //    _fppProxyOutput->SetTransientData(startChannel, nullnumber);
    //}

    wxASSERT(startChannel != -1);
    //_outputNumber = on++;
    _startChannel = startChannel;
    startChannel += GetChannels();
}
#pragma endregion

#pragma region Start and Stop
bool OPCOutput::Open() {

    if (!_enabled) return true;
    if (_ip == "") return false;
    if (!ip_utils::IsIPValid(_resolvedIp)) return false;

    _ok = IPOutput::Open();
    //if (_fppProxyOutput) {
    //    return _ok;
    //}

    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);

    _data[0] = (uint8_t)_universe;
    _data[1] = 0x00; // RGB values
    _data[2] = (uint8_t)((_channels & 0xFF00) >> 8);
    _data[3] = (uint8_t)(_channels & 0xFF);

    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(OPC_PORT);

    OpenSocket();

    return _ok && _socket != nullptr;
}

void OPCOutput::Close() {

    if (_socket != nullptr) {
        //while (_socket->WaitForWrite(0, 50));
        delete _socket;
        _socket = nullptr;
    }
    IPOutput::Close();
}
#pragma endregion 

#pragma region Frame Handling
void OPCOutput::StartFrame(long msec) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    return _fppProxyOutput->StartFrame(msec);
    //}

    if (_socket == nullptr && OutputManager::IsRetryOpen()) {
        OpenSocket();
        if (_ok) {
            logger_base.debug("OPCOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void OPCOutput::EndFrame(int suppressFrames) {

    // on really low end controllers I have seen this get so backed up it re-enters
    static bool reentry = false;

    if (!_enabled || _suspend || _tempDisable) return;

    //if (_fppProxyOutput) {
    //    _fppProxyOutput->EndFrame(suppressFrames);
    //    return;
    //}

    if (_socket == nullptr) return;

    if (reentry) return;
    reentry = true;

    if (_changed || NeedToOutput(suppressFrames)) {
        _socket->Write(_data, _channels + OPC_PACKET_HEADERLEN);
        FrameOutput();
    }
    else {
        SkipFrame();
    }
    reentry = false;
}

void OPCOutput::ResetFrame() {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->ResetFrame();
    //    return;
    //}
}
#pragma endregion

#pragma region Data Setting
void OPCOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->SetOneChannel(channel, data);
    //    return;
    //}

    if (_data[channel + OPC_PACKET_HEADERLEN] != data) {
        _data[channel + OPC_PACKET_HEADERLEN] = data;
        _changed = true;
    }
}

void OPCOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->SetManyChannels(channel, data, size);
    //} 
    //else {
        size_t chs = (std::min)(size, (size_t)(GetMaxChannels() - channel));

        if (memcmp(&_data[channel + OPC_PACKET_HEADERLEN], data, chs) == 0) {
            // nothing changed
        } else {
            memcpy(&_data[channel + OPC_PACKET_HEADERLEN], data, chs);
            _changed = true;
        }
    //}
}

void OPCOutput::AllOff() {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->AllOff();
    //} 
    //else {
        memset(&_data[OPC_PACKET_HEADERLEN], 0x00, _channels);
        _changed = true;
    //}
}
#pragma endregion



#pragma region UI
#ifndef EXCLUDENETWORKUI
#include "ControllerEthernet.h"
void OPCOutput::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::UpdateProperties(propertyGrid, c, modelManager, expandProperties);
    ControllerEthernet *ce = dynamic_cast<ControllerEthernet*>(c);

    auto p = propertyGrid->GetProperty("Channels");
    if (p) {
        p->SetValue(GetChannels());
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPG_PROP_READONLY, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
}

void OPCOutput::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty *before , Controller *c, bool allSameSize, std::list<wxPGProperty*>& expandProperties)
{
    IPOutput::AddProperties(propertyGrid, before, c, allSameSize, expandProperties);
    auto p = propertyGrid->Insert(before, new wxUIntProperty("OPC Channel", "Universe", GetUniverse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 255);
    p->SetEditor("SpinCtrl");
    
    p = propertyGrid->Insert(before, new wxUIntProperty("Message Data Size", "Channels", GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", GetMaxChannels());    
}

bool OPCOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller *c)
{
    if (IPOutput::HandlePropertyEvent(event, outputModelManager, c)) return true;
    return false;
}
void OPCOutput::RemoveProperties(wxPropertyGrid* propertyGrid) {
    IPOutput::RemoveProperties(propertyGrid);    
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Channels");
}
#endif
#pragma endregion
