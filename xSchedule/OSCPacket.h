#ifndef OSCPACKET_H
#define OSCPACKET_H

#include <string>

class OSCOptions;

typedef enum
{
    OSCNONE,
    OSCINT,
    OSCFLOAT,
    OSCSTRING
} OSCTYPE;

class OSCPacket
{
    uint8_t* _buffer;
    int _buffsize;
    bool _isSync;
    bool _isOk;
    std::string _stepName;
    std::string _timingName;
    int _ms;
    float _progress;
    std::string _path;

    std::string GetTypeChar(OSCTYPE type);
    std::string GetP(int p) const;

public:

    OSCPacket(const std::string& path);
    OSCPacket(const std::string& path, int32_t value);
    OSCPacket(const std::string& path, float value);
    OSCPacket(uint8_t* buffer, int buffsize, OSCOptions* options, int frameMS);
    void AddParameter(OSCTYPE type, const std::string value);
    bool IsSync() const { return _isSync; }
    bool IsOk() const { return _isOk; }
    std::string GetStepName() const { return _stepName; }
    std::string GetTimingName() const { return _timingName; }
    int GetMS(int lengthMS) const;
    virtual ~OSCPacket();
    uint8_t* GetBuffer() const {return _buffer; }
    int GetBuffSize() const { return _buffsize; };
    void Send(const std::string& ip, int port);
    std::string GetPath() const { return _path; }
    static bool IsPathValid(const std::string& path);
    std::string GetP1() const;
    std::string GetP2() const;
    std::string GetP3() const;
};

#endif 