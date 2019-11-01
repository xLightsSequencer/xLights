#include "E131Receiver.h"

#include <wx/wx.h>
#include <wx/thread.h>
#include <wx/socket.h>

#include "xFadeMain.h"
#include "Settings.h"
#include "PacketData.h"
#include "UniverseData.h"
#include "../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>

#ifndef __WXMSW__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

class E131ReceiverThread : public wxThread
{
    E131Receiver* _receiver = nullptr;
    std::string _localIP; // local ip we are listening on
    volatile bool _stop;
    std::list<int> _universes;

public:

    E131ReceiverThread(E131Receiver* receiver, const std::string& localIP, const std::list<int> universes) : wxThread(wxTHREAD_JOINABLE)
    {
        _receiver = receiver;
        _stop = false;
        _localIP = localIP;
        _universes = universes;
    }

    virtual ~E131ReceiverThread()
    {
        //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        if (!_stop)
        {
            _stop = true;
        }
    }

    void Stop()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Asking E131 Receiver thread to stop");
        _stop = true;
    }
    
    virtual void* Entry() override
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("E131 Receiver thread started");

        wxIPV4address addr;
        if (_localIP == "")
        {
            addr.AnyAddress();
        }
        else
        {
            addr.Hostname(_localIP.c_str());
        }
        addr.Service(E131PORT);
        //create and bind to the address above
        wxDatagramSocket* e131SocketReceive = new wxDatagramSocket(addr);

        if (e131SocketReceive == nullptr)
        {
            logger_base.error("Problem listening for e131. E131 Receiver thread exiting.");
            return nullptr;
        }
        else if (!e131SocketReceive->IsOk())
        {
            logger_base.error("Problem listening for e131. E131 Receiver thread exiting.");
            delete e131SocketReceive;
            e131SocketReceive = nullptr;
            return nullptr;
        }
        else if (e131SocketReceive->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Problem listening for e131 => %d : %s, from %s.", e131SocketReceive->LastError(), (const char*)DecodeIPError(e131SocketReceive->LastError()).c_str(), (const char*)addr.IPAddress().c_str());
            delete e131SocketReceive;
            e131SocketReceive = nullptr;
            return nullptr;
        }

        logger_base.debug("E131 listening on %s", (const char*)addr.IPAddress().c_str());

        for (const auto& it : _universes)
        {
            struct ip_mreq mreq;
            wxString ip = wxString::Format("239.255.%d.%d", it >> 8, it & 0xFF);
            logger_base.debug("E131 registering for multicast on %s.", (const char*)ip.c_str());
            mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
            mreq.imr_interface.s_addr = inet_addr(_localIP.c_str()); // this will only listen on the default interface
            if (!e131SocketReceive->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)& mreq, sizeof(mreq)))
            {
                logger_base.warn("    Error opening E131 multicast listener %s.", (const char*)ip.c_str());
            }
            else
            {
                logger_base.debug("    E131 multicast listener %s registered.", (const char*)ip.c_str());
            }
        }

        e131SocketReceive->Notify(false);
        e131SocketReceive->SetTimeout(1);

        uint8_t buffer[E131_PACKET_HEADERLEN + 512];

        while (!_stop)
        {
            memset(buffer, 0x00, sizeof(buffer));

            e131SocketReceive->Read(buffer, sizeof(buffer));
            if (!_stop)
            {
                //logger_base.debug(" Read done. %ldms", sw.Time());
                int read = e131SocketReceive->GetLastIOReadSize();
                _receiver->StashPacket(buffer, read);
            }
        }

        if (e131SocketReceive != nullptr)
        {
            e131SocketReceive->Close();
            delete e131SocketReceive;
            e131SocketReceive = nullptr;
        }

        logger_base.debug("E131 Receiving thread exiting.");
        return nullptr;
    }
};

std::string E131Receiver::ExtractE131Tag(uint8_t* packet)
{
    std::string res((char*)& packet[44]);
    res = res.substr(0, std::min(64, (int)res.size()));
    return res;
}

bool E131Receiver::IsLeft(uint8_t* packet)
{
    std::string tag = ExtractE131Tag(packet);
    if (tag == UniverseData::__leftTag) return true;
    if (tag == UniverseData::__rightTag) return false;

    if (UniverseData::__leftTag == "")
    {
        UniverseData::__leftTag = tag;
        wxCommandEvent e(EVT_TAG);
        e.SetInt(0);
        e.SetString(UniverseData::__leftTag);
        wxPostEvent(_frame, e);
        return true;
    }
    return false;
}

bool E131Receiver::IsRight(uint8_t* packet)
{
    std::string tag = ExtractE131Tag(packet);
    if (tag == UniverseData::__rightTag) return true;
    if (tag == UniverseData::__leftTag) return false;

    if (UniverseData::__leftTag != "" && UniverseData::__rightTag == "")
    {
        UniverseData::__rightTag = tag;
        wxCommandEvent e(EVT_TAG);
        e.SetInt(1);
        e.SetString(UniverseData::__rightTag);
        wxPostEvent(_frame, e);
        return true;
    }
    return false;
}

UniverseData* E131Receiver::GetUniverseData(int universe)
{
    if (_universes.find(universe) != _universes.end())
    {
        return _universes[universe];
    }
    return nullptr;
}

void E131Receiver::StashPacket(uint8_t* buffer, int size)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (IsSuspended()) return;

    if (size < E131_PACKET_HEADERLEN || size > E131_PACKET_HEADERLEN + 512) {
        wxASSERT(false);
        return;
    }

    int universe = ((int)buffer[113] << 8) + (int)buffer[114];
    if (universe < 1 || universe > 64000)
    {
        wxASSERT(false);
        return;
    }

    UniverseData* ud = GetUniverseData(universe);
    if (ud == nullptr)
    {
        wxASSERT(false);
        return;
    }

    if (IsLeft(buffer))
    {
        if (!ud->UpdateLeft(E131PORT, buffer, size))
        {
            logger_base.debug("Invalid packet.");
            wxASSERT(false);
        }
        else
        {
            _leftReceived++;
            // only flash the LED based on receipt of data for the first universe
            if (universe == _universes.begin()->second->GetUniverse())
            {
                if (ud->GetLeftSequenceNum() % 20 == 0)
                {
                    wxCommandEvent e(EVT_FLASH);
                    e.SetInt(0);
                    wxPostEvent(_frame, e);
                }
            }
        }
    }
    else if (IsRight(buffer))
    {
        if (!ud->UpdateRight(E131PORT, buffer, size))
        {
            logger_base.debug("Invalid packet.");
            wxASSERT(false);
        }
        else
        {
            _rightReceived++;
            // only flash the LED based on receipt of data for the first universe
            if (universe == _universes.begin()->second->GetUniverse())
            {
                if (ud->GetRightSequenceNum() % 20 == 0)
                {
                    wxCommandEvent e(EVT_FLASH);
                    e.SetInt(1);
                    wxPostEvent(_frame, e);
                }
            }
        }
    }
    else
    {
        wxASSERT(false);
    }
}

E131Receiver::E131Receiver(wxWindow* frame, const std::map<int, UniverseData*>& universes, const std::string& localInputIP)
{
    _frame = frame;
    _universes = universes;
    ZeroReceived();
    _stop = false;
    _suspend = false;

    std::list<int> unums;
    for (auto it : _universes)
    {
        unums.push_back(it.first);
    }
    _receiverThread = new E131ReceiverThread(this, localInputIP, unums);
    _receiverThread->Create();
    _receiverThread->Run();
}

E131Receiver::~E131Receiver()
{
    Stop();
}

void E131Receiver::Stop()
{
    _stop = true;

    // tell it to stop ... but it may take a bit of time to stop
    if (_receiverThread != nullptr)
    {
        _receiverThread->Stop();
        _receiverThread->Wait();
        delete _receiverThread;
        _receiverThread = nullptr;
    }
}