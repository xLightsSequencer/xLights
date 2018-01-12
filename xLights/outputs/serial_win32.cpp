// This is the windows implementation of the SerialPort class

#define SERIALPORT_BUFSIZE 6144

enum SerialLineState
{
    /*! Data Carrier Detect (read only) */
    LinestateDcd = 0x040,
    /*! Clear To Send (read only) */
    LinestateCts = 0x020,
    /*! Data Set Ready (read only) */
    LinestateDsr = 0x100,
    /*! Data Terminal Ready (write only) */
    LinestateDtr = 0x002,
    /*! Ring Detect (read only) */
    LinestateRing = 0x080,
    /*! Request To Send (write only) */
    LinestateRts = 0x004,
    /*! no active line state, use this for clear */
    LinestateNull = 0x000
};


#pragma region Constructors and Destructors
SerialPort::SerialPort()
{
    _devName = "";
    _callback = -1;
    memset( &_ov, 0, sizeof( OVERLAPPED ) );
    _fd = INVALID_HANDLE_VALUE;
    _rtsdtr_state = LinestateNull;
}

SerialPort::~SerialPort()
{
    Close();
}
#pragma endregion Constructors and Destructors

#pragma region Start and Stop
int SerialPort::Close()
{
    if (_fd != INVALID_HANDLE_VALUE)
    {
        //FlushFileBuffers(_fd);
        CloseHandle(_ov.hEvent);
        CloseHandle(_fd);
        _fd = INVALID_HANDLE_VALUE;
    }

    return 0;
}

// return 0 on success, negative value on error
int SerialPort::Open(const std::string& devName, int baudRate, const char* protocol)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (strlen(protocol) != 3)
    {
        logger_base.error("Illegal protocol %s -> returning -1.", protocol);
        return -1;
    }

    _fd = CreateFile(wxString(devName),  // device name
                    GENERIC_READ | GENERIC_WRITE,   // O_RDWR
                    0,                              // not shared
                    NULL,                           // default value for object security ?!?
                    OPEN_EXISTING,                  // file (device) exists
                    FILE_FLAG_OVERLAPPED,           // asynchron handling
                    NULL);                          // no more handle flags

    _devName = devName;

    if(_fd == INVALID_HANDLE_VALUE)
    {
        logger_base.error("File creation failed opening serial port %s -> returning -1.", devName.c_str());
        return -1;
    }

    // device control block
    DCB dcb;
    memset(&dcb,0,sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = baudRate;
    dcb.fBinary = 1;

    _rtsdtr_state = LinestateNull;

    // Specifies whether the CTS (clear-to-send) signal is monitored
    // for output flow control. If this member is TRUE and CTS is turned
    // off, output is suspended until CTS is sent again.
    dcb.fOutxCtsFlow = false;

    // Specifies the DTR (data-terminal-ready) flow control.
    // This member can be one of the following values:
    // DTR_CONTROL_DISABLE   Disables the DTR line when the device is
    //                       opened and leaves it disabled.
    // DTR_CONTROL_ENABLE    Enables the DTR line when the device is
    //                       opened and leaves it on.
    // DTR_CONTROL_HANDSHAKE Enables DTR handshaking. If handshaking is
    //                       enabled, it is an error for the application
    //                       to adjust the line by using the
    //                       EscapeCommFunction function.
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    _rtsdtr_state |= LinestateDtr;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    _rtsdtr_state |= LinestateRts;

    // Specifies the XON/XOFF flow control.
    // If fOutX is true (the default is false), transmission stops when the
    // XOFF character is received and starts again, when the XON character
    // is received.
    dcb.fOutX = false;
    
    // If fInX is true (default is false), the XOFF character is sent when
    // the input buffer comes within XoffLim bytes of being full, and the
    // XON character is sent, when the input buffer comes within XonLim
    // bytes of being empty.
    dcb.fInX = false;
    
    // default character for XOFF is 0x13 (hex 13)
    dcb.XoffChar = 0x13;
    
    // default character for XON is 0x11 (hex 11)
    dcb.XonChar = 0x11;
    
    // set the minimum number of bytes allowed in the input buffer before
    // the XON character is sent (1/4 of full size)
    dcb.XonLim = (SERIALPORT_BUFSIZE >> 2);
    
    // set the maximum number of free bytes in the input buffer, before the
    // XOFF character is sent (1/4 of full size)
    dcb.XoffLim = (SERIALPORT_BUFSIZE >> 2);

    // parity settings
    switch( protocol[1] )
    {
    case 'O':
        dcb.Parity = ODDPARITY;
        break;
    case 'E':
        dcb.Parity = EVENPARITY;
        break;
    default:
        dcb.Parity = NOPARITY;
        break;
    }

    // stopbits
    if(protocol[2] == '2')
        dcb.StopBits = TWOSTOPBITS;
    else if (protocol[2] == 'H')
        dcb.StopBits = ONE5STOPBITS; //-DJ
    else
        dcb.StopBits = ONESTOPBIT;
    // stopbits

    // wordlen, valid values are 5,6,7,8
    dcb.ByteSize = protocol[0] - '0';

    if (!SetCommState(_fd, &dcb))
    {
        logger_base.error("Failed to set Comm State DevName: %s BaudRate: %d Protocol: %s -> returning -2.", (const char*) devName.c_str(), baudRate, protocol);
        return -2;
    }

    // create event for overlapped I/O
    // we need a event object, which inform us about the
    // end of an operation (here reading device)
    _ov.hEvent = CreateEvent(NULL, // LPSECURITY_ATTRIBUTES lpsa
                            TRUE,  // BOOL fManualReset
                            TRUE,  // BOOL fInitialState
                            NULL); // LPTSTR lpszEventName
    
    if (_ov.hEvent == INVALID_HANDLE_VALUE)
    {
        logger_base.error("Failed to create event for overlapped I/O DevName: %s -> returning -3.", (const char *) devName.c_str());
        return -3;
    }

    COMMTIMEOUTS cto = {MAXDWORD,0,0,0,0};
    if (!SetCommTimeouts(_fd, &cto))
    {
        logger_base.info("Failed to set Comm timeouts DevName %s -> returning -5.", (const char *) devName.c_str());
        return -5;
    }

    // Leaving this here as part of my EDM transmitter experiments ... if not corrected by 2019 then feel free to remove it
    //if (flowcontrol)
    //{
    //    SetCommMask(_fd, EV_RXCHAR | EV_TXEMPTY);
    //}

    // for a better performance with win95/98 I increased the internal
    // buffer to SERIALPORT_BUFSIZE (normal size is 1024, but this can
    // be a little bit to small, if you use a higher baudrate like 115200)
    if (!SetupComm(_fd, SERIALPORT_BUFSIZE/2, SERIALPORT_BUFSIZE)) return -6;

    return 0;
}

bool SerialPort::IsOpen()
{
    return (_fd != INVALID_HANDLE_VALUE);
}
#pragma endregion Start and Stop

#pragma region Read and Write
int SerialPort::AvailableToRead()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    COMSTAT comStat;
    DWORD   dwErrors;

    // Get and clear current errors on the port.
    if (!ClearCommError(_fd, &dwErrors, &comStat))
    {
        logger_base.error("Failed to clear Comm error.");

        // Report error in ClearCommError.
        return 0;
    }

    return comStat.cbInQue;
}

int SerialPort::WaitingToWrite()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    COMSTAT comStat;
    DWORD   dwErrors;
    
    // Get and clear current errors on the port.
    if (!ClearCommError(_fd, &dwErrors, &comStat))
    {
        logger_base.error("Failed to clear Comm error.");
        // Report error in ClearCommError.
        return 0;
    }

    return comStat.cbOutQue;
}

int SerialPort::Read(char* buf, size_t len)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    DWORD read;
    if (!ReadFile(_fd, buf, len, &read, &_ov))
    {
        // if we use a asynchrone reading, ReadFile always gives FALSE
        // ERROR_IO_PENDING means ok, other values show an error
        if(GetLastError() != ERROR_IO_PENDING)
        {
            logger_base.error("Error reading from serial port %d.", GetLastError());
            // oops..., error in communication
            return -1;
        }
    }
    else
    {
        // ok, we have read all wanted bytes
        return (int)read;
    }
    return 0;
}

int SerialPort::Write(char* buf, size_t len)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    DWORD write;
    if (!WriteFile(_fd, buf, len, &write, &_ov))
    {
        if(GetLastError() != ERROR_IO_PENDING)
        {
            logger_base.error("Error writing to serial port %d.", GetLastError());
            return -1;
        }
    }
    //FlushFileBuffers(_fd);

    return write;
}

int SerialPort::SendBreak()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!SetCommBreak(_fd))
    {
        logger_base.error("Error setting commport break.");
        return -1;
    }

    wxMilliSleep(1);

    if (!ClearCommBreak(_fd))
    {
        logger_base.error("Error clearing commport break.");
        return -1;
    }

    // no error
    return 0;
}

int SerialPort::Purge()
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //if (PurgeComm(_fd, PURGE_RXCLEAR | PURGE_TXCLEAR) == 0)
    //{
    //    logger_base.error("Error purging commport 0x%lx.", (long)GetLastError());
    //    return -1;
    //}

    return 0;
}
#pragma endregion Read and Write
