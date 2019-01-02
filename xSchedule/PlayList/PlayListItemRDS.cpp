#include "PlayListItemRDS.h"
#include "PlayListItemRDSPanel.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include "../../xLights/outputs/serial.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "PlayListStep.h"
#include "../../xLights/AudioManager.h"
#include "PlayList.h"

#define MRDS_STARTBYTEWRITE (uint8_t)0xD6
#define RDS_STARTBYTEREAD (uint8_t)0xD7

class EDMRDSThread : public wxThread
{
    std::string _text;
    std::string _stationName;
    std::string _commPort;

public:
    EDMRDSThread(std::string text, std::string stationName, std::string commPort)
    {
        _text = text;
        _stationName = stationName;
        _commPort = commPort;
    }
    virtual ~EDMRDSThread() { }

    virtual void* Entry() override
    {
        log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("PlayListRDS in thread.");

        logger_base.info("RDS: PS '%s' DPS '%s'.", (const char *)_stationName.c_str(), (const char *)_text.c_str());

        if (_commPort == "")
        {
            logger_base.warn("RDS: No comm port specified.");
            return nullptr;
        }

        auto serial = new SerialPort();

        char serialConfig[4];
        strcpy(serialConfig, "8N1");
        int errcode = serial->Open(_commPort, 19200, serialConfig);
        if (errcode < 0)
        {
            logger_base.warn("RDS: Unable to open serial port %s. Error code = %d", (const char *)_commPort.c_str(), errcode);
            delete serial;
            return nullptr;
        }

        logger_base.debug("Serial port open %s, %d baud, %s.", (const char *)_commPort.c_str(), 19200, serialConfig);

        PlayListItemRDS::InitialiseDTRCTS(serial);

        unsigned char outBuffer[100];
        memset(outBuffer, 0x00, sizeof(outBuffer));

        outBuffer[0] = MRDS_STARTBYTEWRITE;
        outBuffer[1] = 0x00;
        outBuffer[2] = 0xFF;
        outBuffer[3] = 0xFF;
        strncpy((char*)&outBuffer[4], _stationName.c_str(), 8);
        for (int i = _stationName.length(); i < 8; i++)
        {
            outBuffer[4 + i] = ' ';
        }
        outBuffer[12] = 0x00;
        outBuffer[13] = 0x00;
        outBuffer[14] = 0x01; // music
        outBuffer[15] = 0x00; // not traffic
        outBuffer[16] = 0x00; // not traffic
        outBuffer[17] = 0x00; // # alt frequencies
        PlayListItemRDS::Write(serial, &outBuffer[0], 18);

        outBuffer[1] = 0x1F;
        outBuffer[2] = 0x00;
        memset(&outBuffer[3], 0x20, 64);
        strncpy((char*)&outBuffer[3], _text.c_str(), std::min(64, (int)_text.size()));
        outBuffer[67] = 0x00;
        outBuffer[68] = 0x10;
        outBuffer[69] = 0x00;
        outBuffer[70] = 0x00;
        outBuffer[71] = 0x00;
        outBuffer[72] = 0x00;
        outBuffer[73] = 0x00;
        PlayListItemRDS::Write(serial, &outBuffer[0], 74);

        outBuffer[1] = 0x1F;
        outBuffer[2] = 0x01;
        PlayListItemRDS::Write(serial, &outBuffer[0], 3);

        outBuffer[1] = 0x6E;
        outBuffer[2] = 0x00;
        outBuffer[3] = 0x00;
        PlayListItemRDS::Write(serial, &outBuffer[0], 4);

        outBuffer[1] = 0x76;
        outBuffer[2] = 0x00; // zero length
        outBuffer[3] = 0x20;
        PlayListItemRDS::Write(serial, &outBuffer[0], 4);

        outBuffer[1] = 0x72;
        outBuffer[2] = 0xFF;
        outBuffer[3] = 0x00;
        outBuffer[4] = 0x00;
        outBuffer[5] = 0x00;
        outBuffer[6] = 0x00; // DPS length
        PlayListItemRDS::Write(serial, &outBuffer[0], 7);

        delete serial;

        logger_base.debug("Serial port closed.");
        logger_base.debug("PlayListRDS thread done.");

        return nullptr;
    }
};

PlayListItemRDS::PlayListItemRDS(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _stationName = "";
    _commPort = "COM1";
    _text = "";
    PlayListItemRDS::Load(node);
}

void PlayListItemRDS::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _stationName = node->GetAttribute("StationName", "");
    _commPort = node->GetAttribute("CommPort", "");
    _text = node->GetAttribute("Text", "");
}

PlayListItemRDS::PlayListItemRDS() : PlayListItem()
{
    _started = false;
    _stationName = "";
    _commPort = "COM1";
    _text = "";
}

PlayListItemRDS::~PlayListItemRDS() { }

PlayListItem* PlayListItemRDS::Copy() const
{
    PlayListItemRDS* res = new PlayListItemRDS();
    res->_started = false;
    res->_stationName = _stationName;
    res->_commPort = _commPort;
    res->_text = _text;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemRDS::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIRDS");

    node->AddAttribute("StationName", _stationName);
    node->AddAttribute("Text", _text);
    node->AddAttribute("CommPort", _commPort);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemRDS::GetTitle() const
{
    return "RDS";
}

void PlayListItemRDS::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemRDSPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemRDS::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "RDS";
}

std::string PlayListItemRDS::GetTooltip()
{
    return "Available variables:\n    %STEPNAME% - current playlist step\n    %NEXTSTEPNAME% - next playlist step\n    %TITLE% - from mp3\n    %ARTIST% - from mp3\n    %ALBUM% - from mp3";
}

void PlayListItemRDS::Dump(unsigned char* buffer, int buflen)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString debug = "Serial: ";
    for (int i = 0; i < buflen; i++)
    {
        debug += wxString::Format("0x%02X ", (uint8_t)buffer[i]);
    }
    logger_base.debug("%s", (const char*)debug.c_str());
}

void PlayListItemRDS::Write(SerialPort* serial, unsigned char* buffer, int buflen)
{
    Dump(buffer, buflen);
    SendWithDTRCTS(serial, (char*)buffer, buflen);
}

// Note on windows wxMicrosleep actually calls ::Sleep with 1ms smallest value
#define I2CUNIT 1000

void PlayListItemRDS::SendBitWithDTRCTS(SerialPort* serial, bool bit, int hold)
{
    // Drop CTS
    serial->SetRTS(false);

    // Set DTS to the bit value
    serial->SetDTR(bit);
    wxMicroSleep(I2CUNIT * hold);
    // Now lift CTS
    serial->SetRTS(true);
    wxMicroSleep(I2CUNIT);
}

void PlayListItemRDS::InitialiseDTRCTS(SerialPort* serial)
{
    // Set DTR and CTS in prep for transfer
    serial->SetRTS(true);
    serial->SetDTR(true);
}

int PlayListItemRDS::SendWithDTRCTS(SerialPort* serial, char* buf, size_t len)
{
    wxMicroSleep(I2CUNIT * 5);

    // Start bit
    // Clear DTR then CTS
    serial->SetDTR(false);
    wxMicroSleep(I2CUNIT);
    serial->SetRTS(false);
    wxMicroSleep(I2CUNIT);

    for (int i = 0; i < len; i++)
    {
        uint8_t mask = 0x80;

        for (int j = 0; j < 8; j++)
        {
            bool bit = (buf[i] & mask) != 0;
            SendBitWithDTRCTS(serial, bit); /// start

            mask = mask >> 1;
        }
        SendBitWithDTRCTS(serial, true, 2); // byte stop bit
    }

    // stop bit
    serial->SetRTS(false);
    serial->SetDTR(false);
    wxMicroSleep(I2CUNIT);
    serial->SetRTS(true);
    wxMicroSleep(I2CUNIT);
    serial->SetDTR(true);

    return len;
}

void PlayListItemRDS::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (ms >= _delay && !_started)
    {
        logger_base.warn("Kicking off RDS thread.");

        _started = true;

        wxString text = wxString(_text);

        auto playlist = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
        PlayListStep* step = nullptr;

        if (playlist != nullptr && !playlist->IsRandom())
        {
            step = playlist->GetRunningStep();
            bool dummy;
            auto nextstep = playlist->GetNextStep(dummy);
            if (nextstep != nullptr)
            {
                text.Replace("%NEXTSTEPNAME%", nextstep->GetNameNoTime());
            }
        }
        text.Replace("%NEXTSTEPNAME%", "");

        if (step == nullptr)
        {
            step = xScheduleFrame::GetScheduleManager()->GetStepContainingPlayListItem(GetId());
        }

        wxString stationName = wxString(_stationName);

        if (step != nullptr)
        {
            text.Replace("%STEPNAME%", step->GetNameNoTime());

            AudioManager* audio = step->GetAudioManager();
            if (audio != nullptr)
            {
                text.Replace("%TITLE%", audio->Title());
                text.Replace("%ARTIST%", audio->Artist());
                text.Replace("%ALBUM%", audio->Album());
            }
        }

        EDMRDSThread* thread = new EDMRDSThread(text, stationName, _commPort);
        thread->Run();
        wxMicroSleep(1); // encourage the thread to run
    }
}

void PlayListItemRDS::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}