#ifndef SYNCFPP_H
#define SYNCFPP_H

#include "SyncManager.h"
#include "ScheduleOptions.h"
#include <wx/socket.h>

class ListenerManager;

class SyncBroadcastFPP : public SyncBase
{
    wxDatagramSocket* _fppBroadcastSocket = nullptr;
    wxIPV4address _remoteAddr;

    void SendFPPSync(const std::string& item, uint32_t stepMS, uint32_t frameMS) const;

    public:

        SyncBroadcastFPP(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncBroadcastFPP(SyncBroadcastFPP& from);
        virtual ~SyncBroadcastFPP();
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
        virtual std::string GetType() const override { return "FPPBROADCAST"; }
        virtual void SendStop() const override;
};

class SyncUnicastFPP : public SyncBase
{
    wxDatagramSocket* _fppUnicastSocket = nullptr;
    std::list<std::string> _remotes;

    void SendUnicastSync(const std::string& ip, const std::string& syncItem, size_t msec, size_t frameMS, int action) const;
    void SendFPPSync(const std::string& item, uint32_t stepMS, uint32_t frameMS) const;

    public:

        SyncUnicastFPP(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncUnicastFPP(SyncUnicastFPP& from);
        virtual ~SyncUnicastFPP();
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
        virtual std::string GetType() const override { return "FPPUNICAST"; }
        virtual void SendStop() const override;
};
#endif