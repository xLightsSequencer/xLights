#ifndef LISTENERARTNET_H
#define LISTENERARTNET_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxDatagramSocket;

class ListenerARTNet : public ListenerBase
{
    wxDatagramSocket* _socket;

    bool IsValidHeader(wxByte* buffer);

public:
    ListenerARTNet(ListenerManager* _listenerManager);
    virtual ~ListenerARTNet() {}
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override { return "ARTNet"; }
    virtual void StartProcess() override;
    virtual void StopProcess() override;
    virtual void Poll() override;
};
#endif
