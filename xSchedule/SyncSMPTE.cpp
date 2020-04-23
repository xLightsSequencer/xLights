/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SyncSMPTE.h"
#include "ScheduleOptions.h"
#include "events/ListenerManager.h"

#include <log4cpp/Category.hh>
#include "../xLights/UtilFunctions.h"

SyncSMPTE::SyncSMPTE(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager) : SyncBase(sm, rm)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _frameRate = options.GetSMPTEMode();

    if (sm == SYNCMODE::SMPTEMASTER)
    {
        // not implemented
    }

    if (rm == REMOTEMODE::SMPTESLAVE)
    {
        listenerManager->SetRemoteSMPTE();
    }
}

SyncSMPTE::SyncSMPTE(SyncSMPTE&& from) : SyncBase(from)
{
    _frameRate = from._frameRate;
}

SyncSMPTE::~SyncSMPTE()
{
}

void SyncSMPTE::SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const
{
    // Not implemented
}

void SyncSMPTE::SendStop() const
{
    // Not implemented
}
