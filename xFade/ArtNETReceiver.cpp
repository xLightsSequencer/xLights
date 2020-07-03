#include "ArtNETReceiver.h"

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

class ArtNETReceiverThread : public wxThread
{
    ArtNETReceiver* _receiver = nullptr;
    std::string _localIP; // local ip we are listening on
    volatile bool _stop;
    std::list<int> _universes;

public:

    ArtNETReceiverThread(ArtNETReceiver* receiver, const std::string& localIP, const std::list<int> universes) : wxThread(wxTHREAD_JOINABLE)
    {
        _receiver = receiver;
        _stop = false;
        _localIP = localIP;
        _universes = universes;
    }

    virtual ~ArtNETReceiverThread()
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
        logger_base.debug("Asking artNET Receiver thread to stop");
        _stop = true;
    }
    
    virtual void* Entry() override
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("artNET Receiver thread started");

        wxIPV4address addr;
        if (_localIP == "")
        {
            addr.AnyAddress();
        }
        else
        {
            addr.Hostname(_localIP.c_str());
        }
        addr.Service(ARTNETPORT);
        //create and bind to the address above
        wxDatagramSocket* artNETSocketReceive = new wxDatagramSocket(addr);

        if (artNETSocketReceive == nullptr)
        {
            logger_base.error("Problem listening for artNET. artNET Receiver thread exiting.");
            return nullptr;
        }
        else if (!artNETSocketReceive->IsOk())
        {
            logger_base.error("Problem listening for artNET. artNET Receiver thread exiting.");
            delete artNETSocketReceive;
            artNETSocketReceive = nullptr;
            return nullptr;
        }
        else if (artNETSocketReceive->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Problem listening for artNET => %d : %s, from %s.", artNETSocketReceive->LastError(), (const char*)DecodeIPError(artNETSocketReceive->LastError()).c_str(), (const char*)addr.IPAddress().c_str());
            delete artNETSocketReceive;
            artNETSocketReceive = nullptr;
            return nullptr;
        }

        logger_base.debug("artNET listening on %s", (const char*)addr.IPAddress().c_str());

        artNETSocketReceive->Notify(false);
        artNETSocketReceive->SetTimeout(1);

        uint8_t buffer[ARTNET_PACKET_HEADERLEN + 512];

        while (!_stop)
        {
            memset(buffer, 0x00, sizeof(buffer));

            artNETSocketReceive->Read(buffer, sizeof(buffer));
            if (!_stop)
            {
                //logger_base.debug(" Read done. %ldms", sw.Time());
                int read = artNETSocketReceive->GetLastIOReadSize();
                _receiver->StashPacket(buffer, read);
            }
        }

        if (artNETSocketReceive != nullptr)
        {
            artNETSocketReceive->Close();
            delete artNETSocketReceive;
            artNETSocketReceive = nullptr;
        }

        logger_base.debug("artNET Receiving thread exiting.");
        return nullptr;
    }
};

ArtNETReceiver::ArtNETReceiver(wxWindow* frame, const std::map<int, UniverseData*>& universes, const std::string& localInputIP)
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
    _receiverThread = new ArtNETReceiverThread(this, localInputIP, unums);
    _receiverThread->Create();
    _receiverThread->Run();
}

ArtNETReceiver::~ArtNETReceiver()
{
    Stop();
}

void ArtNETReceiver::Stop()
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

bool ArtNETReceiver::IsLeft(uint8_t* packet)
{
    if (UniverseData::__leftTag == "ARTNET") return true;
    if (UniverseData::__rightTag == "ARTNET") return false;
    if (UniverseData::__leftTag == "" && UniverseData::__rightTag != "")
    {
        UniverseData::__leftTag = "ARTNET";
        wxCommandEvent e(EVT_TAG);
        e.SetInt(0);
        e.SetString(UniverseData::__leftTag);
        wxPostEvent(_frame, e);
        return true;
    }
    return false;
}

bool ArtNETReceiver::IsRight(uint8_t* packet)
{
    if (UniverseData::__leftTag == "ARTNET") return false;
    if (UniverseData::__rightTag == "ARTNET") return true;
    if (UniverseData::__rightTag == "")
    {
        UniverseData::__rightTag = "ARTNET";
        wxCommandEvent e(EVT_TAG);
        e.SetInt(1);
        e.SetString(UniverseData::__rightTag);
        wxPostEvent(_frame, e);
        return true;
    }
    return false;
}

UniverseData* ArtNETReceiver::GetUniverseData(int universe)
{
    if (_universes.find(universe) != _universes.end())
    {
        return _universes[universe];
    }
    return nullptr;
}

void ArtNETReceiver::StashPacket(uint8_t* buffer, int size)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (IsSuspended()) return;

    if (size < ARTNET_PACKET_HEADERLEN || size > ARTNET_PACKET_HEADERLEN + 512) {
        return;
    }

    int universe = ((int)buffer[15] << 8) + (int)buffer[14];

    UniverseData* ud = GetUniverseData(universe);
    if (ud == nullptr) return;

    if (IsLeft(buffer))
    {
        if (!ud->UpdateLeft(ARTNETPORT, buffer, size))
        {
            logger_base.debug("Invalid packet.");
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
        if (!ud->UpdateRight(ARTNETPORT, buffer, size))
        {
            logger_base.debug("Invalid packet.");
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
}