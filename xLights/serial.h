// Name:        serial.h
// Author:      Matt Brown

#ifndef XLIGHTS_SERIAL_H
#define XLIGHTS_SERIAL_H

#include <wx/string.h>

#ifdef __WXMSW__
#include <windows.h>
#else
#include <termios.h>
#endif

class xNetwork_Renard;

class SerialPort
{
friend class xNetwork_Renard; //kludge: give access to devname + other info
protected:
    wxString m_devname;
    int callback;  // used in basic script
#ifdef __WXMSW__
    HANDLE fd;
    OVERLAPPED ov;
    int rtsdtr_state;
#else
    int fd;
    struct termios t;
    speed_t AdaptBaudrate( int baud );
#endif

public:
    SerialPort();
    ~SerialPort();
    void SetCallback(int cb);
    int GetCallback();
    int AvailableToRead();
    int WaitingToWrite();
    int SendBreak();
    int Close();
    int Open( const wxString& portname, int baudrate, const char* protocol = "8N1" );
    bool IsOpen();
    int Read(char* buf,size_t len);
    int Write(char* buf,size_t len);
};

#endif // XLIGHTS_SERIAL_H
