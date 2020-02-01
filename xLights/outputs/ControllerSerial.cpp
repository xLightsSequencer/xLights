#include <wx/xml/xml.h>

#include "ControllerSerial.h"
#include "OutputManager.h"
#include "Output.h"
#include "SerialOutput.h"
#include "../OutputModelManager.h"
#include "../UtilFunctions.h"
#include "../SpecialOptions.h"
#include "../controllers/ControllerCaps.h"

wxPGChoices ControllerSerial::__types;
wxPGChoices ControllerSerial::__ports;
wxPGChoices ControllerSerial::__speeds;
wxPGChoices ControllerSerial::__parities;
wxPGChoices ControllerSerial::__stopBits;
wxPGChoices ControllerSerial::__lorDeviceTypes;
wxPGChoices ControllerSerial::__lorAddressModes;

// This is a fake dialog but it allows the delete to work
class DeleteLorControllerDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    DeleteLorControllerDialogAdapter()
        : wxPGEditorDialogAdapter() {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        wxVariant v("");
        SetValue(v);
        return true;
    }
protected:
};

class DeleteLorControllerProperty : public wxStringProperty
{
    LorControllers& _lc;
public:
    DeleteLorControllerProperty(LorControllers& lc,
        const wxString& label,
        const wxString& name)
        : wxStringProperty(label, name, wxEmptyString), _lc(lc) {
    }
    virtual ~DeleteLorControllerProperty() { }
    const wxPGEditor* DoGetEditorClass() const
    {
        return wxPGEditor_TextCtrlAndButton;
    }
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        // extract the index of the LOR device to delete
        int pos = wxAtoi(GetName().AfterLast('/'));

        // get an iterator pointing to it
        auto it = _lc.GetControllers().begin();
        std::advance(it, pos);
        wxASSERT(it != _lc.GetControllers().end());

        // now erase it
        _lc.GetControllers().erase(it);

        // return a dummy dialog which just does enough to register that a change has happened so we can trigger the refresh
        return new DeleteLorControllerDialogAdapter();
    }
};

void ControllerSerial::InitialiseTypes(bool forceXXX)
{
    if (__types.GetCount() == 0)
    {
        __types.Add(OUTPUT_DMX);
        __types.Add(OUTPUT_LOR);
        __types.Add(OUTPUT_LOR_OPT);
        __types.Add(OUTPUT_OPENDMX);
        __types.Add(OUTPUT_OPENPIXELNET);
        __types.Add(OUTPUT_RENARD);
        __types.Add(OUTPUT_DLIGHT);
        if (forceXXX || SpecialOptions::GetOption("xxx") == "true")
        {
            __types.Add(OUTPUT_xxxSERIAL);
        }
    }
    else if (forceXXX)
    {
        bool found = false;
        for (size_t i = 0; i < __types.GetCount(); i++)
        {
            if (__types.GetLabel(i) == OUTPUT_xxxSERIAL)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            __types.Add(OUTPUT_xxxSERIAL);
        }
    }

    if (__parities.GetCount() == 0)
    {
        __parities.Add("None");
        __parities.Add("Odd");
        __parities.Add("Even");
    }

    if (__stopBits.GetCount() == 0)
    {
        __stopBits.Add("0");
        __stopBits.Add("1");
        __stopBits.Add("2");
    }

    if (__speeds.GetCount() == 0)
    {
        auto s = SerialOutput::GetPossibleBaudRates();
        for (const auto& it : s)
        {
            __speeds.Add(it);
        }
    }

    if (__ports.GetCount() == 0)
    {
        auto p = SerialOutput::GetPossibleSerialPorts();
        for (const auto& it : p)
        {
            __ports.Add(it);
        }
    }

    if (__lorAddressModes.GetCount() == 0)
    {
        __lorAddressModes.Add("Normal");
        __lorAddressModes.Add("Legacy");
        __lorAddressModes.Add("Split");
    }

    if (__lorDeviceTypes.GetCount() == 0)
    {
        __lorDeviceTypes.Add("AC Controller");
        __lorDeviceTypes.Add("RGB Controller");
        __lorDeviceTypes.Add("CCR");
        __lorDeviceTypes.Add("CCB");
        __lorDeviceTypes.Add("Pixie4");
        __lorDeviceTypes.Add("Pixie8");
        __lorDeviceTypes.Add("Pixie16");
    }
}

void ControllerSerial::SetPort(const std::string& port)
{
    if (_outputs.front() != nullptr)
    {
        if (_outputs.front()->GetCommPort() != port)
        {
            _outputs.front()->SetCommPort(port);
            _port = port;
            _dirty = true;
        }
    }
}

void ControllerSerial::SetSpeed(int speed)
{
    if (_outputs.front() != nullptr)
    {
        if (_outputs.front()->GetBaudRate() != speed)
        {
            _outputs.front()->SetBaudRate(speed);
            _speed = speed;
            _dirty = true;
        }
    }
}

void ControllerSerial::SetChannels(int channels)
{
    if (_outputs.front() != nullptr)
    {
        if (_outputs.front()->GetChannels() != channels)
        {
            _outputs.front()->SetChannels(channels);
            _dirty = true;
        }
    }
}

void ControllerSerial::SetProtocol(const std::string& type)
{
    if (_outputs.front() != nullptr)
    {
        if (_outputs.front()->GetType() != type)
        {
            _type = type;
            auto s = _outputs.front()->GetBaudRate();
            auto p = _outputs.front()->GetCommPort();
            auto c = _outputs.front()->GetChannels();
            delete _outputs.front();
            _outputs.pop_front();

            Output* o = nullptr;
            if (type == OUTPUT_DLIGHT)
            {
                o = new DLightOutput();
            }
            else if (type == OUTPUT_DMX)
            {
                o = new DMXOutput();
            }
            else if (type == OUTPUT_LOR)
            {
                o = new LOROutput();
            }
            else if (type == OUTPUT_LOR_OPT)
            {
                o = new LOROptimisedOutput();
                SetAutoSize(false);
            }
            else if (type == OUTPUT_OPENDMX)
            {
                o = new OpenDMXOutput();
            }
            else if (type == OUTPUT_OPENPIXELNET)
            {
                o = new OpenPixelNetOutput();
            }
            else if (type == OUTPUT_RENARD)
            {
                o = new RenardOutput();
            }
            else
            {
                wxASSERT(false);
            }
            o->SetCommPort(p);
            o->SetBaudRate(s);
            o->SetChannels(c);
            _outputs.push_front(o);
            _dirty = true;
        }
    }
}

std::string ControllerSerial::GetChannelMapping(int32_t ch) const
{
    return wxString::Format("Channel %ld maps to ...\nType: %s\nName: %s\nComPort: %s\nChannel: %ld", 
        ch, GetProtocol(), GetName(), GetPort(), ch - GetStartChannel() + 1) + (IsActive() ? _("\n") : _(" INACTIVE\n"));
}

wxXmlNode* ControllerSerial::Save()
{
    wxXmlNode* um = Controller::Save();

    um->AddAttribute("Port", _port);
    um->AddAttribute("Speed", wxString::Format("%d", _speed));
    um->AddAttribute("Parity", _parity);
    um->AddAttribute("Bits", wxString::Format("%d", _bits));
    um->AddAttribute("StopBits", wxString::Format("%d", _stopBits));
    um->AddAttribute("Protocol", _type);

    return um;
}

Output::PINGSTATE ControllerSerial::Ping()
{
    _lastPingResult = dynamic_cast<SerialOutput*>(_outputs.front())->Ping();
    return GetLastPingState();
}

std::string ControllerSerial::GetLongDescription() const
{
    std::string res = "";

    if (!IsActive()) res += "INACTIVE ";
    res += GetName() + " " + GetProtocol() + " " + GetPort();
    res += " (" + std::string(wxString::Format(wxT("%d"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

void ControllerSerial::SetId(int id)
{
    SetTheId(id);
    if (GetProtocol() == OUTPUT_LOR_OPT)
    {
        dynamic_cast<LOROptimisedOutput*>(GetFirstOutput())->SetId(id);
    }
    else
    {
        dynamic_cast<SerialOutput*>(GetFirstOutput())->SetId(id);
    }
}

void ControllerSerial::AddProperties(wxPropertyGrid* propertyGrid)
{
    Controller::AddProperties(propertyGrid);

    wxPGProperty* p = propertyGrid->Append(new wxEnumProperty("Port", "Port", __ports, Controller::EncodeChoices(__ports, _port)));
    p->SetHelpString("This must be unique across all controllers.");

    p = propertyGrid->Append(new wxEnumProperty("Speed", "Speed", __speeds, Controller::EncodeChoices(__speeds, wxString::Format("%d", _speed))));
    if (dynamic_cast<SerialOutput*>(_outputs.front()))
    {
        if (!dynamic_cast<SerialOutput*>(_outputs.front())->AllowsBaudRateSetting())
        {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetHelpString("Speed is fixed for this protocol.");
        }
    }
    //p = propertyGrid->Append(new wxEnumProperty("Stop Bits", "StopBits", __stopBits, Controller::EncodeChoices(__stopBits, wxString::Format("%d", _stopBits))));
    //p = propertyGrid->Append(new wxEnumProperty("Parity", "Parity", __parities, Controller::EncodeChoices(__parities, _parity)));

    p = propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", __types, Controller::EncodeChoices(__types, _type)));

    p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", _outputs.front()->GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _outputs.front()->GetMaxChannels());

    if (IsAutoSize())
    {
        p->ChangeFlag(wxPG_PROP_READONLY, true);
        p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
    }
    else
    {
        p->SetEditor("SpinCtrl");
    }

    if (_type == OUTPUT_LOR_OPT)
    {
        auto lo = dynamic_cast<LOROptimisedOutput*>(_outputs.front());

        if (lo != nullptr)
        {
            auto devs = lo->GetControllers().GetControllers();
            p = propertyGrid->Append(new wxUIntProperty("Devices", "Devices", devs.size()));
            p->SetAttribute("Min", 1);
            p->SetAttribute("Max", 32);
            p->SetEditor("SpinCtrl");

            int i = 0;
            for (const auto& it : devs)
            {
                wxPGProperty* p2 = propertyGrid->Append(new wxPropertyCategory(it->GetType() + " : " + it->GetDescription(), wxString::Format("Device%d", i)));

                p = propertyGrid->AppendIn(p2, new DeleteLorControllerProperty(lo->GetControllers(), _("Delete this device"), wxString::Format("DeleteDevice/%d", i)));
                propertyGrid->LimitPropertyEditing(p);

                p = propertyGrid->AppendIn(p2, new wxEnumProperty("Device Type", wxString::Format("DeviceType/%d", i), __lorDeviceTypes, Controller::EncodeChoices(__lorDeviceTypes, it->GetType())));

                p = propertyGrid->AppendIn(p2, new wxUIntProperty("Channels", wxString::Format("DeviceChannels/%d", i), it->GetNumChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");

                p = propertyGrid->AppendIn(p2, new wxUIntProperty("Unit ID", wxString::Format("DeviceUnitID/%d", i), it->GetUnitId()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxUnitId());
                p->SetEditor("SpinCtrl");

                p = propertyGrid->AppendIn(p2, new wxStringProperty("Unit ID - Hex", wxString::Format("DeviceUnitIDHex/%d", i), wxString::Format("0x%02x", it->GetUnitId())));
                p->ChangeFlag(wxPG_PROP_READONLY, true);

                p = propertyGrid->AppendIn(p2, new wxEnumProperty("Address Mode", wxString::Format("DeviceAddressMode/%d", i), __lorAddressModes, it->GetAddressMode()));

                p = propertyGrid->AppendIn(p2, new wxStringProperty("Description", wxString::Format("DeviceDescription/%d", i), it->GetDescription()));

                i++;
            }
        }
    }
}

bool ControllerSerial::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager)
{
    if (Controller::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString name = event.GetPropertyName();
    wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());

    if (name == "Port")
    {
        SetPort(Controller::DecodeChoices(__ports, event.GetValue().GetLong()));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Port", nullptr);
        return true;
    }
    else if (name == "Speed")
    {
        SetSpeed(wxAtoi(Controller::DecodeChoices(__speeds, event.GetValue().GetLong())));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Speed", nullptr);
        return true;
    }
    else if (name == "Protocol")
    {
        SetProtocol(Controller::DecodeChoices(__types, event.GetValue().GetLong()));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Protocol");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        return true;
    }
    else if (name == "Channels")
    {
        SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        return true;
    }
    else if (StartsWith(name, "DeleteDevice/"))
    {
        auto lor = dynamic_cast<LOROptimisedOutput*>(_outputs.front());
        if (lor != nullptr)
        {
            lor->CalcTotalChannels();
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeleteDevice");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeleteDevice", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeleteDevice", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeleteDevice", nullptr);
        return true;
    }
    else if (name == "Devices")
    {
        auto lor = dynamic_cast<LOROptimisedOutput*>(_outputs.front());
        if (lor != nullptr)
        {
            while (event.GetValue().GetLong() < lor->GetControllers().GetControllers().size())
            {
                delete lor->GetControllers().GetControllers().back();
                lor->GetControllers().GetControllers().pop_back();
            }
            while (event.GetValue().GetLong() > lor->GetControllers().GetControllers().size())
            {
                lor->GetControllers().GetControllers().push_back(new LorController());
            }
            lor->CalcTotalChannels();
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Devices");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Devices", nullptr);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Devices", nullptr);
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Devices", nullptr);
        }
    }
    else if (StartsWith(name, "DeviceType/"))
    {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto lor = dynamic_cast<LOROptimisedOutput*>(_outputs.front());
        if (lor != nullptr)
        {
            auto it = lor->GetControllers().GetControllers().begin();
            std::advance(it, index);
            wxASSERT(it != lor->GetControllers().GetControllers().end());
            (*it)->SetType(Controller::DecodeChoices(__lorDeviceTypes, event.GetValue().GetLong()));
            lor->GetControllers().SetDirty();
            lor->CalcTotalChannels();
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceType");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceType", nullptr);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceType", nullptr);
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceType", nullptr);
        }
    }
    else if (StartsWith(name, "DeviceChannels/"))
    {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto lor = dynamic_cast<LOROptimisedOutput*>(_outputs.front());
        if (lor != nullptr)
        {
            auto it = lor->GetControllers().GetControllers().begin();
            std::advance(it, index);
            wxASSERT(it != lor->GetControllers().GetControllers().end());
            (*it)->SetNumChannels(event.GetValue().GetLong());
            lor->GetControllers().SetDirty();
            lor->CalcTotalChannels();
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceChannels");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceChannels", nullptr);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceChannels", nullptr);
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceChannels", nullptr);
        }
    }    
    else if (StartsWith(name, "DeviceUnitID/"))
    {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto lor = dynamic_cast<LOROptimisedOutput*>(_outputs.front());
        if (lor != nullptr)
        {
            auto it = lor->GetControllers().GetControllers().begin();
            std::advance(it, index);
            wxASSERT(it != lor->GetControllers().GetControllers().end());
            (*it)->SetUnitID(event.GetValue().GetLong());
            lor->GetControllers().SetDirty();
            lor->CalcTotalChannels();
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceUnitID");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceUnitID", nullptr);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceUnitID", nullptr);
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceUnitID", nullptr);
        }
    }
    else if (StartsWith(name, "DeviceAddressMode/"))
    {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto lor = dynamic_cast<LOROptimisedOutput*>(_outputs.front());
        if (lor != nullptr)
        {
            auto it = lor->GetControllers().GetControllers().begin();
            std::advance(it, index);
            wxASSERT(it != lor->GetControllers().GetControllers().end());
            (*it)->SetMode((LorController::AddressMode)event.GetValue().GetLong());
            lor->GetControllers().SetDirty();
            lor->CalcTotalChannels();
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode", nullptr);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode", nullptr);
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode", nullptr);
        }
    }
    else if (StartsWith(name, "DeviceDescription/"))
    {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto lor = dynamic_cast<LOROptimisedOutput*>(_outputs.front());
        if (lor != nullptr)
        {
            auto it = lor->GetControllers().GetControllers().begin();
            std::advance(it, index);
            wxASSERT(it != lor->GetControllers().GetControllers().end());
            (*it)->SetDescription(event.GetValue().GetString());
            lor->GetControllers().SetDirty();
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceDescription");
        }
    }

    return false;
}

void ControllerSerial::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const
{
    Controller::ValidateProperties(om, propGrid);

    for (const auto& it : om->GetControllers())
    {
        auto s = dynamic_cast<ControllerSerial*>(it);

        // Port must be unique
        auto p = propGrid->GetPropertyByName("Port");
        if (s != nullptr && it->GetName() != GetName() && s->GetPort() == GetPort() && GetPort() != "NotConnected")
        {
            p->SetBackgroundColour(*wxRED);
            break;
        }
        else
        {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    auto p = propGrid->GetPropertyByName("Protocol");
    auto caps = ControllerCaps::GetControllerConfig(this);
    if (caps != nullptr && p != nullptr)   
    {
        // controller must support the protocol
        if (!caps->IsValidSerialProtocol(Lower(_type)))
        {
            p->SetBackgroundColour(*wxRED);
        }       
        else
        {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = propGrid->GetPropertyByName("Channels");
    if (p != nullptr)
    {
        if (_outputs.front()->GetMaxChannels() < GetChannels() || GetChannels() < 1)
        {
            p->SetBackgroundColour(*wxRED);
        }
        else
        {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }
}

ControllerSerial::ControllerSerial(OutputManager* om, wxXmlNode* node, const std::string& showDir) : Controller(om, node, showDir)
{
    _type = node->GetAttribute("Protocol");
    InitialiseTypes(_type == OUTPUT_xxxSERIAL);
    _port = node->GetAttribute("Port");
    _parity = node->GetAttribute("Parity");
    _speed = wxAtoi(node->GetAttribute("Speed"));
    _stopBits = wxAtoi(node->GetAttribute("StopBits"));
    _dirty = false;
}

ControllerSerial::ControllerSerial(OutputManager* om) : Controller(om)
{
    InitialiseTypes(false);
    _name = om->UniqueName("Serial_");
    SerialOutput* o = new DMXOutput();
    o->SetChannels(512);
    _outputs.push_back(o);
    _type = OUTPUT_DMX;
    _port = _outputManager->GetFirstUnusedCommPort();
    o->SetCommPort(_port);
    _speed = o->GetBaudRate();
    _parity = o->GetParity();
    _stopBits = o->GetStopBits();
}

void ControllerSerial::Convert(wxXmlNode* node, std::string showDir)
{
    Controller::Convert(node, showDir);

    _outputs.push_back(Output::Create(node, showDir));
    if (_name == "" || StartsWith(_name, "Serial_"))
    {
        if (_outputs.back()->GetDescription() != "")
        {
            _name = _outputManager->UniqueName(_outputs.back()->GetDescription());
        }
        else
        {
            _name = _outputManager->UniqueName("Unnamed");
        }
    }
    if (_outputs.back() == nullptr)
    {
        // this shouldnt happen unless we are loading a future file with an output type we dont recognise
        _outputs.pop_back();
    }

    if (_outputs.size() > 0)
    {
        _port = _outputs.front()->GetCommPort();
        _speed = _outputs.front()->GetBaudRate();
        _parity = ((SerialOutput*)_outputs.front())->GetParity();
        _stopBits = ((SerialOutput*)_outputs.front())->GetStopBits();
        _type = _outputs.front()->GetType();
        _id = _outputs.front()->GetUniverse();
    }
}
