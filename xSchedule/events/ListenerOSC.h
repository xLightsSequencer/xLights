#ifndef LISTENEROSC_H
#define LISTENEROSC_H

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
#endif
