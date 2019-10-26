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

UDController::UDController(const std::string &ip, const std::string &hostname, ModelManager* mm, OutputManager* om, const std::list<int>* selected, std::string& check)
{
    _ipAddress = ip;
    _hostName = hostname;

    // get the list of outputs going to this controller
    _outputs = om->GetAllOutputs(_ipAddress, _hostName, *selected);

    for (const auto& ito : _outputs)
    {
        // this universe is sent to the falcon

        // find all the models in this range
        for (auto it = mm->begin(); it != mm->end(); ++it)
        {
            if (!ModelProcessed(it->second) && it->second->GetDisplayAs() != "ModelGroup")
            {
                int32_t modelstart = it->second->GetNumberFromChannelString(it->second->ModelStartChannel);
                int32_t modelend = modelstart + it->second->GetChanCount() - 1;
                if ((modelstart >= ito->GetStartChannel() && modelstart <= ito->GetEndChannel()) ||
                    (modelend >= ito->GetStartChannel() && modelend <= ito->GetEndChannel()))
                {
                    //logger_base.debug("Model %s start %d end %d found on controller %s output %d start %d end %d.",
                    //    (const char *)it->first.c_str(), modelstart, modelend,
                    //    (const char *)_ip.c_str(), node, currentcontrollerstartchannel, currentcontrollerendchannel);
                    if (!it->second->IsControllerConnectionValid())
                    {
                        // only warn if we have not already warned
                        if (std::find(_noConnectionModels.begin(), _noConnectionModels.end(), it->second) == _noConnectionModels.end())
                        {
                            _noConnectionModels.push_back(it->second);
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
                                    int32_t startChannel = it->second->GetStringStartChan(i) + 1;
                                    int32_t sc;
                                    Output* oo = om->GetOutput(startChannel, sc);
                                    if (oo != nullptr &&
                                        ((oo->GetIP() == _ipAddress) || (oo->GetResolvedIP() == _ipAddress)  || (oo->GetIP() == _hostName)))
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

    for (const auto& it : _noConnectionModels)
    {
        bool ok = false;
        for (const auto& p : _pixelPorts)
        {
            if (p.second->GetStartChannel() <= it->GetFirstChannel()+1 &&
                p.second->GetEndChannel() >= it->GetLastChannel()+1)
            {
                ok = true;
                break;
            }
        }

        if (!ok)
        {
            check += wxString::Format("WARN: Controller Upload: Model %s on controller %s does not have its Controller Connection details completed. Model ignored.\n", (const char *)it->GetFullName().c_str(), (const char *)ip.c_str()).ToStdString();
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
    for (auto it = _pixelPorts.begin(); it != _pixelPorts.end(); ++it)
    {
        if (it->second->GetPort() == port)
        {
            return it->second;
        }
    }

    _pixelPorts[port] = new UDControllerPort(port);
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
    if (_hostName != "" && _hostName != _ipAddress) {
        logger_base.debug("   HostName %s.", (const char *)_hostName.c_str());
    }
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

bool UDController::Check(const ControllerRules* rules, std::string& res)
{
    bool success = true;

    // all serial ports must be valid ports for this controller
    // all pixel ports must be valid for this controller
    if (rules->GetMaxPixelPort() == 0 && _pixelPorts.size() > 0)
    {
        res += wxString::Format("ERR: Attempt to upload pixel port %d but this controller does not support pixel ports.\n", _pixelPorts.begin()->second->GetPort()).ToStdString();
        success = false;
    }
    else
    {
        for (const auto& it : _pixelPorts)
        {
            if (rules->SupportsVirtualStrings())
            {
                it.second->CreateVirtualStrings(rules->MergeConsecutiveVirtualStrings());
            }

            success &= it.second->Check(this, true, rules, _outputs, res);

            if (it.second->GetPort() < 1 || it.second->GetPort() > rules->GetMaxPixelPort())
            {
                res += wxString::Format("ERR: Pixel port %d is not valid on this controller. Valid ports %d-%d.\n", it.second->GetPort(), 1, rules->GetMaxPixelPort()).ToStdString();
                success = false;
            }
        }
    }

    if (!rules->SupportsMultipleInputProtocols())
    {
        std::string protocol = "";
        for (const auto& o : _outputs)
        {
            if (protocol == "")
            {
                protocol = o->GetType();
            }
            else
            {
                if (o->GetType() != protocol)
                {
                    res += wxString::Format("ERR: Controller only support a single input protocol at a time. %s and %s found.\n", protocol, o->GetType()).ToStdString();
                    success = false;
                }
            }
        }
    }

    if (!rules->SupportsMultipleProtocols())
    {
        std::string protocol;
        for (const auto& it : _pixelPorts)
        {
            if (protocol == "") protocol = it.second->GetProtocol();

            if (protocol != it.second->GetProtocol())
            {
                res += wxString::Format("ERR: Pixel ports only support a single protocol at a time. %s and %s found.\n", protocol, it.second->GetProtocol()).ToStdString();
                success = false;
            }
        }

        protocol = "";
        for (auto it = _serialPorts.begin(); it != _serialPorts.end(); ++it)
        {
            if (protocol == "") protocol = it->second->GetProtocol();

            if (protocol != it->second->GetProtocol())
            {
                res += wxString::Format("ERR: Serial ports only support a single protocol at a time. %s and %s found.\n", protocol, it->second->GetProtocol()).ToStdString();
                success = false;
            }
        }
    }

    if (!rules->SupportsSmartRemotes())
    {
        for (const auto& it : _pixelPorts)
        {
            for (const auto& it2 : it.second->GetModels())
            {
                if (it2->GetSmartRemote() != 0)
                {
                    res += wxString::Format("ERR: Port %d has model %s with smart remote set ... but this controller does not support smart remotes.\n", 
                        it.second->GetPort(), it2->GetName());
                    success = false;
                }
            }
        }
    }
    else
    {
        for (const auto& it : _pixelPorts)
        {
            int countNoSmart = 0;
            int countSmart = 0;
            for (const auto& it2 : it.second->GetModels())
            {
                if (it2->GetSmartRemote() == 0)
                {
                    countNoSmart++;
                }
                else
                {
                    countSmart++;
                }
            }
            if (countSmart > 0 && countNoSmart > 0)
            {
                res += wxString::Format("ERR: Port %d has a mix of models with smart and non smart remotes. This is not supported.\n",
                    it.second->GetPort());
                success = false;
            }
        }
    }

    if (rules->AllUniversesSameSize())
    {
        int size = -1;
        for (const auto& it : _outputs)
        {
            if (size == -1) size = it->GetChannels();
            if (size != it->GetChannels())
            {
                res += wxString::Format("ERR: All universes must be the same size. %d and %d found.\n", size, (int)it->GetChannels()).ToStdString();
                success = false;
            }
        }
    }

    if (rules->UniversesMustBeSequential())
    {
        int seq = -1;

        for (const auto& it : _outputs)
        {
            if (seq == -1) seq = it->GetUniverse() - 1;
            if (seq + 1 != it->GetUniverse())
            {
                res += wxString::Format("ERR: All universes must be sequential. %d followed %d.\n", it->GetUniverse(), seq).ToStdString();
                success = false;
            }
            seq = it->GetUniverse();
        }
    }

    auto inputprotocols = rules->GetSupportedInputProtocols();
    if (inputprotocols.size() > 0)
    {
        for (const auto& it : _outputs)
        {
            if (std::find(inputprotocols.begin(), inputprotocols.end(), it->GetType()) == inputprotocols.end())
            {
                res += wxString::Format("ERR: Output %s is not a protocol this controller supports.\n", it->GetType()).ToStdString();
                success = false;
            }
        }
    }

    if (rules->GetMaxSerialPort() == 0 && _serialPorts.size() > 0)
    {
        res += wxString::Format("ERR: Attempt to upload serial port %d but this controller does not support serial ports.\n", _serialPorts.begin()->second->GetPort()).ToStdString();
        success = false;
    }
    else
    {
        for (const auto& it : _serialPorts)
        {
            success &= it.second->Check(this, false, rules, _outputs, res);

            if (it.second->GetPort() < 1 || it.second->GetPort() > rules->GetMaxSerialPort())
            {
                res += wxString::Format("ERR: Serial port %d is not valid on this controller. Valid ports %d-%d.\n", it.second->GetPort(), 1, rules->GetMaxSerialPort()).ToStdString();
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
    for (const auto& it : _pixelPorts)
    {
        if (it.second->GetPort() == port)
        {
            return true;
        }
    }
    return false;
}

bool UDController::ModelProcessed(Model* m)
{
    for (const auto& it : _pixelPorts)
    {
        if (it.second->ContainsModel(m))
        {
            return true;
        }
    }

    for (const auto& it : _serialPorts)
    {
        if (it.second->ContainsModel(m))
        {
            return true;
        }
    }

    return false;
}
int UDControllerPortModel::GetChannelsPerPixel() {
    return _model->GetNodeChannelCount(_model->GetStringType());
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
    return 1;
}

UDControllerPortModel::UDControllerPortModel(Model* m, OutputManager* om, int string)
{
    _model = m;
    _string = string;
    _protocol = _model->GetControllerProtocol();
    _smartRemote = _model->GetSmartRemote();

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
        logger_base.debug("                Model %s. Controller Connection %s. Start Channel %d. End Channel %d. Channels %d. Pixels %d. Start Channel #%d:%d",
            (const char*)_model->GetName().c_str(), (const char *)_model->GetControllerConnectionRangeString().c_str(), 
            _startChannel, _endChannel, Channels(), (int)(Channels() / 3), GetUniverse(), GetUniverseStartChannel());
    }
    else
    {
        logger_base.debug("                Model %s. String %d. Controller Connection %s. Start Channel %d. End Channel %d.",
            (const char*)_model->GetName().c_str(), _string + 1, (const char *)_model->GetControllerConnectionRangeString().c_str(),
            _startChannel, _endChannel);
    }
}

bool UDControllerPortModel::ChannelsOnOutputs(std::list<Output*>& outputs) const
{
    int32_t lastChecked = _startChannel - 1;
    int32_t ll = lastChecked;

    while (lastChecked < _endChannel)
    {
        for (const auto& it : outputs)
        {
            if (lastChecked + 1 >= it->GetStartChannel() &&
                lastChecked + 1 <= it->GetEndChannel())
            {
                lastChecked = it->GetEndChannel();
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

bool UDControllerPortModel::Check(const UDControllerPort* port, bool pixel, const ControllerRules* rules, std::list<Output*>& outputs, std::string& res) const
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

    for (const auto& it : _models)
    {
        if (*it < *first)
        {
            first = it;
        }
    }

    return first;
}

UDControllerPortModel* UDControllerPort::GetLastModel() const
{
    wxASSERT(_models.size() > 0);
    UDControllerPortModel* last = _models.front();

    for (const auto& it : _models)
    {
        if (it->GetEndChannel() > last->GetEndChannel())
        {
            last = it;
        }
    }

    return last;
}

bool compare_modelsc(const UDControllerPortModel* first, const UDControllerPortModel* second)
{
    if (first->GetSmartRemote() == second->GetSmartRemote())
    {
        return first->GetStartChannel() < second->GetStartChannel();
    }
    return first->GetSmartRemote() < second->GetSmartRemote();
}

void UDControllerPort::AddModel(Model* m, OutputManager* om, int string)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxASSERT(!ContainsModel(m));
    _models.push_back(new UDControllerPortModel(m, om, string));
    if (_protocol == "")
    {
        _protocol = m->GetControllerProtocol();
    }
    _models.sort(compare_modelsc);

    // now remove any overlapping models
    bool erased = true;
    while (erased)
    {
        erased = false;
        for (auto it = _models.begin(); it != _models.end(); ++it)
        {
            auto it2 = it;
            ++it2;

            for (; it2 != _models.end(); ++it2)
            {
                if ((*it2)->GetStartChannel() <= (*it)->GetEndChannel())
                {
                    // this model overlaps at least slightly
                    if ((*it2)->GetEndChannel() <= (*it)->GetEndChannel())
                    {
                        // it2 is totally inside it
                        logger_base.debug("CUD add model removed model %s as it totally overlaps with model %s",
                            (const char*)(*it2)->GetName().c_str(),
                            (const char*)(*it)->GetName().c_str()
                        );
                        _models.erase(it2);
                        erased = true;
                    }
                    else if ((*it)->GetStartChannel() == (*it2)->GetStartChannel() && (*it2)->GetEndChannel() > (*it)->GetEndChannel())
                    {
                        // i1 totally inside it2
                        logger_base.debug("CUD add model removed model %s as it totally overlaps with model %s",
                            (const char*)(*it)->GetName().c_str(),
                            (const char*)(*it2)->GetName().c_str()
                        );
                        _models.erase(it);
                        erased = true;
                    }
                    else
                    {
                        // so this is the difficult partial overlap case ... to prevent issues i will just erase model 2 and the user will need to fix it
                        logger_base.debug("CUD add model removed model %s as it PARTIALLY overlaps with model %s. This will cause issues but it cannot be handled by the upload.",
                            (const char*)(*it2)->GetName().c_str(),
                            (const char*)(*it)->GetName().c_str()
                        );
                        _models.erase(it2);
                        erased = true;
                    }
                }
                if (erased) break; // iterators are invalid so need to break out
            }
            if (erased) break; // iterators are invalid so need to break out
        }
    }
}

int32_t UDControllerPort::GetStartChannel() const
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

int32_t UDControllerPort::GetEndChannel() const
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

int32_t UDControllerPort::Channels() const
{
    if (_virtualStrings.size() == 0)
    {
        return GetEndChannel() - GetStartChannel() + 1;
    }
    else
    {
        int c = 0;
        for (const auto& it : _virtualStrings)
        {
            c += it->Channels();
        }
        return c;
    }
}

bool UDControllerPort::IsPixelProtocol() const
{
    return Model::IsPixelProtocol(_protocol);
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

    logger_base.debug("            Port %d. Protocol %s. Valid %s. Invalid Reason '%s'. Channels %d. Pixels %d. Start #%d:%d.", _port, (const char *)_protocol.c_str(), (_valid ? "TRUE" : "FALSE"), (const char *)_invalidReason.c_str(), Channels(), (int)(Channels() / 3), GetUniverse(), GetUniverseStartChannel());
    for (const auto& it : _models)
    {
        it->Dump();
    }
}

bool UDControllerPort::Check(const UDController* controller, bool pixel, const ControllerRules* rules, std::list<Output*>& outputs, std::string& res) const
{
    bool success = true;

    // protocols must be valid for these output types and controller
    if (pixel)
    {
        if (!rules->IsValidPixelProtocol(_protocol))
        {
            res += wxString::Format("ERR: Invalid protocol on pixel port %d: %s\n", _port, _protocol).ToStdString();
            success = false;
        }
        else
        {
            for (const auto& it : _models)
            {
                if (it->GetProtocol() != _protocol)
                {
                    res += wxString::Format("ERR: Model %s on pixel port %d has protocol %s but port protocol has been set to %s. This is because you have mixed protocols on your models.", it->GetName(), _port, it->GetProtocol(), _protocol).ToStdString();
                    success = false;
                }
            }
        }

        // port must not have too many pixels on it
        if (Channels() > rules->GetMaxPixelPortChannels())
        {
            res += wxString::Format("ERR: Pixel port %d has %d nodes allocated but maximum is %d.\n", _port, (int)Channels() / 3, rules->GetMaxPixelPortChannels() / 3).ToStdString();
            success = false;
        }
    }
    else
    {
        if (!rules->IsValidSerialProtocol(_protocol))
        {
            res += wxString::Format("ERR: Invalid protocol on serial port %d: %s\n", _port, _protocol).ToStdString();
            success = false;
        }
        else
        {
            for (const auto& it : _models)
            {
                if (it->GetProtocol() != _protocol)
                {
                    res += wxString::Format("ERR: Model %s on serial port %d has protocol %s but port protocol has been set to %s. This is because you have mixed protocols on your models.\n", it->GetName(), _port, it->GetProtocol(), _protocol).ToStdString();
                    success = false;
                }
            }
        }
        if (Channels() > rules->GetMaxSerialPortChannels())
        {
            res += wxString::Format("ERR: Serial port %d has %d channels allocated but maximum is %d.\n", _port, (int)Channels(), rules->GetMaxSerialPortChannels()).ToStdString();
            success = false;
        }
    }

    int32_t ch = -1;
    int lastSmartRemote = 0;
    for (const auto& it : _models)
    {
        if (ch == -1) ch = it->GetStartChannel() - 1;
        if (it->GetStartChannel() > ch + 1 && lastSmartRemote == it->GetSmartRemote())
        {
            if (it->GetSmartRemote() == 0)
            {
                res += wxString::Format("WARN: Gap in models on port %d channel %d to %d.\n", _port, ch, it->GetStartChannel()).ToStdString();
            }
            else
            {
                res += wxString::Format("WARN: Gap in models on port %d smart remote %d channel %d to %d.\n", _port, it->GetSmartRemote(), ch, it->GetStartChannel()).ToStdString();
            }
        }
        lastSmartRemote = it->GetSmartRemote();
        ch = it->GetEndChannel();
    }

    for (const auto& it : _models)
    {
        // all models must be fully contained within the outputs on this controller
        success &= it->Check(this, pixel, rules, outputs, res);
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

void UDControllerPort::CreateVirtualStrings(bool mergeSequential)
{
    while (_virtualStrings.size() > 0)
    {
        delete _virtualStrings.front();
        _virtualStrings.pop_front();
    }

    int32_t lastEndChannel = -1000;
    UDVirtualString* current = nullptr;
    for (const auto& it : _models)
    {
        bool first = false;
        int brightness = it->GetBrightness(-9999);
        int nullPixels = it->GetNullPixels(-9999);
        int smartRemote = it->GetSmartRemote();
        std::string reverse = it->GetDirection("unknown");
        std::string colourOrder = it->GetColourOrder("unknown");
        float gamma = it->GetGamma(-9999);
        int groupCount = it->GetGroupCount(-9999);

        if (current == nullptr || !mergeSequential)
        {
            // this is automatically a new virtual string
            current = new UDVirtualString();
            _virtualStrings.push_back(current);
            first = true;
            lastEndChannel = it->GetEndChannel();
        }
        else
        {
            wxASSERT(current != nullptr);
            if ((brightness != -9999 && current->_brightness != brightness) ||
                (nullPixels != -9999) ||
                (current->_smartRemote != smartRemote) ||
                (reverse == "unknown" && current->_reverse == "Reverse") ||
                (reverse != "unknown" && (current->_reverse != reverse || current->_reverse == "Reverse")) ||
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

        wxASSERT(current != nullptr);
        current->_endChannel = it->GetEndChannel();
        current->_models.push_back(it);

        if (first)
        {
            current->_startChannel = it->GetStartChannel();
            current->_description = it->GetName();
            current->_protocol = it->GetProtocol();
            current->_universe = it->GetUniverse();
            current->_universeStartChannel = it->GetUniverseStartChannel();
            current->_channelsPerPixel = it->GetChannelsPerPixel();
            current->_smartRemote = it->GetSmartRemote();

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
