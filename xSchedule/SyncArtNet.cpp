/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SyncArtNet.h"
#include "ScheduleOptions.h"
#include "events/ListenerManager.h"
#include "../xLights/outputs/IPOutput.h"
#include "PlayList/PlayList.h"
#include "ScheduleManager.h"

#include <log4cpp/Category.hh>
#include "../xLights/UtilFunctions.h"
#include "../xLights/outputs/ArtNetOutput.h"

class ArtNetTimecodeThread : public wxThread
{
    std::atomic<bool> _stop;
    SyncArtNet* _syncArtNet = nullptr;
    std::atomic<bool> _running;
    std::atomic<bool> _suspend;
    ScheduleManager* _scheduleManager = nullptr;
    bool _toSendStop = false; // prevents us sending multiple stops

public:
    ArtNetTimecodeThread(SyncArtNet* syncArtNet, ScheduleManager* scheduleManager)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        _suspend = false;
        _running = false;
        _stop = false;
        _syncArtNet = syncArtNet;
        _scheduleManager = scheduleManager;

        if (Run() != wxTHREAD_NO_ERROR)
        {
            logger_base.error("Failed to start ArtNet Timecode thread");
        }
        else
        {
            logger_base.info("ArtNet Timecode thread created.");
        }
    }
    virtual ~ArtNetTimecodeThread()
    {
        Stop();
    }

    void Stop()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("ArtNet Timecode thread stopping.");
        _stop = true;
    }

    void UpdateSyncArtNet(SyncArtNet* syncArtNet)
    {
        _suspend = true;
        wxMilliSleep(100); // ensure it is suspended ... this is lazy ... i really should use sync objects
        _syncArtNet = syncArtNet;
        _suspend = false;
    }

    void* Entry()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        wxLongLong last = 0;
        double interval = _syncArtNet->GetInterval() * 1000.0; 
        _running = true;
        while (!_stop)
        {
            if (!_suspend)
            {
                long long sleepfor = 0;

                if (wxGetUTCTimeUSec() - last > interval)
                {
                    // get our absolute position
                    PlayList* pl = _scheduleManager->GetRunningPlayList();
                    if (pl != nullptr)
                    {
                        // sent a sync
                        auto ms = pl->GetPosition();
                        _syncArtNet->SendSync(0, 0, 0, ms, "", "", "", "");
                        _toSendStop = true;
                    }
                    else
                    {
                        if (_toSendStop)
                        {
                            _syncArtNet->SendStop();
                            _toSendStop = false;
                        }
                    }

                    sleepfor = (int64_t)((last.GetValue()) + interval - wxGetUTCTimeUSec().GetValue());
                    last = wxGetUTCTimeUSec().GetValue();
                }

                if (sleepfor > 0)
                {
                    wxMicroSleep(sleepfor);
                }
            }
        }
        _running = false;
        logger_base.info("ArtNet Timecode thread stopped.");
        return nullptr;
    }
};

SyncArtNet::SyncArtNet(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager) : SyncBase(sm, rm)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _timeCodeFormat = options.GetARTNetTimeCodeFormat();

    _remoteAddr.Hostname("255.255.255.255");
    _remoteAddr.Service(ARTNET_PORT);

    if (sm == SYNCMODE::ARTNETMASTER)
    {
        wxIPV4address localaddr;
        if (IPOutput::GetLocalIP() == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(IPOutput::GetLocalIP());
        }

        _artnetSocket = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);
        if (_artnetSocket == nullptr)
        {
            logger_base.error("Error opening datagram for ARTNet Sync as master. %s", (const char *)localaddr.IPAddress().c_str());
        }
        else if (!_artnetSocket->IsOk())
        {
            logger_base.error("Error opening datagram for ARTNet Sync as master. %s OK : FALSE", (const char *)localaddr.IPAddress().c_str());
            delete _artnetSocket;
            _artnetSocket = nullptr;
        }
        else if (_artnetSocket->Error())
        {
            logger_base.error("Error opening datagram for ARTNet Sync as master. %d : %s",
                _artnetSocket->LastError(),
                (const char*)DecodeIPError(_artnetSocket->LastError()).c_str());
            delete _artnetSocket;
            _artnetSocket = nullptr;
        }
        else
        {
            logger_base.info("ARTNet Sync as master datagram opened successfully.");
            _threadTimecode = new ArtNetTimecodeThread(this, listenerManager->GetScheduleManager());
        }
    }

    if (rm == REMOTEMODE::ARTNETSLAVE)
    {
        listenerManager->SetRemoteArtNet();
    }
}

SyncArtNet::SyncArtNet(SyncArtNet&& from) noexcept : SyncBase(from)
{
    _threadTimecode = from._threadTimecode;
    from._threadTimecode = nullptr; // this is a transfer of ownership
    if (_threadTimecode != nullptr) {
        _threadTimecode->UpdateSyncArtNet(this);
    }

    _artnetSocket = from._artnetSocket;
    from._artnetSocket = nullptr; // this is a transfer of ownership
    _remoteAddr = from._remoteAddr;
}

SyncArtNet::~SyncArtNet()
{
    // close the sending thread
    if (_threadTimecode != nullptr)
    {
        //logger_base.debug("MIDI Timecode stopping.");
        _threadTimecode->Stop();
        _threadTimecode->Delete();
        _threadTimecode = nullptr;
    }

    if (_artnetSocket != nullptr) {
        _artnetSocket->Close();
        delete _artnetSocket;
        _artnetSocket = nullptr;
    }
}

double SyncArtNet::GetInterval()
{
    switch (_timeCodeFormat)
    {
    default:
    case TIMECODEFORMAT::F24: // 24 fps
        return 1000.0 / 24.0;
    case TIMECODEFORMAT::F25: // 25 fps
        return 1000.0 / 25.0;
    case TIMECODEFORMAT::F2997: // 29.97 fps
        return 1000.0 / 29.97;
    case TIMECODEFORMAT::F30: // 30 fps
        return 1000.0 / 30.0;
        break;
    }
    return 1000.0 / 25.0;
}

void SyncArtNet::SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const
{
    if (_artnetSocket == nullptr) return;

    // Artnet sends sync for every frame

    std::vector<uint8_t> buffer(19);

    buffer[0] = 'A';
    buffer[1] = 'r';
    buffer[2] = 't';
    buffer[3] = '-';
    buffer[4] = 'N';
    buffer[5] = 'e';
    buffer[6] = 't';
    buffer[9] = 0x97;
    buffer[11] = 0x0E;

    size_t ms = playlistMS;

    if (ms == 0xFFFFFFFF)
    {
        ms = 0;
    }

    buffer[17] = ms / (3600000);
    ms = ms % 3600000;

    buffer[16] = ms / 60000;
    ms = ms % 60000;

    buffer[15] = ms / 1000;
    ms = ms % 1000;

    buffer[18] = static_cast<int>(_timeCodeFormat);

    switch (static_cast<TIMECODEFORMAT>(buffer[16]))
    {
    case TIMECODEFORMAT::F24: //24 fps
        buffer[14] = ms * 24 / 1000;
        break;
    case TIMECODEFORMAT::F25: // 25 fps
        buffer[14] = ms * 25 / 1000;
        break;
    case TIMECODEFORMAT::F2997: // 29.97 fps
        buffer[14] = ms * 2997 / 100000;
        break;
    case TIMECODEFORMAT::F30: // 30 fps
        buffer[14] = ms * 30 / 1000;
        break;
    default:
        break;
    }

    _artnetSocket->SendTo(_remoteAddr, &buffer[0], buffer.size());
}

void SyncArtNet::SendStop() const
{
    SendSync(50, 0, 0, 0xFFFFFFFF, "", "", "", "");
}
