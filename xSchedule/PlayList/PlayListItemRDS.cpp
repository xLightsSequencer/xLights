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

PlayListItemRDS::PlayListItemRDS(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _highSpeed = false;
    _stationDuration = 0;
    _stationName = "";
    _commPort = "COM1";
    _lineDuration = 2;
    _mode = 1;
    _serialSpeed = 19200;
    _text = "";
    PlayListItemRDS::Load(node);
}

void PlayListItemRDS::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _stationDuration = wxAtoi(node->GetAttribute("StationDuration", "0"));
    _lineDuration = wxAtoi(node->GetAttribute("LineDuration", "2"));
    _mode = wxAtoi(node->GetAttribute("Mode", "1"));
    _serialSpeed = wxAtoi(node->GetAttribute("SerialSpeed", "19200"));
    _stationName = node->GetAttribute("StationName", "");
    _commPort = node->GetAttribute("CommPort", "");
    _text = node->GetAttribute("Text", "");
    _highSpeed = (node->GetAttribute("HighSpeed", "FALSE") == "TRUE");
}

PlayListItemRDS::PlayListItemRDS() : PlayListItem()
{
    _started = false;
    _highSpeed = false;
    _stationDuration = 0;
    _stationName = "";
    _commPort = "COM1";
    _lineDuration = 2;
    _mode = 1;
    _serialSpeed = 19200;
    _text = "";
}

PlayListItem* PlayListItemRDS::Copy() const
{
    PlayListItemRDS* res = new PlayListItemRDS();
    res->_started = false;
    res->_highSpeed = _highSpeed;
    res->_stationDuration = _stationDuration;
    res->_stationName = _stationName;
    res->_commPort = _commPort;
    res->_lineDuration = _lineDuration;
    res->_mode = _mode;
    res->_serialSpeed = _serialSpeed;
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
    if (_highSpeed)
    {
        node->AddAttribute("HighSpeed", "TRUE");
    }
    node->AddAttribute("StationDuration", wxString::Format(wxT("%i"), _stationDuration));
    node->AddAttribute("LineDuration", wxString::Format(wxT("%i"), _lineDuration));
    node->AddAttribute("Mode", wxString::Format(wxT("%i"), _mode));
    node->AddAttribute("SerialSpeed", wxString::Format(wxT("%i"), _serialSpeed));

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
    return "Available variables:\n    %STEPNAME% - current playlist step\n    %TITLE% - from mp3\n    %ARTIST% - from mp3\n    %ALBUM% - from mp3";
}

void PlayListItemRDS::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        auto step = xScheduleFrame::GetScheduleManager()->GetRunningPlayList()->GetRunningStep();
        
        if (step == nullptr)
        {
            step = xScheduleFrame::GetScheduleManager()->GetStepContainingPlayListItem(GetId());
        }

        wxString text = wxString(_text);
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

        logger_base.info("RDS: PS '%s' DPS '%s'.", (const char *)stationName.c_str(), (const char *)text.c_str());

        if (_commPort == "")
        {
            logger_base.warn("RDS: No comm port specified.");
            return;
        }

        auto serial = new SerialPort();

        char serialConfig[4];
        strcpy(serialConfig, "8N1");
        int errcode = serial->Open(_commPort, _serialSpeed, serialConfig);
        if (errcode < 0)
        {
            logger_base.warn("RDS: Unable to open serial port %s. Error code = %d", (const char *)_commPort.c_str(), errcode);
            delete serial;
            return;
        }

        char outBuffer[100];

        // 
        outBuffer[0] = (wxByte)0xFE;
        outBuffer[1] = (wxByte)0x02;
        strncpy(&outBuffer[2], stationName.c_str(), 8);
        for (int i = stationName.length(); i < 8; i++)
        {
            outBuffer[2 + i] = ' ';
        }
        outBuffer[10] = (wxByte)0xFF;
        serial->Write(&outBuffer[0], 11);

        outBuffer[1] = (wxByte)0x0C;
        outBuffer[2] = (wxByte)1;
        outBuffer[3] = (wxByte)0xFF;
        serial->Write(&outBuffer[0], 4);

        outBuffer[1] = (wxByte)0x72;
        outBuffer[2] = _stationDuration;
        serial->Write(&outBuffer[0], 4);

        outBuffer[1] = (wxByte)0x73;
        outBuffer[2] = _mode;
        serial->Write(&outBuffer[0], 4);

        outBuffer[1] = (wxByte)0x74;
        outBuffer[2] = (wxByte)_lineDuration;
        serial->Write(&outBuffer[0], 4);

        outBuffer[1] = (wxByte)0x75;
        outBuffer[2] = (wxByte)(_highSpeed ? 0: 1);
        serial->Write(&outBuffer[0], 4);

        outBuffer[1] = (wxByte)0x76;
        outBuffer[2] = (wxByte)0;
        serial->Write(&outBuffer[0], 4);

        outBuffer[1] = (wxByte)0x77;
        strncpy(&outBuffer[1], _text.c_str(), 80);
        for (int i = text.length(); i < 80; i++)
        {
            outBuffer[2 + i] = ' ';
        }
        outBuffer[82] = (wxByte)0xFF;
        serial->Write(&outBuffer[0], 83);

        outBuffer[1] = (wxByte)0x76;
        outBuffer[2] = (wxByte)text.length();
        outBuffer[3] = (wxByte)0xFF;
        serial->Write(&outBuffer[0], 4);

        delete serial;
    }
}

void PlayListItemRDS::Start()
{
    _started = false;
}

std::string PlayListItemRDS::GetMode() const
{
    switch(_mode)
    {
    case 0:
        return "Mode 0 - Fixed 8 Characters";
    case 1:
        return "Mode 1 - Scroll 1 Character";
    case 2:
        return "Mode 2 - Scroll Word Aligned";
    case 3:
        return "Mode 3 - Scroll 1 Character Leading Spaces";
    }

    return "";
}

void PlayListItemRDS::SetMode(const std::string& mode)
{
   if (mode == "Mode 0 - Fixed 8 Characters")
   {
       _mode = 0;
   }
   else if (mode == "Mode 1 - Scroll 1 Character")
   {
       _mode = 1;
   }
   else if (mode == "Mode 2 - Scroll Word Aligned")
   {
       _mode = 2;
   }
   else if (mode == "Mode 3 - Scroll 1 Character Leading Spaces")
   {
       _mode = 3;
   }
}



