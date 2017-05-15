/////////////////////////////////////////////////////////////////////////////
// Name:        ser_osx.cpp
// Author:      Matt Brown
// Copyright:   (c) 2012 Matt Brown
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <wx/utils.h>
#include "serial.h"

// OS/X version
#include <IOKit/serial/ioss.h>

SerialPort::SerialPort()
{
    _devName = "";
    _callback = -1;
    _fd = -1;
};

SerialPort::~SerialPort()
{
    Close();
};

/*!
\brief adaptor member function, to convert the plattform independent
type wxBaud into a linux conform value.
\param baud the baudrate as wxBaud type
\return speed_t linux specific data type, defined in termios.h
*/
speed_t SerialPort::AdaptBaudrate( int baud )
{
    switch(baud)
    {
    case 150:
        return B150;
    case 300:
        return B300;
    case 600:
        return B600;
    case 1200:
        return B1200;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    default:
        return B38400;
    }
};

int SerialPort::Close()
{
    int err = 0;
    // only close an open file handle
    if(_fd < 0) return EBADF;
    // With some systems, it is recommended to flush the serial port's
    // Output before closing it, in order to avoid a possible hang of
    // the process...
    tcflush(_fd, TCOFLUSH);

    // Don't recover the orgin settings while the device is open. This
    // implicate a mismatched data output!
    // Just close device
    err = close(_fd);

    _fd = -1;

    return err;
};

int SerialPort::Open(const std::string& devName, int baudRate, const char* protocol)
{
    if (strlen(protocol) != 3) return -1;

    // save the device name
    _devName = devName;

    _fd = open(wxString(devName).fn_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(_fd < 0) return _fd;

    // exclusive use
    if ( ioctl( _fd, TIOCEXCL ) == -1 ) return -1;

    tcgetattr(_fd, &_t);

    // set baud rate
    speed_t configuredRate = AdaptBaudrate( baudRate );
    cfsetspeed(&_t, configuredRate );

    // parity settings
    switch( protocol[1] )
    {
    case 'N':
        _t.c_cflag &= ~PARENB;
        break;
    case 'O':
        _t.c_cflag |= PARENB;
        _t.c_cflag |= PARODD;
        break;
    case 'E':
        _t.c_cflag |= PARENB;
        _t.c_cflag &= ~PARODD;
        break;
    }

    // stopbits
    if(protocol[2] == '2')
        _t.c_cflag |= CSTOPB;
    else
        _t.c_cflag &= ~CSTOPB;

    // wordlen
    _t.c_cflag &= ~CSIZE;
    switch( protocol[0] )
    {
    case '5':
        _t.c_cflag |= CS5;
        break;
    case '6':
        _t.c_cflag |= CS6;
        break;
    case '7':
        _t.c_cflag |= CS7;
        break;
    default:
        _t.c_cflag |= CS8;
        break;
    }
    _t.c_oflag &= ~OPOST;

    // look out!
    // MIN = 1 means, in TIME (1/10 secs) defined timeout
    // will be started AFTER receiving the first byte
    // so we must set MIN = 0. (timeout starts immediately, abort
    // also when no input to read)
    _t.c_cc[VMIN] = 0;

    // timeout in 1/10 secs
    // no timeout for non blocked transfer
    _t.c_cc[VTIME] = 0;

    // write the settings
    if (tcsetattr(_fd, TCSANOW, &_t) == -1) return -1;
    
    if (configuredRate != baudRate) {
        configuredRate = baudRate;
        if ( ioctl(_fd, IOSSIOSPEED, &configuredRate ) == -1 )
        {
            printf( "Error %d calling ioctl( ..., IOSSIOSPEED, ... )\n", errno );
        }
    }

    return _fd;
};

bool SerialPort::IsOpen()
{
    return (_fd != -1);
};

int SerialPort::AvailableToRead()
{
    int bytes = 0;
    ioctl(_fd, FIONREAD, &bytes);
    return bytes;
}

int SerialPort::WaitingToWrite()
{
    int bytes = 0;
    ioctl(_fd, TIOCOUTQ, &bytes);
    return bytes;
}

int SerialPort::SendBreak()
{
    ioctl(_fd, TIOCSBRK);
    wxMilliSleep(1);
    ioctl(_fd, TIOCCBRK);
    return 0;
};

int SerialPort::Read(char* buf,size_t len)
{
    // Read() (using read() ) will return an 'error' EAGAIN as it is
    // set to non-blocking. This is not a true error within the
    // functionality of Read, and thus should be handled by the caller.
    int n = read(_fd, buf, len);
    if((n < 0) && (errno == EAGAIN)) return 0;
    return n;
};

int SerialPort::Write(char* buf,size_t len)
{
    // Write() (using write() ) will return an 'error' EAGAIN as it is
    // set to non-blocking. This is not a true error within the
    // functionality of Write, and thus should be handled by the caller.
    int n = write(_fd, buf, len);
    if((n < 0) && (errno == EAGAIN)) return 0;
    return n;
};

int SerialPort::Purge()
{
    return 0;
    //return (tcflush(_fd, TCIOFLUSH));
};
