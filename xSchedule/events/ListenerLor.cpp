/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerLor.h"
#include <log4cpp/Category.hh>
#include "../../xLights/outputs/serial.h"
#include "ListenerManager.h"

ListenerLor::ListenerLor(ListenerManager* listenerManager, std::string commPort, std::string serialConfig, int baudRate, std::string protocol, std::string unit_id_str)
	: ListenerSerial(listenerManager, commPort, serialConfig, baudRate, protocol), rcv_state(IDLE), new_ids_ready(true)
{
    sw.Start(0);
    sw2.Start(0);
    int unit_id = std::strtol(unit_id_str.c_str(), nullptr, 16);
    cur_unit_ids.push_back(unit_id);
}

ListenerLor::~ListenerLor()
{
    new_unit_ids.clear();
    cur_unit_ids.clear();
}

void ListenerLor::EndUnitIdList()
{
    new_ids_ready = true;
}

void ListenerLor::AddNewUnitId( int unit_id )
{
    for (auto it = new_unit_ids.begin(); it != new_unit_ids.end(); ++it)
    {
        if ((*it) == unit_id)
        {
            return;
        }
    }
    new_unit_ids.push_back(unit_id);
}

void ListenerLor::Poll()
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStopWatch sw3;
	int inputs1 = 0;
	int inputs2 = 0;
    size_t idx = 0;
    uint8_t d[8];
	bool done = false;

    if (_serial == nullptr) return;

    if( new_ids_ready )
    {
        new_ids_ready = false;
        cur_unit_ids.clear();
        for (auto it = new_unit_ids.begin(); it != new_unit_ids.end(); ++it)
        {
            cur_unit_ids.push_back(*it);
        }
        new_unit_ids.clear();
    }

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

        for (auto it = cur_unit_ids.begin(); it != cur_unit_ids.end(); ++it)
        {
            int unit_id = *it;
            // send the polling request
            idx = 0;
            d[idx++] = 0;
            d[idx++] = unit_id;
            d[idx++] = 0x88;
            d[idx++] = 0x64;
            d[idx++] = 0x2D;
            d[idx++] = 0x0;
            _serial->Write((char *)d, idx);
            rcv_state = WAITING_FOR_FE;

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

                _listenerManager->ProcessPacket(GetType(), _commPort, &_buffer[0], 8, unit_id);

                // send the acknowledge
                idx = 0;
                d[idx++] = 0;
                d[idx++] = unit_id;
                d[idx++] = 0x88;
                d[idx++] = 0x69;
                d[idx++] = 0x2D;
                d[idx++] = 0x0;
                _serial->Write((char *)d, idx);

                rcv_state = IDLE;
            }
        }
    }

    // given we check for events in multiples of 100ms lets sleep to ensure that we release the CPU
    wxMilliSleep(100);
}
