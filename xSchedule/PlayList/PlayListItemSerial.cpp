#include "PlayListItemSerial.h"
#include "PlayListItemSerialPanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "PlayList.h"
#include "PlayListStep.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include "../RunningSchedule.h"
#include "../xLights/outputs/serial.h"
#include "../../xLights/outputs/SerialOutput.h"

PlayListItemSerial::PlayListItemSerial(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _commPort = "COM1";
    _configuration = "8N1";
	_speed = 19200;
    _data = "";
    PlayListItemSerial::Load(node);
}

void PlayListItemSerial::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _commPort = node->GetAttribute("CommPort", "COM1");
    _configuration = node->GetAttribute("Configuration", "8N1");
    _speed = wxAtoi(node->GetAttribute("Speed", "19200"));
    _data = node->GetAttribute("Data", "");
}

PlayListItemSerial::PlayListItemSerial() : PlayListItem()
{
    _started = false;
    _commPort = "COM1";
    _configuration = "8N1";
    _speed = 19200;
    _data = "";
}

PlayListItem* PlayListItemSerial::Copy() const
{
    PlayListItemSerial* res = new PlayListItemSerial();
    res->_commPort = _commPort;
    res->_configuration = _configuration;
    res->_speed = _speed;
    res->_data = _data;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemSerial::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLISERIAL");

    node->AddAttribute("CommPort", _commPort);
    node->AddAttribute("Configuration", _configuration);
    node->AddAttribute("Speed", wxString::Format("%d", _speed));
    node->AddAttribute("Data", _data);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemSerial::GetTitle() const
{
    return "Serial";
}

void PlayListItemSerial::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemSerialPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemSerial::GetNameNoTime() const
{
    if (_name != "") return _name;

    return _commPort;
}

std::string PlayListItemSerial::GetTooltip()
{
    return "Use \\xAA to enter binary values where AA is a hexadecimal value.\n\nAvailable variables:\n    %RUNNING_PLAYLIST% - current playlist\n    %RUNNING_PLAYLISTSTEP% - step name\n    %RUNNING_PLAYLISTSTEPMS% - Position in current step\n    %RUNNING_PLAYLISTSTEPMSLEFT% - Time left in current step\n    %RUNNING_SCHEDULE% - Name of schedule";
}

bool isHexChar(char c)
{
    return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}

char HexToChar(char c)
{
    if (c >= '0' && c <= '9')
    {
        return (int)c - 0x30;
    }
    if (c >= 'A' && c <= 'F')
    {
        return (int)c - 65 + 10;
    }
    return (int)c - 97 + 10;
}

char HexToChar(char c1, char c2)
{
    return (HexToChar(c1) << 8) + HexToChar(c2);
}

unsigned char* PlayListItemSerial::PrepareData(const std::string s, int& used)
{
	wxString working(s);

	PlayList* pl = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
	if (pl != nullptr)
	{
		if (working.Contains("%RUNNING_PLAYLIST%"))
		{
			working.Replace("%RUNNING_PLAYLIST%", pl->GetNameNoTime(), true);
		}
		PlayListStep* pls = pl->GetRunningStep();
		if (pls != nullptr)
		{
			if (working.Contains("%RUNNING_PLAYLISTSTEP%"))
			{
				working.Replace("%RUNNING_PLAYLISTSTEP%", pls->GetNameNoTime(), true);
			}
			if (working.Contains("%RUNNING_PLAYLISTSTEPMS%"))
			{
				working.Replace("%RUNNING_PLAYLISTSTEPMS%", wxString::Format(wxT("%i"), pls->GetLengthMS()), true);
			}
			if (working.Contains("%RUNNING_PLAYLISTSTEPMSLEFT%"))
			{
				working.Replace("%RUNNING_PLAYLISTSTEPMSLEFT%", wxString::Format(wxT("%i"), pls->GetLengthMS() - pls->GetPosition()), true);
			}
		}
	}
	if (working.Contains("%RUNNING_SCHEDULE%"))
	{
		RunningSchedule* rs = xScheduleFrame::GetScheduleManager()->GetRunningSchedule();
		if (rs != nullptr && rs->GetPlayList()->IsRunning())
		{
			working.Replace("%RUNNING_SCHEDULE%", rs->GetSchedule()->GetName(), true);
		}
	}

	unsigned char* buffer = (unsigned char*)malloc(working.size());
	used = 0;
	
	for (int i = 0; i < working.size(); i++)
	{
		if (working[i] == '\\')
		{
			if (i+1 < working.size())
			{
				if (working[i+1] == '\\')
				{
					buffer[used++] = working[i];
					i++; // skip the second '\\' 
				}
				if (working[i+1] == 'x' || working[i+1] == 'X')
				{
					// up to next 2 characters if 0-F will be treated as a hex code
					i++;
					i++;
					if (i+1 < working.size() && isHexChar(working[i]) && isHexChar(working[i+1]))
					{
						buffer[used++] = (char)HexToChar(working[i], working[i+1]);
						i++;
					}
					else if (i < working.size() && isHexChar(working[i]))
					{
						buffer[used++] = (char)HexToChar(working[i]);
					}
					else
					{
						// \x was not followed by a hex digit so put in \x
						buffer[used++] = '\\';
						buffer[used++] = 'x';
						i--;
					}
				}
			}
			else
			{
				buffer[used++] = working[i];
			}
		}
		else
		{
			buffer[used] = working[i];
		}
	}

	unsigned char* res = (unsigned char*)malloc(used);
	memcpy(res, buffer, used);
	free(buffer);
	return res;
}

void PlayListItemSerial::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (ms >= _delay && !_started)
    {
        _started = true;

		int dbuffsize;
		unsigned char* dbuffer = PrepareData(_data, dbuffsize);

        if (dbuffer != nullptr)
        {
            logger_base.info("Sending serial %s.", (const char *)_data.c_str());

            if (_commPort == "NotConnected")
            {
                logger_base.warn("Serial port %s not opened for %s as it is tagged as not connected.", (const char *)_commPort.c_str(), (const char *)GetNameNoTime().c_str());
                // dont set ok to false ... while this is not really open it is not an error as the user meant it to be not connected.
            }
            else
            {
                SerialPort* serial = new SerialPort();

                logger_base.debug("Opening serial port %s. Baud rate = %d. Config = %s.", (const char *)_commPort.c_str(), _speed, (const char *)_configuration.c_str());

                int errcode = serial->Open(_commPort, _speed, _configuration.c_str());
                if (errcode < 0)
                {
                    delete serial;
                    serial = nullptr;

                    logger_base.warn("Unable to open serial port %s. Error code = %d", (const char *)_commPort.c_str(), errcode);

                    std::string p = "";
                    auto ports = SerialOutput::GetAvailableSerialPorts();
                    for (auto it = ports.begin(); it != ports.end(); ++it)
                    {
                        if (p != "") p += ", ";
                        p += *it;
                    }

                    wxString msg = wxString::Format(_("Error occurred while connecting to %s (Available Ports %s) \n\n") +
                        _("Things to check:\n") +
                        _("1. Are all required cables plugged in?\n") +
                        _("2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.\n") +
                        _("3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n") +
                        _("Unable to open serial port %s. Error code = %d"),
                        (const char *)GetCommPort().c_str(),
                        (const char *)p.c_str(),
                        (const char *)_commPort.c_str(),
                        errcode);
                    //wxMessageBox(msg, _("Communication Error"), wxOK);
                }
                else
                {
                    logger_base.debug("    Serial port %s open.", (const char *)_commPort.c_str());
                }

                if (serial != nullptr && serial->IsOpen())
                {
                    int written = serial->Write((char *)dbuffer, dbuffsize);

                    int i = 0;
                    while (serial->WaitingToWrite() != 0 && (i < 10))
                    {
                        wxMilliSleep(5);
                        i++;
                    }
                    serial->Close();
                    delete serial;
                    serial = nullptr;
                    logger_base.debug("    Serial port %s closed.", (const char *)_commPort.c_str());
                }
            }

            delete dbuffer;
        }
    }
}

void PlayListItemSerial::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
