#include "Emitter.h"
#include <log4cpp/Category.hh>
#include <wx/wx.h>
#include <wx/thread.h>
#include <wx/socket.h>
#include "../xLights/UtilFunctions.h"
#include "xFadeMain.h"
#include "Settings.h"
#include "PacketData.h"

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
        //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        if (!_stop)
        {
            _stop = true;
        }
    }

    void Stop()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Asking emitter thread to stop");
        _stop = true;
    }
    
    void Blend(uint8_t* buffer, uint8_t* blendBuffer, size_t channels, float pos, std::list<int> excludeChannels)
    {
        float inv = 1.0 - pos;
        for (size_t i = 0; i < channels; ++i)
        {
            if (std::find(excludeChannels.begin(), excludeChannels.end(), i+1) != excludeChannels.end())
            {
                if (pos < 0.5)
                {
                    // dont change anything
                }
                else
                {
                    *(buffer + i) = *(blendBuffer + i);
                }
            }
            else
            {
                *(buffer + i) = (uint8_t)((float)*(buffer + i) * inv + (float)*(blendBuffer + i) * pos);
            }
        }
    }

    void PrepareData(PacketData* target, PacketData* source, long protocol)
    {
        if (protocol == 0 || source->_type == protocol)
        {
            // no conversion required
            target->CopyFrom(source, source->_type);
        }
        else
        {
            // conversion required
            target->CopyFrom(source, protocol);
        }
    }

    virtual void* Entry() override
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Emitter thread started");

        std::map<int, std::string> ips = _emitter->GetIps();

        PacketData sendData;

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
            logger_base.error("E131 Output: Error opening datagram. Network may not be connected? OK : FALSE, From %s", (const char*)localaddr.IPAddress().c_str());
        }
        else if (!e131SocketSend->IsOk())
        {
            logger_base.error("E131 Output: Error opening datagram. Network may not be connected? OK : FALSE, From %s", (const char*)localaddr.IPAddress().c_str());
            delete e131SocketSend;
            e131SocketSend = nullptr;
        }
        else if (e131SocketSend->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating E131 datagram => %d : %s, from %s.", e131SocketSend->LastError(), (const char*)DecodeIPError(e131SocketSend->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
            delete e131SocketSend;
            e131SocketSend = nullptr;
        }

        artNETSocketSend = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
        if (artNETSocketSend == nullptr)
        {
            logger_base.error("artNET Output: Error opening datagram. Network may not be connected? OK : FALSE, From %s", (const char*)localaddr.IPAddress().c_str());
        }
        else if (!artNETSocketSend->IsOk())
        {
            logger_base.error("artNET Output: Error opening datagram. Network may not be connected? OK : FALSE, From %s", (const char*)localaddr.IPAddress().c_str());
            delete artNETSocketSend;
            artNETSocketSend = nullptr;
        }
        else if (artNETSocketSend->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating artNET datagram => %d : %s, from %s.", artNETSocketSend->LastError(), (const char*)DecodeIPError(artNETSocketSend->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
            delete artNETSocketSend;
            artNETSocketSend = nullptr;
        }

        while (!_stop)
        {
            auto start = wxDateTime::UNow();
            int diffMS = 0;

            {
                // output the frames now
                for (const auto& it : ips)
                {
                    std::list<int> excludeChannels = _emitter->GetSettings()->GetExcludeChannels(it.first);

                    PacketData l = _emitter->GetLeft(it.first);
                    PacketData r = _emitter->GetRight(it.first);

                    if (l._length == 0 && r._length > 0)
                    {
                        l.InitialiseLength(r._type, r._length, it.first);
                    }
                    else if (r._length == 0 && l._length > 0)
                    {
                        r.InitialiseLength(l._type, l._length, it.first);
                    }

                    auto protocol = _emitter->GetProtocol(it.first);

                    float pos = _emitter->GetPos();

                    if (pos == 0.0)
                    {
                        PrepareData(&sendData, &l, protocol);
                        sendData.ApplyBrightness(_emitter->GetLeftBrightness(), excludeChannels);
                    }
                    else if (pos == 1.0)
                    {
                        PrepareData(&sendData, &r, protocol);
                        sendData.ApplyBrightness(_emitter->GetRightBrightness(), excludeChannels);
                    }
                    else
                    {
                        int sz = std::min(l.GetDataLength(), r.GetDataLength());

                        l.ApplyBrightness(_emitter->GetLeftBrightness(), excludeChannels);
                        r.ApplyBrightness(_emitter->GetRightBrightness(), excludeChannels);

                        Blend(l.GetDataPtr(), r.GetDataPtr(), sz, pos, excludeChannels);
                        PrepareData(&sendData, &l, protocol);
                    }

                    sendData.Send(e131SocketSend, artNETSocketSend, it.second);
                    _emitter->IncrementSent();
                }

                auto diff = wxDateTime::UNow() - start;
                diffMS = _emitter->GetFrameMS() - diff.GetMilliseconds().ToLong();
            }

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

        logger_base.debug("Emitter thread exiting.");
        return nullptr;
    }
};

Emitter::Emitter(std::map<int, std::string>* ip, std::map<int, PacketData>* left, std::map<int, PacketData>* right, std::map<int, std::string>* protocol, std::mutex* lock, std::string localIP, Settings* settings)
{
    _settings = settings;
    _sent = 0;
    _localIP = localIP;
    _stop = false;
    _lock = lock;
    _frameMS = 50;
    _targetIP = ip;
    _leftData = left;
    _rightData = right;
    _protocol = protocol;
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

void Emitter::Restart()
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

    _stop = false;
    _emitterThread = new EmitterThread(this);
    _emitterThread->Create();
    _emitterThread->Run();
}

std::map<int, std::string> Emitter::GetIps() const
{
    std::unique_lock<std::mutex> mutLock(*_lock);

    std::map<int, std::string> res;

    if (!_stop)
    {
        for (const auto& it : *_targetIP)
        {
            res[it.first] = it.second;
        }
    }

    return res;
}

PacketData Emitter::GetLeft(int u) const
{
    std::unique_lock<std::mutex> mutLock(*_lock);

    if (!_stop)
    {
        if (_leftData->find(u) != _leftData->end())
        {
            PacketData d((*_leftData)[u]);
            return d;
        }
    }

    return PacketData();
}

PacketData Emitter::GetRight(int u) const
{
    std::unique_lock<std::mutex> mutLock(*_lock);

    if (!_stop)
    {
        if (_rightData->find(u) != _rightData->end())
        {
            PacketData d((*_rightData)[u]);
            return d;
        }
    }

    return PacketData();
}

long Emitter::GetProtocol(int u) const
{
    std::unique_lock<std::mutex> mutLock(*_lock);

    if (!_stop)
    {
        if (_protocol->find(u) != _protocol->end())
        {
            auto p = (*_protocol)[u];

            if (p == "As per input")
            {
                return 0;
            }
            else if (p == "E1.31")
            {
                return xFadeFrame::ID_E131SOCKET;
            }
            else if (p == "ArtNET")
            {
                return xFadeFrame::ID_ARTNETSOCKET;
            }
        }
    }

    return 0;
}
