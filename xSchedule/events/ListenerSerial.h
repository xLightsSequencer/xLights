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

#include "ListenerBase.h"
#include <wx/wx.h>
#include <string>

class SerialPort;

class ListenerSerial : public ListenerBase {
protected:
    std::string _commPort;
    SerialPort* _serial;
    std::string _serialConfig;
    int _baudRate;
    std::string _protocol;
    unsigned char _buffer[2048];
    int _valid; // how many chars in buffer are valid

public:
    ListenerSerial(ListenerManager* _listenerManager, const std::string& commPort, const std::string& serialConfig, int baudRate, const std::string& protocol);
    virtual ~ListenerSerial() {
    }
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override {
        return "Serial";
    }
    std::string GetCommPort() const {
        return _commPort;
    }
    virtual void StartProcess(const std::string& localIP) override;
    virtual void StopProcess() override;
    virtual void Poll() override;
    std::string GetProtocol() const {
        return _protocol;
    }
    int GetSpeed() const {
        return _baudRate;
    }
};
