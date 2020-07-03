/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventMIDI.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "../ScheduleManager.h"
#include "../wxMIDI/src/wxMidi.h"

void EventMIDI::DoSetStatus(std::string status)
{
    _status = status;
    if (_status == "0x8n - Note Off")
    {
        _statusByte = 0x80;
    }
    else if (_status == "0x9n - Note On")
    {
        _statusByte = 0x90;
    }
    else if (_status == "0xAn - Polyphonic Key Pressure")
    {
        _statusByte = 0xA0;
    }
    else if (_status == "0xBn - Control Change")
    {
        _statusByte = 0xB0;
    }
    else if (_status == "0xCn - Program Change")
    {
        _statusByte = 0xC0;
    }
    else if (_status == "0xDn - Channel Pressure")
    {
        _statusByte = 0xD0;
    }
    else if (_status == "0xEn - Pitch Bend")
    {
        _statusByte = 0xE0;
    }
    else
    {
        _statusByte = -1;
    }
}

void EventMIDI::DoSetChannel(std::string channel)
{
    _channel = channel;
    if (_channel == "ANY")
    {
        _channelByte = -1;
    }
    else
    {
        _channelByte = wxHexToDec(channel.substr(2));
    }
}

bool EventMIDI::IsDataMatch(uint8_t value, const std::string& testData, int data, uint8_t lastValue)
{
    if (testData == "Any") return true;

    if (testData == "Equals") return data == value;

    if (testData == "Not Equals") return data != value;

    if (testData == "Greater Than") return value > data;

    if (testData == "Less Than") return value < data;

    if (testData == "Less Than or Equals") return value <= data;

    if (testData == "Greater Than or Equals") return value >= data;

    if (testData == "On Change") return value != lastValue;

    return false;
}

EventMIDI::EventMIDI() : EventBase()
{
    _device = "";
    _status = "0x9n - Note On";
    _statusByte = 0x90;
    _channel = "ANY";
    _testData1 = "Any";
    _testData2 = "Any";
    _data1 = 0;
    _data2 = 0;
    _channelByte = -1;
}

EventMIDI::EventMIDI(wxXmlNode* node) : EventBase(node)
{
    _device = node->GetAttribute("Device", "");
    DoSetStatus(node->GetAttribute("Status", "0x9n - Note On").ToStdString());
    DoSetChannel(node->GetAttribute("Channel", "ANY").ToStdString());

    auto oldd1 = node->GetAttribute("Data1", "XYZZY");
    auto oldd2 = node->GetAttribute("Data2", "XYZZY");
    if (oldd1 != "XYZZY")
    {
        if (oldd1 == "ANY")
        {
            _testData1 = "Any";
            _data1 = 0;
        }
        else if (oldd1 == "Not 0x00")
        {
            _testData1 = "Not Equals";
            _data1 = 0;
        }
        else
        {
            _testData1 = "Equals";
            _data1 = wxHexToDec(oldd1.substr(2));
        }

        if (oldd2 == "ANY")
        {
            _testData2 = "Any";
            _data2 = 0;
        }
        else if (oldd2 == "Not 0x00")
        {
            _testData2 = "Not Equals";
            _data2 = 0;
        }
        else
        {
            _testData2 = "Equals";
            _data2 = wxHexToDec(oldd2.substr(2));
        }
    }
    else
    {
        _testData1 = node->GetAttribute("TestData1", "Any");
        _testData2 = node->GetAttribute("TestData2", "Any");
        _data1 = wxAtoi(node->GetAttribute("NewData1", "0"));
        _data2 = wxAtoi(node->GetAttribute("NewData2", "0"));
    }
}

wxXmlNode* EventMIDI::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventMIDI");
    en->AddAttribute("Device", _device);
    en->AddAttribute("Status", _status);
    en->AddAttribute("Channel", _channel);
    en->AddAttribute("NewData1", wxString::Format("%d", _data1));
    en->AddAttribute("NewData2", wxString::Format("%d", _data2));
    en->AddAttribute("TestData1", _testData1);
    en->AddAttribute("TestData2", _testData2);
    EventBase::Save(en);
    return en;
}

void EventMIDI::Process(uint8_t status, uint8_t channel, uint8_t data1, uint8_t data2, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (GetStatusByte() == status)
    {
        if (IsAnyChannel() || channel == GetChannelByte())
        {
            if (IsDataMatch(data1, _testData1, _data1, _lastData1) && IsDataMatch(data2, _testData2, _data2, _lastData2))
            {
                _lastData1 = data1;
                _lastData2 = data2;
                int st = status;
                logger_base.debug("Event fired %s:%s -> %02x", (const char*)GetType().c_str(), (const char*)GetName().c_str(), st);
                ProcessMIDICommand(data1, data2, scheduleManager);
                logger_base.debug("    Event processed.");
            }
        }
    }
}

void EventMIDI::ProcessMIDICommand(uint8_t data1, uint8_t data2, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString p1 = _parm1;
    wxString p2 = _parm2;
    wxString p3 = _parm3;

    int d1100 = data1 * 100 / 127;
    int d1255 = data1 * 255 / 127;
    int d2100 = data2 * 100 / 127;
    int d2255 = data2 * 255 / 127;

    p1.Replace("%DATA1%", wxString::Format("%d", (int)data1));
    p2.Replace("%DATA1%", wxString::Format("%d", (int)data1));
    p3.Replace("%DATA1%", wxString::Format("%d", (int)data1));
    p1.Replace("%DATA2%", wxString::Format("%d", (int)data2));
    p2.Replace("%DATA2%", wxString::Format("%d", (int)data2));
    p3.Replace("%DATA2%", wxString::Format("%d", (int)data2));
    // Scale value from 0-255 to 0-100
    p1.Replace("%DATA1100%", wxString::Format("%d", d1100));
    p2.Replace("%DATA1100%", wxString::Format("%d", d1100));
    p3.Replace("%DATA2100%", wxString::Format("%d", d1100));
    p1.Replace("%DATA2100%", wxString::Format("%d", d2100));
    p2.Replace("%DATA2100%", wxString::Format("%d", d2100));
    p3.Replace("%DATA2100%", wxString::Format("%d", d2100));
    // Scale value from 0-100 to 0-255
    p1.Replace("%DATA1255%", wxString::Format("%d", d1255));
    p2.Replace("%DATA1255%", wxString::Format("%d", d1255));
    p3.Replace("%DATA1255%", wxString::Format("%d", d1255));
    p1.Replace("%DATA2255%", wxString::Format("%d", d2255));
    p2.Replace("%DATA2255%", wxString::Format("%d", d2255));
    p3.Replace("%DATA2255%", wxString::Format("%d", d2255));

    std::string parameters = p1.ToStdString();
    if (p2 != "") parameters += "," + p2.ToStdString();
    if (p3 != "") parameters += "," + p3.ToStdString();

    logger_base.debug("Event fired %s:%s -> %s:%s", (const char *)GetType().c_str(), (const char *)GetName().c_str(),
        (const char *)_command.c_str(), (const char *)parameters.c_str());

    size_t rate = 0;
    wxString msg;
    scheduleManager->Action(_command, parameters, "", nullptr, nullptr, nullptr, rate, msg);
    logger_base.debug("    Event processed.");
}

std::list<std::string> EventMIDI::GetDevices()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Input MIDI Devices:");

    std::list<std::string> res;

    wxMidiSystem* midiSystem = wxMidiSystem::GetInstance();
    int devices = midiSystem->CountDevices();
    for (int i = 0; i < devices; i++)
    {
        wxMidiInDevice* midiDev = new wxMidiInDevice(i);
        if (midiDev->IsInputPort()) 
        {
            auto devname = wxString::Format("Input %s [%s] %d", midiDev->DeviceName(), midiDev->InterfaceUsed(), i).ToStdString();
            res.push_back(devname);
            logger_base.debug("    %s", (const char*)devname.c_str());
        }
        delete midiDev;
    }

    return res;
}

std::list<std::string> EventMIDI::GetOutputDevices()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Output MIDI Devices:");

    std::list<std::string> res;

    wxMidiSystem* midiSystem = wxMidiSystem::GetInstance();
    int devices = midiSystem->CountDevices();
    for (int i = 0; i < devices; i++)
    {
        wxMidiInDevice* midiDev = new wxMidiInDevice(i);
        if (midiDev->IsOutputPort())
        {
            auto devname = wxString::Format("Output %s [%s] %d", midiDev->DeviceName(), midiDev->InterfaceUsed(), i).ToStdString();
            res.push_back(devname);
            logger_base.debug("    %s", (const char*)devname.c_str());
        }
        delete midiDev;
    }

    return res;
}

int EventMIDI::GetDeviceId() const
{
    if (_device == "") return -1;

    return wxAtoi(wxString(_device).AfterLast(' '));
}

std::string EventMIDI::GetParmToolTip()
{
    return "Available:\n\n   %DATA1% - Data 1 value\n   %DATA2% - Data 2 vale\n   %DATAx100% - Scale data x value from 0-127 to 0-100\n   %DATAx255% - Scale data x value from 0-127 to 0-255\n";
}
