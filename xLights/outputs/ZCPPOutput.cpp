#include "ZCPPOutput.h"

#include <wx/xml/xml.h>
#include <wx/file.h>

#include "ZCPPDialog.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "../controllers/ControllerUploadData.h"
#include "../models/ModelManager.h"

#include <list>

#include <log4cpp/Category.hh>
#include <wx/filename.h>

#pragma region Constructors and Destructors
ZCPPOutput::ZCPPOutput(wxXmlNode* node, std::string showdir) : IPOutput(node)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    memset(_modelData, 0x00, sizeof(_modelData));
    _lastMinute = -1;
    _sequenceNum = 0;
    _datagram = nullptr;
    _data = (wxByte*)malloc(_channels);
    memset(_data, 0, _channels);
    memset(_packet, 0, sizeof(_packet));

    wxString fileName = GetIP();
    fileName.Replace(".", "_");
    fileName += ".zcpp";
    fileName = showdir + wxFileName::GetPathSeparator() + fileName;

    if (wxFile::Exists(fileName))
    {
        wxFile zf;
        if (zf.Open(fileName))
        {
            zf.Read(_modelData, sizeof(_modelData));
            zf.Close();
            logger_base.debug("ZCPP Model data file %s loaded.", (const char*)fileName.c_str());
        }
        else
        {
            logger_base.warn("ZCPP Model data file %s could not be opened.", (const char*)fileName.c_str());
        }
    }
    else
    {
        logger_base.warn("ZCPP Model data file %s not found.", (const char*)fileName.c_str());
    }
}

ZCPPOutput::ZCPPOutput() : IPOutput()
{
    memset(_modelData, 0x00, sizeof(_modelData));
    _lastMinute = -1;
    _channels = 1;
    _universe = 64001;
    _sequenceNum = 0;
    _datagram = nullptr;
    _data = (wxByte*)malloc(_channels);
    memset(_data, 0, _channels);
    memset(_packet, 0, sizeof(_packet));
}

ZCPPOutput::~ZCPPOutput()
{
    if (_datagram != nullptr) delete _datagram;
    if (_data != nullptr) free(_data);
}
#pragma endregion Constructors and Destructors

bool ZCPPOutput::SetModelData(unsigned char* buffer, size_t bufsize, std::string showDir)
{
    // before byte 9 there can be differences
    if (memcmp(&_modelData[8], &buffer[8], std::min(bufsize, sizeof(_modelData)) - 8) == 0)
    {
        // nothing has changed
        return false;
    }

    wxString fileName = GetIP();
    fileName.Replace(".", "_");
    fileName += ".zcpp";
    fileName = showDir + wxFileName::GetPathSeparator() + fileName;

    wxFile zf;
    if (zf.Create(fileName, true))
    {
        zf.Write(buffer, bufsize);
        zf.Close();
    }

    wxASSERT(bufsize <= ZCPP_MODELDATASIZE);
    memcpy(_modelData, buffer, std::min(bufsize, sizeof(_modelData)));
    _lastMinute = -1;

    return true;
}

wxXmlNode* ZCPPOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    IPOutput::Save(node);

    return node;
}

#pragma region Static Functions
void ZCPPOutput::SendSync(int syncUniverse)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static wxByte syncdata[ZCPP_SYNCPACKET_LEN];
    static wxByte syncSequenceNum = 0;
    static bool initialised = false;
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket *syncdatagram = nullptr;

    if (!initialised)
    {
        logger_base.debug("Initialising ZCPP Sync.");

        initialised = true;

        memset(syncdata, 0x00, sizeof(syncdata));

        syncdata[0] = 'Z';   // RLP preamble size (low)
        syncdata[1] = 'C';   // ACN Packet Identifier (12 bytes)
        syncdata[2] = 'P';
        syncdata[3] = 'P';
        syncdata[4] = 21;
        syncdata[5] = 0x00;

        wxIPV4address localaddr;
        if (IPOutput::__localIP == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(IPOutput::__localIP);
        }

        if (syncdatagram != nullptr)
        {
            delete syncdatagram;
        }

        syncdatagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);

        if (syncdatagram == nullptr)
        {
            logger_base.error("Error initialising ZCPP sync datagram.");
        }
        else if (!syncdatagram->IsOk())
        {
            logger_base.error("Error initialising ZCPP sync datagram ... is network connected? OK : FALSE");
            delete syncdatagram;
            syncdatagram = nullptr;
        }
        else if (syncdatagram->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating ZCPP sync datagram => %d : %s.", syncdatagram->LastError(), (const char *)DecodeIPError(syncdatagram->LastError()).c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
        }

        // multicast - universe number must be in lower 2 bytes
        wxString ipaddrWithUniv = "255.255.255.255";
        syncremoteAddr.Hostname(ipaddrWithUniv);
        syncremoteAddr.Service(ZCPP_PORT);
    }

    syncdata[6] = syncSequenceNum++;   // sequence number

    // bail if we dont have a datagram to use
    if (syncdatagram != nullptr)
    {
        syncdatagram->SendTo(syncremoteAddr, syncdata, ZCPP_SYNCPACKET_LEN);
    }
}

std::list<Output*> ZCPPOutput::Discover(OutputManager* outputManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::list<Output*> res;

    wxByte packet[7];

    packet[0] = 'Z';
    packet[1] = 'C';
    packet[2] = 'P';
    packet[3] = 'P';
    packet[4] = 0x00;
    packet[5] = 0x00;
    packet[6] = 0x00;

    wxIPV4address sendlocaladdr;
    if (IPOutput::__localIP == "")
    {
        sendlocaladdr.AnyAddress();
    }
    else
    {
        sendlocaladdr.Hostname(IPOutput::__localIP);
    }

    wxDatagramSocket* datagram = new wxDatagramSocket(sendlocaladdr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);

    if (datagram == nullptr)
    {
        logger_base.error("Error initialising ZCPP discovery datagram.");
    }
    else if (!datagram->IsOk())
    {
        logger_base.error("Error initialising ZCPP discovery datagram ... is network connected? OK : FALSE");
        delete datagram;
        datagram = nullptr;
    }
    else if (datagram->Error() != wxSOCKET_NOERROR)
    {
        logger_base.error("Error creating ZCPP discovery datagram => %d : %s.", datagram->LastError(), (const char *)DecodeIPError(datagram->LastError()).c_str());
        delete datagram;
        datagram = nullptr;
    }
    else
    {
        logger_base.info("ZCPP discovery datagram opened successfully.");
    }

    // multicast - universe number must be in lower 2 bytes
    wxIPV4address remoteaddr;
    wxString ipaddrWithUniv = "255.255.255.255";
    remoteaddr.Hostname(ipaddrWithUniv);
    remoteaddr.Service(ZCPP_PORT);

    // bail if we dont have a datagram to use
    if (datagram != nullptr)
    {
        logger_base.info("ZCPP sending discovery packet.");
        datagram->SendTo(remoteaddr, packet, sizeof(packet));
        if (datagram->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error sending ZCPP discovery datagram => %d : %s.", datagram->LastError(), (const char *)DecodeIPError(datagram->LastError()).c_str());
        }
        else
        {
            logger_base.info("ZCPP sent discovery packet. Sleeping for 2 seconds.");

            // give the controllers 2 seconds to respond
            wxMilliSleep(2000);

            unsigned char buffer[2048];

            int lastread = 1;

            while (lastread > 0)
            {
                wxStopWatch sw;
                logger_base.debug("Trying to read ZCPP discovery packet.");
                memset(buffer, 0x00, sizeof(buffer));
                datagram->Read(&buffer[0], sizeof(buffer));
                lastread = datagram->LastReadCount();

                if (lastread > 0)
                {
                    logger_base.debug(" Read done. %d bytes %ldms", lastread, sw.Time());

                    if (buffer[0] == 'Z' && buffer[1] == 'C' && buffer[2] == 'P' && buffer[3] == 'P' && buffer[4] == 0x01)
                    {
                        logger_base.debug(" Valid response.");
                        long channels = ((long)buffer[66] << 24) + ((long)buffer[67] << 16) + ((long)buffer[68] << 8) + (long)buffer[69];
                        ZCPPOutput* output = new ZCPPOutput();
                        output->SetId(1);
                        output->SetDescription(std::string((char*)&buffer[31]));
                        output->SetChannels(channels);
                        auto ip = wxString::Format("%d.%d.%d.%d", (int)buffer[90], (int)buffer[91], (int)buffer[92], (int)buffer[93]);
                        output->SetIP(ip.ToStdString());

                        // now search for it in outputManager
                        auto outputs = outputManager->GetOutputs();
                        for (auto it = outputs.begin(); it != outputs.end(); ++it)
                        {
                            if ((*it)->GetIP() == output->GetIP())
                            {
                                // we already know about this controller
                                logger_base.info("ZCPP Discovery we already know about this controller %s.", (const char*)output->GetIP().c_str());
                                delete output;
                                output = nullptr;
                                break;
                            }
                        }

                        if (output != nullptr)
                        {
                            logger_base.info("ZCPP Discovery adding controller %s.", (const char*)output->GetIP().c_str());
                            res.push_back(output);
                        }
                    }
                    else
                    {
                        // non discovery response packet
                        logger_base.info("ZCPP Discovery strange packet received.");
                    }
                }
            }
            logger_base.info("ZCPP Discovery Done looking for response.");
        }
        datagram->Close();
        delete datagram;
    }

    logger_base.info("ZCPP Discovery Finished.");

    return res;
}
#pragma endregion Static Functions

#pragma region Start and Stop
bool ZCPPOutput::Open()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_enabled) return true;

    _lastMinute = -1;

    _ok = IPOutput::Open();

    memset(_packet, 0x00, sizeof(_packet));
    _sequenceNum = 0;

    _packet[0] = 'Z';
    _packet[1] = 'C';
    _packet[2] = 'P';
    _packet[3] = 'P';
    _packet[4] = 20;
    _packet[5] = 0x00;

    wxIPV4address localaddr;
    if (IPOutput::__localIP == "")
    {
        localaddr.AnyAddress();
    }
    else
    {
        localaddr.Hostname(IPOutput::__localIP);
    }

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
    if (_datagram == nullptr)
    {
        logger_base.error("ZCPPOutput: Error opening datagram.");
    }
    else if (!_datagram->IsOk())
    {
        logger_base.error("ZCPPOutput: Error opening datagram. Network may not be connected? OK : FALSE");
        delete _datagram;
        _datagram = nullptr;
    }
    else if (_datagram->Error() != wxSOCKET_NOERROR)
    {
        logger_base.error("Error creating ZCPP datagram => %d : %s.", _datagram->LastError(), (const char *)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }

    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(ZCPP_PORT);

    return _ok && _datagram != nullptr;
}

void ZCPPOutput::Close()
{
}
#pragma endregion Start and Stop

void ZCPPOutput::SetTransientData(int on, long startChannel, int nullnumber)
{
    _outputNumber = on;
    _startChannel = startChannel;
    if (nullnumber > 0) _nullNumber = nullnumber;
}

#pragma region Frame Handling
void ZCPPOutput::StartFrame(long msec)
{
    if (!_enabled) return;

    _timer_msec = msec;
}

void ZCPPOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend) return;

    if (_datagram == nullptr) return;

    int min = wxDateTime::Now().GetMinute();
    if (min != _lastMinute)
    {
        if (_modelData[0] != 0x00)
        {
            _lastMinute = min;

            _datagram->SendTo(_remoteAddr, _modelData, sizeof(_modelData));
        }
    }

    if (_changed || NeedToOutput(suppressFrames))
    {
        int i = 0;
        while (i < _channels)
        {
            _packet[6] = _sequenceNum;
            long startAddress = i;
            _packet[7] = (wxByte)((startAddress >> 24) & 0xFF);
            _packet[8] = (wxByte)((startAddress >> 16) & 0xFF);
            _packet[9] = (wxByte)((startAddress >> 8) & 0xFF);
            _packet[10] = (wxByte)((startAddress) & 0xFF);
            _packet[11] = OutputManager::IsSyncEnabled_() ? 1 : 0;
            int packetlen = _channels - i > ZCPP_PACKET_LEN - 14 ? ZCPP_PACKET_LEN - 14 : _channels - i;
            _packet[12] = (wxByte)((packetlen >> 8) & 0xFF);
            _packet[13] = (wxByte)((packetlen) & 0xFF);
            memcpy(&_packet[14], &_data[i], packetlen);
            _datagram->SendTo(_remoteAddr, _packet, 14 + packetlen);
            _sequenceNum = _sequenceNum == 255 ? 0 : _sequenceNum + 1;
            i += packetlen;
        }

        FrameOutput();
    }
    else
    {
        SkipFrame();
    }
}

void ZCPPOutput::ResetFrame()
{
    if (!_enabled) return;
}
#pragma endregion Frame Handling

#pragma region Data Setting
void ZCPPOutput::SetOneChannel(long channel, unsigned char data)
{
    if (!_enabled) return;

    if (_data[channel] != data) {
        _data[channel] = data;
        _changed = true;
    }
}

void ZCPPOutput::SetManyChannels(long channel, unsigned char data[], long size)
{
        long chs = std::min(size, _channels - channel);

        if (memcmp(&_data[channel], data, chs) == 0)
        {
            // nothing changed
        }
        else
        {
            memcpy(&_data[channel], data, chs);
            _changed = true;
        }
}

void ZCPPOutput::AllOff()
{
    memset(_data, 0x00, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
long ZCPPOutput::GetEndChannel() const
{
    return _startChannel + _channels - 1;
}

std::string ZCPPOutput::GetLongDescription() const
{
    std::string res = "";

        if (!_enabled) res += "INACTIVE ";
        res += "ZCPP " + _ip;
        res += " [1-" + std::string(wxString::Format(wxT("%li"), (long)_channels)) + "] ";
        res += "(" + std::string(wxString::Format(wxT("%li"), (long)GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%li"), (long)GetActualEndChannel())) + ") ";
        res += _description;

    return res;
}

std::string ZCPPOutput::GetChannelMapping(long ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%li"), ch)) + " maps to ...\n";

    res += "Type: ZCPP\n";
    long channeloffset = ch - GetStartChannel() + 1;
    res += "IP: " + _ip + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%li"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";
    return res;
}

std::string ZCPPOutput::GetUniverseString() const
{
    return IPOutput::GetUniverseString();
}
#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* ZCPPOutput::Configure(wxWindow* parent, OutputManager* outputManager)
{
    ZCPPDialog dlg(parent, this, outputManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}
#endif
#pragma endregion UI
