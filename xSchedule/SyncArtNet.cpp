#include "SyncArtNet.h"
#include "ScheduleOptions.h"
#include "events/ListenerManager.h"
#include "../xLights/outputs/IPOutput.h"

#include <log4cpp/Category.hh>
#include "../xLights/UtilFunctions.h"
#include "../xLights/outputs/ArtNetOutput.h"

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
        }
    }

    if (rm == REMOTEMODE::ARTNETSLAVE)
    {
        listenerManager->SetRemoteArtNet();
    }
}

SyncArtNet::SyncArtNet(SyncArtNet&& from) : SyncBase(from)
{
    _artnetSocket = from._artnetSocket;
    from._artnetSocket = nullptr; // this is a transfer of ownership
    _remoteAddr = from._remoteAddr;
}

SyncArtNet::~SyncArtNet()
{
    if (_artnetSocket != nullptr) {
        _artnetSocket->Close();
        delete _artnetSocket;
        _artnetSocket = nullptr;
    }
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
    ms = ms % 360000;

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
