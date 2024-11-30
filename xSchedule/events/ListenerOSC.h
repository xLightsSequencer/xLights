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

class wxDatagramSocket;

class ListenerOSC : public ListenerBase {
    int _frameMS;
    wxDatagramSocket* _socket;

public:
    ListenerOSC(ListenerManager* _listenerManager, const std::string& localIP);
    virtual ~ListenerOSC() {
    }
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override {
        return "OSC";
    }
    virtual void StartProcess(const std::string& localIP) override;
    virtual void StopProcess() override;
    virtual void Poll() override;
};
