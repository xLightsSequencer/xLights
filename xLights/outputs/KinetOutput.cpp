
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>

#include "KinetOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "ControllerEthernet.h"
#include "../utils/ip_utils.h"

#include <log4cpp/Category.hh>

#pragma region Static Variables
#pragma endregion

#pragma region Private Functions
void KinetOutput::OpenDatagram() {

    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_datagram != nullptr) return;

    wxIPV4address localaddr;
    if (GetForceLocalIPToUse() == "") {
        localaddr.AnyAddress();
    }
    else {
        localaddr.Hostname(GetForceLocalIPToUse());
    }

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_BLOCK); // dont use NOWAIT as it can result in dropped packets
    if (_datagram == nullptr) {
        logger_base.error("Error initialising Kinet datagram for %s %d. %s", (const char*)_ip.c_str(), GetUniverse(), (const char*)localaddr.IPAddress().c_str());
        _ok = false;
    }
    else if (!_datagram->IsOk()) {
        logger_base.error("Error initialising Kinet datagram for %s %d. %s OK : FALSE", (const char*)_ip.c_str(), GetUniverse(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
    else if (_datagram->Error()) {
        logger_base.error("Error creating Kinet datagram => %d : %s. %s", _datagram->LastError(), (const char*)DecodeIPError(_datagram->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
}

void KinetOutput::PopulateHeader()
{
    memset(_data, 0x00, sizeof(_data));

    _data[0] = 0x04;   // magic[4] 0401dc4a
    _data[1] = 0x01;
    _data[2] = 0xdc;
    _data[3] = 0x4a;
	
    _data[4] = 0x01; // ver [2] 0100
    _data[5] = 0x00;
	
	if (_version == 1)
	{
		_data[6] = 0x01;  // type [2] 
	}
	else 
	{
		_data[6] = 0x08; 
	}
	_data[7] = 0x01;
	
	_data[8] = 0x00; // seq [4]
    _data[9] = 0x00;
    _data[10] = 0x00;
    _data[11] = 0x00;

	if (_version == 1)
	{	
		_data[12] = (_universe - 1) & 0xFF; // port
		_data[13] = 0x00; // padding

		_data[14] = 0x00; // flags[2]
		_data[15] = 0x00; 
		
		_data[16] = 0xFF; // time [4]
		_data[17] = 0xFF;
		_data[18] = 0xFF;
		_data[19] = 0xFF;

		_data[20] = 0x00; // uni
	}
	else
	{
		_data[12] = 0xFF; // time [4]
		_data[13] = 0xFF;
		_data[14] = 0xFF;
		_data[15] = 0xFF;
		
		_data[16] = (_universe - 1) & 0xFF; // port
		_data[17] = _sequenceNum; // ?

		_data[18] = 0x00;
		_data[19] = 0x00;
		_data[20] = 0x00;
		_data[21] = 0x02;
		_data[22] = 0x00;
		_data[23] = 0x00;
	}
}
#pragma endregion

#pragma region Constructors and Destructors
KinetOutput::KinetOutput(wxXmlNode* node, bool isActive) : IPOutput(node, isActive) {

    if (_channels > 512) SetChannels(512);
    _sequenceNum = 0;
    _datagram = nullptr;
    _version = wxAtoi(node->GetAttribute("Version", "2"));
    memset(_data, 0, sizeof(_data));
}

KinetOutput::KinetOutput() : IPOutput() {

    _channels = 512;
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

KinetOutput::KinetOutput(const KinetOutput& from) :
    IPOutput(from)
{
    _channels = from._channels;
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

KinetOutput::~KinetOutput() {
    if (_datagram != nullptr) delete _datagram;
}

wxXmlNode* KinetOutput::Save()
{

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    IPOutput::Save(node);

    node->AddAttribute("Version", wxString::Format(wxT("%i"), _version));

    return node;
}
#pragma endregion

#pragma region Static Functions
#pragma endregion

#pragma region Getters and Setters
std::string KinetOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "Kinet {" + GetUniverseString() + "} ";
    res += "[1-" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";

    return res;
}

std::string KinetOutput::GetExport() const {

    return wxString::Format(",%ld,%ld,,%s,%s,,,,%d,%i",
        GetStartChannel(),
        GetEndChannel(),
        GetType(),
        GetIP(),
        GetUniverse(),
        GetChannels());
}
#pragma endregion

#pragma region Start and Stop
bool KinetOutput::Open() {
    if (!_enabled) return true;
    if (!ip_utils::IsIPValid(_resolvedIp)) return false;

    _ok = IPOutput::Open();

	PopulateHeader();

    _sequenceNum = 0;

    OpenDatagram();

    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(KINET_PORT);

    return _ok;
}

void KinetOutput::Close() {

    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

#pragma region Frame Handling
void KinetOutput::StartFrame(long msec) {

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return;

    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            logger_base.debug("KinetOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void KinetOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _tempDisable|| _datagram == nullptr) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        _data[8] = _sequenceNum & 0xFF;
        _data[9] = (_sequenceNum >> 8) & 0xFF;
        _data[10] = (_sequenceNum >> 16) & 0xFF;
        _data[11] = (_sequenceNum >> 24) & 0xFF;
        _datagram->SendTo(_remoteAddr, _data, GetHeaderPacketLength() + _channels);
        _sequenceNum++;
        FrameOutput();
        _changed = false;
    }
    else {
        SkipFrame();
    }
}
#pragma endregion

#pragma region Data Setting
void KinetOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    wxASSERT(channel < _channels);

    if (_data[channel + GetHeaderPacketLength()] != data) {
        _data[channel + GetHeaderPacketLength()] = data;
        _changed = true;
    }
}

void KinetOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (!_enabled) return;
    wxASSERT(channel + size <= _channels);

    size_t chs = (std::min)((int32_t)size, _channels - channel);

    if (memcmp(&_data[channel + GetHeaderPacketLength()], data, chs) == 0) {
        // nothing has changed
    }
    else {
        memcpy(&_data[channel + GetHeaderPacketLength()], data, chs);
        _changed = true;
    }
}

void KinetOutput::AllOff() {

    if (!_enabled) return;
    memset(&_data[GetHeaderPacketLength()], 0x00, _channels);
    _changed = true;
}
#pragma endregion




#pragma region UI
#ifndef EXCLUDENETWORKUI
#include "../models/ModelManager.h"
void KinetOutput::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::UpdateProperties(propertyGrid, c, modelManager, expandProperties);
    ControllerEthernet *ce = dynamic_cast<ControllerEthernet*>(c);
    auto p = propertyGrid->GetProperty("Version");
    if (p) {
        p->SetValue(GetVersion());
    }
    
    p = propertyGrid->GetProperty("Universes");
    if (p) {
        p->SetValue((int)c->GetOutputs().size());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Universes Count cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPG_PROP_READONLY, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    p = propertyGrid->GetProperty("UniversesDisplay");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(ce->GetOutputs().front()->GetUniverseString() + " - " + ce->GetOutputs().back()->GetUniverseString());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IndivSizes");
    if (p) {
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Individual Sizes cannot be changed when an output is set to Auto Size.");
        } else {
            bool v = !ce->AllSameSize() || ce->IsForcingSizes() || ce->IsUniversePerString();
            p->SetValue(v);
            p->ChangeFlag(wxPG_PROP_READONLY, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    if (!ce->AllSameSize() || ce->IsForcingSizes()) {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(true);
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(false);
            if (ce->IsExpanded()) {
                expandProperties.push_back(p2);
            }
            while (propertyGrid->GetFirstChild(p2)) {
                propertyGrid->RemoveProperty(propertyGrid->GetFirstChild(p2));
            }
            for (const auto& it : ce->GetOutputs()) {
                p = propertyGrid->AppendIn(p2, new wxUIntProperty(it->GetUniverseString(), "Channels/" + it->GetUniverseString(), it->GetChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");
                auto modelsOnUniverse = modelManager->GetModelsOnChannels(it->GetStartChannel(), it->GetEndChannel(), 4);
                p->SetHelpString(wxString::Format("[%d-%d]\n", it->GetStartChannel(), it->GetEndChannel()) + modelsOnUniverse);
                if (modelsOnUniverse != "") {
                    if (IsDarkMode()) {
                        p->SetBackgroundColour(wxColour(104, 128, 79));
                    } else {
                        p->SetBackgroundColour(wxColour(208, 255, 158));
                    }
                }
            }
        }
    } else {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(false);
            p->SetValue(GetChannels());
            if (ce->IsAutoSize()) {
                p->ChangeFlag(wxPG_PROP_READONLY, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
            } else {
                p->SetEditor("SpinCtrl");
                p->ChangeFlag(wxPG_PROP_READONLY, false);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                p->SetHelpString("");
            }
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(true);
        }
    }

}
void KinetOutput::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty *before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::AddProperties(propertyGrid, before, c, allSameSize, expandProperties);
    auto p = propertyGrid->Insert(before, new wxUIntProperty("Version", "Version", 1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 2);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("Kinet protocol version.");
    
        
    p = propertyGrid->Insert(before, new wxUIntProperty("Start Port", "Universe", GetUniverse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 255);
    p->SetEditor("SpinCtrl");
    
    
    p = propertyGrid->Insert(before, new wxUIntProperty("Port Count", "Universes", c->GetOutputs().size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);

    p = propertyGrid->Insert(before, new wxStringProperty("Ports", "UniversesDisplay", ""));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));


    p = propertyGrid->Insert(before, new wxBoolProperty("Individual Sizes", "IndivSizes", false));
    p->SetEditor("CheckBox");
    
    p = propertyGrid->Insert(before, new wxUIntProperty("Channels per Port", "Channels", GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", GetMaxChannels());
    
    propertyGrid->Insert(before, new wxPropertyCategory("Sizes", "Sizes"));
}
void KinetOutput::RemoveProperties(wxPropertyGrid* propertyGrid) {
    IPOutput::RemoveProperties(propertyGrid);
    propertyGrid->DeleteProperty("Version");
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Universes");
    propertyGrid->DeleteProperty("UniversesDisplay");
    propertyGrid->DeleteProperty("IndivSizes");
    propertyGrid->DeleteProperty("Channels");
    propertyGrid->DeleteProperty("Sizes");
}
#endif
#pragma endregion UI

