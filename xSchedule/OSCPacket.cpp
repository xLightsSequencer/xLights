#include "OSCPacket.h"
#include "../xLights/UtilFunctions.h"
#include "ScheduleOptions.h"

OSCPacket::OSCPacket(const std::string& path, int32_t value)
{
    _isOk = false;
    _stepName = "";
    _timingName = "";
    _ms = 0;
    _progress = -2;
    _buffsize = roundTo4(path.size() + 1) + 4 + sizeof(value);
    _buffer = (uint8_t*)malloc(_buffsize);
    memset(_buffer, 0x00, _buffsize);
    strcpy((char*)_buffer, path.c_str());
    strcpy((char*)_buffer + _buffsize - 4 - sizeof(value), ",i");
    *(_buffer + _buffsize - sizeof(value)) = *((int8_t*)&value + 3);
    *(_buffer + _buffsize - sizeof(value) + 1) = *((int8_t*)&value + 2);
    *(_buffer + _buffsize - sizeof(value) + 2) = *((int8_t*)&value + 1);
    *(_buffer + _buffsize - sizeof(value) + 3) = *((int8_t*)&value);
}

OSCPacket::OSCPacket(const std::string& path, float value)
{
    _isOk = false;
    _stepName = "";
    _timingName = "";
    _ms = 0;
    _progress = -2;
    _buffsize = roundTo4(path.size() + 1) + 4 + sizeof(value);
    _buffer = (uint8_t*)malloc(_buffsize);
    memset(_buffer, 0x00, _buffsize);
    strcpy((char*)_buffer, path.c_str());
    strcpy((char*)_buffer + _buffsize - 4 - sizeof(value), ",f");
    *(_buffer + _buffsize - sizeof(value)) = *((int8_t*)&value + 3);
    *(_buffer + _buffsize - sizeof(value) + 1) = *((int8_t*)&value + 2);
    *(_buffer + _buffsize - sizeof(value) + 2) = *((int8_t*)&value + 1);
    *(_buffer + _buffsize - sizeof(value) + 3) = *((int8_t*)&value);
}

OSCPacket::OSCPacket(uint8_t* buffer, int buffsize, OSCOptions* options, int frameMS)
{
    _isOk = false;
    _stepName = "";
    _timingName = "";
    _ms = 0;
    _buffer = nullptr;
    _buffsize = 0;
    _progress = -2;
    if (strlen((char*)buffer) > buffsize || strlen((char*)buffer) < 10) return;
    std::string path((char*)buffer);

    if (path.substr(0, 10) != "/Timecode/") return;

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
        switch(options->GetTimeCode())
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
        switch(options->GetFrameCode())
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
