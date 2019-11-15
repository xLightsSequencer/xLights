#include "ListenerE131.h"
#include <log4cpp/Category.hh>
#include <wx/socket.h>
#include "../../xLights/outputs/E131Output.h"
#include "ListenerManager.h"
#include "../../xLights/UtilFunctions.h"

bool ListenerE131::IsValidHeader(uint8_t* buffer)
{
    return
        buffer[0] == 0x00 &&
        buffer[1] == 0x10 &&
        buffer[4] == 'A' &&
        buffer[5] == 'S' &&
        buffer[6] == 'C' &&
        buffer[7] == '-' &&
        buffer[8] == 'E' &&
        buffer[9] == '1' &&
        buffer[10] == '.' &&
        buffer[11] == '1' &&
        buffer[12] == '7';
}

ListenerE131::ListenerE131(ListenerManager* listenerManager) : ListenerBase(listenerManager)
{
    _socket = nullptr;
}

void ListenerE131::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("E131 listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerE131::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop)
    {
        logger_base.debug("E131 listener stopping.");
        if (_socket != nullptr)
            _socket->SetTimeout(0);
        if (_thread != nullptr)
        {
            _stop = true;
            _thread->Stop();
            _thread->Delete();
            delete _thread;
            _thread = nullptr;
        }
    }
}

void ListenerE131::StartProcess()
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
    localaddr.Service(E131_PORT);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
    if (_socket == nullptr)
    {
        logger_base.error("Error opening datagram for E131 reception. %s", (const char *)localaddr.IPAddress().c_str());
    }
    else if (!_socket->IsOk())
    {
        logger_base.error("Error opening datagram for E131 reception. %s OK : FALSE", (const char *)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    }
    else if (_socket->Error())
    {
        logger_base.error("Error opening datagram for E131 reception. %d : %s %s", _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str(), (const char *)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    }
    else
    {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("E131 reception datagram opened successfully.");
        _isOk = true;
    }
}

void ListenerE131::StopProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("E131 Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _isOk = false;
}

void ListenerE131::Poll()
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr)
    {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        //wxStopWatch sw;
        //logger_base.debug("Trying to read E131 packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        if (_stop) return;
        //logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0)
        {
            _socket->WaitForRead(0, 50);
        }
        else
        {
            if (IsValidHeader(buffer))
            {
                int size = ((buffer[16] << 8) + buffer[17]) & 0x0FFF;
                int universe = (buffer[113] << 8) + buffer[114];
                //logger_base.debug("Processing packet.");
                _listenerManager->ProcessPacket(GetType(), universe, &buffer[126], size - 126);
                //logger_base.debug("Processing packet done.");
            }
        }
    }
}
