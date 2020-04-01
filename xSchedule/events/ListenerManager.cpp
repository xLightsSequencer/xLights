#include "ListenerManager.h"
#include <wx/wx.h>
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include <log4cpp/Category.hh>
#include "EventBase.h"
#include "EventSerial.h"
#include "EventLor.h"
#include "ListenerE131.h"
#include "ListenerFPP.h"
#include "ListenerCSVFPP.h"
#include "ListenerMIDI.h"
#include "ListenerMQTT.h"
#include "ListenerSerial.h"
#include "ListenerSMPTE.h"
#include "ListenerLor.h"
#include "ListenerARTNet.h"
#include "ListenerOSC.h"
#include "EventMIDI.h"
#include "EventMQTT.h"

wxDEFINE_EVENT(EVT_MIDI, wxCommandEvent);

ListenerManager::ListenerManager(ScheduleManager* scheduleManager) :
    _sync(0),
    _stop(false),
    _pause(false),
    _scheduleManager(scheduleManager),
    _notifyScan(nullptr)
{
    StartListeners();
}

void ListenerManager::StartListeners()
{
    auto it = _listeners.begin();
    while (it != _listeners.end())
    {
        if ((*it)->GetType() == "E131")
        {
            ListenerE131* l = (ListenerE131*)(*it);
            bool found = false;
            for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
            {
                if (it2->GetType() == "E131")
                {
                    found = true;
                }
            }
            if (!found)
            {
                l->Stop();
                _listeners.erase(it++);
                delete l;
            }
            else
            {
                ++it;
            }
        }
        else if ((*it)->GetType() == "Serial")
        {
            ListenerSerial* l = (ListenerSerial*)(*it);
            bool found = false;
            for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
            {
                if (it2->GetType() == "Serial" && l->GetCommPort() == ((EventSerial*)it2)->GetCommPort())
                {
                    if (l->GetSpeed() == ((EventSerial*)it2)->GetSpeed() &&
                        l->GetProtocol() == ((EventSerial*)it2)->GetProtocol())
                    {
                        found = true;
                    }
                }
            }
            if (!found)
            {
                l->Stop();
                _listeners.erase(it++);
                delete l;
            }
            else
            {
                ++it;
            }
        }
        else if ((*it)->GetType() == "LOR")
        {
            ListenerLor* l = (ListenerLor*)(*it);
            bool found = false;
            for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
            {
                if (it2->GetType() == "LOR" && l->GetCommPort() == ((EventLor*)it2)->GetCommPort())
                {
                    if (l->GetSpeed() == ((EventLor*)it2)->GetSpeed() &&
                        l->GetProtocol() == ((EventLor*)it2)->GetProtocol())
                    {
                        found = true;
                    }
                }
            }
            if (!found)
            {
                l->Stop();
                _listeners.erase(it++);
                delete l;
            }
            else
            {
                ++it;
            }
        }
        else if ((*it)->GetType() == "MIDI")
        {
            if (_sync == 5)
            {
                ++it;
            }
            else
            {
                ListenerMIDI* l = (ListenerMIDI*)(*it);
                bool found = false;
                for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
                {
                    if (it2->GetType() == "MIDI" && ((EventMIDI*)it2)->GetDeviceId() == l->GetDeviceId())
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    l->Stop();
                    _listeners.erase(it++);
                    delete l;
                }
                else
                {
                    ++it;
                }
            }
        }
        else if ((*it)->GetType() == "SMPTE")
        {
            if (_sync == 7)
            {
                ++it;
            }
            else
            {
                ListenerSMPTE* l = (ListenerSMPTE*)(*it);
                bool found = false;
                for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
                {
                    if (it2->GetType() == "SMPTE")
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    l->Stop();
                    _listeners.erase(it++);
                    delete l;
                }
                else
                {
                    ++it;
                }
            }
        }
        else if ((*it)->GetType() == "MQTT")
        {
            ListenerMQTT* l = (ListenerMQTT*)(*it);
            bool found = false;
            for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
            {
                if (it2->GetType() == "MQTT" && l->GetBrokerIP() == ((EventMQTT*)it2)->GetBrokerIP() && l->GetBrokerPort() == ((EventMQTT*)it2)->GetBrokerPort())
                {
                    found = true;
                }
            }
            if (!found)
            {
                l->Stop();
                _listeners.erase(it++);
                delete l;
            }
            else
            {
                ++it;
            }
        }
        else if ((*it)->GetType() == "ARTNet" || (*it)->GetType() == "ARTNetTrigger")
        {
            if (_sync == 3)
            {
                ++it;
            }
            else
            {
                ListenerARTNet* l = (ListenerARTNet*)(*it);
                bool found = false;
                for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
                {
                    if (it2->GetType() == "ARTNet" || it2->GetType() == "ARTNetTrigger")
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    l->Stop();
                    _listeners.erase(it++);
                    delete l;
                }
                else
                {
                    ++it;
                }
            }
        }
        else if ((*it)->GetType() == "FPP")
        {
            if (_sync == 1)
            {
                ++it;
            }
            else
            {
                ListenerFPP* l = (ListenerFPP*)(*it);
                bool found = false;
                for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
                {
                    if (it2->GetType() == "FPP")
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    l->Stop();
                    _listeners.erase(it++);
                    delete l;
                }
                else
                {
                    ++it;
                }
            }
        }
        else if ((*it)->GetType() == "FPP CSV")
        {
            if (_sync == 6)
            {
                ++it;
            }
            else
            {
                ListenerCSVFPP* l = (ListenerCSVFPP*)(*it);
                bool found = false;
                for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
                {
                    if (it2->GetType() == "FPP CSV")
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    l->Stop();
                    _listeners.erase(it++);
                    delete l;
                }
                else
                {
                    ++it;
                }
            }
        }
        else if ((*it)->GetType() == "OSC")
        {
            if (_sync == 2)
            {
                ++it;
            }
            else
            {
                ListenerOSC* l = (ListenerOSC*)(*it);
                bool found = false;
                for (auto it2 : *_scheduleManager->GetOptions()->GetEvents())
                {
                    if (it2->GetType() == "OSC")
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    l->Stop();
                    _listeners.erase(it++);
                    delete l;
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    bool update_lor_unit_ids = false;

    for (auto it3 : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it3->GetType() == "E131")
        {
            bool e131Exists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "E131")
                {
                    e131Exists = true;
                    break;
                }
            }

            if (!e131Exists)
            {
                _listeners.push_back(new ListenerE131(this));
                _listeners.back()->Start();
            }
        }
        else if (it3->GetType() == "ARTNet" || it3->GetType() == "ARTNetTrigger")
        {
            bool artnetExists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "ARTNet")
                {
                    artnetExists = true;
                    break;
                }
            }

            if (!artnetExists)
            {
                _listeners.push_back(new ListenerARTNet(this));
                _listeners.back()->Start();
            }
        }
        else if (it3->GetType() == "FPP")
        {
            ListenerBase* current = nullptr;
            bool fppExists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "FPP")
                {
                    current = it2;
                    fppExists = true;
                    break;
                }
            }

            if (!fppExists)
            {
                _listeners.push_back(new ListenerFPP(this));
                _listeners.back()->Start();
            }
            else
            {
                // because FPP binds to the type of sync packet it sees (broadcast/multicast/unicast) we need to delete and recreate to ensure it can pick up anything new
                current->Stop();
                _listeners.erase(std::find(_listeners.begin(), _listeners.end(), current));
                _listeners.push_back(new ListenerFPP(this));
                _listeners.back()->Start();
            }
        }
        else if (it3->GetType() == "FPP CSV")
        {
            ListenerBase* current = nullptr;
            bool fppExists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "FPP CSV")
                {
                    current = it2;
                    fppExists = true;
                    break;
                }
            }

            if (!fppExists)
            {
                _listeners.push_back(new ListenerCSVFPP(this));
                _listeners.back()->Start();
            }
            else
            {
                // because FPP binds to the type of sync packet it sees (broadcast/multicast/unicast) we need to delete and recreate to ensure it can pick up anything new
                current->Stop();
                _listeners.erase(std::find(_listeners.begin(), _listeners.end(), current));
                _listeners.push_back(new ListenerCSVFPP(this));
                _listeners.back()->Start();
            }
        }
        else if (it3->GetType() == "Serial")
        {
            EventSerial* e = (EventSerial*)it3;
            bool portExists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "Serial" && ((ListenerSerial*)it2)->GetCommPort() == e->GetCommPort())
                {
                    portExists = true;
                    break;
                }
            }

            if (!portExists)
            {
                _listeners.push_back(new ListenerSerial(this, e->GetCommPort(), e->GetSerialConfig(), e->GetSpeed(), e->GetProtocol()));
                _listeners.back()->Start();
            }
        }
        else if (it3->GetType() == "LOR")
        {
            EventLor* e = (EventLor*)it3;
            bool portExists = false;
            for (auto& it2 : _listeners)
            {
                if (it2->GetType() == "LOR" && ((ListenerLor*)it2)->GetCommPort() == e->GetCommPort())
                {
                    portExists = true;
                    ((ListenerLor*)it2)->AddNewUnitId(e->GetUnitId());
                    update_lor_unit_ids = true;
                    break;
                }
            }

            if (!portExists)
            {
                _listeners.push_back(new ListenerLor(this, e->GetCommPort(), e->GetSerialConfig(), e->GetSpeed(), e->GetProtocol(), e->GetUnitIdString()));
                _listeners.back()->Start();
                ((ListenerLor*)(_listeners.back()))->AddNewUnitId(e->GetUnitId());
                update_lor_unit_ids = true;
            }
        }
        else if (it3->GetType() == "MIDI")
        {
            EventMIDI* e = (EventMIDI*)it3;
            bool midiExists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "MIDI" && e->GetDeviceId() == ((ListenerMIDI*)it2)->GetDeviceId())
                {
                    midiExists = true;
                    break;
                }
            }

            if (!midiExists && e->GetDeviceId() >= 0)
            {
                _listeners.push_back(new ListenerMIDI(e->GetDeviceId(), this));
                _listeners.back()->Start();
            }
        }
        // No SMPTPE as there are no events
        else if (it3->GetType() == "MQTT")
        {
            EventMQTT* e = (EventMQTT*)it3;
            bool portExists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "MQTT" && ((ListenerMQTT*)it2)->GetBrokerIP() == e->GetBrokerIP() && ((ListenerMQTT*)it2)->GetBrokerPort() == e->GetBrokerPort())
                {
                    ((ListenerMQTT*)it2)->Subscribe(e->GetTopic());
                    portExists = true;
                    break;
                }
            }

            if (!portExists)
            {
                _listeners.push_back(new ListenerMQTT(this, e->GetBrokerIP(), e->GetBrokerPort(), e->GetUsername(), e->GetPassword(), e->GetClientId()));
                _listeners.back()->Start();
                ((ListenerMQTT*)_listeners.back())->Subscribe(e->GetTopic());
            }
        }
        else if (it3->GetType() == "OSC")
        {
            bool oscExists = false;
            for (auto it2 : _listeners)
            {
                if (it2->GetType() == "OSC")
                {
                    oscExists = true;
                    break;
                }
            }

            if (!oscExists)
            {
                _listeners.push_back(new ListenerOSC(this));
                _listeners.back()->Start();
            }
        }
        else if (it3->GetType() == "Data")
        {
            // No listener required
        }
        else
        {
            wxASSERT(false);
        }
    }

    // need to tell LOR listeners to update Unit Ids they need to poll
    if (update_lor_unit_ids)
    {
        for (auto& it2 : _listeners)
        {
            if (it2->GetType() == "LOR")
            {
                ((ListenerLor*)it2)->EndUnitIdList();
            }
        }
    }

    if (_sync == 1)
    {
        bool fppExists = false;
        for (auto it2 : _listeners)
        {
            if (it2->GetType() == "FPP")
            {
                fppExists = true;
                break;
            }
        }

        if (!fppExists)
        {
            _listeners.push_back(new ListenerFPP(this));
            _listeners.back()->Start();
        }
    }
    else if (_sync == 2)
    {
        bool oscExists = false;
        for (auto it2 : _listeners)
        {
            if (it2->GetType() == "OSC")
            {
                oscExists = true;
                break;
            }
        }

        if (!oscExists)
        {
            _listeners.push_back(new ListenerOSC(this));
            _listeners.back()->Start();
        }
    }
    else if (_sync == 3)
    {
        bool artnetExists = false;
        for (auto it2 : _listeners)
        {
            if (it2->GetType() == "ARTNet")
            {
                artnetExists = true;
                break;
            }
        }

        if (!artnetExists)
        {
            _listeners.push_back(new ListenerARTNet(this));
            _listeners.back()->Start();
        }
    }
    else if (_sync == 5)
    {
        int devid = wxAtoi(wxString(_scheduleManager->GetOptions()->GetMIDITimecodeDevice()).AfterLast(' '));
        bool midiExists = false;
        for (auto it2 : _listeners)
        {
            if (it2->GetType() == "MIDI" && ((ListenerMIDI*)it2)->GetDeviceId() == devid)
            {
                midiExists = true;
                break;
            }
        }

        if (!midiExists)
        {
            if (_scheduleManager->GetOptions()->GetMIDITimecodeDevice() != "")
            {
                _listeners.push_back(new ListenerMIDI(devid, this));
                _listeners.back()->Start();
            }
        }
    }
    else if (_sync == 6)
    {
        bool fppExists = false;
        for (auto it2 : _listeners)
        {
            if (it2->GetType() == "FPP CSV")
            {
                fppExists = true;
                break;
            }
        }

        if (!fppExists)
        {
            _listeners.push_back(new ListenerCSVFPP(this));
            _listeners.back()->Start();
        }
    }
    else if (_sync == 7)
    {
        int mode = _scheduleManager->GetOptions()->GetSMPTEMode();
        bool smpteExists = false;
        for (auto it2 : _listeners)
        {
            if (it2->GetType() == "SMPTE")
            {
                smpteExists = true;
                break;
            }
        }

        if (!smpteExists)
        {
            _listeners.push_back(new ListenerSMPTE(mode, this));
            _listeners.back()->Start();
        }
    }
}

void ListenerManager::SetRemoteOSC()
{
    _sync = 2;
    StartListeners();
}

void ListenerManager::SetRemoteFPP()
{
    _sync = 1;
    StartListeners();
}

void ListenerManager::SetRemoteCSVFPP()
{
    _sync = 6;
    StartListeners();
}

void ListenerManager::SetRemoteMIDI()
{
    _sync = 5;
    StartListeners();
}

void ListenerManager::SetRemoteSMPTE()
{
    _sync = 7;
    StartListeners();
}

void ListenerManager::SetRemoteNone()
{
    _sync = 0;
    StartListeners();
}

void ListenerManager::SetRemoteArtNet()
{
    _sync = 3;
    StartListeners();
}

void ListenerManager::MidiRedirect(wxWindow* notify, int deviceId)
{
    if (notify == nullptr && _notifyScan == nullptr)
    {
        // nothing to do
    }
    else if (notify == nullptr && _notifyScan != nullptr)
    {
        _notifyScan = nullptr;
        // this will stop the MIDI if it isnt meant to be running
        StartListeners();
    }
    else if (_notifyScan == nullptr && notify != nullptr)
    {
        _notifyScan = notify;
        bool found = false;
        for (auto it : _listeners)
        {
            if (it->GetType() == "MIDI" && ((ListenerMIDI*)it)->GetDeviceId() == deviceId)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            ListenerMIDI* lm = new ListenerMIDI(deviceId, this);
            _listeners.push_back(lm);
            lm->Start();
        }
    }
    else
    {
        wxASSERT(false);
    }
}

#define MIN_SYNC_INTERVAL_MS 500

int ListenerManager::Sync(const std::string filename, long ms, const std::string& type)
{
    if ((_sync == 1 && type == "FPP") || (_sync == 6 && type == "FPP CSV"))
    {
        _lastFrameMS = _scheduleManager->Sync(_scheduleManager->FindStepForFSEQ(filename), ms);
    }
    else if ((_sync == 3 && type == "ARTNet") ||
        (_sync == 2 && type == "OSC") ||
        (_sync == 5 && type == "MIDI") ||
        (_sync == 7 && type == "SMPTE"))
    {
		if (_lastSyncMS < 0 || _lastSyncMS >= ms || ms > _lastSyncMS + MIN_SYNC_INTERVAL_MS)
		{
			_lastFrameMS = _scheduleManager->Sync(filename, ms);
		}
		_lastSyncMS = ms;
    }
    else
    {
        _lastFrameMS = 50;
    }
	return _lastFrameMS;
}

void ListenerManager::SetFrameMS(int frameMS)
{
    for (auto it: _listeners)
    {
        it->SetFrameMS(frameMS);
    }
}

ListenerManager::~ListenerManager()
{
    Stop();
    while (_listeners.size() > 0)
    {
        delete _listeners.back();
        _listeners.pop_back();
    }
}

void ListenerManager::Pause(bool pause)
{
    _pause = pause;
}

void ListenerManager::ProcessFrame(uint8_t* buffer, long buffsize)
{
    if (_pause || _stop) return;

    // handle any data events
    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->IsFrameProcess())
        {
            it->Process(buffer, buffsize, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, int universe, uint8_t* buffer, long buffsize)
{
    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->GetType() == source)
        {
            it->Process(universe, buffer, buffsize, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& state, long buffsize)
{
    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->GetType() == source)
        {
            it->Process(state, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, int deviceId, uint8_t status, uint8_t channel, uint8_t data1, uint8_t data2)
{
    if (_notifyScan != nullptr && source == "MIDI")
    {
        wxCommandEvent event(EVT_MIDI);
        int value = (((int)status & 0xF0) << 24) +
            (((int)channel & 0x0F) << 16) +
            ((int)data1 << 8) +
            (int)data2;
        event.SetInt(value);
        wxPostEvent(_notifyScan, event);
        return;
    }

    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->GetType() == source)
        {
            if (source == "MIDI")
            {
                if (((EventMIDI*)it)->GetDeviceId() == deviceId)
                {
                    it->Process(status, channel, data1, data2, _scheduleManager);
                }
            }
            else
            {
                it->Process(status, channel, data1, data2, _scheduleManager);
            }
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& commPort, uint8_t* buffer, long buffsize, int subtype)
{
    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if ((it->GetType() == source) && (subtype == it->GetSubType()))
        {
            it->Process(commPort, buffer, buffsize, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& id)
{
    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->GetType() == source)
        {
            it->Process(id, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& path, const std::string& p1, const std::string& p2, const std::string& p3)
{
    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->GetType() == source)
        {
            it->Process(path, p1, p2, p3, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, bool result, const std::string& ip)
{
    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->GetType() == source)
        {
            it->Process(result, ip, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& topic, const std::string& data)
{
    if (_pause || _stop) return;

    for (auto& it : *_scheduleManager->GetOptions()->GetEvents())
    {
        if (it->GetType() == source)
        {
            it->Process(topic, data, _scheduleManager);
        }
    }
}

void ListenerManager::Stop()
{
    _stop = true;
    for (auto& it : _listeners)
    {
        it->Stop();
    }
}
