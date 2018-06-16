#include "ListenerARTNet.h"
#include <log4cpp/Category.hh>
#include <wx/socket.h>
#include "../../xLights/outputs/ArtNetOutput.h"
#include "ListenerManager.h"
#include "../../xLights/UtilFunctions.h"

bool ListenerARTNet::IsValidHeader(wxByte* buffer)
{
    return
        buffer[0] == 'A' &&
        buffer[1] == 'r' &&
        buffer[2] == 't' &&
        buffer[3] == '-' &&
        buffer[4] == 'N' &&
        buffer[5] == 'e' &&
        buffer[6] == 't' &&
        buffer[7] == 0x00;
}

ListenerARTNet::ListenerARTNet(ListenerManager* listenerManager) : ListenerBase(listenerManager)
{
    _socket = nullptr;
}

void ListenerARTNet::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ARTNet listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerARTNet::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ARTNet listener stopping.");
    _stop = true;
    if (_thread != nullptr)
    {
        if (_socket != nullptr)
            _socket->SetTimeout(0);
        _thread->Stop();
    }
}

void ListenerARTNet::StartProcess()
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
    localaddr.Service(ARTNET_PORT);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
    if (_socket == nullptr)
    {
        logger_base.error("Error opening datagram for ARTNet reception.");
    }
    else if (!_socket->IsOk())
    {
        logger_base.error("Error opening datagram for ARTNet reception. OK : FALSE");
        delete _socket;
        _socket = nullptr;
    }
    else if (_socket->Error())
    {
        logger_base.error("Error opening datagram for ARTNet reception. %d : %s", _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str());
        delete _socket;
        _socket = nullptr;
    }
    else
    {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("ARTNet reception datagram opened successfully.");
    }
}

void ListenerARTNet::StopProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("ARTNet Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
}

void ListenerARTNet::Poll()
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr)
    {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        //wxStopWatch sw;
        //logger_base.debug("Trying to read ARTNet packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        if (_stop) return;
        //logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0)
        {
            _socket->WaitForRead(0, 500);
        }
        else
        {
            if (IsValidHeader(buffer))
            {
                int size = ((buffer[16] << 8) + buffer[17]) & 0x0FFF;
                //logger_base.debug("Processing packet.");
                if (buffer[9] == 0x50)
                {
                    // ARTNet data packet
                    int universe = (buffer[13] << 8) + buffer[14];
                    _listenerManager->ProcessPacket(GetType(), universe, &buffer[ARTNET_PACKET_HEADERLEN], size);
                }
                else if (buffer[9] == 0x99)
                {
                    // Trigger data packet
                    wxByte key = buffer[14];
                    wxByte subkey = buffer[15];
                    // TODO add event using ARTNet trigger packets
                    //_listenerManager->ProcessPacket(GetType() + " Trigger", (key << 8) + subkey, &buffer[16], size);
                }
                else if (buffer[9] == 0x97)
                {
                    // Timecode data packet
                    wxByte frames = buffer[14];
                    wxByte secs = buffer[15];
                    wxByte mins = buffer[16];
                    wxByte hours = buffer[17];
                    wxByte mode = buffer[18];

                    long ms = ((hours * 60 + mins) * 60 + secs) * 1000;
                    switch (mode)
                    {
                    case 0:
                        //24 fps
                        ms += frames * 1000 / 24;
                        break;
                    case 1:
                        //25 fps
                        ms += frames * 1000 / 25;
                        break;
                    case 2:
                        //29.97 fps
                        ms += frames * 100000 / 2997;
                        break;
                    case 3:
                        //30 fps
                        ms += frames * 1000 / 30;
                        break;
                    default:
                        break;
                    }
                    // TODO add sync using ARTNet timecode packets
                    _listenerManager->Sync("", ms, GetType());
                }
                //logger_base.debug("Processing packet done.");
            }
        }
    }
}
