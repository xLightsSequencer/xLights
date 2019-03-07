#include <wx/msgdlg.h>
#include <wx/xml/xml.h>

#include "ControllerUploadData.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"
#include "../models/ModelManager.h"
#include "../models/Model.h"

#include <log4cpp/Category.hh>

//Need to double check all the errors the falcon upload detects this detects
//Need to further reduce the falcon code ... build richer functions
//like GetMaxPixelChannel(startPort, endPort);

UDController::UDController(std::string ip, ModelManager* mm, OutputManager* om, std::list<int>* selected, std::string& check)
{
    std::list<Model*> noConnectionModels;

    _ipAddress = ip;

    // get the list of outputs going to this controller
    _outputs = om->GetAllOutputs(ip, *selected);

    for (auto ito = _outputs.begin(); ito != _outputs.end(); ++ito)
    {
        // this universe is sent to the falcon

        // find all the models in this range
        for (auto it = mm->begin(); it != mm->end(); ++it)
        {
            if (!ModelProcessed(it->second) && it->second->GetDisplayAs() != "ModelGroup")
            {
                long modelstart = it->second->GetNumberFromChannelString(it->second->ModelStartChannel);
                long modelend = modelstart + it->second->GetChanCount() - 1;
                if ((modelstart >= (*ito)->GetStartChannel() && modelstart <= (*ito)->GetEndChannel()) ||
                    (modelend >= (*ito)->GetStartChannel() && modelend <= (*ito)->GetEndChannel()))
                {
                    //logger_base.debug("Model %s start %d end %d found on controller %s output %d start %d end %d.",
                    //    (const char *)it->first.c_str(), modelstart, modelend,
                    //    (const char *)_ip.c_str(), node, currentcontrollerstartchannel, currentcontrollerendchannel);
                    if (!it->second->IsControllerConnectionValid())
                    {
                        // only warn if we have not already warned
                        if (std::find(noConnectionModels.begin(), noConnectionModels.end(), it->second) == noConnectionModels.end())
                        {
                            noConnectionModels.push_back(it->second);
                        }
                    }
                    else
                    {
                        // model uses channels in this universe

                        int port = it->second->GetControllerPort();

                        if (it->second->IsPixelProtocol())
                        {
                            if (it->second->GetNumPhysicalStrings() == 1)
                            {
                                GetControllerPixelPort(port)->AddModel(it->second, om, -1);
                            }
                            else
                            {
                                for (int i = 0; i < it->second->GetNumPhysicalStrings(); i++)
                                {
                                    long startChannel = it->second->GetStringStartChan(i) + 1;
                                    long sc;
                                    Output* oo = om->GetOutput(startChannel, sc);
                                    if (oo != nullptr && oo->GetIP() == _ipAddress)
                                    {
                                        GetControllerPixelPort(port + i)->AddModel(it->second, om, i);
                                    }
                                    else
                                    {
                                        port = -1 * i;
                                    }
                                }
                            }
                        }
                        else
                        {
                            GetControllerSerialPort(port)->AddModel(it->second, om, -1);
                        }
                    }
                }
            }
        }
    }

    for (auto it = noConnectionModels.begin(); it != noConnectionModels.end(); ++it)
    {
        bool ok = false;

        for (auto p = _pixelPorts.begin(); !ok && p != _pixelPorts.end(); ++p)
        {
            if (p->second->GetStartChannel() <= (*it)->GetFirstChannel()+1 &&
                p->second->GetEndChannel() >= (*it)->GetLastChannel()+1)
            {
                ok = true;
            }
        }

        if (!ok)
        {
            check += wxString::Format("WARNING: Controller Upload: Model %s on controller %s does not have its Controller Connection details completed. Model ignored.\n", (const char *)(*it)->GetFullName().c_str(), (const char *)ip.c_str()).ToStdString();
        }
    }
}

UDController::~UDController()
{
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        delete it->second;
    }
    _pixelPorts.clear();
    for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
    {
        delete it->second;
    }
    _serialPorts.clear();
}

UDControllerPort* UDController::GetControllerPixelPort(int port)
{
    bool found = false;
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        if (it->second->GetPort() == port)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        _pixelPorts[port] = new UDControllerPort(port);
    }

    return _pixelPorts[port];
}

UDControllerPort* UDController::GetControllerSerialPort(int port)
{
    if (!HasSerialPort(port))
    {
        _serialPorts[port] = new UDControllerPort(port);
    }

    return _serialPorts[port];
}

bool UDController::IsValid(ControllerRules* rules) const
{
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        if (!it->second->IsValid()) return false;
    }
    for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
    {
        if (!it->second->IsValid()) return false;
    }

    return true;
}

int UDController::GetMaxSerialPort() const
{
    int last = 0;
    for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
    {
        if (it->second->GetPort() > last)
        {
            last = it->second->GetPort();
        }
    }

    return last;
}

int UDController::GetMaxPixelPort() const
{
    int last = 0;
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        if (it->second->GetPort() > last)
        {
            last = it->second->GetPort();
        }
    }

    return last;
}

void UDController::Dump() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("UDController Dump");

    logger_base.debug("   IP Address %s.", (const char *)_ipAddress.c_str());
    logger_base.debug("   Outputs:");
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        logger_base.debug("        %s", (*it)->GetLongDescription().c_str());
    }
    logger_base.debug("   Pixel Ports %d. Maximum port Number %d.", (int)_pixelPorts.size(), GetMaxPixelPort());
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        it->second->Dump();
    }
    logger_base.debug("   Serial Ports %d. Maximum port Number %d.", (int)_serialPorts.size(), GetMaxSerialPort());
    for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
    {
        it->second->Dump();
    }
}

bool UDController::HasSerialPort(int port) const
{
    for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
    {
        if (it->second->GetPort() == port)
        {
            return true;
        }
    }
    return false;
}

bool UDController::Check(ControllerRules* rules, std::string& res)
{
    bool success = true;

    // all serial ports must be valid ports for this controller
    // all pixel ports must be valid for this controller
    if (rules->GetMaxPixelPort() == 0 && _pixelPorts.size() > 0)
    {
        res += wxString::Format("ERROR: Attempt to upload pixel port %d but this controller does not support pixel ports.\n", _pixelPorts.begin()->second->GetPort()).ToStdString();
        success = false;
    }
    else
    {
        for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
        {
            success &= it->second->Check(this, true, rules, _outputs, res);

            if (it->second->GetPort() < 1 || it->second->GetPort() > rules->GetMaxPixelPort())
            {
                res += wxString::Format("ERROR: Pixel port %d is not valid on this controller. Valid ports %d-%d.\n", it->second->GetPort(), 1, rules->GetMaxPixelPort()).ToStdString();
                success = false;
            }
        }
    }

    if (!rules->SupportsMultipleInputProtocols())
    {
        std::string protocol = "";
        for (auto o : _outputs)
        {
            if (protocol == "")
            {
                protocol = o->GetType();
            }
            else
            {
                if (o->GetType() != protocol)
                {
                    res += wxString::Format("ERROR: Controller only support a single input protocol at a time. %s and %s found.\n", protocol, o->GetType()).ToStdString();
                    success = false;
                }
            }
        }
    }

    if (!rules->SupportsMultipleProtocols())
    {
        std::string protocol;
        for (auto it : _pixelPorts)
        {
            if (protocol == "") protocol = it.second->GetProtocol();

            if (protocol != it.second->GetProtocol())
            {
                res += wxString::Format("ERROR: Pixel ports only support a single protocol at a time. %s and %s found.\n", protocol, it.second->GetProtocol()).ToStdString();
                success = false;
            }
        }

        protocol = "";
        for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
        {
            if (protocol == "") protocol = it->second->GetProtocol();

            if (protocol != it->second->GetProtocol())
            {
                res += wxString::Format("ERROR: Serial ports only support a single protocol at a time. %s and %s found.\n", protocol, it->second->GetProtocol()).ToStdString();
                success = false;
            }
        }
    }

    if (rules->AllUniversesSameSize())
    {
        int size = -1;
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            if (size == -1) size = (*it)->GetChannels();
            if (size != (*it)->GetChannels())
            {
                res += wxString::Format("ERROR: All universes must be the same size. %d and %d found.\n", size, (int)(*it)->GetChannels()).ToStdString();
                success = false;
            }
        }
    }

    if (rules->UniversesMustBeSequential())
    {
        int seq = -1;

        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            if (seq == -1) seq = (*it)->GetUniverse() - 1;
            if (seq + 1 != (*it)->GetUniverse())
            {
                res += wxString::Format("ERROR: All universes must be sequential. %d followed %d.\n", (*it)->GetUniverse(), seq).ToStdString();
                success = false;
            }
            seq = (*it)->GetUniverse();
        }
    }

    auto inputprotocols = rules->GetSupportedInputProtocols();
    if (inputprotocols.size() > 0)
    {
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            if (std::find(inputprotocols.begin(), inputprotocols.end(), (*it)->GetType()) == inputprotocols.end())
            {
                res += wxString::Format("ERROR: Output %s is not a protocol this controller supports.\n", (*it)->GetType()).ToStdString();
                success = false;
            }
        }
    }

    if (rules->GetMaxSerialPort() == 0 && _serialPorts.size() > 0)
    {
        res += wxString::Format("ERROR: Attempt to upload serial port %d but this controller does not support serial ports.\n", _serialPorts.begin()->second->GetPort()).ToStdString();
        success = false;
    }
    else
    {
        for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
        {
            success &= it->second->Check(this, false, rules, _outputs, res);

            if (it->second->GetPort() < 1 || it->second->GetPort() > rules->GetMaxSerialPort())
            {
                res += wxString::Format("ERROR: Serial port %d is not valid on this controller. Valid ports %d-%d.\n", it->second->GetPort(), 1, rules->GetMaxSerialPort()).ToStdString();
                success = false;
            }
        }
    }

    return success;
}

Output* UDController::GetFirstOutput() const
{
    if (_outputs.size() == 0) return nullptr;

    return _outputs.front();
}

bool UDController::HasPixelPort(int port) const
{
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        if (it->second->GetPort() == port)
        {
            return true;
        }
    }
    return false;
}

bool UDController::ModelProcessed(Model* m)
{
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        if (it->second->ContainsModel(m))
        {
            return true;
        }
    }

    for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
    {
        if (it->second->ContainsModel(m))
        {
            return true;
        }
    }

    return false;
}

int UDControllerPortModel::GetBrightness(int currentBrightness)
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("brightness"))  return wxAtoi(node->GetAttribute("brightness"));
    return currentBrightness;
}

int UDControllerPortModel::GetNullPixels(int currentNullPixels)
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("nullNodes"))  return wxAtoi(node->GetAttribute("nullNodes"));
    return currentNullPixels;
}

float UDControllerPortModel::GetGamma(int currentGamma)
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("gamma"))  return wxAtof(node->GetAttribute("gamma"));
    return currentGamma;
}

std::string UDControllerPortModel::GetColourOrder(const std::string& currentColourOrder)
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("colorOrder"))  return node->GetAttribute("colorOrder");
    return currentColourOrder;
}

std::string UDControllerPortModel::GetDirection(const std::string& currentDirection)
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("reverse"))  return wxAtoi(node->GetAttribute("reverse")) == 1 ? "Reverse" : "Forward";
    return currentDirection;
}

int UDControllerPortModel::GetGroupCount(int currentGroupCount)
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("groupCount"))  return wxAtoi(node->GetAttribute("groupCount"));
    return currentGroupCount;
}

int UDControllerPortModel::GetDMXChannelOffset()
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("channel"))  return wxAtoi(node->GetAttribute("channel"));
    return 0;
}

UDControllerPortModel::UDControllerPortModel(Model* m, OutputManager* om, int string)
{
    _model = m;
    _string = string;
    _protocol = _model->GetControllerProtocol();

    if (string == -1)
    {
        _startChannel = _model->GetNumberFromChannelString(_model->ModelStartChannel);
        _endChannel = _startChannel + _model->GetChanCount() - 1;
    }
    else
    {
        _startChannel = _model->GetStringStartChan(string) + 1;
        _endChannel = _startChannel + _model->NodesPerString(string) * _model->GetChanCountPerNode() - 1;
    }

    Output* o = om->GetOutput(_startChannel, _universeStartChannel);
    if (o == nullptr)
    {
        _universe = -1;
    }
    else
    {
        _universe = o->GetUniverse();
    }
}

std::string UDControllerPortModel::GetName() const
{
    if (_string == -1)
    {
        return _model->GetName();
    }
    else
    {
        return _model->GetName() + "-str-" + wxString::Format("%d", _string + 1).ToStdString();
    }
}

void UDControllerPortModel::Dump() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_string == -1)
    {
        logger_base.debug("                Model %s. Controller Connection %s. Start Channel %ld. End Channel %ld. Channels %ld. Pixels %d. Start Channel #%d:%d",
            (const char*)_model->GetName().c_str(), (const char *)_model->GetControllerConnectionRangeString().c_str(), 
            _startChannel, _endChannel, Channels(), (int)(Channels() / 3), GetUniverse(), GetUniverseStartChannel());
    }
    else
    {
        logger_base.debug("                Model %s. String %d. Controller Connection %s. Start Channel %ld. End Channel %ld.",
            (const char*)_model->GetName().c_str(), _string + 1, (const char *)_model->GetControllerConnectionRangeString().c_str(),
            _startChannel, _endChannel);
    }
}

bool UDControllerPortModel::ChannelsOnOutputs(std::list<Output*>& outputs) const
{
    long lastChecked = _startChannel - 1;
    long ll = lastChecked;

    while (lastChecked < _endChannel)
    {
        for (auto it = outputs.begin(); it != outputs.end(); ++it)
        {
            if (lastChecked + 1 >= (*it)->GetStartChannel() &&
                lastChecked + 1 <= (*it)->GetEndChannel())
            {
                lastChecked = (*it)->GetEndChannel();
                break;
            }
        }

        if (ll == lastChecked)
        {
            return false;
        }
        ll = lastChecked;
    }

    return true;
}

bool UDControllerPortModel::Check(const UDControllerPort* port, bool pixel, ControllerRules* rules, std::list<Output*>& outputs, std::string& res) const
{
    bool success = true;
    if (!ChannelsOnOutputs(outputs))
    {
        res += wxString::Format("WARN: Model %s uses channels not being sent to this controller.\n", GetName());
    }

    return success;
}

UDControllerPort::~UDControllerPort()
{
    while (_models.size() > 0)
    {
        delete _models.back();
        _models.pop_back();
    }
    while (_virtualStrings.size() > 0)
    {
        delete _virtualStrings.front();
        _virtualStrings.pop_front();
    }
}

UDControllerPortModel* UDControllerPort::GetFirstModel() const
{
    wxASSERT(_models.size() > 0);
    UDControllerPortModel* first = _models.front();

    for (auto it = _models.begin(); it != _models.end(); ++it)
    {
        if (**it < *first)
        {
            first = *it;
        }
    }

    return first;
}

UDControllerPortModel* UDControllerPort::GetLastModel() const
{
    wxASSERT(_models.size() > 0);
    UDControllerPortModel* last = _models.front();

    for (auto it = _models.begin(); it != _models.end(); ++it)
    {
        if ((*it)->GetEndChannel() > last->GetEndChannel())
        {
            last = *it;
        }
    }

    return last;
}

bool compare_modelsc(const UDControllerPortModel* first, const UDControllerPortModel* second)
{
    return first->GetStartChannel() < second->GetStartChannel();
}

void UDControllerPort::AddModel(Model* m, OutputManager* om, int string)
{
    wxASSERT(!ContainsModel(m));
    _models.push_back(new UDControllerPortModel(m, om, string));
    if (_protocol == "")
    {
        _protocol = m->GetControllerProtocol();
    }
    _models.sort(compare_modelsc);
}

long UDControllerPort::GetStartChannel() const
{
    if (_models.size() == 0)
    {
        return -1;
    }
    else
    {
        return GetFirstModel()->GetStartChannel();
    }
}

long UDControllerPort::GetEndChannel() const
{
    if (_models.size() == 0)
    {
        return -1;
    }
    else
    {
        return GetLastModel()->GetEndChannel();
    }
}

long UDControllerPort::Channels() const
{
    return GetEndChannel() - GetStartChannel() + 1;
}

std::string UDControllerPort::GetPortName() const
{
    if (_models.size() == 0)
    {
        return "";
    }
    else
    {
        return GetFirstModel()->GetName();
    }
}

bool UDControllerPort::ContainsModel(Model* m) const
{
    for (auto it = _models.begin(); it != _models.end(); ++it)
    {
        if ((*it)->GetModel() == m)
        {
            return true;
        }
    }
    return false;
}

void UDControllerPort::Dump() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("            Port %d. Protocol %s. Valid %s. Invalid Reason '%s'. Channels %ld. Pixels %d. Start #%d:%d.", _port, (const char *)_protocol.c_str(), (_valid ? "TRUE" : "FALSE"), (const char *)_invalidReason.c_str(), Channels(), (int)(Channels() / 3), GetUniverse(), GetUniverseStartChannel());
    for (auto it = _models.begin(); it != _models.end(); ++it)
    {
        (*it)->Dump();
    }
}

bool UDControllerPort::Check(const UDController* controller, bool pixel, ControllerRules* rules, std::list<Output*>& outputs, std::string& res) const
{
    bool success = true;

    // protocols must be valid for these output types and controller
    if (pixel)
    {
        if (!rules->IsValidPixelProtocol(_protocol))
        {
            res += wxString::Format("ERROR: Invalid protocol on pixel port %d: %s\n", _port, _protocol).ToStdString();
            success = false;
        }
        else
        {
            for (auto it = _models.begin(); it != _models.end(); ++it)
            {
                if ((*it)->GetProtocol() != _protocol)
                {
                    res += wxString::Format("ERROR: Model %s on pixel port %d has protocol %s but port protocol has been set to %s. This is because you have mixed protocols on your models.", (*it)->GetName(), _port, (*it)->GetProtocol(), _protocol).ToStdString();
                    success = false;
                }
            }
        }

        // port must not have too many pixels on it
        if (Channels() > rules->GetMaxPixelPortChannels())
        {
            res += wxString::Format("ERROR: Pixel port %d has %d nodes allocated but maximum is %d.\n", _port, (int)Channels() / 3, rules->GetMaxPixelPortChannels() / 3).ToStdString();
            success = false;
        }
    }
    else
    {
        if (!rules->IsValidSerialProtocol(_protocol))
        {
            res += wxString::Format("ERROR: Invalid protocol on serial port %d: %s\n", _port, _protocol).ToStdString();
            success = false;
        }
        else
        {
            for (auto it = _models.begin(); it != _models.end(); ++it)
            {
                if ((*it)->GetProtocol() != _protocol)
                {
                    res += wxString::Format("ERROR: Model %s on serial port %d has protocol %s but port protocol has been set to %s. This is because you have mixed protocols on your models.\n", (*it)->GetName(), _port, (*it)->GetProtocol(), _protocol).ToStdString();
                    success = false;
                }
            }
        }
        if (Channels() > rules->GetMaxSerialPortChannels())
        {
            res += wxString::Format("ERROR: Serial port %d has %d channels allocated but maximum is %d.\n", _port, (int)Channels(), rules->GetMaxSerialPortChannels()).ToStdString();
            success = false;
        }
    }

    long ch = -1;
    for (auto it = _models.begin(); it != _models.end(); ++it)
    {
        if (ch == -1) ch = (*it)->GetStartChannel() - 1;
        if ((*it)->GetStartChannel() > ch + 1)
        {
            res += wxString::Format("WARNING: Gap in models on port %d channel %ld to %ld.\n", _port, ch, (*it)->GetStartChannel()).ToStdString();
        }
        ch = (*it)->GetEndChannel();
    }


    for (auto it = _models.begin(); it != _models.end(); ++it)
    {
        // all models must be fully contained within the outputs on this controller
        success &= (*it)->Check(this, pixel, rules, outputs, res);
    }

    return success;
}

int UDControllerPort::GetUniverse() const
{
    if (_models.size() == 0)
    {
        return -1;
    }
    else
    {
        return GetFirstModel()->GetUniverse();
    }
}

int UDControllerPort::GetUniverseStartChannel() const
{
    if (_models.size() == 0)
    {
        return -1;
    }
    else
    {
        return GetFirstModel()->GetUniverseStartChannel();
    }
}

void UDControllerPort::CreateVirtualStrings()
{
    while (_virtualStrings.size() > 0)
    {
        delete _virtualStrings.front();
        _virtualStrings.pop_front();
    }

    long lastEndChannel = -1000;
    UDVirtualString* current = nullptr;
    for (auto it : _models)
    {
        bool first = false;
        int brightness = it->GetBrightness(-9999);
        int nullPixels = it->GetNullPixels(-9999);
        std::string reverse = it->GetDirection("unknown");
        std::string colourOrder = it->GetColourOrder("unknown");
        float gamma = it->GetGamma(-9999);
        int groupCount = it->GetGroupCount(-9999);

        if (it == _models.front())
        {
            // this is automatically a new virtual string
            current = new UDVirtualString();
            _virtualStrings.push_back(current);
            first = true;
            lastEndChannel = it->GetEndChannel();
        }
        else
        {
            if ((brightness != -9999 && current->_brightness != brightness) ||
                (nullPixels != -9999) ||
                (reverse != "unknown" && current->_reverse != reverse) ||
                (colourOrder != "unknown" && current->_colourOrder != colourOrder) ||
                (gamma != -9999 && current->_gamma != gamma) ||
                (groupCount != -9999 && current->_groupCount != groupCount) ||
                lastEndChannel + 1 != it->GetStartChannel())
            {
                current = new UDVirtualString();
                _virtualStrings.push_back(current);
                first = true;
            }
            lastEndChannel = it->GetEndChannel();
        }

        current->_endChannel = it->GetEndChannel();

        if (first)
        {
            current->_startChannel = it->GetStartChannel();
            current->_description = it->GetName();
            current->_protocol = it->GetProtocol();
            current->_universe = it->GetUniverse();
            current->_universeStartChannel = it->GetUniverseStartChannel();

            if (gamma == -9999)
            {
                current->_gammaSet = false;
                current->_gamma = 0;
            }
            else
            {
                current->_gammaSet = true;
                current->_gamma = gamma;
            }
            if (nullPixels == -9999)
            {
                current->_nullPixelsSet = false;
                current->_nullPixels = 0;
            }
            else
            {
                current->_nullPixelsSet = true;
                current->_nullPixels = nullPixels;
            }
            if (brightness == -9999)
            {
                current->_brightnessSet = false;
                current->_brightness = 0;
            }
            else
            {
                current->_brightnessSet = true;
                current->_brightness = brightness;
            }
            if (groupCount == -9999)
            {
                current->_groupCountSet = false;
                current->_groupCount = 0;
            }
            else
            {
                current->_groupCountSet = true;
                current->_groupCount = groupCount;
            }
            if (reverse == "unknown")
            {
                current->_reverseSet = false;
                current->_reverse = "";
            }
            else
            {
                current->_reverseSet = true;
                current->_reverse = reverse;
            }
            if (colourOrder == "unknown")
            {
                current->_colourOrderSet = false;
                current->_colourOrder = "";
            }
            else
            {
                current->_colourOrderSet = true;
                current->_colourOrder = colourOrder;
            }
        }
    }
}
