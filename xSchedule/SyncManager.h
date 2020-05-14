#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <memory>
#include <string>

class ScheduleOptions;
class ScheduleManager;

enum class TIMECODEFORMAT
{
    F24,
    F25,
    F2997,
    F30
};

// These are or-ed together
enum class SYNCMODE
{
    STANDALONE = 0,
    FPPBROADCASTMASTER = 1,
    FPPUNICASTMASTER = 2,
    ARTNETMASTER = 4,
    OSCMASTER = 8,
    MIDIMASTER = 16,
    FPPMULTICASTMASTER = 32,
    FPPUNICASTCSVMASTER = 64,
    SMPTEMASTER = 128 // not currently useable
};

enum class REMOTEMODE
{
    DISABLED,
    FPPBROADCASTSLAVE,
    FPPUNICASTSLAVE,
    ARTNETSLAVE,
    OSCSLAVE,
    MIDISLAVE,
    FPPSLAVE,
    FPPCSVSLAVE,
    SMPTESLAVE
};

class SyncBase
{
    protected:

        uint32_t _ms = 0;
        std::string _song = "";
        SYNCMODE _mode = SYNCMODE::STANDALONE;
        REMOTEMODE _remoteMode = REMOTEMODE::DISABLED;

    public:

        SyncBase(SYNCMODE mode, REMOTEMODE remoteMode) : _mode(mode), _remoteMode(remoteMode) {}
		virtual ~SyncBase() {}
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const = 0;
        virtual std::string GetType() const = 0;
        virtual void SendStop() const = 0;
        uint32_t GetMS() const { return _ms; }
        std::string GetSong() const { return _song; }
        bool IsMode(SYNCMODE mode) const { return _mode == mode; }
        bool IsRemoteMode(REMOTEMODE mode) const { return _remoteMode == mode; }
        virtual bool IsReactive() const { return true; }
};

class SyncManager
{
	std::list<std::unique_ptr<SyncBase>> _masters;
	std::unique_ptr<SyncBase> _remote = nullptr;
	ScheduleManager* _scheduleManager = nullptr;

    std::unique_ptr<SyncBase> CreateSync(SYNCMODE sm, REMOTEMODE rm) const;

	public:

        SyncManager(ScheduleManager* scheduleManager);
        virtual ~SyncManager() {
            _remote = nullptr;
            ClearMasters(); 
        }

		void AddMaster(SYNCMODE sm);
        void RemoveMaster(SYNCMODE sm);
		void SetRemote(REMOTEMODE rm);
        void ClearRemote();
        void ClearMasters();
        void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const; // send out to all masters
        void Start(int mode, REMOTEMODE remoteMode);
        void Stop();
        bool IsSlave() const { return _remote != nullptr; }
        bool IsFPPRemoteOrMaster() const;
        bool IsMaster(SYNCMODE mode) const;
        void SendStop() const;
};
