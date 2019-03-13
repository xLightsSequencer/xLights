#include "SyncManager.h"
#include "ScheduleOptions.h"
#include "ScheduleManager.h"
#include "events/ListenerManager.h"

#include "SyncOSC.h"
#include "SyncFPP.h"
#include "SyncMIDI.h"
#include "SyncArtNet.h"

std::unique_ptr<SyncBase> SyncManager::CreateSync(SYNCMODE sm, REMOTEMODE rm) const
{
    if (sm == SYNCMODE::OSCMASTER || rm == REMOTEMODE::OSCSLAVE)
    {
        return std::make_unique<SyncOSC>(SyncOSC(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::ARTNETMASTER || rm == REMOTEMODE::ARTNETSLAVE)
    {
        return std::make_unique<SyncArtNet>(SyncArtNet(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::FPPBROADCASTMASTER || rm == REMOTEMODE::FPPBROADCASTSLAVE)
    {
        return std::make_unique<SyncBroadcastFPP>(SyncBroadcastFPP(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::FPPUNICASTMASTER || rm == REMOTEMODE::FPPUNICASTSLAVE)
    {
        return std::make_unique<SyncUnicastFPP>(SyncUnicastFPP(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::MIDIMASTER || rm == REMOTEMODE::MIDISLAVE)
    {
        return std::make_unique<SyncMIDI>(SyncMIDI(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else
    {
        wxASSERT(false);
    }

    return nullptr;
}

void SyncManager::AddMaster(SYNCMODE sm)
{
    RemoveMaster(sm);
    _masters.emplace_back(CreateSync(sm, REMOTEMODE::DISABLED));
}

void SyncManager::RemoveMaster(SYNCMODE sm)
{
    for (auto it = begin(_masters); it != end(_masters); ++it)
    {
        if ((*it)->IsMode(sm))
        {
            _masters.erase(it);
            return;
        }
    }
}

void SyncManager::SetRemote(REMOTEMODE rm)
{
    _remote = CreateSync(SYNCMODE::STANDALONE, rm);
}

void SyncManager::ClearRemote()
{
    if (_scheduleManager->GetListenerManager() != nullptr)
    {
        _scheduleManager->GetListenerManager()->SetRemoteNone();
    }
    _remote = nullptr;
}

void SyncManager::ClearMasters()
{
    while (_masters.size() > 0)
    {
        _masters.remove(_masters.front());
    }
}

void SyncManager::SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const
{ 
    for (auto& it : _masters)
    {
        it->SendSync(frameMS, stepLengthMS, stepMS, playlistMS, fseq, media, step, timeItem);
    }
}

void SyncManager::Start(int mode, REMOTEMODE remoteMode)
{
    if (mode & static_cast<int>(SYNCMODE::FPPBROADCASTMASTER)) {
        AddMaster(SYNCMODE::FPPBROADCASTMASTER);
    }
    else {
        RemoveMaster(SYNCMODE::FPPBROADCASTMASTER);
    }

    if (mode & static_cast<int>(SYNCMODE::FPPUNICASTMASTER)) {
        AddMaster(SYNCMODE::FPPUNICASTMASTER);
    }
    else {
        RemoveMaster(SYNCMODE::FPPUNICASTMASTER);
    }

    if (mode & static_cast<int>(SYNCMODE::OSCMASTER)) {
        AddMaster(SYNCMODE::OSCMASTER);
    }
    else {
        RemoveMaster(SYNCMODE::OSCMASTER);
    }

    if (mode & static_cast<int>(SYNCMODE::MIDIMASTER)) {
        AddMaster(SYNCMODE::MIDIMASTER);
    }
    else {
        RemoveMaster(SYNCMODE::MIDIMASTER);
    }

    if (mode & static_cast<int>(SYNCMODE::ARTNETMASTER)) {
        AddMaster(SYNCMODE::ARTNETMASTER);
    }
    else {
        RemoveMaster(SYNCMODE::ARTNETMASTER);
    }

    if (remoteMode == REMOTEMODE::MIDISLAVE) {
        SetRemote(REMOTEMODE::MIDISLAVE);
    }
    else if (remoteMode == REMOTEMODE::FPPBROADCASTSLAVE) {
        SetRemote(REMOTEMODE::FPPBROADCASTSLAVE);
    }
    else if (remoteMode == REMOTEMODE::OSCSLAVE) {
        SetRemote(REMOTEMODE::OSCSLAVE);
    }
    else if (remoteMode == REMOTEMODE::FPPUNICASTSLAVE) {
        SetRemote(REMOTEMODE::FPPUNICASTSLAVE);
    }
    else if (remoteMode == REMOTEMODE::DISABLED) {
        ClearRemote();
    }
    else if (remoteMode == REMOTEMODE::ARTNETSLAVE) {
        SetRemote(REMOTEMODE::ARTNETSLAVE);
    }

    _scheduleManager->GetListenerManager()->StartListeners();
}

bool SyncManager::IsMaster(SYNCMODE mode) const
{
    bool res = false;

    for (auto& it: _masters)
    {
        res |= it->IsMode(mode);
    }

    return res;
}

void SyncManager::SendStop() const
{
    for (auto& it : _masters)
    {
        it->SendStop();
    }
}
