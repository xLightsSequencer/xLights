#include "Emitter.h"

#include <wx/wx.h>
#include <wx/thread.h>
#include <wx/socket.h>

#include "xFadeMain.h"
#include "Settings.h"
#include "PacketData.h"
#include "../xLights/UtilFunctions.h"

#include "spdlog/spdlog.h"

class EmitterThread : public wxThread
{
    Emitter* _emitter;
    volatile bool _stop;

public:

    EmitterThread(Emitter* emitter) : wxThread(wxTHREAD_JOINABLE)
    {
        _emitter = emitter;
        _stop = false;
    }

    virtual ~EmitterThread()
    {
        if (!_stop)
        {
            _stop = true;
        }
    }

    void Stop()
    {
        spdlog::debug("Asking emitter thread to stop");
        _stop = true;
    }
    
    virtual void* Entry() override
    {
        spdlog::debug("Emitter thread started");

        wxIPV4address localaddr;
        wxDatagramSocket* e131SocketSend = nullptr;
        wxDatagramSocket* artNETSocketSend = nullptr;

        if (_emitter->GetLocalIP() == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(_emitter->GetLocalIP());
        }

        e131SocketSend = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
        if (e131SocketSend == nullptr)
        {
            spdlog::error("E131 Output: Error opening datagram. Network may not be connected? OK : FALSE, From {}", localaddr.IPAddress().ToStdString());
        }
        else if (!e131SocketSend->IsOk())
        {
            spdlog::error("E131 Output: Error opening datagram. Network may not be connected? OK : FALSE, From {}", localaddr.IPAddress().ToStdString());
            delete e131SocketSend;
            e131SocketSend = nullptr;
        }
        else if (e131SocketSend->Error() != wxSOCKET_NOERROR)
        {
            spdlog::error("Error creating E131 datagram => {} : {}, from {}.", (int)e131SocketSend->LastError(), DecodeIPError(e131SocketSend->LastError()), localaddr.IPAddress().ToStdString());
            delete e131SocketSend;
            e131SocketSend = nullptr;
        }

        artNETSocketSend = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
        if (artNETSocketSend == nullptr)
        {
            spdlog::error("artNET Output: Error opening datagram. Network may not be connected? OK : FALSE, From {}", localaddr.IPAddress().ToStdString());
        }
        else if (!artNETSocketSend->IsOk())
        {
            spdlog::error("artNET Output: Error opening datagram. Network may not be connected? OK : FALSE, From {}", localaddr.IPAddress().ToStdString());
            delete artNETSocketSend;
            artNETSocketSend = nullptr;
        }
        else if (artNETSocketSend->Error() != wxSOCKET_NOERROR)
        {
            spdlog::error("Error creating artNET datagram => {} : {}, from {}.", (int)artNETSocketSend->LastError(), DecodeIPError(artNETSocketSend->LastError()), localaddr.IPAddress().ToStdString());
            delete artNETSocketSend;
            artNETSocketSend = nullptr;
        }

        PacketData sendData;
        auto universes = _emitter->GetUniverses();

        while (!_stop)
        {
            auto start = wxDateTime::UNow();
            int diffMS = 0;

            int lb = _emitter->GetLeftBrightness();
            int rb = _emitter->GetRightBrightness();
            float pos = _emitter->GetPos();

            wxASSERT(lb >= 0 && lb <= 100);
            wxASSERT(rb >= 0 && rb <= 100);
            wxASSERT(pos >= 0.0 && pos <= 1.0);

            // output the frames now
            for (const auto& it : universes)
            {
                it.second->GetOutput(&sendData, lb, rb, pos);
                sendData.Send(e131SocketSend, artNETSocketSend, it.second->GetTargetIP());
                _emitter->IncrementSent();
            }

            auto diff = wxDateTime::UNow() - start;
            diffMS = _emitter->GetFrameMS() - diff.GetMilliseconds().ToLong();

            if (diffMS > 0)
                wxMilliSleep(diffMS);
        }

        if (e131SocketSend != nullptr)
        {
            e131SocketSend->Close();
            delete e131SocketSend;
            e131SocketSend = nullptr;
        }

        if (artNETSocketSend != nullptr)
        {
            artNETSocketSend->Close();
            delete artNETSocketSend;
            artNETSocketSend = nullptr;
        }

        spdlog::debug("Emitter thread exiting.");
        return nullptr;
    }
};

Emitter::Emitter(const std::map<int, UniverseData*>& universes, std::string localIP, Settings* settings)
{
    _universes = universes;
    _settings = settings;
    _sent = 0;
    _pos = 0.0;
    _localIP = localIP;
    _stop = false;
    _frameMS = 50;
    _leftBrightness = 100;
    _rightBrightness = 100;

    _emitterThread = new EmitterThread(this);
    _emitterThread->Create();
    _emitterThread->Run();
}

Emitter::~Emitter()
{
    Stop();
}

void Emitter::Stop()
{
    _stop = true;

    // tell it to stop ... but it may take a bit of time to stop
    if (_emitterThread != nullptr)
    {
        _emitterThread->Stop();
        _emitterThread->Wait();
        delete _emitterThread;
        _emitterThread = nullptr;
    }
}
