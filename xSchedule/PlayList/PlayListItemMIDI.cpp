#include <wx/xml/xml.h>
#include <wx/notebook.h>

#include "PlayListItemMIDI.h"
#include "PlayListItemMIDIPanel.h"
#include "../../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>
#include "../wxMIDI/src/wxMidi.h"

PlayListItemMIDI::PlayListItemMIDI(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _device = "";
    _status = "0x9n - Note On";
    _channel = "0x00";
    _data1 = "0x00";
    _data2 = "0x00";
    PlayListItemMIDI::Load(node);
}

void PlayListItemMIDI::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _device = node->GetAttribute("Device", "");
    _status = node->GetAttribute("Status", "0x9n - Note On");
    _channel = node->GetAttribute("Channel", "0x00");
    _data1 = node->GetAttribute("Data1", "0x00");
    _data2 = node->GetAttribute("Data2", "0x00");
}

PlayListItemMIDI::PlayListItemMIDI() : PlayListItem()
{
    _type = "PLIMIDI";
    _started = false;
    _device = "";
    _status = "0x9n - Note On";
    _channel = "0x00";
    _data1 = "0x00";
    _data2 = "0x00";
}

PlayListItem* PlayListItemMIDI::Copy() const
{
    PlayListItemMIDI* res = new PlayListItemMIDI();
    res->_device = _device;
    res->_status = _status;
    res->_channel = _channel;
    res->_data1 = _data1;
    res->_data2 = _data2;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemMIDI::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Device", _device);
    node->AddAttribute("Status", _status);
    node->AddAttribute("Channel", _channel);
    node->AddAttribute("Data1", _data1);
    node->AddAttribute("Data2", _data2);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemMIDI::GetTitle() const
{
    return "MIDI";
}

void PlayListItemMIDI::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemMIDIPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemMIDI::GetNameNoTime() const
{
    if (_name != "") return _name;

    return _device;
}

void PlayListItemMIDI::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (ms >= _delay && !_started)
    {
        _started = true;

        auto midi = new wxMidiOutDevice(wxAtoi(wxString(_device).AfterLast(' ')));
        if (midi->IsOutputPort())
        {
            wxMidiError err = midi->Open(0);
            if (err == wxMIDI_NO_ERROR)
            {
                int data1 = wxHexToDec(_data1.substr(2));
                int data2 = wxHexToDec(_data2.substr(2));
                int channel = wxHexToDec(_channel.substr(2));
                wxString s = _status.substr(2, 1) + "0";
                int status = wxHexToDec(s);
                wxMidiShortMessage msg(status + channel, data1, data2);
                msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
                logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
                midi->Write(&msg);
            }
            else
            {
                logger_base.error("PlayListItemMIDI failed to open MIDI device %s : %d", (const char*)_device.c_str(), err);
            }
        }
        midi->Close();
        delete midi;
    }
}

void PlayListItemMIDI::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}

std::list<std::string> PlayListItemMIDI::GetDevices()
{
    std::list<std::string> res;

    wxMidiSystem* midiSystem = wxMidiSystem::GetInstance();
    int devices = midiSystem->CountDevices();
    for (int i = 0; i < devices; i++)
    {
        wxMidiOutDevice* midiDev = new wxMidiOutDevice(i);
        if (midiDev->IsOutputPort())
        {
            res.push_back(wxString::Format("Output %s [%s] %d", midiDev->DeviceName(), midiDev->InterfaceUsed(), i).ToStdString());
        }
        delete midiDev;
    }

    return res;
}