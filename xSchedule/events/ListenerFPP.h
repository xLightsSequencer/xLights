#ifndef LISTENERFPP_H
#define LISTENERFPP_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxDatagramSocket;

class ListenerFPP : public ListenerBase
{
    wxDatagramSocket* _socket;

    bool IsValidHeader(wxByte* buffer);

	public:
        ListenerFPP(ListenerManager* _listenerManager);
		virtual ~ListenerFPP() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const override { return "FPP"; }
        virtual void StartProcess() override;
        virtual void StopProcess() override;
        virtual void Poll() override;
};
#endif
