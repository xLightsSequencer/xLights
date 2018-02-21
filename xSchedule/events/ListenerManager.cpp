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
#include "ListenerFPPUnicast.h"
#include "ListenerSerial.h"
#include "ListenerLor.h"
#include "ListenerARTNet.h"
#include "ListenerOSC.h"

ListenerManager::ListenerManager(ScheduleManager* scheduleManager) :
    _scheduleManager(scheduleManager),
    _pause(false),
    _stop(false),
    _sync(0)
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
            for (auto it2 = _scheduleManager->GetOptions()->GetEvents()->begin(); it2 != _scheduleManager->GetOptions()->GetEvents()->end(); ++it2)
            {
                if ((*it2)->GetType() == "E131")
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
            for (auto it2 = _scheduleManager->GetOptions()->GetEvents()->begin(); it2 != _scheduleManager->GetOptions()->GetEvents()->end(); ++it2)
            {
                if ((*it2)->GetType() == "Serial" && l->GetCommPort() == ((EventSerial*)(*it2))->GetCommPort())
                {
                    if (l->GetSpeed() == ((EventSerial*)(*it2))->GetSpeed() &&
                        l->GetProtocol() == ((EventSerial*)(*it2))->GetProtocol())
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
            for (auto it2 = _scheduleManager->GetOptions()->GetEvents()->begin(); it2 != _scheduleManager->GetOptions()->GetEvents()->end(); ++it2)
            {
                if ((*it2)->GetType() == "LOR" && l->GetCommPort() == ((EventLor*)(*it2))->GetCommPort())
                {
                    if (l->GetSpeed() == ((EventLor*)(*it2))->GetSpeed() &&
                        l->GetProtocol() == ((EventLor*)(*it2))->GetProtocol())
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
        else if ((*it)->GetType() == "ARTNet")
        {
            if (_sync == 3)
            {
                ++it;
            }
            else
            {
                ListenerARTNet* l = (ListenerARTNet*)(*it);
                bool found = false;
                for (auto it2 = _scheduleManager->GetOptions()->GetEvents()->begin(); it2 != _scheduleManager->GetOptions()->GetEvents()->end(); ++it2)
                {
                    if ((*it2)->GetType() == "ARTNet")
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
                for (auto it2 = _scheduleManager->GetOptions()->GetEvents()->begin(); it2 != _scheduleManager->GetOptions()->GetEvents()->end(); ++it2)
                {
                    if ((*it2)->GetType() == "FPP")
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
                for (auto it2 = _scheduleManager->GetOptions()->GetEvents()->begin(); it2 != _scheduleManager->GetOptions()->GetEvents()->end(); ++it2)
                {
                    if ((*it2)->GetType() == "OSC")
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
        else if ((*it)->GetType() == "FPP Unicast")
        {
            if (_sync != 4)
            {
                ListenerFPPUnicast* l = (ListenerFPPUnicast*)(*it);
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

    for (auto it3 = _scheduleManager->GetOptions()->GetEvents()->begin(); it3 != _scheduleManager->GetOptions()->GetEvents()->end(); ++it3)
    {
        if ((*it3)->GetType() == "E131")
        {
            bool e131Exists = false;
            for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
            {
                if ((*it2)->GetType() == "E131")
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
        else if ((*it3)->GetType() == "ARTNet")
        {
            bool artnetExists = false;
            for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
            {
                if ((*it2)->GetType() == "ARTNet")
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
        else if ((*it3)->GetType() == "FPP")
        {
            bool fppExists = false;
            for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
            {
                if ((*it2)->GetType() == "FPP")
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
        else if ((*it3)->GetType() == "Serial")
        {
            EventSerial* e = (EventSerial*)(*it3);
            bool portExists = false;
            for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
            {
                if ((*it2)->GetType() == "Serial" && ((ListenerSerial*)(*it2))->GetCommPort() == e->GetCommPort())
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
        else if ((*it3)->GetType() == "LOR")
        {
            EventLor* e = (EventLor*)(*it3);
            bool portExists = false;
            for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
            {
                if ((*it2)->GetType() == "LOR" && ((ListenerLor*)(*it2))->GetCommPort() == e->GetCommPort())
                {
                    portExists = true;
                    break;
                }
            }

            if (!portExists)
            {
                _listeners.push_back(new ListenerLor(this, e->GetCommPort(), e->GetSerialConfig(), e->GetSpeed(), e->GetProtocol(), e->GetUnitId()));
                _listeners.back()->Start();
            }
        }
        else if ((*it3)->GetType() == "OSC")
        {
            bool oscExists = false;
            for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
            {
                if ((*it2)->GetType() == "OSC")
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
        else if ((*it3)->GetType() == "Data")
        {
            // No listener required
        }
    }

    if (_sync == 1)
    {
        bool fppExists = false;
        for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
        {
            if ((*it2)->GetType() == "FPP")
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
        for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
        {
            if ((*it2)->GetType() == "OSC")
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
        for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
        {
            if ((*it2)->GetType() == "ARTNet")
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
    else if (_sync == 4)
    {
        bool fppUnicastExists = false;
        for (auto it2 = _listeners.begin(); it2 != _listeners.end(); ++it2)
        {
            if ((*it2)->GetType() == "FPP Unicast")
            {
                fppUnicastExists = true;
                break;
            }
        }

        if (!fppUnicastExists)
        {
            _listeners.push_back(new ListenerFPPUnicast(this));
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

void ListenerManager::SetRemoteFPPUnicast()
{
    _sync = 4;
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

int ListenerManager::Sync(const std::string filename, long ms, const std::string& type)
{
    if ((_sync == 3 && type == "ARTNet") ||
        (_sync == 4 && type == "FPP Unicast") ||
        (_sync == 2 && type == "OSC") ||
        (_sync == 1 && type == "FPP"))
    {
        return _scheduleManager->Sync(filename, ms);
    }
    else
    {
        return 50;
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

void ListenerManager::ProcessFrame(wxByte* buffer, long buffsize)
{
    if (_pause || _stop) return;

    // handle any data events
    for (auto it = _scheduleManager->GetOptions()->GetEvents()->begin(); it != _scheduleManager->GetOptions()->GetEvents()->end(); ++it)
    {
        if ((*it)->IsFrameProcess())
        {
            (*it)->Process(buffer, buffsize, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, int universe, wxByte* buffer, long buffsize)
{
    if (_pause || _stop) return;

    for (auto it = _scheduleManager->GetOptions()->GetEvents()->begin(); it != _scheduleManager->GetOptions()->GetEvents()->end(); ++it)
    {
        if ((*it)->GetType() == source)
        {
            (*it)->Process(universe, buffer, buffsize, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& commPort, wxByte* buffer, long buffsize)
{
    if (_pause || _stop) return;

    for (auto it = _scheduleManager->GetOptions()->GetEvents()->begin(); it != _scheduleManager->GetOptions()->GetEvents()->end(); ++it)
    {
        if ((*it)->GetType() == source)
        {
            (*it)->Process(commPort, buffer, buffsize, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& id)
{
    if (_pause || _stop) return;

    for (auto it = _scheduleManager->GetOptions()->GetEvents()->begin(); it != _scheduleManager->GetOptions()->GetEvents()->end(); ++it)
    {
        if ((*it)->GetType() == source)
        {
            (*it)->Process(id, _scheduleManager);
        }
    }
}

void ListenerManager::ProcessPacket(const std::string& source, const std::string& path, const std::string& p1, const std::string& p2, const std::string& p3)
{
    if (_pause || _stop) return;

    for (auto it = _scheduleManager->GetOptions()->GetEvents()->begin(); it != _scheduleManager->GetOptions()->GetEvents()->end(); ++it)
    {
        if ((*it)->GetType() == source)
        {
            (*it)->Process(path, p1, p2, p3, _scheduleManager);
        }
    }
}

void ListenerManager::Stop()
{
    _stop = true;
    for (auto it = _listeners.begin(); it != _listeners.end(); ++it)
    {
        (*it)->Stop();
    }
}
