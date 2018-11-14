#include "ListenerFPP.h"
#include <log4cpp/Category.hh>
#include <wx/socket.h>
#include "ListenerManager.h"
#include "../../xLights/outputs/IPOutput.h"
#include "../Control.h"
#include "../../xLights/UtilFunctions.h"

bool ListenerFPP::IsValidHeader(wxByte* buffer)
{
    return  buffer[0] == 'F' &&
            buffer[1] == 'P' &&
            buffer[2] == 'P' &&
            buffer[3] == 'D';
}

ListenerFPP::ListenerFPP(ListenerManager* listenerManager) : ListenerBase(listenerManager)
{
    _frameMS = 50;
    _socket = nullptr;
}

void ListenerFPP::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerFPP::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop)
    {
        logger_base.debug("FPP listener stopping.");
        if (_socket != nullptr)
            _socket->SetTimeout(0);
        if (_thread != nullptr)
        {
            _stop = true;
            _thread->Stop();
        }
    }
}

void ListenerFPP::StartProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address localaddr;
    if (IPOutput::GetLocalIP() == "")
    {
        localaddr.AnyAddress();
    }
    else
    {
        localaddr.Hostname(IPOutput::GetLocalIP());
    }
    localaddr.Service(FPP_CTRL_PORT);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
    if (_socket == nullptr)
    {
        logger_base.error("Error opening datagram for FPP reception. %s", (const char *)localaddr.IPAddress().c_str());
    }
    else if (!_socket->IsOk())
    {
        logger_base.error("Error opening datagram for FPP reception. %s OK : FALSE", (const char *)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    }
    else if (_socket->Error())
    {
        logger_base.error("Error opening datagram for FPP reception. %d : %s %s", _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str(), (const char *)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    }
    else
    {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("FPP reception datagram opened successfully.");
        _isOk = true;
    }
}

void ListenerFPP::StopProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("FPP Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _isOk = false;
}

void ListenerFPP::Poll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr)
    {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        //wxStopWatch sw;
        //logger_base.debug("Trying to read FPP packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        if (_stop) return;
        //logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0)
        {
            //logger_base.debug("Waiting for read.");
            _socket->WaitForRead(0, 50);
            //logger_base.debug("Waiting for read done.");
        }
        else
        {
            if (IsValidHeader(buffer))
            {
                ControlPkt* cp = (ControlPkt*)&buffer[0];
                if (cp->pktType == CTRL_PKT_SYNC)
                {
                    SyncPkt* sp = (SyncPkt*)(&buffer[0] + sizeof(ControlPkt));

                    if (sp->fileType == SYNC_FILE_SEQ)
                    {
                        uint8_t packetType = sp->pktType;
                        std::string fileName = std::string(sp->filename);
                        uint32_t frameNumber = sp->frameNumber;
                        long ms = frameNumber * _frameMS;

                        if (packetType != -1)
                        {
                            switch (packetType)
                            {
                            case SYNC_PKT_START:
                            {
                                logger_base.debug("!!!!!!!!!!!!!!!!!!!!!!!!!!! Remote start %s.", (const char *)fileName.c_str());

                                _frameMS = _listenerManager->Sync(fileName, 0, GetType());
                            }
                            break;
                            case SYNC_PKT_STOP:
                            {
                                logger_base.debug("!!!!!!!!!!!!!!!!!!!!!!!!!!! Remote stop %s.", (const char *)fileName.c_str());
                                _listenerManager->Sync(fileName, 0xFFFFFFFF, GetType());
                            }
                            break;
                            case SYNC_PKT_SYNC:
                            {
                                _listenerManager->Sync(fileName, ms, GetType());
                            }
                            break;
                            default:
                                break;
                            }
                        }
                        else
                        {
                            // media file ... not sure what to do with this ... so ignoring it
                        }
                    }
                }
                else if (cp->pktType == CTRL_PKT_EVENT)
                {
                    _listenerManager->ProcessPacket(GetType(), std::string((char*)(buffer + sizeof(ControlPkt))));
                }
            }
        }
    }
}
