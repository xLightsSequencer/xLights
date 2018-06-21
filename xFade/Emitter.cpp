#include "Emitter.h"
#include <log4cpp/Category.hh>
#include <wx/wx.h>
#include <wx/thread.h>
#include "xFadeMain.h"

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
    
    void Blend(wxByte* buffer, wxByte* blendBuffer, size_t channels, float pos)
    {
        float inv = 1.0 - pos;
        for (size_t i = 0; i < channels; ++i)
        {
            *(buffer + i) = (wxByte)((float)*(buffer + i) * inv + (float)*(blendBuffer + i) * pos);
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
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Emitter thread started");

        PacketData sendData;
        sendData.SetLocalIP(_emitter->GetLocalIP());

        while (!_stop)
        {
            auto start = wxDateTime::UNow();
            int diffMS = 0;

            {
                // output the frames now
                auto ips = _emitter->GetIps();

                for (auto it = ips.begin(); it != ips.end(); ++it)
                {
                    auto l = _emitter->GetLeft(it->first);
                    auto r = _emitter->GetRight(it->first);

                    if (l._length == 0 && r._length > 0)
                    {
                        l.InitialiseLength(r._type, r._length, it->first);
                    }
                    else if (r._length == 0 && l._length > 0)
                    {
                        r.InitialiseLength(l._type, l._length, it->first);
                    }

                    auto protocol = _emitter->GetProtocol(it->first);

                    float pos = _emitter->GetPos();

                    if (pos == 0.0)
                    {
                        PrepareData(&sendData, &l, protocol);
                        sendData.ApplyBrightness(_emitter->GetLeftBrightness());
                    }
                    else if (pos == 1.0)
                    {
                        PrepareData(&sendData, &r, protocol);
                        sendData.ApplyBrightness(_emitter->GetRightBrightness());
                    }
                    else
                    {
                        int sz = std::min(l.GetDataLength(), r.GetDataLength());

                        l.ApplyBrightness(_emitter->GetLeftBrightness());
                        r.ApplyBrightness(_emitter->GetRightBrightness());

                        Blend(l.GetDataPtr(), r.GetDataPtr(), sz, pos);
                        PrepareData(&sendData, &l, protocol);
                    }

                    sendData.Send(it->second);
                    _emitter->IncrementSent();
                }

                int ms = _emitter->GetFrameMS();

                auto diff = wxDateTime::UNow() - start;
                diffMS = _emitter->GetFrameMS() - diff.GetMilliseconds().ToLong();
            }

            if (diffMS > 0)
                wxMilliSleep(diffMS);
        }
        logger_base.debug("Emitter thread exiting.");
        return nullptr;
    }
};

Emitter::Emitter(std::map<int, std::string>* ip, std::map<int, PacketData>* left, std::map<int, PacketData>* right, std::map<int, std::string>* protocol, std::mutex* lock, std::string localIP)
{
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
        for (auto it = _targetIP->begin(); it != _targetIP->end(); ++it)
        {
            res[it->first] = it->second;
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
