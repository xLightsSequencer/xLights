#ifndef LISTENERSMPTE_H
#define LISTENERSMPTE_H

#include "ListenerBase.h"
#include "../libltc/ltc.h"
#include <string>
#include <wx/wx.h>

class ListenerSMPTE : public ListenerBase
{
    int _mode = 0;
    LTCDecoder* _decoder = nullptr;
    LTCFrameExt _frame;
    unsigned short _buffer[4096];
    long int _total = 0;

    void DoSync(int mode, int hours, int mins, int secs, int frames);

	public:
        ListenerSMPTE(int mode, ListenerManager* _listenerManager);
		virtual ~ListenerSMPTE() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const override { return "SMPTE"; }
        virtual void StartProcess() override;
        virtual void StopProcess() override;
        virtual void Poll() override;
};
#endif