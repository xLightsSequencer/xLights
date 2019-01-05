#ifndef LISTENERE131_H
#define LISTENERE131_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxSocketEvent;
class wxDatagramSocket;

class ListenerE131 : public ListenerBase
{
    wxDatagramSocket* _socket;

    bool IsValidHeader(uint8_t* buffer);

	public:
        ListenerE131(ListenerManager* _listenerManager);
		virtual ~ListenerE131() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const override { return "E131"; }
        virtual void StartProcess() override;
        virtual void StopProcess() override;
        virtual void Poll() override;
};
#endif
