#include "Output.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

#include "E131Output.h"
#include "ArtNetOutput.h"
#include "DDPOutput.h"
#include "NullOutput.h"
#include "LOROutput.h"
#include "DLightOutput.h"
#include "DMXOutput.h"
#include "PixelNetOutput.h"
#include "RenardOutput.h"
#include "OpenPixelNetOutput.h"
#include "OpenDMXOutput.h"
#include "../UtilFunctions.h"

#pragma region Constructors and Destructors
Output::Output(Output* output)
{
#ifdef USECHANGEDETECTION
    _changed = true;
    _skipCount = 0;
#endif
    _timer_msec = 0;
    _outputNumber = -1;
    _nullNumber = -1;
    _startChannel = -1;
    _ok = true;
    _baudRate = 0;
    _universe = 0;

    _dirty = output->IsDirty();
    _enabled = output->IsEnabled();
    _description = output->GetDescription();
    _channels = output->GetChannels();
    _controller = output->GetController();
}

Output::Output(wxXmlNode* node)
{
    #ifdef USECHANGEDETECTION
        _changed = true;
        _skipCount = 0;
    #endif
    _timer_msec = 0;
    _outputNumber = -1;
    _nullNumber = -1;
    _startChannel = -1;
    _ip = "";
    _universe = 0;
    _baudRate = 0;
    _commPort = "";
    _controller = nullptr;
    _dirty = false;
    _ok = true;

    _enabled = (node->GetAttribute("Enabled", "Yes") == "Yes");
    _description = UnXmlSafe(node->GetAttribute("Description").ToStdString());
    _dirty = false;
    _channels = wxAtoi(node->GetAttribute("MaxChannels"));
    std::string controller = UnXmlSafe(node->GetAttribute("Controller").ToStdString());
    if (controller != "")
    {
        _controller = Controller::GetController(controller);
    }
}

Output::Output()
{
#ifdef USECHANGEDETECTION
    _changed = true;
    _skipCount = 0;
#endif
    _timer_msec = 0;
    _outputNumber = -1;
    _nullNumber = -1;
    _startChannel = -1;
    _universe = 0;
    _baudRate = 0;
    _commPort = "";
    _controller = nullptr;
    _dirty = false;
    _enabled = true;
    _description = "";
    _dirty = true;
    _channels = 0;
    _ip = "";
    _ok = true;
}
#pragma endregion Constructors and Destructors

void Output::Save(wxXmlNode* node)
{
    node->AddAttribute("NetworkType", wxString(GetType().c_str()));
    if (!_enabled)
    {
        node->AddAttribute("Enabled", "No");
    }

    if (_description != "")
    {
        node->AddAttribute("Description", XmlSafe(_description));
    }

    if (_controller != nullptr)
    {
        node->AddAttribute("Controller", XmlSafe(_controller->GetId()));
    }

    node->AddAttribute("MaxChannels", wxString::Format("%d", _channels));

    _dirty = false;
}

wxXmlNode* Output::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}

#pragma region Static Functions
Output* Output::Create(wxXmlNode* node)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string type = node->GetAttribute("NetworkType", "").ToStdString();

    if (type == OUTPUT_E131)
    {
        return new E131Output(node);
    }
    else if (type == OUTPUT_NULL)
    {
        return new NullOutput(node);
    }
    else if (type == OUTPUT_ARTNET)
    {
        return new ArtNetOutput(node);
    }
    else if (type == OUTPUT_DDP)
    {
        return new DDPOutput(node);
    }
    else if (type == OUTPUT_DMX)
    {
        return new DMXOutput(node);
    }
    else if (type == OUTPUT_PIXELNET)
    {
        return new PixelNetOutput(node);
    }
    else if (type == OUTPUT_LOR)
    {
        return new LOROutput(node);
    }
    else if (type == OUTPUT_DLIGHT)
    {
        return new DLightOutput(node);
    }
    else if (type == OUTPUT_RENARD)
    {
        return new RenardOutput(node);
    }
    else if (type == OUTPUT_OPENDMX)
    {
        return new OpenDMXOutput(node);
    }
    else if (type == OUTPUT_OPENPIXELNET)
    {
        return new OpenPixelNetOutput(node);
    }

    logger_base.warn("Unknown network type %s ignored.", (const char *)type.c_str());
    return nullptr;
}
#pragma endregion Static Functions

#pragma region Getters and Setters
void Output::SetController(const std::string& id)
{
    if (id == "")
    {
        _controller = nullptr;
    }
    else
    {
        _controller = Controller::GetController(id);
    }

    _dirty = true;
}

void Output::SetTransientData(int on, long startChannel, int nullnumber)
{
    _outputNumber = on;
    _startChannel = startChannel;
    if (nullnumber > 0) _nullNumber = nullnumber;
}

void Output::SetIP(const std::string& ip)
{
    _ip = IPOutput::CleanupIP(ip);
    _dirty = true;
}

int Output::GetBaudRate() const
{
    if (_baudRate == 0)
        return 115200;
    else
        return _baudRate;
}
#pragma endregion Getters and Setters

#pragma region Operators
bool Output::operator==(const Output& output) const
{
    if (GetType() != output.GetType()) return false;

    if (IsIpOutput())
    {
        return _universe == output.GetUniverse() && _ip == output.GetIP();
    }
    else
    {
        return _commPort == output.GetCommPort();
    }
}
#pragma endregion Operators

#pragma region Start and Stop
bool Output::Open()
{
#ifdef USECHANGEDETECTION
    _skipCount = 0;
#endif

    return true;
}
#pragma endregion Start and Stop

#pragma region Data Setting
// channel here is 0 based
void Output::SetManyChannels(long channel, unsigned char data[], long size)
{
#ifdef _MSC_VER
    long chs = min(size, _channels - channel);
#else
    long chs = std::min(size, GetMaxChannels() - channel);
#endif

    for (long i = 0; i < chs; i++)
    {
        SetOneChannel(channel + i, data[i]);
    }

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}
#pragma endregion Data Setting
