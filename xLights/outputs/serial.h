#ifndef XLIGHTS_SERIAL_H
#define XLIGHTS_SERIAL_H

// Author:      Joachim Buermann (adapted for xLights by Matt Brown)
// Copyright:   (c) 2010 Joachim Buermann

#include <wx/wx.h>

#ifdef __WXMSW__
    #include <windows.h>
#else
    #include <termios.h>
#endif

class SerialPort
{
protected:

    #pragma region Member Variables
    std::string _devName;
    int _callback;  // used in basic script

#ifdef __WXMSW__
    HANDLE _fd;
    OVERLAPPED _ov;
    int _rtsdtr_state;
#else
    int _fd;
    struct termios _t;
    speed_t AdaptBaudrate(int baud);
#endif
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    SerialPort();
    ~SerialPort();
    #pragma endregion Constructors and Destructors

    void SetCallback(int cb);
    int GetCallback();

    #pragma region Reading and Writing
    int AvailableToRead();
    int WaitingToWrite();
    int SendBreak();
    int Read(char* buf, size_t len);
    int Write(char* buf, size_t len);
    #pragma endregion Reading and Writing

    #pragma region Start and Stop
    int Close();
    int Open( const std::string& portName, int baudRate, const char* protocol = "8N1" );
    bool IsOpen();
    int Purge();
    #pragma endregion Start and Stop
};

#endif // XLIGHTS_SERIAL_H
