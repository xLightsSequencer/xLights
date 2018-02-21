#ifndef LISTENERSERIAL_H
#define LISTENERSERIAL_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class SerialPort;

class ListenerSerial : public ListenerBase
{
    protected:
        std::string _commPort;
        SerialPort* _serial;
        std::string _serialConfig;
        int _baudRate;
        std::string _protocol;
        unsigned char _buffer[2048];
        int _valid; // how many chars in buffer are valid

	public:
        ListenerSerial(ListenerManager* _listenerManager, std::string commPort, std::string serialConfig, int baudRate, std::string protocol);
		virtual ~ListenerSerial() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const { return "Serial"; }
        std::string GetCommPort() const { return _commPort; }
        virtual void StartProcess() override;
        virtual void StopProcess() override;
        virtual void Poll() override;
        std::string GetProtocol() const { return _protocol; }
        int GetSpeed() const { return _baudRate; }
};
#endif
