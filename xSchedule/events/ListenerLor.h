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

#include "ListenerSerial.h"
#include <string>
#include <wx/wx.h>

class SerialPort;

class ListenerLor : public ListenerSerial
{
	enum ReceiveState
	{
		IDLE,
		WAITING_FOR_FE,
		WAITING_FOR_65,
		WAITING_FOR_BYTE1,
		WAITING_FOR_BYTE2,
		PROCESS_DATA
	};

    wxStopWatch sw;
    wxStopWatch sw2;
	enum ReceiveState rcv_state;
	std::vector<int> new_unit_ids;
	std::vector<int> cur_unit_ids;
	bool new_ids_ready;

	public:
        ListenerLor(ListenerManager* _listenerManager, std::string commPort, std::string serialConfig, int baudRate, std::string protocol, std::string unit_id_str);
		virtual ~ListenerLor();
        virtual std::string GetType() const override { return "LOR"; }
        virtual void Poll() override;

        void EndUnitIdList();
        void AddNewUnitId( int unit_id );
};

