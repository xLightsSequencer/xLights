#ifndef LISTENERLOR_H
#define LISTENERLOR_H

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
    bool waiting_for_data;
    int _unit_id;
	enum ReceiveState rcv_state;

	public:
        ListenerLor(ListenerManager* _listenerManager, std::string commPort, std::string serialConfig, int baudRate, std::string protocol, std::string unit_id);
		virtual ~ListenerLor() {}
        virtual std::string GetType() const { return "LOR"; }
        virtual void Poll() override;
};
#endif
