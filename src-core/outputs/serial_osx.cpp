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
#include <chrono>
#include <thread>

#include "serial.h"

#include <log.h>

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

    _fd = open(devName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
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
            spdlog::error("serial: ioctl(IOSSIOSPEED) failed setting baud rate {}: errno {}", (int)configuredRate, errno);
        }
    }

    _baudRate = baudRate;

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
    // Real hardware break (TIOCSBRK/TIOCCBRK) is unreliable over many macOS
    // USB-serial drivers - the built-in AppleUSBFTDI driver (and various
    // vendor VCP drivers) can report success on a USB-backed tty without
    // ever asserting a break on the wire, since that requires a chip-specific
    // USB control transfer the generic tty break ioctl doesn't always issue.
    // That's why FTDI-based Open DMX dongles pass the "does it show as
    // connected / does the TX LED flash" test but fixtures never see a
    // valid DMX frame (github #2300). Log a hard ioctl failure for
    // diagnostics, but don't rely on it - always also generate the break in
    // software below, which goes over the normal write path and so actually
    // reaches the wire.
    if (ioctl(_fd, TIOCSBRK) == -1) {
        spdlog::warn("serial (osx): TIOCSBRK ioctl failed on '{}' (errno {}) - hardware break not supported by this device/driver", _devName, errno);
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (ioctl(_fd, TIOCCBRK) == -1) {
            spdlog::warn("serial (osx): TIOCCBRK ioctl failed on '{}' (errno {})", _devName, errno);
        }
    }

    // Software break: momentarily drop to a baud rate low enough that a
    // single zero byte's start bit + 8 zero data bits (9 bit periods, all
    // held low) produce a break comfortably longer than DMX's >=92us
    // minimum, then restore the real baud rate. The following stop bit's
    // mark plus the time taken to restore the baud rate provides the
    // mark-after-break gap before the caller writes the actual DMX data.
    speed_t breakSpeed = 50000; // 9 bits @ 50000 baud ~= 180us low
    if (ioctl(_fd, IOSSIOSPEED, &breakSpeed) == -1) {
        spdlog::warn("serial (osx): failed to set software-break baud rate on '{}' (errno {}) - break may not reach the device", _devName, errno);
        return -1;
    }

    char zero = 0x00;
    write(_fd, &zero, 1);
    tcdrain(_fd);

    speed_t restoreSpeed = (speed_t)_baudRate;
    if (ioctl(_fd, IOSSIOSPEED, &restoreSpeed) == -1) {
        spdlog::error("serial (osx): failed to restore baud rate {} on '{}' after software break (errno {})", _baudRate, _devName, errno);
        return -1;
    }

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

void SerialPort::SetRTS(bool state)
{
    int RTS_flag = TIOCM_RTS;
    if (state) {
        ioctl(_fd, TIOCMBIS, &RTS_flag);
    } else {
        ioctl(_fd, TIOCMBIC, &RTS_flag);
    }
};

void SerialPort::SetDTR(bool state)
{
    int DTR_flag = TIOCM_DTR;
    if (state) {
        ioctl(_fd, TIOCMBIS, &DTR_flag);         /* setting DTR = 1,~DTR = 0 */
    } else {
        ioctl(_fd, TIOCMBIC, &DTR_flag);         /* setting DTR = 0,~DTR = 1 */
    }
};
