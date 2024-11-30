#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerBase.h"
#include <wx/wx.h>
#include <list>
#include <string>

wxDECLARE_EVENT(EVT_MIDI, wxCommandEvent);

class ScheduleManager;

class ListenerManager {
protected:
    std::list<ListenerBase*> _listeners;
    int _sync;
    bool _stop;
    bool _pause;
    ScheduleManager* _scheduleManager;
    wxWindow* _notifyScan;
    long _lastSyncMS = -1;
    int _lastFrameMS = 50;

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
    void StartListeners(const std::string& localIP);
    void SetRemoteOSC();
    void SetRemoteFPP();
    void SetRemoteCSVFPP();
    void SetRemoteMIDI();
    void SetRemoteSMPTE();
    void SetRemoteNone();
    void SetRemoteArtNet();
    void MidiRedirect(wxWindow* notify, int deviceId);
    int Sync(const std::string filename, long ms, const std::string& type);
    ScheduleManager* GetScheduleManager() const {
        return _scheduleManager;
    }
    void SetFrameMS(int frameMS);
    long GetStepMMSSOfset(int& hours, long hradj);
    ListenerBase* GetListener(const std::string listener) const;
};
