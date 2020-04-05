#ifndef SYNCARTNET_H
#define SYNCARTNET_H

#include "SyncManager.h"
#include "ScheduleOptions.h"

#include <wx/sckaddr.h>

class ListenerManager;
class wxDatagramSocket;
class ArtNetTimecodeThread;

class SyncArtNet : public SyncBase
{
    wxDatagramSocket* _artnetSocket = nullptr;
    wxIPV4address _remoteAddr;
    TIMECODEFORMAT _timeCodeFormat = TIMECODEFORMAT::F24;
    ArtNetTimecodeThread* _threadTimecode = nullptr;

    public:

        SyncArtNet(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncArtNet(SyncArtNet&& from) noexcept;
        virtual ~SyncArtNet();
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
        virtual std::string GetType() const override { return "ARTNET"; }
        virtual void SendStop() const override;
        double GetInterval();
        virtual bool IsReactive() const override { return false; }
};
#endif
