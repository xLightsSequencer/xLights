#ifndef OSCPACKET_H
#define OSCPACKET_H

#include <string>

class OSCOptions;

class OSCPacket
{
    uint8_t* _buffer;
    int _buffsize;
    bool _isOk;
    std::string _stepName;
    std::string _timingName;
    int _ms;
    float _progress;

public:

    OSCPacket(const std::string& path, int32_t value);
    OSCPacket(const std::string& path, float value);
    OSCPacket(uint8_t* buffer, int buffsize, OSCOptions* options, int frameMS);
    bool IsOk() const { return _isOk; }
    std::string GetStepName() const { return _stepName; }
    std::string GetTimingName() const { return _timingName; }
    int GetMS(int lengthMS) const;
    virtual ~OSCPacket();
    uint8_t* GetBuffer() const {return _buffer; }
    int GetBuffSize() const { return _buffsize; };
};

#endif 