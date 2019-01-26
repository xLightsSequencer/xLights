#ifndef SYNCOSC_H
#define SYNCOSC_H

#include "SyncManager.h"
#include "ScheduleOptions.h"

#include <wx/socket.h>

class ListenerManager;
class OSCPacket;

class SyncOSC : public SyncBase
{
    wxDatagramSocket* _oscSocket = nullptr;
    std::string _path = "";
    bool _isTime = true;
    OSCTIME _timeType = OSCTIME::TIME_MILLISECONDS;
    OSCFRAME _frameCode = OSCFRAME::FRAME_DEFAULT;
    wxIPV4address _remoteAddr;

    void SendOSC(const OSCPacket& osc) const;

    public:

        SyncOSC(SYNCMODE mode, REMOTEMODE remoteMode, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncOSC(SyncOSC&& from);
        virtual ~SyncOSC();
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
        virtual std::string GetType() const override { return "OSC"; }
        virtual void SendStop() const override;
};
#endif
