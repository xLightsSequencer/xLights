#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cstdint>
#include <string>

class OSCOptions;

typedef enum {
    OSCNONE,
    OSCINT,
    OSCFLOAT,
    OSCSTRING
} OSCTYPE;

class OSCPacket {
    uint8_t* _buffer = nullptr;
    int _buffsize = 0;
    bool _isSync = false;
    bool _isOk = false;
    std::string _stepName;
    std::string _timingName;
    int _ms = 0;
    float _progress = 0;
    std::string _path;

    std::string GetTypeChar(OSCTYPE type);
    std::string GetP(int p) const;

public:
    OSCPacket(const std::string& path);
    OSCPacket(const std::string& path, int32_t value);
    OSCPacket(const std::string& path, float value);
    OSCPacket(uint8_t* buffer, int buffsize, OSCOptions* options, int frameMS);
    void AddParameter(OSCTYPE type, const std::string value);
    bool IsSync() const {
        return _isSync;
    }
    bool IsOk() const {
        return _isOk;
    }
    std::string GetStepName() const {
        return _stepName;
    }
    std::string GetTimingName() const {
        return _timingName;
    }
    int GetMS(int lengthMS) const;
    virtual ~OSCPacket();
    uint8_t* GetBuffer() const {
        return _buffer;
    }
    int GetBuffSize() const {
        return _buffsize;
    };
    void Send(const std::string& ip, int port, const std::string& localIP);
    std::string GetPath() const {
        return _path;
    }
    static bool IsPathValid(const std::string& path);
    std::string GetP1() const;
    std::string GetP2() const;
    std::string GetP3() const;
};
