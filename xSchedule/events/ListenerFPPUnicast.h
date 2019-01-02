#ifndef LISTENERFPPUNICAST_H
#define LISTENERFPPUNICAST_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxDatagramSocket;

class ListenerFPPUnicast : public ListenerBase
{
    wxDatagramSocket* _socket;

    bool IsValidHeader(uint8_t* buffer);

	public:
        ListenerFPPUnicast(ListenerManager* _listenerManager);
		virtual ~ListenerFPPUnicast() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const override { return "FPP Unicast"; }
        virtual void StartProcess() override;
        virtual void StopProcess() override;
        virtual void Poll() override;
};
#endif
