#ifndef SYNCARTNET_H
#define SYNCARTNET_H

#include "SyncManager.h"
#include "ScheduleOptions.h"

#include <wx/sckaddr.h>

class ListenerManager;
class wxDatagramSocket;

class SyncArtNet : public SyncBase
{
    wxDatagramSocket* _artnetSocket = nullptr;
    wxIPV4address _remoteAddr;
    TIMECODEFORMAT _timeCodeFormat = TIMECODEFORMAT::F24;

    public:

        SyncArtNet(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncArtNet(SyncArtNet&& from);
        virtual ~SyncArtNet();
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
        virtual std::string GetType() const override { return "ARTNET"; }
        virtual void SendStop() const override;
};
#endif
