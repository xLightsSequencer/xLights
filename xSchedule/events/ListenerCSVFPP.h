#ifndef LISTENERFPPUNICAST_H
#define LISTENERFPPUNICAST_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxDatagramSocket;

class ListenerCSVFPP : public ListenerBase
{
    wxDatagramSocket* _socket;

    bool IsValidHeader(uint8_t* buffer);

	public:
        ListenerCSVFPP(ListenerManager* _listenerManager);
		virtual ~ListenerCSVFPP() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const override { return "FPP CSV"; }
        virtual void StartProcess() override;
        virtual void StopProcess() override;
        virtual void Poll() override;
};
#endif
