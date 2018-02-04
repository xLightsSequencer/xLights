#ifndef LISTENERMANAGER_H
#define LISTENERMANAGER_H

#include <string>
#include <wx/wx.h>
#include "ListenerBase.h"
#include <list>

class ScheduleManager;

class ListenerManager
{
    protected:
		std::list<ListenerBase*> _listeners;
	    bool _stop;
		bool _pause;
        ScheduleManager* _scheduleManager;
        int _sync;
	
	public:
        ListenerManager(ScheduleManager* scheduleManager);
		virtual ~ListenerManager();
		void Pause(bool pause = true);
		void ProcessFrame(wxByte* buffer, long buffsize);
		void ProcessPacket(const std::string& source, int universe, wxByte* buffer, long buffsize);
		void ProcessPacket(const std::string& source, wxByte status, wxByte channel, wxByte data1, wxByte data2);
		void ProcessPacket(const std::string& source, const std::string& commPort, wxByte* buffer, long buffsize);
		void ProcessPacket(const std::string& source, const std::string& id);
        void ProcessPacket(const std::string& source, const std::string& path, const std::string& p1, const std::string& p2, const std::string& p3);
        void Stop();
        void StartListeners();
        void SetRemoteOSC();
        void SetRemoteFPP();
        void SetRemoteFPPUnicast();
        void SetRemoteNone();
        void SetRemoteArtNet();
        int Sync(const std::string filename, long ms, const std::string& type);
        ScheduleManager* GetScheduleManager() const { return _scheduleManager; }
};
#endif
