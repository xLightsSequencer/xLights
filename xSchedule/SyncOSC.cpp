/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SyncOSC.h"
#include "OSCPacket.h"
#include "../xLights/outputs/IPOutput.h"
#include "ScheduleOptions.h"
#include "events/ListenerManager.h"

#include <wx/socket.h>

#include <log4cpp/Category.hh>
#include "../xLights/UtilFunctions.h"

SyncOSC::SyncOSC(SYNCMODE mode, REMOTEMODE remoteMode, const ScheduleOptions& options, ListenerManager* listenerManager) : SyncBase(mode, remoteMode)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mode == SYNCMODE::OSCMASTER)
    {
        int port = options.GetOSCOptions()->GetServerPort();
        _path = options.GetOSCOptions()->GetMasterPath();
        _isTime = options.GetOSCOptions()->IsTime();
        _timeType = options.GetOSCOptions()->GetTimeCode();
        _frameCode = options.GetOSCOptions()->GetFrameCode();
        _remoteAddr.Hostname(options.GetOSCOptions()->GetIPAddress());
        _remoteAddr.Service(port);
        logger_base.error("OSC Sync sending to %s port %d", (const char*)options.GetOSCOptions()->GetIPAddress().c_str(), port);

        wxIPV4address localaddr;
        if (IPOutput::GetLocalIP() == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(IPOutput::GetLocalIP());
        }

        _oscSocket = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);
        if (_oscSocket == nullptr)
        {
            logger_base.error("Error opening datagram for OSC Sync as master. %s", (const char *)localaddr.IPAddress().c_str());
        }
        else if (!_oscSocket->IsOk())
        {
            logger_base.error("Error opening datagram for OSC Sync as master. %s OK : FALSE", (const char *)localaddr.IPAddress().c_str());
            delete _oscSocket;
            _oscSocket = nullptr;
        }
        else if (_oscSocket->Error())
        {
            logger_base.error("Error opening datagram for OSC Sync as master. %d : %s",
                _oscSocket->LastError(),
                (const char*)DecodeIPError(_oscSocket->LastError()).c_str());
            delete _oscSocket;
            _oscSocket = nullptr;
        }
        else
        {
            logger_base.info("OSC Sync as master datagram opened successfully.");
        }
    }

    if (remoteMode == REMOTEMODE::OSCSLAVE)
    {
        listenerManager->SetRemoteOSC();
    }
}

SyncOSC::SyncOSC(SyncOSC&& from) : SyncBase(from)
{
    _oscSocket = from._oscSocket;
    from._oscSocket = nullptr; // this is a transfer of ownership
    _frameCode = from._frameCode;
    _isTime = from._isTime;
    _path = from._path;
    _remoteAddr = from._remoteAddr;
    _timeType = from._timeType;
}

SyncOSC::~SyncOSC()
{
    if (_oscSocket != nullptr) {
        _oscSocket->Close();
        delete _oscSocket;
        _oscSocket = nullptr;
    }
}

void SyncOSC::SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& stepName, const std::string& timingItemName) const
{
    if (_mode != SYNCMODE::OSCMASTER || _oscSocket == nullptr) return;

    auto path = _path;
    if (!stepName.empty()) Replace(path, "%STEPNAME%", stepName);
    if (!timingItemName.empty()) Replace(path, "%TIMINGITEM%", timingItemName);

    if (_isTime)
    {
        switch (_timeType)
        {
        case OSCTIME::TIME_SECONDS:
            SendOSC(OSCPacket(path, static_cast<float>(stepMS) / 1000.0f));
            break;
        case OSCTIME::TIME_MILLISECONDS:
            SendOSC(OSCPacket(path, static_cast<int>(stepMS)));
            break;
        }
    }
    else
    {
        switch (_frameCode)
        {
        case OSCFRAME::FRAME_24:
            SendOSC(OSCPacket(path, static_cast<int>(stepMS * 24 / 1000)));
            break;
        case OSCFRAME::FRAME_25:
            SendOSC(OSCPacket(path, static_cast<int>(stepMS * 25 / 1000)));
            break;
        case OSCFRAME::FRAME_2997:
            SendOSC(OSCPacket(path, static_cast<int>(static_cast<float>(stepMS) * 29.97 / 1000)));
            break;
        case OSCFRAME::FRAME_30:
            SendOSC(OSCPacket(path, static_cast<int>(stepMS * 30 / 1000)));
            break;
        case OSCFRAME::FRAME_60:
            SendOSC(OSCPacket(path, static_cast<int>(stepMS * 60 / 1000)));
            break;
        case OSCFRAME::FRAME_DEFAULT:
            SendOSC(OSCPacket(path, static_cast<int>(stepMS / frameMS)));
            break;
        case OSCFRAME::FRAME_PROGRESS:
            SendOSC(OSCPacket(path, static_cast<float>(stepMS) / static_cast<float>(stepLengthMS)));
            break;
        }
    }
}

void SyncOSC::SendStop() const
{
    // Do nothing
}

void SyncOSC::SendOSC(const OSCPacket& osc) const
{
    _oscSocket->SendTo(_remoteAddr, osc.GetBuffer(), osc.GetBuffSize());
}
