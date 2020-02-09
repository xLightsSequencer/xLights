#pragma once

// Author:      Joachim Buermann (adapted for xLights)
// Copyright:   (c) 2010 Joachim Buermann

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
    HANDLE _fd = INVALID_HANDLE_VALUE;
    OVERLAPPED _ov;
    int _rtsdtr_state = 0;
#else
    int _fd;
    struct termios _t;
    speed_t AdaptBaudrate(int baud);
#endif
    #pragma endregion 

public:

    #pragma region Constructors and Destructors
    SerialPort();
    ~SerialPort();
    #pragma endregion 

    #pragma region Reading and Writing
    int AvailableToRead();
    int WaitingToWrite();
    int SendBreak();
    int Read(char* buf, size_t len);
    int Write(char* buf, size_t len);

    void SetRTS(bool state);
    void SetDTR(bool state);
    #pragma endregion Reading and Writing

    #pragma region Start and Stop
    int Close();
    int Open( const std::string& portName, int baudRate, const char* protocol = "8N1" );
    bool IsOpen();
    int Purge();
    #pragma endregion Start and Stop
};
