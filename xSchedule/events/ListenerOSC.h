#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxDatagramSocket;

class ListenerOSC : public ListenerBase
{
    int _frameMS;
    wxDatagramSocket* _socket;

public:
    ListenerOSC(ListenerManager* _listenerManager);
    virtual ~ListenerOSC() {}
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override { return "OSC"; }
    virtual void StartProcess() override;
    virtual void StopProcess() override;
    virtual void Poll() override;
};

