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
#include <string>
#include <wx/wx.h>

class wxDatagramSocket;

class ListenerARTNet : public ListenerBase
{
    wxDatagramSocket* _socket;
    uint8_t _lastMode = 0;

    bool IsValidHeader(uint8_t* buffer);

public:
    ListenerARTNet(ListenerManager* _listenerManager, const std::string& localIP);
    virtual ~ListenerARTNet() {}
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override { return "ARTNet"; }
    virtual void StartProcess(const std::string& localIP) override;
    virtual void StopProcess() override;
    virtual void Poll() override;
    virtual std::string FormatTime(size_t timems) const override {
        return ListenerBase::FormatTime(_lastMode, timems);
    }
};
