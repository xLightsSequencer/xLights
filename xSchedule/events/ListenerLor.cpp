#include "ListenerLor.h"
#include <log4cpp/Category.hh>
#include "../../xLights/outputs/serial.h"
#include "../../xLights/outputs/SerialOutput.h"
#include "ListenerManager.h"

ListenerLor::ListenerLor(ListenerManager* listenerManager, std::string commPort, std::string serialConfig, int baudRate, std::string protocol, std::string unit_id)
	: ListenerSerial(listenerManager, commPort, serialConfig, baudRate, protocol),
	waiting_for_data(false), rcv_state(IDLE)
{
    sw.Start(0);
    sw2.Start(0);
    _unit_id = std::strtol(unit_id.c_str(), nullptr, 16);
}

void ListenerLor::Poll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStopWatch sw3;
	int inputs1 = 0;
	int inputs2 = 0;
    size_t idx = 0;
    wxByte d[8];
	bool done = false;

    if (_serial == nullptr) return;

    if( sw.Time() >= 500 )
    {
        sw.Start(0);
        // send a heartbeat
        idx = 0;
        d[idx++] = 0;
        d[idx++] = 0xFF;
        d[idx++] = 0x81;
        d[idx++] = 0x56;
        d[idx++] = 0x0;
        _serial->Write((char *)d, idx);
    }

    if( sw2.Time() >= 100 )
    {
        sw2.Start(0);
        // send the polling request
        idx = 0;
        d[idx++] = 0;
        d[idx++] = _unit_id;
        d[idx++] = 0x88;
        d[idx++] = 0x64;
        d[idx++] = 0x2D;
        d[idx++] = 0x0;
        _serial->Write((char *)d, idx);
        waiting_for_data = true;
		rcv_state = WAITING_FOR_FE;
    }

    if( waiting_for_data )
    {
        waiting_for_data = false;

        sw3.Start(0);
        while( !done && !_stop && (sw3.Time() < 50) )
        {
            int bytes_read = _serial->Read((char*)(_buffer), 128);
 
            int i = 0;
            while (i < bytes_read)
            {
				switch (rcv_state)
				{
				case WAITING_FOR_FE:
					if (_buffer[i] == 0xFE)
					{
						rcv_state = WAITING_FOR_65;
					}
					break;
				case WAITING_FOR_65:
					if (_buffer[i] == 0x65)
					{
						rcv_state = WAITING_FOR_BYTE1;
					}
					break;
				case WAITING_FOR_BYTE1:
					if ((_buffer[i] & 0x80) == 0x80)
					{
						inputs1 = _buffer[i];
						rcv_state = WAITING_FOR_BYTE2;
					}
					break;
				case WAITING_FOR_BYTE2:
					if ((_buffer[i] & 0x80) == 0x80)
					{
						inputs2 = _buffer[i];
						rcv_state = PROCESS_DATA;
					}
					break;
				default:
					break;
				}
                i++;
				if (_stop) return;
			}
        }
		if (_stop) return;

        if( rcv_state == PROCESS_DATA )
        {
            _buffer[0] = (inputs1 & 8) > 0 ? 1 : 0;
            _buffer[1] = (inputs1 & 4) > 0 ? 1 : 0;
            _buffer[2] = (inputs1 & 2) > 0 ? 1 : 0;
            _buffer[3] = (inputs1 & 1) > 0 ? 1 : 0;
            _buffer[4] = (inputs2 & 8) > 0 ? 1 : 0;
            _buffer[5] = (inputs2 & 4) > 0 ? 1 : 0;
            _buffer[6] = (inputs2 & 2) > 0 ? 1 : 0;
            _buffer[7] = (inputs2 & 1) > 0 ? 1 : 0;

            _listenerManager->ProcessPacket(GetType(), _commPort, &_buffer[0], 8);

            // send the acknowledge
            idx = 0;
            d[idx++] = 0;
            d[idx++] = _unit_id;
            d[idx++] = 0x88;
            d[idx++] = 0x69;
            d[idx++] = 0x2D;
            d[idx++] = 0x0;
            _serial->Write((char *)d, idx);

			rcv_state = IDLE;
        }
    }
}
