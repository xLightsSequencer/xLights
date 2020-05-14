/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SyncManager.h"
#include "ScheduleOptions.h"
#include "ScheduleManager.h"
#include "events/ListenerManager.h"
#include "../xLights/UtilFunctions.h"

#include "SyncOSC.h"
#include "SyncFPP.h"
#include "SyncMIDI.h"
#include "SyncArtNet.h"
#include "SyncSMPTE.h"

#include <log4cpp/Category.hh>

std::unique_ptr<SyncBase> SyncManager::CreateSync(SYNCMODE sm, REMOTEMODE rm) const
{
    wxASSERT(_scheduleManager != nullptr);
    if (sm == SYNCMODE::OSCMASTER || rm == REMOTEMODE::OSCSLAVE)
    {
        return std::make_unique<SyncOSC>(SyncOSC(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::ARTNETMASTER || rm == REMOTEMODE::ARTNETSLAVE)
    {
        return std::make_unique<SyncArtNet>(SyncArtNet(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::FPPBROADCASTMASTER)
    {
        return std::make_unique<SyncBroadcastFPP>(SyncBroadcastFPP(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::FPPUNICASTMASTER)
    {
        return std::make_unique<SyncUnicastFPP>(SyncUnicastFPP(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::FPPUNICASTCSVMASTER || rm == REMOTEMODE::FPPCSVSLAVE)
    {
        return std::make_unique<SyncUnicastCSVFPP>(SyncUnicastCSVFPP(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::FPPMULTICASTMASTER)
    {
        return std::make_unique<SyncMulticastFPP>(SyncMulticastFPP(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (rm == REMOTEMODE::FPPSLAVE || rm == REMOTEMODE::FPPBROADCASTSLAVE || rm == REMOTEMODE::FPPUNICASTSLAVE)
    {
        // really doesnt matter which FPP I create
        return std::make_unique<SyncMulticastFPP>(SyncMulticastFPP(sm, REMOTEMODE::FPPSLAVE, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
    else if (sm == SYNCMODE::MIDIMASTER || rm == REMOTEMODE::MIDISLAVE)
    {
        return std::make_unique<SyncMIDI>(SyncMIDI(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
#ifndef __WXOSX__
    else if (rm == REMOTEMODE::SMPTESLAVE)
    {
        return std::make_unique<SyncSMPTE>(SyncSMPTE(sm, rm, *_scheduleManager->GetOptions(), _scheduleManager->GetListenerManager()));
    }
#endif
    else if (rm == REMOTEMODE::DISABLED){
        return nullptr;
    }
    else
    {
        wxASSERT(false);
    }

    return nullptr;
}

SyncManager::SyncManager(ScheduleManager* scheduleManager) : _scheduleManager(scheduleManager) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (scheduleManager == nullptr) {
        logger_base.crit("SyncManager::SyncManager created with nullptr to scheduleManager ... this will not end well.");
    }
    else {
        logger_base.debug("SyncManager::SyncManager created ok.");
    }
}

void SyncManager::AddMaster(SYNCMODE sm)
{
    RemoveMaster(sm);
    _masters.emplace_back(std::move(CreateSync(sm, REMOTEMODE::DISABLED)));
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
        if (it->IsReactive())
        {
            it->SendSync(frameMS, stepLengthMS, stepMS, playlistMS, fseq, media, step, timeItem);
        }
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

    if (mode & static_cast<int>(SYNCMODE::FPPUNICASTCSVMASTER)) {
        AddMaster(SYNCMODE::FPPUNICASTCSVMASTER);
    }
    else {
        RemoveMaster(SYNCMODE::FPPUNICASTCSVMASTER);
    }

    if (mode & static_cast<int>(SYNCMODE::FPPMULTICASTMASTER)) {
        AddMaster(SYNCMODE::FPPMULTICASTMASTER);
    }
    else {
        RemoveMaster(SYNCMODE::FPPMULTICASTMASTER);
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
        SetRemote(REMOTEMODE::FPPSLAVE);
    }
    else if (remoteMode == REMOTEMODE::OSCSLAVE) {
        SetRemote(REMOTEMODE::OSCSLAVE);
    }
    else if (remoteMode == REMOTEMODE::FPPUNICASTSLAVE) {
        SetRemote(REMOTEMODE::FPPSLAVE);
    }
    else if (remoteMode == REMOTEMODE::FPPCSVSLAVE) {
        SetRemote(REMOTEMODE::FPPCSVSLAVE);
    }
    else if (remoteMode == REMOTEMODE::FPPSLAVE) {
        SetRemote(REMOTEMODE::FPPSLAVE);
    }
    else if (remoteMode == REMOTEMODE::DISABLED) {
        ClearRemote();
    }
    else if (remoteMode == REMOTEMODE::ARTNETSLAVE) {
        SetRemote(REMOTEMODE::ARTNETSLAVE);
    }
    else if (remoteMode == REMOTEMODE::SMPTESLAVE) {
        SetRemote(REMOTEMODE::SMPTESLAVE);
    }

    _scheduleManager->GetListenerManager()->StartListeners();
}

void SyncManager::Stop()
{
    SetRemote(REMOTEMODE::DISABLED);
    ClearMasters();
    _scheduleManager->GetListenerManager()->StartListeners();
}

bool SyncManager::IsFPPRemoteOrMaster() const
{
    if (_remote != nullptr && Contains(_remote->GetType(), "FPP")) { return true; }
    for (auto& it : _masters)
    {
        if (Contains(it->GetType(), "FPP")) { return true; }
    }
    return false;
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
