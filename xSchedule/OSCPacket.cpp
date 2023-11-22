/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OSCPacket.h"
#include "../xLights/UtilFunctions.h"
#include "../xLights/utils/ip_utils.h"
#include "ScheduleOptions.h"
#include <wx/sckaddr.h>
#include "../xLights/outputs/IPOutput.h"
#include <log4cpp/Category.hh>
#include <chrono>

std::string OSCPacket::GetTypeChar(OSCTYPE type)
{
    switch(type)
    {
    case OSCTYPE::OSCINT:
        return "i";
    case OSCTYPE::OSCFLOAT:
        return "f";
    case OSCTYPE::OSCSTRING:
        return "s";
    default:
        break;
    }

    return "i";
}

std::string OSCPacket::GetP(int p) const
{
    int pathsize = roundTo4(_path.size() + 1);
    int parmcount = 0;
    int parmsize = 0;

    if (pathsize != _buffsize)
    {
        parmcount = strlen((char *)&_buffer[pathsize]) - 1;
        parmsize = roundTo4(parmcount + 2);
    }

    if (parmcount < p) return "";

    uint8_t* pcur = _buffer + pathsize + parmsize;

    for (int i = 0; i < p; i++)
    {
        char type = *(_buffer + pathsize + i + 1);

        if (i == p - 1)
        {
            switch(type)
            {
            case 'f':
                float f;
                *((int8_t*)&f + 3) = *pcur++;
                *((int8_t*)&f + 2) = *pcur++;
                *((int8_t*)&f + 1) = *pcur++;
                *((int8_t*)&f) = *pcur;
                return wxString::Format("%f", f).ToStdString();
            case 'i':
                int ii;
                *((int8_t*)&ii + 3) = *pcur++;
                *((int8_t*)&ii + 2) = *pcur++;
                *((int8_t*)&ii + 1) = *pcur++;
                *((int8_t*)&ii) = *pcur;
                return wxString::Format("%d", ii).ToStdString();
            case 's':
                return std::string((char*)pcur);
            default:
                break;
            }
        }
        else
        {
            switch (type)
            {
            case 'i':
                pcur += 4;
                break;
            case 'f':
                pcur += 4;
                break;
            case 's':
                pcur += roundTo4(strlen((char*)pcur));
                break;
            default:
                break;
            }
        }
    }

    return "";
}

OSCPacket::OSCPacket(const std::string& path)
{
    _isSync = false;
    _isOk = false;
    _path = path;
    _stepName = "";
    _timingName = "";
    _ms = 0;
    _progress = -2;
    _buffsize = roundTo4(path.size() + 1);
    _buffer = (uint8_t*)malloc(_buffsize);
    memset(_buffer, 0x00, _buffsize);
    strcpy((char*)_buffer, path.c_str());
    _isOk = IsPathValid(path);
}

OSCPacket::OSCPacket(const std::string& path, int32_t value)
{
    _isOk = false;
    _path = path;
    _stepName = "";
    _timingName = "";
    _ms = 0;
    _progress = -2;
    _buffsize = roundTo4(path.size() + 1);
    _buffer = (uint8_t*)malloc(_buffsize);
    memset(_buffer, 0x00, _buffsize);
    strcpy((char*)_buffer, path.c_str());
    AddParameter(OSCTYPE::OSCINT, wxString::Format("%d", value).ToStdString());
    _isOk = IsPathValid(path);
}

OSCPacket::OSCPacket(const std::string& path, float value)
{
    _isOk = false;
    _path = path;
    _stepName = "";
    _timingName = "";
    _ms = 0;
    _progress = -2;
    _buffsize = roundTo4(path.size() + 1);
    _buffer = (uint8_t*)malloc(_buffsize);
    memset(_buffer, 0x00, _buffsize);
    strcpy((char*)_buffer, path.c_str());
    AddParameter(OSCTYPE::OSCFLOAT, wxString::Format("%f", value).ToStdString());
    _isOk = IsPathValid(path);
    _isSync = false;
}

OSCPacket::OSCPacket(uint8_t* buffer, int buffsize, OSCOptions* options, int frameMS)
{
    _isOk = false;
    _isSync = false;
    _stepName = "";
    _timingName = "";
    _ms = 0;
    _buffsize = roundTo4(buffsize);
    _buffer = (uint8_t*)malloc(_buffsize);
    memcpy(_buffer, buffer, buffsize);
    _progress = -2;
    if (strlen((char*)buffer) > buffsize) return;
    std::string path((char*)buffer);
    _path = path;

    if (strlen((char*)buffer) >= 10 && path.substr(0, 10) == "/Timecode/")
    {
        _isSync = true;
        if (options->GetRemotePath() == "/Timecode/%STEPNAME%")
        {
            _stepName = path.substr(10);
            if (_stepName.find('/') != std::string::npos) return;
        }
        else
        {
            _timingName = path.substr(10);
            if (_timingName.find('/') != std::string::npos) return;
        }
        int32_t i = -1;
        float f = -1;

        if (buffsize < roundTo4(path.size() + 1) + 8) return;

        char type = buffer[roundTo4(path.size() + 1) + 1];

        if (type == 'i')
        {
            memcpy(&i, &buffer[roundTo4(path.size() + 1) + 4], sizeof(int32_t));
        }
        else if (type == 'f')
        {
            memcpy(&f, &buffer[roundTo4(path.size() + 1) + 4], sizeof(float));
        }

        if (options->IsTime())
        {
            switch (options->GetTimeCode())
            {
            case OSCTIME::TIME_SECONDS:
                _ms = f * 1000;
                if (f == -1) return;
                break;
            case OSCTIME::TIME_MILLISECONDS:
                _ms = i;
                if (i == -1) return;
                break;
            }
        }
        else
        {
            switch (options->GetFrameCode())
            {
            case OSCFRAME::FRAME_DEFAULT:
                _ms = i * frameMS;
                if (i == -1) return;
                break;
            case OSCFRAME::FRAME_24:
                _ms = i * 1000 / 24;
                if (i == -1) return;
                break;
            case OSCFRAME::FRAME_25:
                _ms = i * 1000 / 25;
                if (i == -1) return;
                break;
            case OSCFRAME::FRAME_2997:
                _ms = (int)((float)i * 1000.0 / 29.97);
                if (i == -1) return;
                break;
            case OSCFRAME::FRAME_30:
                _ms = i * 1000 / 30;
                if (i == -1) return;
                break;
            case OSCFRAME::FRAME_60:
                _ms = i * 1000 / 60;
                if (i == -1) return;
                break;
            case OSCFRAME::FRAME_PROGRESS:
                _progress = f;
                if (f == -1) return;
                break;
            }
        }
    }
    else if (path.substr(0, 1) == "/")
    {
        _isOk = true;
    }
}

void OSCPacket::AddParameter(OSCTYPE type, const std::string value)
{
    if (type == OSCTYPE::OSCNONE) return;

    int pathsize = roundTo4(_path.size() + 1);
    int parmcount = 0;
    int currentParmSize = 0;
    int newParmSize = roundTo4(2);

    if (pathsize != _buffsize)
    {
        parmcount = strlen((char *)&_buffer[pathsize]) - 1;
        currentParmSize = roundTo4(parmcount + 2);
        newParmSize = roundTo4(parmcount + 2 + 1);
    }

    uint8_t* old_buffer = _buffer;
    int old_buffsize = _buffsize;

    if (newParmSize != currentParmSize)
    {
        // make space for parameter
        _buffsize += 4; // it can only go up by 4 adding one parameter
        _buffer = (uint8_t*)malloc(_buffsize);
        memset(_buffer, 0x00, _buffsize);
        if (old_buffsize != 0)
        {
            memcpy(_buffer, old_buffer, pathsize + currentParmSize);
            memcpy(&_buffer[pathsize + newParmSize], &old_buffer[pathsize + currentParmSize], old_buffsize - pathsize - currentParmSize);
        }
        
        if (parmcount == 0)
        {
            strcpy((char *)&_buffer[pathsize], ",");
        }
        free(old_buffer);
    }
    strncat((char*)&_buffer[pathsize], GetTypeChar(type).c_str(), 1);

    switch(type)
    {
    case OSCTYPE::OSCINT:
    {
        uint8_t* obuffer = _buffer;
        int obuffsize = _buffsize;
        _buffsize += 4; // it can only go up by 4 adding one parameter
        _buffer = (uint8_t*)malloc(_buffsize);
        memset(_buffer, 0x00, _buffsize);
        memcpy(_buffer, obuffer, obuffsize);
        free(obuffer);
        int32_t i = wxAtoi(value);
        *(_buffer + _buffsize - sizeof(i)) = *((int8_t*)&i + 3);
        *(_buffer + _buffsize - sizeof(i) + 1) = *((int8_t*)&i + 2);
        *(_buffer + _buffsize - sizeof(i) + 2) = *((int8_t*)&i + 1);
        *(_buffer + _buffsize - sizeof(i) + 3) = *((int8_t*)&i);
    }
        break;
    case OSCTYPE::OSCFLOAT:
    {
        uint8_t* obuffer = _buffer;
        int obuffsize = _buffsize;
        _buffsize += 4; // it can only go up by 4 adding one parameter
        _buffer = (uint8_t*)malloc(_buffsize);
        memset(_buffer, 0x00, _buffsize);
        memcpy(_buffer, obuffer, obuffsize);
        free(obuffer);
        float f = wxAtof(value);
        *(_buffer + _buffsize - sizeof(f)) = *((int8_t*)&f + 3);
        *(_buffer + _buffsize - sizeof(f) + 1) = *((int8_t*)&f + 2);
        *(_buffer + _buffsize - sizeof(f) + 2) = *((int8_t*)&f + 1);
        *(_buffer + _buffsize - sizeof(f) + 3) = *((int8_t*)&f);
    }
        break;
    case OSCTYPE::OSCSTRING:
    {
        uint8_t* obuffer = _buffer;
        int obuffsize = _buffsize;
        _buffsize += roundTo4(value.size() + 1); 
        _buffer = (uint8_t*)malloc(_buffsize);
        memset(_buffer, 0x00, _buffsize);
        memcpy(_buffer, obuffer, obuffsize);
        free(obuffer);
        strcpy((char*)&_buffer[obuffsize], value.c_str());
    }
    break;
    default:
        break;
    }
}

int OSCPacket::GetMS(int lengthMS) const
{
    if (_progress >= 0)
    {
        return (int)((float)lengthMS * _progress);
    }
    else
    {
        return _ms;
    }
}

OSCPacket::~OSCPacket()
{
    if (_buffer != nullptr) delete _buffer;
}

void OSCPacket::Send(const std::string& ip, int port, const std::string& localIP)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!ip_utils::IsIPValidOrHostname(ip))
    {
        logger_base.warn("OSCPacket send failed due to invalid IP address %s.", (const char *)ip.c_str());
        return;
    }
    else if (port < 1 || port > 65535)
    {
        logger_base.warn("OSCPacket send failed due to invalid port %d.", port);
        return;
    }
    else if (!IsOk())
    {
        logger_base.warn("OSCPacket invalid %s.", (const char*)_path.c_str());
        return;
    }

    wxIPV4address localaddr;
    if (localIP == "")
    {
        localaddr.AnyAddress();
    }
    else
    {
        localaddr.Hostname(localIP);
    }

    wxDatagramSocket socket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);

    if (!socket.IsOk())
    {
        logger_base.error("Error opening datagram for OSC send. OK : FALSE");
        return;
    }
    else if (socket.Error())
    {
        logger_base.error("Error opening datagram for OSC send. %d : %s",
            socket.LastError(),
            (const char*)DecodeIPError(socket.LastError()).c_str());
        return;
    }

    wxIPV4address remoteAddr;
    //remoteAddr.BroadcastAddress();
    remoteAddr.Hostname(ip);
    remoteAddr.Service(port);

    socket.SendTo(remoteAddr, GetBuffer(), GetBuffSize());

    socket.Close();
}

bool OSCPacket::IsPathValid(const std::string& path)
{
    wxString p(path);
    return p.StartsWith("/") && !p.EndsWith("/");
}

std::string OSCPacket::GetP1() const
{
    return GetP(1);
}

std::string OSCPacket::GetP2() const
{
    return GetP(2);
}

std::string OSCPacket::GetP3() const
{
    return GetP(3);
}

