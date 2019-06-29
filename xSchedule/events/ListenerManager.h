#ifndef LISTENERMANAGER_H
#define LISTENERMANAGER_H

#include <string>
#include <wx/wx.h>
#include "ListenerBase.h"
#include <list>

wxDECLARE_EVENT(EVT_MIDI, wxCommandEvent);

class ScheduleManager;

class ListenerManager
{
    protected:
		std::list<ListenerBase*> _listeners;
        int _sync;
	    bool _stop;
		bool _pause;
        ScheduleManager* _scheduleManager;
        wxWindow* _notifyScan;

	public:
        ListenerManager(ScheduleManager* scheduleManager);
		virtual ~ListenerManager();
		void Pause(bool pause = true);
		void ProcessFrame(uint8_t* buffer, long buffsize);
        void ProcessPacket(const std::string& source, const std::string& state, long buffsize);
        void ProcessPacket(const std::string& source, int universe, uint8_t* buffer, long buffsize);
		void ProcessPacket(const std::string& source, const std::string& commPort, uint8_t* buffer, long buffsize, int subtype);
		void ProcessPacket(const std::string& source, int deviceId, uint8_t status, uint8_t channel, uint8_t data1, uint8_t data2);
		void ProcessPacket(const std::string& source, const std::string& id);
        void ProcessPacket(const std::string& source, const std::string& path, const std::string& p1, const std::string& p2, const std::string& p3);
        void ProcessPacket(const std::string& source, bool result, const std::string& ip);
        void ProcessPacket(const std::string& source, const std::string& topic, const std::string& data);
        void Stop();
        void StartListeners();
        void SetRemoteOSC();
        void SetRemoteFPP();
        void SetRemoteMIDI();
        void SetRemoteFPPUnicast();
        void SetRemoteNone();
        void SetRemoteArtNet();
        void MidiRedirect(wxWindow* notify, int deviceId);
        int Sync(const std::string filename, long ms, const std::string& type);
        ScheduleManager* GetScheduleManager() const { return _scheduleManager; }
        void SetFrameMS(int frameMS);
};
#endif
