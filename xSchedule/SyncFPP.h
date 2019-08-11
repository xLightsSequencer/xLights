#ifndef SYNCFPP_H
#define SYNCFPP_H

#include "SyncManager.h"
#include "ScheduleOptions.h"
#include <wx/socket.h>

class ListenerManager;

class SyncFPP : public SyncBase
{
    virtual void SendFPPSync(const std::string& item, uint32_t stepMS, uint32_t frameMS) const = 0;

public:

    static void Ping(bool remote);

    SyncFPP(SYNCMODE sm, REMOTEMODE rm) : SyncBase(sm, rm) {}
    virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
    virtual void SendStop() const override;
};

class SyncBroadcastFPP : public SyncFPP
{
    wxDatagramSocket* _fppBroadcastSocket = nullptr;
    wxIPV4address _remoteAddr;

    virtual void SendFPPSync(const std::string& item, uint32_t stepMS, uint32_t frameMS) const override;

    public:

        SyncBroadcastFPP(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncBroadcastFPP(SyncBroadcastFPP&& from) noexcept;
        virtual ~SyncBroadcastFPP();
        virtual std::string GetType() const override { return "FPPBROADCAST"; }
};

class SyncUnicastFPP : public SyncFPP
{
    wxDatagramSocket* _fppUnicastSocket = nullptr;
    std::list<std::string> _remotes;

    void SendUnicastSync(const std::string& ip, const std::string& item, size_t msec, size_t frameMS, int action) const;
    virtual void SendFPPSync(const std::string& item, uint32_t stepMS, uint32_t frameMS) const override;

    public:

        SyncUnicastFPP(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncUnicastFPP(SyncUnicastFPP&& from) noexcept;
        virtual ~SyncUnicastFPP();
        virtual std::string GetType() const override { return "FPPUNICAST"; }
};

class SyncUnicastCSVFPP : public SyncFPP
{
    wxDatagramSocket* _fppUnicastSocket = nullptr;
    std::list<std::string> _remotes;

    void SendUnicastSync(const std::string& ip, const std::string& item, size_t msec, size_t frameMS, int action) const;
    virtual void SendFPPSync(const std::string& item, uint32_t stepMS, uint32_t frameMS) const override;

public:

    SyncUnicastCSVFPP(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
    SyncUnicastCSVFPP(SyncUnicastCSVFPP&& from) noexcept;
    virtual ~SyncUnicastCSVFPP();
    virtual std::string GetType() const override { return "FPPCSV"; }
};

class SyncMulticastFPP : public SyncFPP
{
    wxDatagramSocket* _fppMulticastSocket = nullptr;
    wxIPV4address _remoteAddr;

    virtual void SendFPPSync(const std::string& item, uint32_t stepMS, uint32_t frameMS) const override;

public:

    SyncMulticastFPP(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
    SyncMulticastFPP(SyncMulticastFPP&& from) noexcept;
    virtual ~SyncMulticastFPP();
    virtual std::string GetType() const override { return "FPPMULTICAST"; }
};
#endif