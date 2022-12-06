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

class wxSocketEvent;
class wxDatagramSocket;

class ListenerE131 : public ListenerBase
{
    wxDatagramSocket* _socket;
    std::vector<uint16_t> _multicastUniverses;

    bool IsValidHeader(uint8_t* buffer);
    void Subscribe(uint16_t universe);

	public:
        ListenerE131(ListenerManager* _listenerManager, const std::string& localIP);
		virtual ~ListenerE131() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const override { return "E131"; }
        virtual void StartProcess(const std::string& localIP) override;
        virtual void StopProcess() override;
        virtual void Poll() override;
        void AddMulticast(uint16_t universe);
};
